/*
Switching between a progression of animations by clicking a button. 
Double click resets the progression. 

Using FastLED 3 and 2 LED Strips. 

A lot of the code was based on the work of Mark Kriegsman (FastLED) 
*/

#include <FastLED.h>                                          
#include <Wire.h>
#include "Button.h"

/** 
 * LEDS
 */
// Size of the strip, including both front and back of the strip
#define STRIP_SIZE      60
// Number of LEDs for the front side of the suit (will be mirrored on what's left of the strip in the back)
#define NUM_LEDS        30                                    
#define LED_1           9
#define LED_2           6
// USE_RING = 1 when using a 24 pixels neopixel ring (to LED_1) and both strips are connected to LED_2
#define USE_RING        1
#define RING_SIZE       24

#define MAX_BRIGTHTNESS 80                                   // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[STRIP_SIZE];                                   // Initialize our LED array.

/** 
 * Button Switcher
 */ 
#define BUTTON_PIN 12
Button button(BUTTON_PIN, true);

/** 
 * Color Sensor 
 */ 
#if defined(USE_COLOR_SENSOR)
#include "Adafruit_TCS34725.h"
#include "ColorSensor.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_4X);
#endif 

/** 
 * Animations
 */ 
#include "Animations.h"

/**
 * Microphone
 */
#define MIC_PIN A10
#include "SoundReactive.h"


/**
 * Sequencing
 */
typedef uint8_t (*AnimationPattern)(uint8_t arg1, uint8_t arg2);
typedef struct { 
  AnimationPattern mPattern;
  uint8_t mArg1;
  uint8_t mArg2;
} AnimationPatternArguments;
 

AnimationPatternArguments gAnimations[] = {
  
  {fire, 100, 200}, 
  
  // TODO Fix or kill
  {breathing, 4, 4},
  
  {breathing2, 40000, 0},

  {soundAnimate, 5, 5},
  
  {ripple,  60,  50},
 
  {sinelon,  7, 32},
  {sinelon,  7, 4},
  
  {juggle,   2, 4},
  {juggle,   3, 7},
  {juggle,   4, 8},
  
  // TODO applause became way too fast when 2 leds on same pin
  {applause, HUE_BLUE, HUE_PURPLE},
  {applause, HUE_BLUE, HUE_RED},
  
  // TODO Should probably remove or move to lower energy
  {twinkle,  15, 100},
  
  // TODO Slow it down, like applause
  {twinkle,  50, 224},
  
  {confetti, 20, 10},
  {confetti, 16,  3}
};

AnimationPatternArguments gDropAnimations[] = {
  {aboutToDrop, 100, 200},
  {dropped, 100, 200}
};
 
// Default sequence to main animations
AnimationPatternArguments* gSequence = gAnimations; 
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current


/**
 * Setup
 */ 
void setup() {
  
  delay(2000);                                                // Power-up safety delay or something like that.

  Serial.begin(57600);

  // LEDs
  FastLED.addLeds<NEOPIXEL, LED_2>(leds, STRIP_SIZE).setCorrection(TypicalLEDStrip);
#if USE_RING
  FastLED.addLeds<NEOPIXEL, LED_1>(leds, RING_SIZE);
#else
  FastLED.addLeds<NEOPIXEL, LED_1>(leds, STRIP_SIZE).setCorrection(TypicalLEDStrip);
#endif

  FastLED.setBrightness(MAX_BRIGTHTNESS);
  set_max_power_in_volts_and_milliamps(5, 500);               // FastLED power management set at 5V, 500mA.
  
  // Button
  button.attachClick(onClick);
  button.attachDoubleClick(onDoubleClick); 
  button.attachLongPressStart(onLongPressStart);
  button.attachDuringLongPress(onDuringLongPress);
  button.attachLongPressStop(onLongPressEnd);
  button.attachTripleClick(onTripleClick);
  
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

/**
 * Click Handlers
 */

void onClick() { 
  Serial.println("Click");
  
  static const int numberOfPatterns = sizeof(gAnimations) / sizeof(gAnimations[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
  
  // Make sure we're on the main animation sequence
  gSequence = gAnimations;
}   

void onDoubleClick() { 
  Serial.println("Double click");
  gCurrentPatternNumber = 0; 
}


void onLongPressStart() { 
  
  Serial.println("Long press start");
    
#if defined(USE_COLOR_SENSOR)
  sampleColor();  
#endif
  
  gSequence = gDropAnimations;
  gCurrentPatternNumber = 0;
}

void onDuringLongPress() { 
  Serial.println("During long press");
}

void onLongPressEnd() { 
  Serial.println("Long press end");
  // Activate the drop animation
  gCurrentPatternNumber = 1;
}

void onTripleClick() { 
  Serial.println("Triple click");
  
}

/** 
 * Loop and led management
 */ 
void loop () {
  random16_add_entropy(random());
  
  button.tick();
  
  boolean staticDelay = true;
  
  uint8_t arg1 = gSequence[gCurrentPatternNumber].mArg1;
  uint8_t arg2 = gSequence[gCurrentPatternNumber].mArg2;
  AnimationPattern animate = gSequence[gCurrentPatternNumber].mPattern;
  
  uint8_t animDelay = animate(arg1, arg2);
  
  gHue++;

  mirrorLeds();
  
  // TODO Try lower delays for juggle
  if (animDelay != NO_DELAY) {
    delay_at_max_brightness_for_power(animDelay != RANDOM_DELAY ? 70 : random8(10,100) * 2.5);
  }
    
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
  
} 

void mirrorLeds() { 

  // TODO Left a pixel off in the middle on purpose. should be removed after set up
  for (int i = STRIP_SIZE-1, x = 0; i > NUM_LEDS; i--, x++) { 
    leds[i] = leds[x];
  }
  
}


