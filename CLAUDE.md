# Nixie Display Mux - Component Documentation

## Overview

The NixieDisplay component handles multiplexed nixie tube displays with anti-poison (anti-fatigue) routines. It supports multiple tube types and power board variants via YAML presets.

## File Structure

All core library files live in the component root directory (not in a `lib/` subdirectory):

```
components/nixie_display_mux/
├── __init__.py                    # ESPHome Python configuration schema
├── nixie_display_mux.h            # Main ESPHome component header
├── nixie_display_mux.cpp          # Main ESPHome component implementation
├── NixieDefines.h                 # Library constants, pin maps, tube type selection
├── NixieDisplay.h                 # Main display class header
├── NixieDisplay.cpp               # Main display class implementation
├── NixieShiftRegister.h           # Shift register helper class header
├── NixieShiftRegister.cpp         # Shift register helper class implementation
├── NixieTube.h                    # Individual tube class header
├── NixieTube.cpp                  # Individual tube class implementation
├── IN12A_6.h                      # IN12A tube type definitions (6 digits)
├── Z5660M_4.h                     # Z5660M tube type definitions (4 digits)
└── Z573M_4.h                      # Z573M tube type definitions (4 digits)
```

## Tube Type Presets

Defined in `__init__.py` `TUBE_PRESETS`:

| Tube Type | Tubes | Blank Delay | On Delay | Neon Enabled |
|-----------|-------|-------------|----------|--------------|
| `in12a`   | 6     | 500µs       | 3500µs   | Yes          |
| `z5660m`  | 4     | 500µs       | 3000µs   | Yes          |
| `z573m`   | 4     | 1000µs      | 1000µs   | No           |

Each tube type header provides `PIN_MAP`, `NIXIE_BLANK_DELAY`, `NIXIE_ON_DELAY`, `LED_COUNT`, and `NIXIE_TUBES` defines.

## Power Board Presets

Defined in `__init__.py` `POWER_BOARD_PRESETS`:

| Board | SR Data | SR Clock | SR Latch | Blank | Anode 1 | Anode 2 | Anode 3 | SR Offset |
|-------|---------|----------|----------|-------|---------|---------|---------|-----------|
| `v30` | 12      | 13       | 14       | 21    | 15      | 16      | 17      | 15        |
| `v31` | 27      | 26       | 22       | 21    | 5       | 16      | 17      | 16        |
| `v32` | 27      | 26       | 4        | 2     | 5       | 16      | 17      | 16        |

SR_OFFSET is selected via compile-time `POWER_BOARD_V30/V31/V32` defines in `NixieDefines.h`:
- v30: SR_OFFSET 15
- v31, v32: SR_OFFSET 16

## YAML Configuration

```yaml
nixie_display_mux:
  id: nixie_clock
  tube_type: z573m          # in12a, z5660m, z573m
  power_board: v32          # v30, v31, v32
  # Optional overrides (defaults come from presets)
  num_tubes: 4
  sr_data_pin: 27
  sr_clock_pin: 26
  sr_latch_pin: 4
  blank_pin: 2
  anode_1_pin: 5
  anode_2_pin: 16
  anode_3_pin: 17
```

## Core Classes

### NixieShiftRegister

Manages output to multiple cascaded shift registers for cathode control. Uses a fixed-size array (`bool data[32]`) to avoid heap fragmentation.

```cpp
// Constructors
NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t blankPin);

// Methods
void set(uint8_t pin, byte dataIn);  // Set bit at position (0-31)
void update();                        // Transfer data to shift registers
```

### NixieTube

Represents a single nixie tube with display index, digit value, and lit state.

```cpp
NixieTube();  // Default constructor
NixieTube(uint8_t t_nixieIDX, uint8_t t_nixieValue, bool t_nixieLit);

// Methods
uint8_t get_idx();              // Returns tube display index (1-6)
uint8_t get_value();            // Returns current digit value (0-9)
void set_value(uint8_t t_nixieValue);
bool nixie_lit();               // Returns lit state
void toggle_nixie_lit();        // Toggles lit state
bool fade_skip_{false};         // Skip ON state during crossfade
```

### NixieDisplay

Main display class with multiplexing, anti-poison, and tube management.

```cpp
// 2-anode configurations
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2);
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, NixieShiftRegister* t_sr);

// 3-anode configurations (up to 6 tubes)
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3);
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank);
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, NixieShiftRegister* t_sr);
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank, NixieShiftRegister* t_sr);

// Public API
void set(uint8_t t_tube, uint8_t t_digit);          // Set tube digit (1-indexed)
uint8_t get(uint8_t tube);                           // Get tube digit (1-indexed)
void update();                                        // Main update loop
void on();                                            // Turn on display
void off();                                           // Turn off display

// Anti-poison
void startAntiPoison(bool randomize, bool soft_start);
void antiPoison();                                    // Called every 100ms
uint8_t getAntiPoisonDigit(uint8_t tube_index);       // Get anti-poison digit (0-indexed)
bool isAntiPoisonActive();

// Fade state
uint8_t fade_cycles_{0};         // Cycles per fade phase
int8_t fade_remaining_[6];       // Signed per-tube counter
uint8_t old_cathode_[6];         // Cathode pin for digit being faded out
void triggerFadeOut(uint8_t tube_idx, uint8_t old_cathode_pin);
void setFadeCycles(uint8_t cycles);
```

#### toggleCathode()

Sets cathode for tubes on a given anode. On the current anode, uses the new digit (or old digit if fading out). Explicitly restores old cathodes for tubes on the **other** anode that are still fading out — they were cleared when ALL cathodes were reset.

```cpp
void toggleCathode(uint8_t tmux);
// tmux=1 -> anode 1 active -> tubes 1&3
// tmux=2 -> anode 2 active -> tubes 2&4
```

#### updateNixies()

Multiplexing state machine. Returns delay in microseconds:
- State 0 (OFF): All anodes off, sr_blank LOW
- State 1 (ON): Display current tube. During fade-out, anode stays ON (shows old cathode at reduced duty). During fade-in, skips anode (keeps new tube dark).
- State 2 (BLANK): Update cathode via `toggleCathode()`, sr_blank LOW

Fade state is decremented at the start of each call.

### NixieDisplayMux

ESPHome component wrapper. Manages component lifecycle and exposes YAML configurables.

```cpp
void set_num_tubes(uint8_t num);
void set_sr_data_pin(uint8_t pin);
void set_sr_clock_pin(uint8_t pin);
void set_sr_latch_pin(uint8_t pin);
void set_blank_pin(uint8_t pin);
void set_anode_1_pin(uint8_t pin);
void set_anode_2_pin(uint8_t pin);
void set_anode_3_pin(uint8_t pin);
void set_crossfade_enabled(bool enabled);
void set_crossfade_duration_ms(uint16_t ms);

// Display control
void set_digit(uint8_t tube_index, uint8_t digit);       // 0-indexed
void set_digits(const uint8_t *digits, uint8_t count);
void set_all_tubes(uint8_t digit);
void display_on();
void display_off();

// Anti-poison
void start_anti_poison(bool randomize, bool soft_start);
bool is_anti_poison_active();

// Neon indicators
void neons_on(bool enable);

// Task control
BaseType_t start_multiplexing_task();
void stop_multiplexing_task();
bool get_multiplexing_task_running() const;
```

#### set_digit()

Converts 0-indexed tube to 1-indexed for NixieDisplay. Triggers fade-out if crossfade is enabled and anti-poison is not active.

#### multiplexing_loop()

FreeRTOS task callback at 500Hz (2ms period):
1. Runs anti-poison state machine on display
2. Runs multiplexing update on display

## Anti-Poison Routine

Cycles through digits 0-9 on each tube to prevent burn-in. Each tube has its own sequence, runs for 30 seconds, then auto-stops when all tubes return to TIME mode.

### Member Variables
```cpp
bool anti_poison_enabled_{false};      // Main enable flag
bool ap_random_{true};                 // Randomize digit sequences
bool ap_soft_start_{true};             // Gradual tube transition
uint8_t ap_array[6][10];               // Anti-poison digit sequence per tube
uint8_t ap_state_[6];                  // Per-tube state: 0=TIME, 1=ANTI_POISON
uint8_t ap_current_digit_{0};          // Shared digit position counter (0-9)
unsigned long ap_last_check_{0};       // Timing reference
```

### State Machine
Each tube has two states:
| State | Value | Behavior |
|-------|-------|----------|
| TIME | 0 | Shows actual time digit |
| ANTI_POISON | 1 | Shows anti-poison digit |

### Operation Flow
1. All tubes start in TIME mode
2. Soft start transitions tubes one-by-one to ANTI_POISON
3. Digit counter cycles 0-9 every 100ms
4. Each tube in ANTI_POISON mode for 30 seconds, then returns to TIME
5. When all tubes return to TIME: routine stops automatically

## Multiplexing Details

### 2-Anode Configuration
- Anode 1 (default pin 5): Controls tubes 1 and 3
- Anode 2 (default pin 16): Controls tubes 2 and 4
- Cathodes 3 and 4 use upper shift register branch (+ SR_OFFSET)

### State Machine (500Hz = 2ms period)
| State | Name | Duration | sr_blank | Description |
|-------|------|----------|----------|-------------|
| 0 | OFF | NIXIE_BLANK_DELAY | LOW | Blank period, all tubes off |
| 1 | ON | NIXIE_ON_DELAY | HIGH | Display current tube with anode active |
| 2 | BLANK | NIXIE_BLANK_DELAY | LOW | Prepare cathode for next tube |

## Time Sources

### SNTP (Recommended)
ESP32 gets time directly from NTP servers:
```yaml
time:
  - platform: sntp
    id: esp_time
    timezone: America/Los_Angeles
```

### Home Assistant Time
Time synced from HA via API:
```yaml
time:
  - platform: homeassistant
    id: ha_time
    timezone: America/Los_Angeles
```

## YAML Timer Feature

Countdown timer using YAML-only globals and intervals:
```yaml
globals:
  - id: timer_remaining
    type: uint32_t
    restore_value: no
    initial_value: '0'
  - id: timer_active
    type: bool
    restore_value: no
    initial_value: 'false'

number:
  - platform: template
    id: timer_set_minutes
    name: "Countdown Timer"
    mode: slider
    min_value: 1
    max_value: 1440
    set_action:
      - lambda: |-
          id(timer_remaining) = static_cast<uint32_t>(x) * 60;
          id(timer_active) = true;

interval:
  - interval: 500ms
    then:
      - lambda: |-
          // Display countdown when active, otherwise show clock time
          if (id(timer_active) && id(timer_remaining) > 0) {
            // Show MM:SS or H:MM format on tubes
          } else {
            // Show HH:MM clock time
          }

  - interval: 1s
    then:
      - lambda: |-
          if (id(timer_active) && id(timer_remaining) > 0) {
            id(timer_remaining)--;
            if (id(timer_remaining) == 0) {
              id(timer_active) = false;
            }
          }
```

## SNTP Auto-trigger Example

Anti-poison every 15 minutes at :14:15, :29:15, :44:15, :59:15:
```yaml
time:
  - platform: sntp
    id: esp_time
    timezone: America/Los_Angeles
    on_time:
      - seconds: 15
        minutes: 14,29,44,59
        then:
          - button.press: start_anti_poison
```

## Example Configuration

```yaml
esphome:
  name: nixie-clock
  platform: ESP32
  board: esp32dev

time:
  - platform: sntp
    id: esp_time
    timezone: America/Los_Angeles

nixie_display_mux:
  id: nixie_clock
  tube_type: z573m
  power_board: v32

interval:
  - interval: 500ms
    then:
      - lambda: |-
          auto time = id(esp_time).now();
          if (!time.is_valid()) { id(nixie_clock)->set_all_tubes(0); return; }
          uint8_t digits[4];
          digits[0] = time.hour / 10;
          digits[1] = time.hour % 10;
          digits[2] = time.minute / 10;
          digits[3] = time.minute % 10;
          id(nixie_clock)->set_digits(digits, 4);
```

## Known Issues & Gotchas

### SR_OFFSET
Must match the power board version. Defined in `NixieDefines.h` via `POWER_BOARD_V30/V31/V32` preprocessor conditionals:
- v30: 15
- v31, v32: 16

The old approach of `cg.add_define("SR_OFFSET", ...)` in `__init__.py` did not work because `NixieDefines.h` was already included before the generated define. Use power board version defines instead.

### Cathode Persistence During Fade
The `toggleCathode()` function clears ALL cathodes then sets the current anode's cathodes. Old cathodes for tubes on the **other** anode must be explicitly restored after clearing, otherwise fade-out tubes on the other anode go dark prematurely.

### Timing Accuracy
`NixieDisplay::update()` uses a static timer to track updates. It should not rely on return values from `updateNixies()` since those are instance-timer driven, not return-driven.
