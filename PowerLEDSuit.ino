#include <FastLED.h>                                          
#include <Wire.h>
#include <CapPin.h>

/** 
 * Variable Components
 */
#define USE_2ND_STRIP    0
#define DEBUG
#include "DebugUtils.h"


/** 
 * LEDS
 */   
// Size of the strip, including both front and back of the strip
#define STRIP_SIZE      60
#define LED_PIN         6
struct CRGB leds[STRIP_SIZE];  

#if USE_2ND_STRIP
#define LED2_PIN        2
struct CRGB leds2[STRIP_SIZE];  
#endif

// Number of LEDs for the front side of the suit (will be mirrored on what's left of the strip in the back)
#define NUM_LEDS        40                                    
#define REVERSE_LEDS    0
      
#define DEFAULT_BRIGHTNESS 120                           

                           
/** 
 * Button Switcher
 */ 
#include "Button.h"
#define BUTTON_PIN      12
Button button(BUTTON_PIN, true);

/** 
 * Animations
 */ 
#include "Animations.h"
// TODO Currently only SoundReactive uses these palettes, more animations should use them
// and blend in between for nice transitions
CRGBPalette16 gPalettes[] = {HeatColors_p, LavaColors_p, RainbowColors_p, 
    CloudColors_p, OceanColors_p, ForestColors_p, PartyColors_p};
uint8_t gCurrentPaletteIndex = 0;

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
  
  {soundAnimate, 5, 5},

  {blueFire, 100, 200}, 
  
  {multiFire, 100, 100},
  
  // TODO Fix or kill
  {breathing, 16, 64},
  
  {breathing2, 40000, 0},
  
  {ripple,  60,  40},

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

  {bpm,      15,  2},
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
  
#if USE_2ND_STRIP
  FastLED.addLeds<NEOPIXEL, LED2_PIN>(leds2, STRIP_SIZE).setCorrection(TypicalLEDStrip);
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
  

} 

/**
 * Click Handlers
 */

void onClick() { 
  PRINT("Next animation");
  
  static const int numberOfPatterns = sizeof(gAnimations) / sizeof(gAnimations[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
  
  // Make sure we're on the main animation sequence
  gSequence = gAnimations;
}   

void onDoubleClick() { 
  PRINT("Reseting to first animation");

  if (gCurrentPatternNumber == 0) { 
    // We're already at the first animation - spice things up 
    gCurrentPaletteIndex = (gCurrentPaletteIndex + 1) 
        % (sizeof(gPalettes) / sizeof(gPalettes[0]));
  } else {
    gCurrentPatternNumber = 0; 
  }
}

void onLongPressStart() { 
  PRINT("Loading up, up, up... ");
  
  gSequence = gDropAnimations;
  gCurrentPatternNumber = 0;
}

void onDuringLongPress() { 
  PRINT("More, more, more...");
}

void onLongPressEnd() { 
  PRINT("Dropping the bomb");
  // Activate the drop animation
  gCurrentPatternNumber = 1;
}

void onTripleClick() { 
  PRINT("Opening settings");
  
  SettingsMode settings = SettingsMode(&button);
  settings.showSettings();

  uint8_t brightness = settings.getUserBrightness();
  PRINTX("TripleClick - New brightness: ", brightness);
  FastLED.setBrightness(brightness); 
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
  
  #if REVERSE_LEDS
    reverseLeds();
  #endif
  
  switch(animDelay) { 
    
    case STATIC_DELAY: 
      delay_at_max_brightness_for_power(70);
      break;
      
    case RANDOM_DELAY: 
      // Sync random delay to an increasing BPM as the animations progress 
      uint8_t bpmDelay = beatsin8(gCurrentPatternNumber, 100, 450);
      delay_at_max_brightness_for_power(bpmDelay);
      break;
 
  };

  show_at_max_brightness_for_power();      

  
  #if REVERSE_LEDS
    reverseLeds();
  #endif  
  
  #ifdef DEBUG
  EVERY_N_MILLISECONDS(500) {PRINTX("FPS:", FastLED.getFPS());}
  #endif
} 

void mirrorLeds() { 

  for (int i = STRIP_SIZE-1, x = 0; i >= NUM_LEDS; i--, x++) { 
    leds[i] = leds[x];
#if USE_2ND_STRIP
    leds2[i] = leds[x];
#endif
  }
  
}

void reverseLeds() {
  uint8_t left = 0;
  uint8_t right = STRIP_SIZE-1;
  while (left < right) {
    CRGB temp = leds[left];
    leds[left++] = leds[right];
    leds[right--] = temp;
  }
}


