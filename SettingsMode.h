#include <FastLed.h>

// These should be static members but that would require a cpp file
static uint8_t _userLevel = 1;
static bool _exitingSettings = false;

class SettingsMode
{
public:

  SettingsMode(Button* button) : 
  _button(button) {
  }

  
  void showSettings() { 
    init(); 

    PRINTX("Settings Mode - Current brightness ", FastLED.getBrightness()); 

    // Default to minimum brightness
    _userLevel = 1;
    
    _exitingSettings = false;

    _button->flush();
    
    while (!_exitingSettings) { 
      _button->tick(); 

      fadeToBlackBy(leds, NUM_LEDS, 255);
      
      uint8_t incrementLeds = NUM_LEDS / MAX_LEVEL;
      uint8_t incrementBrightness = MAX_BRIGHTNESS / MAX_LEVEL;

      for(int i = 0; i < _userLevel * incrementLeds; i++) {
        leds[i] = ColorFromPalette((CRGBPalette16)RainbowColors_p, (incrementLeds * i), 
          getUserBrightness());
      }

      delay_at_max_brightness_for_power(100);

      show_at_max_brightness_for_power();
    }

    PRINT("Exiting settings");
    exit();
  }

  static uint8_t getUserBrightness() { 
    return _userLevel * (MAX_BRIGHTNESS/MAX_LEVEL);
  }

  static void onSettingsClick() { 
    // Increase userLevel
    
    _userLevel = ++_userLevel % (MAX_LEVEL + 1);
    
    // Bring up userLevel if under the minimum
    if ((_userLevel * (MAX_BRIGHTNESS / MAX_LEVEL) ) < MIN_BRIGHTNESS) { 
      _userLevel += 1;
    }
    
    PRINTX("New user level:", _userLevel);
    PRINTX("Which sets a brightness of:", SettingsMode::getUserBrightness());
  }

  static void onSettingsLongPressStop() { 
    // Exit settings
   PRINT("Exit clicked");
    _exitingSettings = true;
  }

protected:
  
  void init() { 
    // TODO Would be cleaner to remove/disable all other button handlers
    // to prevent unexpected behavior
    _previousClickHandler = _button->_clickFunc;
    _previousLongPressStopHandler = _button->_longPressStopFunc;

    _button->attachClick(onSettingsClick);
    _button->attachLongPressStop(onSettingsLongPressStop);
  }

  void exit() { 
    _button->attachClick(_previousClickHandler);
    _button->attachLongPressStop(_previousLongPressStopHandler);
    _button->flush();
  }


public:

  Button* _button;

  callbackFunction _previousClickHandler; 
  callbackFunction _previousLongPressStopHandler;

  static const uint8_t MAX_LEVEL = 10; 
  static const uint8_t MAX_BRIGHTNESS = 255;
  static const uint8_t MIN_BRIGHTNESS = 25;
};




