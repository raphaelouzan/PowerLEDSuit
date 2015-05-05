#include <FastLED.h>                                          
#include <Wire.h>

#define DEBUG
#include "DebugUtils.h"

/** 
 * Variable Components
 */
#define USE_COLOR_SENSOR 0
#define USE_RING         1
  
/** 
 * LEDS
 */
// Size of the strip, including both front and back of the strip
#define STRIP_SIZE      60
// Number of LEDs for the front side of the suit (will be mirrored on what's left of the strip in the back)
#define NUM_LEDS        40                                    
#define LED_PIN         6
#define RING_PIN        9
// Ring must be connected to RING_PIN
#define RING_SIZE       24
      
#define DEFAULT_BRIGHTNESS 80                           

struct CRGB leds[STRIP_SIZE];                                   

/** 
 * Button Switcher
 */ 
#include "Button.h"
#define BUTTON_PIN      12
Button button(BUTTON_PIN, true);

/** 
 * Color Sensor 
 */ 
#if USE_COLOR_SENSOR
#include "ColorSensor.h"
#endif 

/** 
 * Animations
 */ 
#include "Animations.h"

/* 
 * Settings UI
 */
#include "SettingsMode.h"

/**
 * Microphone
 */
#define MIC_PIN A10
#include "SoundReactive.h"


/**
 * Sequencing
 */
typedef uint8_t (*Animation)(uint8_t arg1, uint8_t arg2);
typedef struct { 
  Animation mPattern;
  uint8_t mArg1;
  uint8_t mArg2;
} AnimationPattern;
 

AnimationPattern gAnimations[] = {
  
  // Experimental
  {rippleSin, 30, 50}, 

  {soundAnimate, 5, 5},

  {fire, 100, 200}, 
  
  // TODO Fix or kill
  {breathing, 4, 4},
  
  {breathing2, 40000, 0},
  
  // Ripple size should probably be smaller
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
  {confetti, 16,  3}, 

  {bpm,      15,  1},
  {bpm,      62,  3},
  {bpm,      125, 7}
};

AnimationPattern gDropAnimations[] = {
  {aboutToDrop, 100, 200},
  {dropped, 100, 200}
};
 
// Default sequence to main animations
AnimationPattern* gSequence = gAnimations; 
// Index number of which pattern is current
uint8_t gCurrentPatternNumber = 0; 


/**
 * Setup
 */ 
void setup() {
  
  delay(2000);                                                

  DEBUG_START(57600)

  PRINT("PowerLEDSuit starting...");

  // LEDs
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, STRIP_SIZE).setCorrection(TypicalLEDStrip);
#if USE_RING
  FastLED.addLeds<NEOPIXEL, RING_PIN>(leds, RING_SIZE).setCorrection(TypicalLEDStrip);
#endif

  FastLED.setBrightness(DEFAULT_BRIGHTNESS);

  // FastLED power management set at 5V, 500mA.
  set_max_power_in_volts_and_milliamps(5, 500);               
  
  // Button
  button.attachClick(onClick);
  button.attachDoubleClick(onDoubleClick); 
  button.attachLongPressStart(onLongPressStart);
  button.attachDuringLongPress(onDuringLongPress);
  button.attachLongPressStop(onLongPressEnd);
  button.attachTripleClick(onTripleClick);
  
#if USE_COLOR_SENSOR
  setupColorSensor();
#endif

} 

/**
 * Click Handlers
 */

void onClick() { 
  Serial.println("Click - next animation");
  
  static const int numberOfPatterns = sizeof(gAnimations) / sizeof(gAnimations[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
  
  // Make sure we're on the main animation sequence
  gSequence = gAnimations;
}   

void onDoubleClick() { 
  Serial.println("Double click - reseting to first animation");
  gCurrentPatternNumber = 0; 
}

void onLongPressStart() { 
  Serial.println("Long press start - loading up, up, up... ");
    
#if USE_COLOR_SENSOR
  sampleColor();  
#endif
  
  gSequence = gDropAnimations;
  gCurrentPatternNumber = 0;
}

void onDuringLongPress() { 
  Serial.println("During long press - more, more, more...");
}

void onLongPressEnd() { 
  Serial.println("Long press end - Dropping the bomb");
  // Activate the drop animation
  gCurrentPatternNumber = 1;
}

void onTripleClick() { 
  Serial.println("Triple click - opening settings");
  
  SettingsMode settings = SettingsMode(&button);
  settings.showSettings();
  Serial.println("TripleClick - Finished showing settings");

  uint8_t brightness = settings.getUserBrightness();
  Serial.print("TripleClick - New brightness: ");
  FastLED.setBrightness(brightness); 
  Serial.println(brightness);
}

/** 
 * Loop and LED management
 */ 
void loop() {
  random16_add_entropy(random());
  
  button.tick();
  
  uint8_t arg1 = gSequence[gCurrentPatternNumber].mArg1;
  uint8_t arg2 = gSequence[gCurrentPatternNumber].mArg2;
  Animation animate = gSequence[gCurrentPatternNumber].mPattern;
  
  uint8_t animDelay = animate(arg1, arg2);
  
  gHue++;

  mirrorLeds();
  
  if (animDelay != NO_DELAY) {
    delay_at_max_brightness_for_power(animDelay != RANDOM_DELAY ? 70 : random8(10,100) * 2.5);
  }
    
  show_at_max_brightness_for_power();                         // Power managed display of LEDs.
  
} 

void mirrorLeds() { 

  // TODO Left a pixel off in the middle on purpose. should be removed after set up
  for (int i = STRIP_SIZE-1, x = 0; i > NUM_LEDS; i--, x++) { 
    leds[i] = leds[x];
  }
  
}


