/*
Switching between a progression of animations by clicking a button. 
Double click resets the progression. 

Using FastLED 2.1 and 2 LED Strips. 

A lot of the code hsa been based on the work of Mark Kriegsman (FastLED) 
*/

#include <FastLED.h>                                          
#include <OneButton.h>

// LEDs
#define NUM_LEDS        60                                    // Number of LED's.
#define MAX_BRIGTHTNESS 60                                   // Overall brightness definition. It can be changed on the fly.
struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

// Animations
uint8_t gHue = 0; 

// Ripple animation
#define RIPPLE_FADE_RATE 255

// Switcher
#define BUTTON_PIN 12
int state = 0, maxStates = 8;
OneButton button(BUTTON_PIN, true);


void setup() {
  
  delay(2000);                                                // Power-up safety delay or something like that.

  Serial.begin(57600);

  // LEDs
  FastLED.addLeds<NEOPIXEL, 9>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGTHTNESS);
  set_max_power_in_volts_and_milliamps(5, 500);               // FastLED power management set at 5V, 500mA.
  
  // Button
  button.attachClick(onClick);
  button.attachDoubleClick(onDoubleClick); 
} 

void onClick() { 
  state = (state + 1) % maxStates;  
}   

void onDoubleClick() { 
  state = 0; 
}

void loop () {
  random16_add_entropy(random());
  
  button.tick();
  
  boolean staticDelay = true;
  
  switch(state) { 
    case 0:
      // 16-40 max ripple length, fading to black at 75% (192/256ths)
      ripple(random8(16,40), 192);   
      break;
    
    case 1:
      juggle(2,4); 
      break;
    
    case 2: 
      sinelon(13, 20);
      break;
      
    case 4: 
      applause(HUE_BLUE, HUE_PURPLE); 
      staticDelay = false;
      break; 
      
    case 5:
      twinkle(50, 224); 
      staticDelay = false;
      break;
      
    case 6: 
      confetti(20, 10);
      break;
     
   case 7: 
       bpm(62, 2);
     break;
     
     
  }
  
  gHue++;

  delay_at_max_brightness_for_power(staticDelay ? 125 : random8(1,100)*2.5);
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
} 

 

void juggle(uint8_t numDots, uint8_t baseBpmSpeed) {
   // numDots colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 100);
  byte dothue = 0;
  for(int i = 0; i < numDots; i++) {
    leds[beatsin16(i+baseBpmSpeed, 0, NUM_LEDS)] |= CHSV(dothue, 255, 224);
    dothue += (256 / numDots);
  }
}
 

void bpm(uint8_t bpmSpeed, uint8_t stripeWidth)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(bpmSpeed, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*stripeWidth), beat);
  }
}

void sinelon(uint8_t bpmSpeed, uint8_t fadeAmount)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, fadeAmount);
  int pos = beatsin16(bpmSpeed, 0, NUM_LEDS);
  leds[pos] += CHSV(gHue, 255, 192);
}

// An animation to play while the crowd goes wild after the big performance
void applause(uint8_t minHue, uint8_t maxHue)
{
  static uint16_t lastPixel = 0;
  fadeToBlackBy(leds, NUM_LEDS, 32);
  leds[lastPixel] = CHSV(random8(minHue, maxHue), 255, 255);
  lastPixel = random16(NUM_LEDS);
  leds[lastPixel] = CRGB::White;
}

void confetti(uint8_t colorVariation, uint8_t fadeAmount)
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, fadeAmount);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(colorVariation), 200, 255);
}

// @param chanceOfTwinkle  The higher the number, lowers the chance for a pixel to light up. (50)
// by @atuline
void twinkle(uint8_t chanceOfTwinkle, uint8_t fadeRate) {
  if (chanceOfTwinkle > NUM_LEDS) chanceOfTwinkle = NUM_LEDS;               // Make sure we're at least utilizing ALL the LED's.
  int index = random16(0, chanceOfTwinkle);
  if (index < NUM_LEDS) {                                      // Only the lowest probability twinkles will do.
    leds[index] = random();                                    // The idex LED is set to a random 32 bit value
  }
  for (int i = 0; i < NUM_LEDS; i++) 
    leds[i].nscale8(fadeRate); // Go through the array and reduce each RGB value by a percentage.
} 



// Ripple (inspired by @atuline)
// Ripple effect with trailing dots (alternatively), color randomized for each ripple
void ripple(int rippleSize, uint8_t fadeToBlackRate) {

  static int step = -1; 
  static int center = 0;  // Center of the current ripple      
  static uint8_t color; // Ripple colour
  static boolean trailingDots; // whether to add trailing dots to the ripple
  
  fadeToBlackBy(leds, NUM_LEDS, fadeToBlackRate);
  
  if (step == -1) {
    
    // Initalizing ripple 
    center = random(NUM_LEDS); 
    color = random16(0, 256);
    trailingDots = random(0, 2) % 2;
    step = 0;
    
  } else if (step == 0) {
    
    // First pixel of the ripple
    leds[center] = CHSV(color, 255, 255);
    step++;
    
  } else if (step < rippleSize) {
    
    // In the Ripple
    uint8_t fading = RIPPLE_FADE_RATE/step * 2;
    leds[wrap(center + step)] += CHSV(color, 255, fading);   // Display the next pixels in the range for one side.
    leds[wrap(center - step)] += CHSV(color, 255, fading);   // Display the next pixels in the range for the other side.
    step ++;
    
    if (trailingDots && step > 3) {
      // Add trailing dots
      leds[wrap(center + step - 3)] = CHSV(color, 255, fading);     
      leds[wrap(center - step + 3)] = CHSV(color, 255, fading);   
    }
    
  } else { 
    // Ending the ripple
    step = -1;
  }
} 
 
// Wrap around the strip
int wrap(int step) {
  if(step < 0) return NUM_LEDS + step;
  if(step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
}

