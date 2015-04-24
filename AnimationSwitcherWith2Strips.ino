/*
Switching between a progression of animations by clicking a button. 
Double click resets the progression. 

Using FastLED 2.1 and 2 LED Strips. 

A lot of the code hsa been based on the work of Mark Kriegsman (FastLED) 
*/

#include <FastLED.h>                                          
#include <OneButton.h>

// LEDs
#define NUM_LEDS        60                                    // Number of LED's.
#define MAX_BRIGTHTNESS 124                                   // Overall brightness definition. It can be changed on the fly.
struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

// Animations
uint8_t gHue = 0; 

// Twinkle animation 
int     ranamount =  50;                                      // The higher the number, lowers the chance for a pixel to light up.
uint8_t   fadeval = 224;                                      // Fade rate
uint8_t twinkleDelay = 50;

// Ripple
#define MAX_STEPS 16                                           

uint8_t bgcol = 0;                                            // Background colour rotates.

// Switcher
#define BUTTON_PIN 12
int state = 0, maxStates = 8;
OneButton button(BUTTON_PIN, true);


void setup() {
  
  delay(2000);                                                // Power-up safety delay or something like that.

  Serial.begin(57600);

  // LEDs
  FastLED.addLeds<NEOPIXEL, 9>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MAX_BRIGTHTNESS);
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
      ripple(true);  
      break;
    
    case 1:
      juggle(2,4); 
      break;
    
    case 2: 
      sinelon(13, 20);
      break;
      
    case 4: 
      applause(); 
      break; 
      
    case 5:
      twinkle(); 
      staticDelay = false;
      break;
      
    case 6: 
       confetti();
      break;
     
   case 7: 
       bpm(62, 2);
     break;
     
     
  }
  
  if (random8(2) % 2) 
    gHue++;

  delay_at_max_brightness_for_power(staticDelay ? 125 : random8(1,100)*2.5);
  show_at_max_brightness_for_power();                         // Power managed display of LED's.
} 

 

void juggle(uint8_t numDots, uint8_t baseBpmSpeed) {
   // numDots colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 100);
  byte dothue = 0;
  for(int i = 0; i < numDots; i++) {
    leds[beatsin16(i+baseBpmSpeed,0,NUM_LEDS)] |= CHSV(dothue, 255, 224);
    dothue += (256 / numDots);
  }
}
 

void bpm(uint8_t bpmSpeed, uint8_t stripeWidth)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(bpmSpeed, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*stripeWidth), beat);
  }
}

void sinelon(uint8_t bpmSpeed, uint8_t fadeAmount)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, fadeAmount);
  int pos = beatsin16(bpmSpeed, 0, NUM_LEDS);
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



// Ripple from @atuline
// TODO Rewrite
void ripple(boolean randomizeColor) {

  static int step = -1; 
  static int center = 0;  // Center of the current ripple.       
  static uint8_t fade = 255; // Starting brightness.  
  static uint8_t colour; // Ripple colour is randomized.
  
  for (int i = 0; i < NUM_LEDS; ++i) leds[i] = CRGB::Black;
  
  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = randomizeColor ? random16(0, 256) : gHue;
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case MAX_STEPS:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
        leds[wrap(center + step)] += CHSV(colour, 255, fade/step * 2);   // Display the next pixels in the range for one side.
        leds[wrap(center - step)] += CHSV(colour, 255, fade/step * 2);   // Display the next pixels in the range for the other side.
        step ++;                                                         // Next step.
        break;  
  }
} 
 
// TODO Remove this
int wrap(int step) {
  if(step < 0) return NUM_LEDS + step;
  if(step > NUM_LEDS - 1) return step - NUM_LEDS;
  return step;
}

