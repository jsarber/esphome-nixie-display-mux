#ifndef Nixie_Defines_h
#define Nixie_Defines_h

#include "esphome/core/defines.h"

// Tube type header selection (controlled by tube_type preset in YAML)
#if defined TUBE_Z5660M
#include "Z5660M_4.h"
#define PIN_MAP PIN_MAP_Z5660M
#elif defined TUBE_IN12A_6
#include "IN12A_6.h"
#define PIN_MAP PIN_MAP_IN12A
#else
#include "Z573M_4.h"
#define PIN_MAP PIN_MAP_Z573M
#endif

#define SHIFT_REGISTER_COUNT 4

// SR_OFFSET - cathode bit position offset (controlled by power_board preset)
#ifdef POWER_BOARD_V32
#define SR_OFFSET 16
#elif defined POWER_BOARD_V31
#define SR_OFFSET 16
#else
#define SR_OFFSET 15
#endif

#define NIXIE_ANODE_2 16
#define NIXIE_ANODE_3 17

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

#endif

