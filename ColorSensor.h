#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define COLOR_SENSOR_READ_TIME 700

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_4X);
byte gammatable[256]; // RGB -> eye recognizable color  

void setupColorSensor() { 
  loadGammaTable();
  if (tcs.begin()) {
    Serial.println("Found sensor");
    tcs.setInterrupt(true); // turn LED off
  } else {
    Serial.println("No TCS34725 found ... check your connections");
  }
}

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


void sampleColor() { 
  uint16_t clear, red, green, blue;

  tcs.setInterrupt(false);      // turn on LED

  delay(COLOR_SENSOR_READ_TIME);  // takes 50ms to read 
  
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
