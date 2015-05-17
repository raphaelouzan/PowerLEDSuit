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
  
  // WIP
  {breathing, 16, 64},
  
  {breathing2, 40000, 0},
  
  {ripple,  60,  40},
  
  {ripple, 40, 20},
 
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

AnimationPattern gDancingAnimations[] = {
  
  {soundAnimate, 5, 5},

  {blueFire, 100, 200}, 
  
  {multiFire, 100, 100},
 
  {juggle,   2, 4},
  {juggle,   3, 7},
  {juggle,   4, 8},
  
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
 

AnimationPattern* gSequences[] = {gAnimations, gDancingAnimations, gDropAnimations};
uint8_t gCurrentSequenceNumber = 0;

// Index number of which pattern is current
uint8_t gCurrentPatternNumber = 0; 

void nextAnimation() { 
  AnimationPattern* sequence = gSequences[gCurrentSequenceNumber];
  int numberOfPatterns = sizeof(sequence) / sizeof(sequence[0]);  
  gCurrentPatternNumber = (gCurrentPatternNumber+1) % numberOfPatterns;
}

void nextSequence() { 
  static const int numberOfSequences = sizeof(gSequences) / sizeof(gSequences[0]);

  gCurrentSequenceNumber = (gCurrentSequenceNumber +1) % numberOfSequences;
  // Switch to next sequence
  
  gCurrentPatternNumber = 0; 
}

void loadDropAnimation() { 
  gCurrentSequenceNumber = 2;
  gCurrentPatternNumber = 0; 
}

void dropTheBomb() {
	gCurrentSequenceNumber = 2;
	gCurrentPatternNumber = 1;
}


