#ifndef IN12A_6_h
#define IN12A_6_h
#include "Arduino.h"

#define NIXIE_BLANK_DELAY 500
#define NIXIE_ON_DELAY 3500

#define LED_COUNT 6
#define NIXIE_TUBES 6
#define NIXIE_NEON_ENABLE true

// Cathode pin mapping per tube (only included once)
const uint8_t PIN_MAP_IN12A[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};

#endif
