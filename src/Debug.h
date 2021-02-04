#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED
#include <Arduino.h>


#define DEBUG 0
#define debug_print(str)   do { if (DEBUG) Serial.print(str); } while (0)
#define debug_println( str) do { if (DEBUG) Serial.println(str); } while (0)


#define DEBUG1 0
#define debugL1_print(str)   do { if (DEBUG1) Serial.print(str); } while (0)
#define debugL1_println( str) do { if (DEBUG1) Serial.println(str); } while (0)

#define DEBUG2 0
#define debugL2_print(str)   do { if (DEBUG2) Serial.print(str); } while (0)
#define debugL2_println( str) do { if (DEBUG2) Serial.println(str); } while (0)


#define DEBUG3 0
#define debugL3_print(str)   do { if (DEBUG3) Serial.print(str); } while (0)
#define debugL3_println( str) do { if (DEBUG3) Serial.println(str); } while (0)

#endif // DEBUG_H_INCLUDED
