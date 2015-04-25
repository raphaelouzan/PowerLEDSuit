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
#define NUM_LEDS        60                                    // Number of LED's.
#define LED_1           9
#define LED_2           6
#define MAX_BRIGTHTNESS 230                                   // Overall brightness definition. It can be changed on the fly.
struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

// Switcher
#define BUTTON_PIN 12
OneButton button(BUTTON_PIN, true);

// Color Sensor
#include "ColorSensor.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_4X);

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
  {soundAnimate, 5, 5},
  {ripple,  40,  50},
 
  {juggle,   2, 4},
  {juggle,   3, 7},
  {juggle,   4, 13},
  
  {sinelon,  7, 4},
  {sinelon,  13, 10},
  
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
  FastLED.addLeds<NEOPIXEL, LED_1>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, LED_2>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGTHTNESS);
  set_max_power_in_volts_and_milliamps(5, 500);               // FastLED power management set at 5V, 500mA.
  
  // Button
  button.attachClick(onClick);
  button.attachDoubleClick(onDoubleClick); 
  button.attachLongPressStart(onLongPress);
  
  // Color Sensor
  loadGammaTable();
  if (tcs.begin()) {
    Serial.println("Found sensor");
    tcs.setInterrupt(true); // turn LED off
  } else {
    Serial.println("No TCS34725 found ... check your connections");
  }
} 

void onClick() { 
  static const int numberOfPatterns = sizeof(gPatternsAndArguments) / sizeof(gPatternsAndArguments[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
}   

void onDoubleClick() { 
  gCurrentPatternNumber = 0; 
}

void onLongPress() { 
  sampleColor();  
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

  // Try lower delays for sinelon & juggle
  if (animDelay != NO_DELAY) {
    delay_at_max_brightness_for_power(animDelay != RANDOM_DELAY ? 70 : random8(1,100) * 2.5);
  }
    
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
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
    color = random16(0, 256);
    maxSteps =  random(rippleSize / 2, rippleSize); // Randomize ripple size
    trailingDots = random(0, 2) % 2;
    step = 0;
    
  } else if (step == 0) {
    
    // First pixel of the ripple
    leds[center] = CHSV(color, 255, 255);
    step++;
    
  } else if (step < maxSteps) {
    
    // In the Ripple
    uint8_t fading = RIPPLE_FADE_RATE/step * 2;
    leds[wrap(center + step)] += CHSV(color, 255, fading);   // Display the next pixels in the range for one side.
    leds[wrap(center - step)] += CHSV(color, 255, fading);   // Display the next pixels in the range for the other side.
    step++;
    
    if (trailingDots && step > 3) {
      // Add trailing dots
      leds[wrap(center + step - 3)] = CHSV(color, 255, fading);     
      leds[wrap(center - step + 3)] = CHSV(color, 255, fading);   
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

void sampleColor() { 
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(700);  // takes 50ms to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  tcs.setInterrupt(true);  // turn off LED
  
  Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);

  // Figure out some basic hex code for visualization
  uint32_t sum = red;
  sum += green;
  sum += blue;
  sum += clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.println();

  Serial.print((int)r ); Serial.print(" "); Serial.print((int)g);Serial.print(" ");  Serial.println((int)b );

  // TODO Cool animation  
  fill_solid(leds, NUM_LEDS, CRGB((int)r, (int)g, (int)b)) ;
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
  
  delay(5000);

  Serial.println("Done with color sensing");
}

