#include <FastLed.h>

// These should be static members but that would require a cpp file
static uint8_t _userLevel = 1;
static bool _exitingSettings = false;

class SettingsMode
{
public:

  SettingsMode(Button& button) : 
  _button(button) {
  }

  
  void showSettings() { 
    init(); 

    Serial.print("Settings Mode - Current brightness "); 
    Serial.println(FastLED.getBrightness());

    // Default to minimum brightness
    _userLevel = 1;

    while (!_exitingSettings) { 
      button.tick(); 
      // Show user brightness level
      // 5 increments, representing 51 brightness points

      // Show the max brightness limit in red
      leds[MAX_LEVEL] = CRGB::Red;

      // Show the user level of brightness 
      // TODO Should be a proper gradient based on palette
      fill_gradient_RGB(leds, 0, CRGB::Green, _userLevel, CRGB::Red);

      // Set current brightness to represent user level
      FastLED.setBrightness(getUserBrightness());

      Serial.print("User set brightness at level: ");
      Serial.print(_userLevel);
      Serial.print(" which is :");
      Serial.println(getUserBrightness());

      delay_at_max_brightness_for_power(500);

      show_at_max_brightness_for_power();
    }

    exit();
  }

  uint8_t getUserBrightness() { 
    return map(_userLevel, 0, MAX_LEVEL, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  }

  static void onSettingsClick() { 
    // Increase userLevel
    _userLevel = +_userLevel % MAX_LEVEL;
  }

  static void onSettingsDoubleClick() { 
    // Exit settings
    _exitingSettings = true;
  }

protected:
  
  void init() { 
    _previousClickHandler = button._clickFunc;
    _previousDoubleClickHandler = button._doubleClickFunc;

    button.attachClick(onSettingsClick);
    button.attachDoubleClick(onSettingsDoubleClick);
  }

  void exit() { 
    button.attachClick(_previousClickHandler);
    button.attachDoubleClick(_previousDoubleClickHandler);
  }


public:

  Button& _button;

  callbackFunction _previousClickHandler; 
  callbackFunction _previousDoubleClickHandler;

  static const uint8_t MAX_LEVEL = 5; 
  static const uint8_t MAX_BRIGHTNESS = 255;
  static const uint8_t MIN_BRIGHTNESS = 25;
};




