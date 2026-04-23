#ifndef Z5660M_4_h
#define Z5660M_4_h
#include "Arduino.h"

#define NIXIE_BLANK_DELAY 500
#define NIXIE_ON_DELAY 3000

#define LED_COUNT 4
#define NIXIE_TUBES 4
// Cathode pin mapping per tube (only included once)
const uint8_t PIN_MAP_Z5660M[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};

#endif
