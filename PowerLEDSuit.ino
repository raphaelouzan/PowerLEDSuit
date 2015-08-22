#include <FastLED.h>                                          
#include <Wire.h>

/** 
 * Variable Components
 */
#define USE_2ND_STRIP    1
#define USE_TOUCHSENSORS 0
#define USE_SETTINGS     1
#define DEBUG
#define DEBUG_ANIMATIONS
#include "DebugUtils.h"


/** 
 * LEDS
 */   
// Size of the strip, including both front and back of the strip
#define STRIP_SIZE      60
#define LED_PIN         6
struct CRGB leds[STRIP_SIZE];  

#if USE_2ND_STRIP
#define STRIP2_SIZE     40
#define LED2_PIN        9
struct CRGB leds2[STRIP2_SIZE];  
#endif

// Number of LEDs for the front side of the suit (will be mirrored on what's left of the strip in the back)
#define NUM_LEDS        40                                    
#define REVERSE_LEDS    0
      
#define DEFAULT_BRIGHTNESS 160                           
#define FRAMES_PER_SECOND  100
                           
/** 
 * Button Switcher
 */ 
#include "Button.h"
#define BUTTON_PIN      12
Button button(BUTTON_PIN, true);

/**
 * Touch Sensors
 */ 
#if USE_TOUCHSENSORS
#define I2C_MASTER_ID 4
typedef enum {
  LEFT_PRESSED = 0, 
  LEFT_RELEASED = 1, 
  RIGHT_PRESSED = 2, 
  RIGHT_RELEASED = 3, 
  BOTH_PRESSED = 4, 
  BOTH_RELEASED = 5, 
  BOTTOM_LEFT_PRESSED = 6, 
  BOTTOM_LEFT_RELEASED = 7, 
  BOTTOM_RIGHT_PRESSED = 8, 
  BOTTOM_RIGHT_RELEASED = 9, 
  HUG_PRESSED = 10, 
  HUG_RELEASED = 11
} sensorState;
#endif

/** 
 * Animations
 */ 
#include "Animations.h"

/* 
 * Settings UI
 */
#if USE_SETTINGS
#include "SettingsMode.h"
#endif

/**
 * Microphone
 */
#define MIC_PIN A10
#include "SoundReactive.h"
  

/**
 * Sequencing
 */

AnimationPattern gAnimations[] = {
    
  {soundAnimate, 5, 5},
  
  {pride,    0,   0}, 
  
  {discostrobe, 120, 4},
  
  {blueFire, 100, 200}, 
  {multiFire, 100, 100},
  
  {breathing, 16, 64},
  
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

  {pride,    0,   0}, 
  
  {bpm,      15,  2},
  {bpm,      62,  3},
  {bpm,      125, 7}
};

AnimationPattern gDropAnimations[] = {
  {aboutToDrop, 100, 200},
  {dropped, 100, 200}
};

#if USE_TOUCHSENSORS
AnimationPattern gTouchAnimations[] = { 
  {sinelon, 120, 4}, 
  {discostrobe, 30, 120},
  {bpm, 120, 5}, 
  {fadeOut, 255, 0}
};
#endif
 
// Default sequence to main animations
volatile AnimationPattern* gSequence = gAnimations; 
// Index number of which pattern is current
volatile uint8_t gCurrentPatternNumber = 0; 


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
  FastLED.addLeds<NEOPIXEL, LED2_PIN>(leds2, STRIP2_SIZE).setCorrection(TypicalLEDStrip);
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
  
#if USE_TOUCHSENSORS  
  Wire.begin(I2C_MASTER_ID);             
  Wire.onReceive(onReceive);
#endif   
} 
  
/**
 * Event Handlers
 */
#if USE_TOUCHSENSORS

void onReceive(int size) { 
  byte r = Wire.read();
  
  Serial.println(r);
  switch (r) { 
    case LEFT_PRESSED:   digitalWrite(7, HIGH); onLeftTouched(false); break;
    case LEFT_RELEASED:  digitalWrite(7, LOW); onLeftTouched(true); break;
    case RIGHT_PRESSED:  digitalWrite(7, HIGH); onRightTouched(false); break;
    case RIGHT_RELEASED: digitalWrite(7, LOW); onRightTouched(true); break; 
    case BOTH_PRESSED: digitalWrite(7, HIGH); onBothTouched(false); break;
    case BOTH_RELEASED: digitalWrite(7, LOW); onBothTouched(true); break;
    case BOTTOM_LEFT_PRESSED: digitalWrite(7, HIGH); onBottomLeftTouched(false); break;
    case BOTTOM_LEFT_RELEASED: digitalWrite(7, LOW); onBottomLeftTouched(true); break;
    case BOTTOM_RIGHT_PRESSED: digitalWrite(7, HIGH); onBottomRightTouched(false); break;
    case BOTTOM_RIGHT_RELEASED: digitalWrite(7, LOW); onBottomRightTouched(true); break;
    case HUG_PRESSED: digitalWrite(7, HIGH); onHugTouched(false); break; 
    case HUG_RELEASED: digitalWrite(7, LOW); onHugTouched(true); break; 
  }
 
}

// TODO double touch to activate sensors
// double, double touch bring it back to normal
void onLeftTouched(bool isReleased) { 
  PRINTX("Left Touch - released? :", isReleased);

  gCurrentPatternNumber = 0;
  gSequence = gTouchAnimations;
  gRenderingSettings = LEFT_STRIP_ONLY;
  
  if (isReleased) { 
    // Clear out the strip 
    gCurrentPatternNumber = 3;
  } 
}

void onRightTouched(bool isReleased) { 
  PRINTX("Right Touch - released? :", isReleased);
  
  gCurrentPatternNumber = 1;
  gSequence = gTouchAnimations;
  gRenderingSettings = RIGHT_STRIP_ONLY;
  
  if (isReleased) { 
    // Clear out the strip 
    gCurrentPatternNumber = 3;
  } 
}

void onBothTouched(bool isReleased) { 
  PRINT("Both touched!");
  gCurrentPatternNumber = 2;
  gSequence = gTouchAnimations;
}

void onBottomLeftTouched(bool isReleased) { 
  PRINTX("Bottom Left:", isReleased);  
  
  if (isReleased) { 
//    gSequence = gTouchAnimations;
//    // Clear out the strip 
//    gCurrentPatternNumber = 3;
  } else { 
    onClick();  
  }
}

void onBottomRightTouched(bool isReleased) {
  PRINTX("Bottom Right:", isReleased); 
    Serial.println("Bottom right touched");
  
  gSequence = gTouchAnimations;
  gCurrentPatternNumber = 2;
  
   if (isReleased) { 
    gSequence = gTouchAnimations;
    // Clear out the strip 
    gCurrentPatternNumber = 3;
  } 
}

void onHugTouched(bool isReleased) { 
  PRINTX("Hug Touched:", isReleased); 
  
  gSequence = gDropAnimations;
  gCurrentPatternNumber = 1;
  
  if (isReleased) { 
    gSequence = gTouchAnimations;
    // Clear out the strip 
    gCurrentPatternNumber = 3;
  } 
  
}


#endif

void onClick() { 
  //Next animation
  Serial.println("ONCLICK!");
  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % 
    (sizeof(gAnimations) / sizeof(gAnimations[0]));
  
  // Make sure we're on the main animation sequence
  gSequence = gAnimations;
}   

void onDoubleClick() { 
  //Reseting to first animation

  if (gCurrentPatternNumber == 0) { 
    // We're already at the first animation - spice things up 
    gCurrentPaletteIndex = (gCurrentPaletteIndex + 1) 
        % (sizeof(gPalettes) / sizeof(gPalettes[0]));
    // TODO should use nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);     
        
  } else {
    gCurrentPatternNumber = 0; 
  }
}

void onLongPressStart() { 
  gSequence = gDropAnimations;
  gCurrentPatternNumber = 0;
}

void onDuringLongPress() { 
}

void onLongPressEnd() { 
  // Drop the bomb
  gCurrentPatternNumber = 1;
}

void onTripleClick() { 
#if USE_SETTINGS  
  SettingsMode settings = SettingsMode(&button);
  settings.showSettings();

  uint8_t brightness = settings.getUserBrightness();
  FastLED.setBrightness(brightness); 
#endif  
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

  mirrorLeds();
  
  #if REVERSE_LEDS
    reverseLeds();
  #endif
  
  switch(animDelay) { 
    
    case RANDOM_DELAY: {
      // Sync random delay to an increasing BPM as the animations progress 
      uint8_t bpmDelay = beatsin8(gCurrentPatternNumber, 100, 255);
      delay_at_max_brightness_for_power(bpmDelay);
      break;
    }

    case SYNCED_DELAY: delayToSyncFrameRate(FRAMES_PER_SECOND); break;
    
    case STATIC_DELAY: delay_at_max_brightness_for_power(70); break;
      
    
  };

  show_at_max_brightness_for_power();      

  
  #if REVERSE_LEDS
    reverseLeds();
  #endif  
  
  gHue++;
    
  #ifdef DEBUG_ANIMATIONS
  EVERY_N_MILLISECONDS(500)  {Serial.print("FPS: ");Serial.println(FastLED.getFPS());}
  #endif
  
  #ifdef DEBUG
  EVERY_N_MILLISECONDS(2000) {PRINTX("FREE RAM:", freeRam());}
  #endif
} 


void mirrorLeds() { 
  
  for (int i = 0; i < STRIP_SIZE; i++) { 
    if (i < NUM_LEDS) { 
      
#ifdef USE_2ND_STRIP
      if (gRenderingSettings != LEFT_STRIP_ONLY) {
        leds2[i] = leds[i];
      } else { 
        leds2[i] = CRGB::Black;
      }
      
      if (gRenderingSettings == RIGHT_STRIP_ONLY) { 
        leds[i] = CRGB::Black;
      }
     
#endif 

      if (i < STRIP_SIZE - NUM_LEDS) { 
        // Copy to the front side
        leds[STRIP_SIZE-i-1] = leds[i];
        // Dim the back by max 50%
        leds[i].fadeLightBy(128*(1/i+1));
      }
   }

  }

  // Go back to default
  gRenderingSettings = BOTH_STRIPS;
  
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

static void delayToSyncFrameRate(uint8_t framesPerSecond) {
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  if(msdelta < mstargetdelta) {
    delay_at_max_brightness_for_power(mstargetdelta - msdelta);
  }
  msprev = mscur;
}


