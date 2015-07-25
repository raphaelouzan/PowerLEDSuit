
// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

int ledNum = 0;
int randomR = 0;

void fadeLEDs(int fadeVal);

//================================================================================================
// beatTriggered() : This is the LED animation sequence when the heart beats
//================================================================================================
void beatTriggered(){

  //Ignite 30 LEDs with a red value between 0 to 255
  for(int i = 0; i<30; i++){
    //The red channel is randomised to a value between 0 to 255
    leds[ledNum].r=random8();
//    FastLED.show();

    //Call the fadeLEDs method after every 3rd LED is lit.
    if(ledNum%3==0){
      fadeLEDs(5);
    }

    //Move to the next LED
    ledNum++;

    //Make sure to move back to the beginning if the animation falls off the end of the strip
    if(ledNum>(NUM_LEDS-1)){
      ledNum=0;
    }
  }

  //Ignite 20 LEDS with a blue value between 0 to 120
  for(int i = 0; i<20; i++){
    //The blue channel is randomised to a value between 0 to 120
    leds[ledNum].b=random8(120);

    //Call the fadeLEDs method after every 3rd LED is lit.
    if(ledNum%3==0){
      fadeLEDs(5);
    }

    //Move to the next LED
    ledNum++;

    //Make sure to move back to the beginning if the animation falls off the end of the strip
    if(ledNum>(NUM_LEDS-1)){
      ledNum=0;
    }
  }
}

void fadeLEDs(int fadeVal){
  for (int i = 0; i<NUM_LEDS; i++){
    //Fade every LED by the fadeVal amount
    leds[i].fadeToBlackBy(fadeVal);

    //Randomly re-fuel some of the LEDs that are currently lit (1% chance per cycle)
    //This enhances the twinkling effect.
    if(leds[i].r>10){
      randomR = random8(100);
      if(randomR<1){
        //Set the red channel to a value of 80
        leds[i].r=80;
        //Increase the green channel to 20 - to add to the effect
        leds[i].g=20;
      }
    }
  }
}

uint8_t pulseSensor(uint8_t arg0, uint8_t arg1) {
//  PRINTX("Signal: ", Signal);

  leds[0] = Pulse ? CRGB::Red : CRGB::Black;
  
  if (QS) {
    PRINT("Got QS");
    QS = false;

    beatTriggered();
  }

  fadeLEDs(5);
  
  return NO_DELAY;
}


void setupPulseSensor() {
  pinMode(blinkPin,OUTPUT);
  interruptSetup();
}

void disablePulseSensor() {
  TIMSK1 = 0x00;
}

