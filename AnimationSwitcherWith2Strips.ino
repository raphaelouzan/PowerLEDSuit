
#include <FastLED.h>                                          
#include <OneButton.h>

// LEDs
#define NUM_LEDS 60                                           // Number of LED's.
uint8_t max_bright = 124;                                      // Overall brightness definition. It can be changed on the fly.
struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

// Animations
uint8_t gHue = 0; 

// Rainbow animation
uint8_t rainbowDelay = 5;                                        // A delay value for the sequence(s)
uint8_t rainbowHue = 0;                                             // Starting hue value.
uint8_t rainbowDeltaHue = 1;                                        // Hue change between pixels.
int8_t rainbowRot = 1;                                           // Hue rotation speed. Includes direction.
bool rainbowDir = 0;                                             // I use a direction variable, so I can plug into inputs in a standar fashion.


// Confetti Animation
uint8_t  thisfade = 8;                                        // How quickly does it fade? Lower = slower fade rate.
int       thishue = 50;                                       // Starting hue.
uint8_t   thisinc = 1;                                        // Incremental value for rotating hues
uint8_t   thissat = 100;                                      // The saturation, where 255 = brilliant colours.
uint8_t   thisbri = 255;                                      // Brightness of a sequence. Remember, max_bright is the overall limiter.
int       huediff = 256;                                      // Range of random #'s to use for hue
uint8_t thisdelay = 50;                                        // We don't need much delay (if any)


// Twinkle animation 
int     ranamount =  50;                                      // The higher the number, lowers the chance for a pixel to light up.
uint8_t   fadeval = 224;                                      // Fade rate
uint8_t twinkleDelay = 50;

// Ripple
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.

uint8_t bgcol = 0;                                            // Background colour rotates.

// Switcher
#define BUTTON_PIN 12
int state = 0;
int maxStates = 6;
OneButton button(BUTTON_PIN, true);


void setup() {
  
  delay(2000);                                                // Power-up safety delay or something like that.
  
  Serial.begin(57600);

  // LEDs
  FastLED.addLeds<NEOPIXEL, 9>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(max_bright);
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
      ripple();      
      break;
    
    case 1:
      juggle(2,4); 
      break;
    
    case 2: 
      sinelon();
      break;
      
    case 3:
      twinkle(); 
      staticDelay = false;
      break;
      
    case 4: 
       confetti();
      break;
     
   case 5: 
       bpm();
     break;
     
  }
  
  if (random8(2) % 2) 
    gHue++;

  delay_at_max_brightness_for_power(staticDelay ? 125 : random8(1,100)*2.5);
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
} 

 
 
 void juggle( uint8_t numDots, uint8_t baseBpmSpeed) {
 
   // numDots colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 100);
  byte dothue = 0;
  for(int i = 0; i < numDots; i++) {
    leds[beatsin16(i+baseBpmSpeed,0,NUM_LEDS)] |= CHSV(dothue, 255, 224);
    dothue += (256 / numDots);
  }
}
 
 void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

// An animation to play while the crowd goes wild after the big performance
void applause()
{
  static uint16_t lastPixel = 0;
  fadeToBlackBy( leds, NUM_LEDS, 32);
  leds[lastPixel] = CHSV(random8(HUE_BLUE,HUE_PURPLE),255,255);
  lastPixel = random16(NUM_LEDS);
  leds[lastPixel] = CRGB::White;
}
 
void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void twinkle() {
  if (ranamount >NUM_LEDS) ranamount = NUM_LEDS;               // Make sure we're at least utilizing ALL the LED's.
  int idex = random16(0, ranamount);
  if (idex < NUM_LEDS) {                                      // Only the lowest probability twinkles will do.
    leds[idex] = random();                                    // The idex LED is set to a random 32 bit value
  }
  for (int i = 0; i <NUM_LEDS; i++) leds[i].nscale8(fadeval); // Go through the array and reduce each RGB value by a percentage.
} // twinkle()



// Ripple 
void ripple() {

  for (int i = 0; i < NUM_LEDS; ++i) leds[i] = CRGB::Black;
  
  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = random16(0,256);
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
        leds[wrap(center + step)] += CHSV(colour, 255, myfade/step*2);   // Display the next pixels in the range for one side.
        leds[wrap(center - step)] += CHSV(colour, 255, myfade/step*2);   // Display the next pixels in the range for the other side.
        step ++;                                                         // Next step.
        break;  
  } // switch step
} // ripple()
 

int wrap(int step) {
  if(step < 0) return NUM_LEDS + step;
  if(step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
} // wrap()

