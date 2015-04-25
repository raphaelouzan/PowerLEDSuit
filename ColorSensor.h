#include <Wire.h>
#include "Adafruit_TCS34725.h"

byte gammatable[256]; // RGB -> eye recognizable color  


void loadGammaTable() { 
  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
      
    gammatable[i] = x;      
    //Serial.println(gammatable[i]);
  }
}

