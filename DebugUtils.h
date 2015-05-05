/*
DebugUtils.h - Simple debugging utilities.
*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <WProgram.h>

#ifdef DEBUG

  #define DEBUG_START(baudRate) \
    Serial.begin(baudRate);
  
  #define PRINT_NOLN(str)    \
     Serial.print(millis());     \
     Serial.print(": ");    \
     Serial.print(__PRETTY_FUNCTION__); \
     Serial.print(' ');      \
     Serial.print(__FILE__);     \
     Serial.print(':');      \
     Serial.print(__LINE__);     \
     Serial.print(' ');      \
     Serial.print(str); 
     
  #define PRINT(str) \
    PRINT_NOLN(str) \ 
    Serial.println("");
  
  #define PRINTX(str, obj) \
     PRINT_NOLN(str) \
     Serial.print(" "); \
     Serial.println(obj);
     
#else

  #define PRINT(str)
  #define PRINT_NOLN(str)
  #define PRINTX(str, obj)
  #define DEBUG_START(baudRate)

#endif

#endif
