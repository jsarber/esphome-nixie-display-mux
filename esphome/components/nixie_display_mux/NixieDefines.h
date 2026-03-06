#ifndef Nixie_Defines_h
#define Nixie_Defines_h

// #include "IN12A_6.h"
// #include "Z5660M_4.h"
#include "Z573M_4.h"

#define NIXIE_POWER_V3_0
// #define NIXIE_POWER_V3_1
// #define NIXIE_POWER_V3_2

#ifdef NIXIE_POWER_V3_1
#define SHIFT_REGISTER_DATA 27
#define SHIFT_REGISTER_CLK 26
#define SHIFT_REGISTER_LATCH 22
#elif defined NIXIE_POWER_V3_2
#define SHIFT_REGISTER_DATA 27
#define SHIFT_REGISTER_CLK 26
#define SHIFT_REGISTER_LATCH 4
#else
#define SHIFT_REGISTER_DATA 12
#define SHIFT_REGISTER_CLK 13
#define SHIFT_REGISTER_LATCH 14
#endif

#ifdef NIXIE_POWER_V3_2
#define SHIFT_REGISTER_BLANK 2
#else
#define SHIFT_REGISTER_BLANK 21
#endif

#define SHIFT_REGISTER_COUNT 4

#if defined NIXIE_POWER_V3_1 || defined NIXIE_POWER_V3_2
#define NIXIE_ANODE_1 5
#else
#define NIXIE_ANODE_1 15
#endif
#define NIXIE_ANODE_2 16
#define NIXIE_ANODE_3 17
#define NIXIE_DIRECT_ANODE
#define NIXIE_DIRECT_CATHODE

// Multiplexing frequency (50Hz for 20ms period)
#define MULTIPLEXING_FREQUENCY_HZ 50

// Anti-poison routine constants
#define AP_MAX_LOOPS_DEFAULT 300  // Number of loops before returning to normal display (30 seconds runtime)
#define AP_TIME_CHECK_DELAY 100  // 100ms - cycles through digits every 100ms (10 digits per second)
#define AP_MATCH_TOLERANCE 3  // Max difference for time match (0-3 digits off is acceptable)

#define NEON_1 18
#define NEON_2 19

#define LED_DATA_PIN 25
#define BUTTONS A5

#if defined NIXIE_POWER_V3_1 || defined NIXIE_POWER_V3_2
#define SR_OFFSET 16
#else
#define SR_OFFSET 15
#endif

#endif

