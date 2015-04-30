#include <FastLed.h>

#define RIPPLE_FADE_RATE 255

typedef enum delayType {
RANDOM_DELAY = 2,
STATIC_DELAY = 3, 
NO_DELAY     = 1
} delayType;


uint8_t gHue = 0;




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


// Wrap around the strip
int wrap(int step) {
  if(step < 0) return NUM_LEDS + step;
  if(step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
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

uint8_t beatQuad8x(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, int type = 0, int offset = 0)
{
    uint8_t beat = beat8(beats_per_minute);
    beat += offset;
    uint8_t beatsin = 0;
    switch(type) { 
      case 0: beatsin = ease8InOutQuad(beat); break;
      case 1: beatsin = triwave8(beat); break;
      case 2: beatsin = ease8InOutCubic(beat); break;
      case 3: beatsin = cubicwave8(beat); break;
      case 4: beatsin = ease8InOutApprox(beat); break;
    }
    
    uint8_t rangewidth = highest - lowest;
    uint8_t scaledbeat = scale8(beatsin, rangewidth);
    uint8_t result = lowest + scaledbeat;
    return result;
}


void breath();

uint8_t breathing(uint8_t bpmSpeed, uint8_t fadeAmount) { 
  
  for (int i = 0; i < NUM_LEDS; i++) { 
    int val = beatQuad8x(bpmSpeed, 0, 200, 3);  
    leds[i] = CHSV(HUE_BLUE, 255, val);
  }
  
  return NO_DELAY; 
}


const uint8_t KEYFRAMES[]  = {
  // Rising
  20, 21, 22, 24, 26, 28, 31, 34, 38, 41, 45, 50, 55, 60, 66, 73, 80, 87, 95,
  103, 112, 121, 131, 141, 151, 161, 172, 182, 192, 202, 211, 220, 228, 236,
  242, 247, 251, 254, 255,

  // Falling
  254, 251, 247, 242, 236, 228, 220, 211, 202, 192, 182, 172, 161, 151, 141,
  131, 121, 112, 103, 95, 87, 80, 73, 66, 60, 55, 50, 45, 41, 38, 34, 31, 28,
  26, 24, 22, 21, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 
};

unsigned long lastBreath = 0.0;
int keyframePointer = 0;

uint8_t breathing2(uint8_t breathingCycleTime = 5000, uint8_t baseColorFake = 0) {
  int numKeyframes = sizeof(KEYFRAMES) - 1;
  float period = breathingCycleTime / numKeyframes;
  unsigned long now = millis();
  
  if ((now - lastBreath) > period) {
    lastBreath = now;

    for (int i = 0; i < NUM_LEDS; i++) {
      uint8_t color = (127 * KEYFRAMES[keyframePointer]) / 256;
      leds[i] = color;
    } 

    // Increment the keyframe pointer.
    if (++keyframePointer > numKeyframes) {
      // Reset to 0 after the last keyframe.
      keyframePointer = 0;
    }   
  }
  return NO_DELAY;
}

