#ifndef Z5660M_4_h
#define Z5660M_4_h
#include "Arduino.h"

#define NIXIE_BLANK_DELAY 1000
#define NIXIE_ON_DELAY 3000

#define LED_COUNT 4
#define NIXIE_TUBES 4
#define NIXIE_NEON_ENABLE true
#define NEONS_ENABLED

#ifndef ESP32
const uint8_t PIN_MAP[10] = {1,0,9,8,7,6,5,4,3,2};;
#else
const uint8_t PIN_MAP[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
// const uint8_t PIN_MAP[10] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};
#endif
#endif
