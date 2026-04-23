#ifndef Z573M_4_h
#define Z573M_4_h
#include "Arduino.h"

#define NIXIE_BLANK_DELAY 1000
#define NIXIE_ON_DELAY 1000

#define LED_COUNT 4
#define NIXIE_TUBES 4
// Cathode pin mapping per tube (only included once)
const uint8_t PIN_MAP_Z573M[10] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};

#endif
