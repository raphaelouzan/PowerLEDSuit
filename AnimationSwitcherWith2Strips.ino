/*
Switching between a progression of animations by clicking a button. 
Double click resets the progression. 

Using FastLED 3 and 2 LED Strips. 

A lot of the code was based on the work of Mark Kriegsman (FastLED) 
*/

#include <FastLED.h>                                          
#include <OneButton.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// LEDs
// Size of the strip, including both front and back of the strip
#define STRIP_SIZE      60
// Number of LEDs for the front side of the suit (will be mirrored on what's left of the strip in the back)
#define NUM_LEDS        30                                    
#define LED_1           9
#define LED_2           6
#define MAX_BRIGTHTNESS 80                                   // Overall brightness definition. It can be changed on the fly.
struct CRGB leds[STRIP_SIZE];                                   // Initialize our LED array.

// Switcher
#define BUTTON_PIN 12
OneButton button(BUTTON_PIN, true);

// Color Sensor
#if defined(USE_COLOR_SENSOR)
#include "ColorSensor.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_4X);
#endif 

// Animations
uint8_t gHue = 0; 

#define RANDOM_DELAY  2
#define STATIC_DELAY  3 
#define NO_DELAY      1

// Microphone
// TODO "SoundReactive shouldn't rely on the delay #defines 
#define MIC_PIN A10
#include "SoundReactive.h"

#define RIPPLE_FADE_RATE 255

// Animation Sequencing 
typedef uint8_t (*AnimationPattern)(uint8_t arg1, uint8_t arg2);
typedef struct { 
  AnimationPattern mPattern;
  uint8_t mArg1;
  uint8_t mArg2;
} AnimationPatternArguments;
 
 

AnimationPatternArguments gPatternsAndArguments[] = {
// TODO breathing animation (intensity ramp with the blue/red colors)

  {sinus, 5, 4},

  {soundAnimate, 5, 5},
  
  {ripple,  60,  50},
 
  {sinelon,  7, 32},
  {sinelon,  7, 4},
  
  {juggle,   2, 4},
  {juggle,   3, 7},
  {juggle,   4, 8},
  
  // TODO applause is way too fast, could be extremely slow
  {applause, HUE_BLUE, HUE_PURPLE},
  {applause, HUE_BLUE, HUE_RED},
  
  {twinkle,  15, 100},
  {twinkle,  50, 224},
  
  {confetti, 20, 10},
  {confetti, 16,  3},
  
  {bpm,      62,  3},
  {bpm,      125, 7},
  {bpm,      15,  1}
};
 
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current



void setup() {
  
  delay(2000);                                                // Power-up safety delay or something like that.

  Serial.begin(57600);

  // LEDs
  

  FastLED.addLeds<NEOPIXEL, LED_2>(leds, STRIP_SIZE).setCorrection(TypicalLEDStrip);
#if defined(USE_RING)
  FastLED.addLeds<NEOPIXEL, LED_1>(leds, 24);
#else
  FastLED.addLeds<NEOPIXEL, LED_1>(leds, STRIP_SIZE).setCorrection(TypicalLEDStrip);
#endif


  FastLED.setBrightness(MAX_BRIGTHTNESS);
  set_max_power_in_volts_and_milliamps(5, 500);               // FastLED power management set at 5V, 500mA.
  
  // Button
  button.attachClick(onClick);
  button.attachDoubleClick(onDoubleClick); 
  button.attachLongPressStart(onLongPress);
  
  
#if defined(USE_COLOR_SENSOR)
  // Color Sensor
  loadGammaTable();
  if (tcs.begin()) {
    Serial.println("Found sensor");
    tcs.setInterrupt(true); // turn LED off
  } else {
    Serial.println("No TCS34725 found ... check your connections");
  }
#endif

} 

void onClick() { 
  static const int numberOfPatterns = sizeof(gPatternsAndArguments) / sizeof(gPatternsAndArguments[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
}   

void onDoubleClick() { 
  gCurrentPatternNumber = 0; 
}

void onLongPress() { 
  // TODO change palette
#if defined(USE_COLOR_SENSOR)
  sampleColor();  
#endif
}

uint8_t beatsin8x( accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, int type = 0, int offset = 0)
{
    uint8_t beat = beat8(beats_per_minute);
    beat += offset;
    uint8_t beatsin = 0;
    switch(type) { 
      case 0: beatsin = ease8InOutQuad(beat); break;
      case 1: beatsin = triwave8(beat); break;
      case 2: beatsin = ease8InOutCubic(beat); break;
      case 3: beatsin = cubicwave8(beat + 30); break;
      case 4: beatsin = ease8InOutApprox(beat); break;
    }
    
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8(beatsin, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
}


uint8_t sinus(uint8_t bpmSpeed, uint8_t fadeAmount) { 

  
  for (int i = 0; i < NUM_LEDS; i++) { 
    int val = beatsin8x(bpmSpeed, 0, 200, 3);  
    leds[i] = CHSV(HUE_BLUE, 255, val);
  }
 

  return NO_DELAY;
}

void loop () {
  random16_add_entropy(random());
  
  button.tick();
  
  boolean staticDelay = true;
  
  uint8_t arg1 = gPatternsAndArguments[gCurrentPatternNumber].mArg1;
  uint8_t arg2 = gPatternsAndArguments[gCurrentPatternNumber].mArg2;
  AnimationPattern animate = gPatternsAndArguments[gCurrentPatternNumber].mPattern;
  
  uint8_t animDelay = animate(arg1, arg2);
  
  gHue++;

  mirrorLeds();
  
  // TODO Try lower delays for juggle
  if (animDelay != NO_DELAY) {
    delay_at_max_brightness_for_power(animDelay != RANDOM_DELAY ? 70 : random8(1,100) * 2.5);
  }
    
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
  
} 

void mirrorLeds() { 

  for (int i = STRIP_SIZE-1, x = 0; i > NUM_LEDS; i--, x++) { 
    leds[i] = leds[x];
  }
  
}

 

uint8_t juggle(uint8_t numDots, uint8_t baseBpmSpeed) {
   // numDots colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 100);
  byte dothue = 0;
  for(int i = 0; i < numDots; i++) {
    leds[beatsin16(i+baseBpmSpeed, 0, NUM_LEDS)] |= CHSV(dothue, 255, 224);
    dothue += (256 / numDots);
  }
  
  return STATIC_DELAY;
}
 

uint8_t bpm(uint8_t bpmSpeed, uint8_t stripeWidth)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(bpmSpeed, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*stripeWidth), beat);
  }
  
  return NO_DELAY;

}

uint8_t sinelon(uint8_t bpmSpeed, uint8_t fadeAmount)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, fadeAmount);
  int pos = beatsin16(bpmSpeed, 0, NUM_LEDS);
  leds[pos] += CHSV(gHue, 255, 192);
  
  return NO_DELAY;
}

// An animation to play while the crowd goes wild after the big performance
uint8_t applause(uint8_t minHue, uint8_t maxHue)
{
  static uint16_t lastPixel = 0;
  fadeToBlackBy(leds, NUM_LEDS, 32);
  leds[lastPixel] = CHSV(random8(minHue, maxHue), 255, 255);
  lastPixel = random16(NUM_LEDS);
  leds[lastPixel] = CRGB::White;
  
  return RANDOM_DELAY;
}

uint8_t confetti(uint8_t colorVariation, uint8_t fadeAmount)
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, fadeAmount);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(colorVariation), 200, 255);
  
  return RANDOM_DELAY; 
  
}

// @param chanceOfTwinkle  The higher the number, lowers the chance for a pixel to light up. (50)
// by @atuline
uint8_t twinkle(uint8_t chanceOfTwinkle, uint8_t fadeRate) {
  if (chanceOfTwinkle > NUM_LEDS) chanceOfTwinkle = NUM_LEDS;               // Make sure we're at least utilizing ALL the LED's.
  int index = random16(0, chanceOfTwinkle);
  if (index < NUM_LEDS) {                                      // Only the lowest probability twinkles will do.
    leds[index] = random();                                    // The idex LED is set to a random 32 bit value
  }
  for (int i = 0; i < NUM_LEDS; i++) 
    leds[i].nscale8(fadeRate); // Go through the array and reduce each RGB value by a percentage.
    
  return RANDOM_DELAY;
} 



// Ripple (inspired by @atuline)
// Ripple effect with trailing dots (alternatively), color randomized for each ripple
// TODO Ripples should be spaced out by some sinus function instead of a static delay to make it feel more organic
uint8_t ripple(uint8_t rippleSize, uint8_t fadeToBlackRate) {

  static int step = -1; 
  static int center = 0;  // Center of the current ripple      
  static uint8_t color; // Ripple colour
  static boolean trailingDots; // whether to add trailing dots to the ripple
  static int maxSteps;
  
  fadeToBlackBy(leds, NUM_LEDS, fadeToBlackRate);
  
  if (step == -1) {
    
    // Initalizing ripple 
    center = random(NUM_LEDS); 
    color = gHue;
    maxSteps =  min(random(rippleSize / 2, rippleSize), NUM_LEDS); // Randomize ripple size
    trailingDots = random(0, 2) % 2;
    step = 0;
    
  } else if (step == 0) {
    
    // First pixel of the ripple
    leds[center] = CHSV(color, 255, 255);
    step++;
    
  } else if (step < maxSteps) {
    
    // In the Ripple
    uint8_t fading = RIPPLE_FADE_RATE/step * 2;
    leds[wrap(center + step)] += CHSV(color+step, 255, fading);   // Display the next pixels in the range for one side.
    leds[wrap(center - step)] += CHSV(color-step, 255, fading);   // Display the next pixels in the range for the other side.
    step++;
    
    if (trailingDots && step > 3) {
      // Add trailing dots
      leds[wrap(center + step - 3)] = CHSV(color-step, 255, fading);     
      leds[wrap(center - step + 3)] = CHSV(color+step, 255, fading);   
    }
    
  } else { 
    // Ending the ripple
    step = -1;
  }
  
  return STATIC_DELAY;
} 
 
// Wrap around the strip
int wrap(int step) {
  if(step < 0) return NUM_LEDS + step;
  if(step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
}


