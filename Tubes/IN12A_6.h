#ifndef IN12A_6_h
#define IN12A_6_h
#include <Arduino.h>

#define NIXIE_BLANK_DELAY 500
#define NIXIE_ON_DELAY 3500

#define LED_COUNT 6
#define NIXIE_TUBES 6
#define NEONS_ENABLED

#ifndef ESP32
const uint8_t PIN_MAP[10] = {1,0,9,8,7,6,5,4,3,2};
#else
const uint8_t PIN_MAP[10] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
#endif
#endif
