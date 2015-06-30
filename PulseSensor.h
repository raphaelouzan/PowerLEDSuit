
// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, must be seeded! 
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

CRGB pulsePalette[14];

void setupPulsePalette() {
  fill_solid(pulsePalette, 14, CRGB::Black);
  pulsePalette[0] = CRGB::Red;
  pulsePalette[1] = CRGB::Purple;
  pulsePalette[2] = CRGB::Red;
  
  pulsePalette[0+11] = CRGB::Red;
  pulsePalette[1+11] = CRGB::Purple;
  pulsePalette[2+11] = CRGB::Red;
}

int pulseRate = 60;
unsigned long startTime = 0;
unsigned long secondStartTime = 0;

void showLedAtLocation(int ledLocation) {
  for (int i=0; i < 14; i++) {
    if (ledLocation - i < 0) break;
    leds[ledLocation-i] = pulsePalette[13-i];
  }
}

unsigned long showLedsWithStartTime(long startTime) {
  int millisPerLocation = 20;
  if (startTime > 0) {
    unsigned long millisPassed = millis() - startTime;
    long ledLocation = (millisPassed / millisPerLocation);
    if (ledLocation <  NUM_LEDS) {
      showLedAtLocation(ledLocation);
      return startTime;
    } else {
      return 0;
    }
  }
}

uint8_t pulseSensor(uint8_t arg0, uint8_t arg1) {
//  PRINTX("Signal: ", Signal);
  if (QS) {
    Serial.println("Got QS");
    secondStartTime = startTime;
    startTime = millis();
    QS = false;
  }

//  Serial.println(startTime);

  FastLED.clear();
  startTime = showLedsWithStartTime(startTime);

//  f_show();
  FastLED.show();

//  FastLED.delay(10);
  
  return 10;
}

void setupPulseSensor() {
  pinMode(blinkPin,OUTPUT);
  setupPulsePalette();
  interruptSetup();
}

void disablePulseSensor() {
  TIMSK1 = 0x00;
}

