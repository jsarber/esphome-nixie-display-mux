# Nixie Display Mux - Component Documentation

## Overview

The NixieDisplay component handles multiplexed nixie tube displays with anti-poison (anti-fatigue) routines. It supports multiple anode configurations and provides robust state management for multiplexing operations.

## File Structure

This component uses a modular library structure. The core Nixie library files are located in the `lib/` subdirectory:

```
components/nixie_display_mux/
├── __init__.py                    # ESPHome Python configuration schema
├── nixie_display_mux.h            # Main ESPHome component header
├── nixie_display_mux.cpp          # Main ESPHome component implementation
├── CLAUDE.md                      # This documentation file
└── lib/                           # Nixie library files
    ├── NixieDefines.h             # Library constants and type definitions
    ├── NixieDisplay.h             # Main display class header
    ├── NixieDisplay.cpp           # Main display class implementation
    ├── NixieShiftRegister.h       # Shift register helper class header
    ├── NixieShiftRegister.cpp     # Shift register helper class implementation
    ├── NixieTube.h                # Individual tube class header
    ├── NixieTube.cpp              # Individual tube class implementation
    └── Z573M_4.h                  # Tube type definitions (IN12A/6 digit)
```

All ESPHome source files (`nixie_display_mux.*`) include the library files from the `lib/` directory.

## Core Classes

### NixieShiftRegister

## Core Classes

### NixieShiftRegister

Manages output to multiple cascaded shift registers used for controlling cathode pins on nixie tubes. Uses a fixed-size stack-based array (`bool data[32]`) to avoid heap fragmentation on embedded systems.

#### Class Members
| Member | Type | Description |
|--------|------|-------------|
| `shiftRegisterCount` | `uint8_t` | Number of cascaded shift registers (max 4) |
| `dataPin` | `uint8_t` | Pin connected to shift register data output |
| `clockPin` | `uint8_t` | Pin connected to shift register clock |
| `latchPin` | `uint8_t` | Pin connected to shift register latch/strobe |
| `data` | `bool[32]` | Fixed-size array storing bit data (max 4 registers × 8 bits) |

#### Constructors
```cpp
NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t blankPin);
```

Both constructors initialize pin modes to OUTPUT and zero-initialize the `data` array to prevent garbage values.

#### Public Methods

**set()**
```cpp
void set(uint8_t pin, byte dataIn);
```
Sets the data bit at the specified position.
- `pin`: Bit position index (0-31)
- `dataIn`: Byte value (0-255) to store at that position

**update()**
```cpp
void update();
```
Transfers all stored data to the physical shift registers via SPI-like serial output.
1. Drives latch pin LOW
2. Sends data in 8-bit chunks using `shiftOut()` with MSBFIRST ordering
3. Sends `shiftRegisterCount` bytes total
4. Drives latch pin HIGH to capture data

---

### NixieTube

Represents a single nixie tube with its display index, digit value, and lit state. Used by `NixieDisplay` to manage up to 6 tubes.

#### Class Members
| Member | Type | Description |
|--------|------|-------------|
| `nixieIDX` | `uint8_t` | Tube display index (1-6) |
| `nixieValue` | `uint8_t` | Current digit value to display (0-9) |
| `nixieLit` | `bool` | Whether the tube is currently lit/active |

#### Constructors
```cpp
NixieTube();  // Default constructor - all values zero/false
NixieTube(uint8_t t_nixieIDX, uint8_t t_nixieValue, bool t_nixieLit);
```

#### Public Methods

**get_idx()**
```cpp
uint8_t get_idx();
```
Returns the tube's display index (1-6).

**get_value()**
```cpp
uint8_t get_value();
```
Returns the current digit value displayed on this tube (0-9).

**set_value()**
```cpp
void set_value(uint8_t t_nixieValue);
```
Sets the digit value to display on this tube.

**nixie_lit()**
```cpp
bool nixie_lit();
```
Returns whether the tube is currently lit/active.

**toggle_nixie_lit()**
```cpp
void toggle_nixie_lit();
```
Toggles the lit state between true and false.

---

## NixieDisplay Class

## NixieDisplay Class

### Constructor Functions

All constructors accept the number of tubes and anode pin assignments. The `NixieShiftRegister*` parameter is passed but NOT owned by this class (managed by NixieDisplayMux).

#### 2-Anode Configuration (Multiplexed)
```cpp
// 2-anode with shift register (optional) - tubes 1&3 share anode 1, tubes 2&4 share anode 2
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2);

// 2-anode with shift register for cathode control
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, NixieShiftRegister* t_sr);
```

#### 3-Anode Configuration (Up to 6 tubes)
```cpp
// 3-anode configuration - each tube has its own anode
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3);

// 3-anode with shift register blank pin
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank);

// 3-anode with shift register (pointer not owned)
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, NixieShiftRegister* t_sr);

// Full 3-anode configuration
NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank, NixieShiftRegister* t_sr);
```

### Public API Functions

#### set()
```cpp
void set(uint8_t t_tube, uint8_t t_digit);
```
Sets the digit value displayed on a specific tube.
- `t_tube`: 1-indexed tube number (1-6)
- `t_digit`: Value to display (0-9)
- **Important**: When anti-poison is enabled, this function is bypassed to prevent overwriting anti-poison digits

#### get()
```cpp
uint8_t get(uint8_t tube);
```
Gets the current digit value from a specific tube.
- `tube`: 1-indexed tube number (1-6)
- Returns: Current digit value, or 0 if invalid index

#### update()
```cpp
void update();
```
Main update loop that controls nixie tube multiplexing at 50Hz (20ms period).
- Uses instance timer instead of static to avoid race conditions with FreeRTOS tasks
- Only updates when 1000us have passed since last update
- State machine cycles through OFF → ON → BLANK states

#### on()
```cpp
void on();
```
Turns on the nixie display.
- Sets state to BLANK (2) and triggers initial update
- Enables shift register output (sr_blank HIGH) for display updates

#### off()
```cpp
void off();
```
Turns off the nixie display.
- Sets state to OFF (0) and triggers update to blank all tubes
- Disables shift register output (sr_blank LOW)

### Anti-Poison Functions

The anti-poison routine prevents nixie tube degradation by cycling through all digit positions (0-9) for each tube over approximately 30 seconds.

#### startAntiPoison()
```cpp
void startAntiPoison(bool randomize, bool soft_start);
```
Starts the anti-poison routine.
- `randomize`: Whether to randomize the digit sequence (currently uses sequential order 0-9)
- `soft_start`: Whether to gradually transition tubes to anti-poison mode
- After initialization, the routine cycles through digits and automatically stops when displayed digit matches the anti-poison digit (within tolerance)

#### antiPoison()
```cpp
void antiPoison();
```
Main anti-poison routine - called periodically by update() every 100ms.
- **Soft Start Phase**: Transitions tubes one by one to ANTI_POISON mode
- **Cycling Phase**: All tubes cycle through digits 0-9
- **Exit Check**: For each tube in ANTI_POISON mode, checks if the anti-poison digit matches the time digit (within AP_MATCH_TOLERANCE)
- **Auto-Stop**: When all tubes return to TIME mode, disables the routine automatically

#### getAntiPoisonDigit()
```cpp
uint8_t getAntiPoisonDigit(uint8_t tube_index);
```
Gets the digit that should be displayed according to anti-poison state.
- `tube_index`: 0-indexed tube number (0 to numTubes_-1)
- Returns: Anti-poison digit if tube is in ANTI_POISON mode, otherwise actual time digit
- Returns 0 if tube_index is out of range

#### setAntiPoisonEnabled()
```cpp
void setAntiPoisonEnabled(bool enabled);
```
Enables or disables the anti-poison routine.

#### isAntiPoisonActive()
```cpp
bool isAntiPoisonActive();
```
Checks if the anti-poison routine is currently active.

### Private Helper Functions

#### toggleAnode()
```cpp
void toggleAnode(uint8_t tmux);
```
Toggles anode pins based on mux selection for 2-anode multiplexing:
- `tmux = 0`: All anodes OFF (blank period), sr_blank LOW
- `tmux = 1`: Anode 1 active (controls tubes 1 and 3 in 2-anode config)
- `tmux = 2`: Anode 2 active (controls tubes 2 and 4 in 2-anode config)
- `tmux = 3`: Anode 3 active (for 3-anode configurations)

#### toggleCathode()
```cpp
void toggleCathode(uint8_t tube_index);
```
Sets cathode for a specific tube via shift register.
- Iterates through all shift register bits
- Sets the cathode pin for the specified tube to HIGH
- Clears all other cathode pins to LOW

#### updateNixies()
```cpp
uint16_t updateNixies();
```
Updates the nixie multiplexing state machine. Returns delay in microseconds.
- State 0 (OFF): Blank period, all anodes off, sr_blank LOW, returns NIXIE_BLANK_DELAY
- State 1 (ON): Display current tube with anode active, sr_blank HIGH, returns NIXIE_ON_DELAY
- State 2 (BLANK): Prepare next tube, update shift register, sr_blank LOW, returns NIXIE_BLANK_DELAY
- The t_mux_ variable tracks which tube is being displayed and alternates between anodes

## Anti-Poison Routine Details

### Digit Cycling
The routine cycles through digits 0-9 on each tube to prevent "burn-in":
1. **Underlying Time**: Actual time digits stored in `tubeArray` (e.g., "12:34")
2. **Anti-Poison Digits**: Each tube has its own sequence that cycles through 0-9
3. **State Selection**: Tube shows either time digit or anti-poison digit based on state

### Member Variables
```cpp
bool anti_poison_enabled_{false};      // Main enable flag
bool ap_random_{true};                 // Randomize digit sequences (sequential mode used)
bool ap_soft_start_{true};             // Gradual tube transition
uint8_t ap_array[6][10];               // Anti-poison digit sequence per tube
uint8_t ap_state_[6];                  // Per-tube state: 0=TIME, 1=ANTI_POISON
uint8_t ap_current_digit_{0};          // Shared digit position counter (0-9)
uint8_t ap_loop_{0};                   // Soft start progress counter
uint8_t ap_max_loops_{AP_MAX_LOOPS_DEFAULT};  // Max loops before exit check
unsigned long ap_last_check_{0};       // Timing reference for state checks
```

### State Machine
Each tube has two states:
| State | Value | Behavior |
|-------|-------|----------|
| TIME | 0 | Shows actual time digit from `tubeArray` |
| ANTI_POISON | 1 | Shows anti-poison digit from `ap_array[tube][ap_current_digit_]` |

### Operation Flow
```
User presses "Start Anti-Poison" button
         ↓
startAntiPoison() initializes:
   - All tubes set to TIME mode (state = 0)
   - ap_current_digit_ = 0
   - ap_loop_ = 0
         ↓
antiPoison() called every 100ms:
   Step 1: Increment ap_current_digit_ (cycles 0→1→2→...→9→0)
   Step 2: During soft start, transition tubes one-by-one to ANTI_POISON mode
   Step 3: After all tubes in ANTI_POISON mode:
           For each tube: Check if ap_digit matches time_digit (within tolerance)
           If match found: Set tube back to TIME mode
   Step 4: When all tubes return to TIME mode: Disable anti-poison routine
```

### Soft Start and Soft Stop
**Soft Start**: Tubes transition one at a time (Tube 0 → Tube 1 → ...) preventing sudden visual changes.

**Match Tolerance**: Anti-poison digit matches time digit when `|ap_digit - time_digit| ≤ AP_MATCH_TOLERANCE` (default = 3).

Example: If time digit is "1", tube exits ANTI_POISON mode when showing digits 0-4.

### Timing Configuration
```cpp
#define AP_TIME_CHECK_DELAY 100  // ms - Time between state checks
#define AP_MAX_LOOPS_DEFAULT 30  // Number of loops before exit check
```
- Digit cycles through 0-9 every 100ms (10 digits per second)
- Each loop lasts ~1 second
- 30 loops = 30 seconds total runtime

## Multiplexing Details

### 2-Anode Configuration
- Anode 1 (pin 5): Controls tubes 1 and 3
- Anode 2 (pin 16): Controls tubes 2 and 4
- Each tube has its own cathode pin (0-9 for digits 0-9)

### State Machine (50Hz = 20ms period)
| State | Name | Duration | sr_blank | Description |
|-------|------|----------|----------|-------------|
| 0 | OFF | NIXIE_BLANK_DELAY | LOW | Blank period, all tubes off |
| 1 | ON | NIXIE_ON_DELAY | HIGH | Display current tube with anode active |
| 2 | BLANK | NIXIE_BLANK_DELAY | LOW | Prepare cathode for next tube, update shift register |

### Protection Against Conflicts
The `set()` method is protected when anti-poison is enabled to prevent the YAML interval from overwriting anti-poison digits.

## Constants
```cpp
#define NIXIE_BLANK_DELAY     // Delay during blank period (microseconds)
#define NIXIE_ON_DELAY        // Delay during ON period (microseconds)
#define SHIFT_REGISTER_COUNT  4  // Number of shift registers
#define SR_OFFSET             // Offset for shift register pins
#define AP_MAX_LOOPS_DEFAULT  30
#define AP_TIME_CHECK_DELAY   100  // ms
#define AP_MATCH_TOLERANCE    3    // Max difference for time match
```

## Example Usage

### Basic NixieDisplay Usage
```cpp
// Create 4-tube display with 2-anode configuration
NixieDisplay display(4, NIXIE_ANODE_1, NIXIE_ANODE_2, sr);

// Set a digit on tube 1 (shows value 7)
display.set(1, 7);

// Turn on the display
display.on();

// Update display (called in loop)
display.update();

// Start anti-poison routine
display.startAntiPoison(true, true);

// Check if anti-poison is active
if (display.isAntiPoisonActive()) {
    // Get what digit should be shown (considering anti-poison)
    uint8_t digit = display.getAntiPoisonDigit(0);
}
```

### NixieShiftRegister Usage
```cpp
// Create shift register for cathode control (4 registers for 32 bits)
NixieShiftRegister sr(4, SHIFT_REGISTER_DATA, SHIFT_REGISTER_CLK, SHIFT_REGISTER_LATCH, SHIFT_REGISTER_BLANK);

// Set individual cathode bits
sr.set(0, 1);  // Set bit 0 (tube 1, digit 0 cathode)
sr.set(1, 1);  // Set bit 1 (tube 1, digit 1 cathode)
// ... set more bits as needed

// Transfer data to shift registers
sr.update();
```

### NixieTube Usage
```cpp
// Create a tube object
NixieTube tube(1, 7, true);  // Tube 1, showing digit 7, lit

// Access properties
uint8_t idx = tube.get_idx();        // Returns 1
uint8_t value = tube.get_value();    // Returns 7
bool lit = tube.nixie_lit();         // Returns true

// Modify properties
tube.set_value(3);  // Now shows digit 3
tube.toggle_nixie_lit();  // Turns off (now false)
```

### Complete Initialization Example
```cpp
// Create shift register for cathode control
NixieShiftRegister* sr = new NixieShiftRegister(4, SHIFT_REGISTER_DATA, SHIFT_REGISTER_CLK, SHIFT_REGISTER_LATCH, SHIFT_REGISTER_BLANK);

// Create display with shift register (pointer not owned by display)
NixieDisplay display(4, NIXIE_ANODE_1, NIXIE_ANODE_2, sr);

// Set up display values
display.set(1, 1);  // Hours tens
display.set(2, 2);  // Hours units
display.set(3, 3);  // Minutes tens
display.set(4, 4);  // Minutes units

// Initialize and run
display.on();
display.update();

// Start anti-poison routine (optional)
display.startAntiPoison(true, true);
```

---

## NixieDisplayMux Class

### Overview

`NixieDisplayMux` is the main ESPHome component that integrates with ESPHome's framework to control a multiplexed nixie tube display. It uses FreeRTOS tasks for high-frequency multiplexing (500Hz) and manages both `NixieShiftRegister` and `NixieDisplay` instances.

### Key Features

- **FreeRTOS Task**: Runs multiplexing at 500Hz (2ms period) in a dedicated task to ensure consistent timing
- **Stack-Based Memory**: Uses fixed-size arrays to avoid heap fragmentation on embedded systems
- **Anti-Poison Support**: Provides anti-fatigue routine for nixie tubes
- **Multiple Anode Configurations**: Supports 2-anode (up to 4 tubes) and 3-anode (up to 6 tubes) configurations

### Constructor and Destructor

```cpp
NixieDisplayMux();                    // Default constructor
~NixieDisplayMux();                   // Destructor - cleans up display and shift register
```

### Component Lifecycle Methods

#### setup()
```cpp
void setup() override;
```
Initializes the component:
1. Creates `NixieShiftRegister` instance (4 registers)
2. Creates `NixieDisplay` instance with 3-anode configuration
3. Initializes all tubes to display 0
4. Starts FreeRTOS multiplexing task

#### loop()
```cpp
void loop() override;
```
Minimal implementation for task cleanup when not running. Multiplexing is handled by the FreeRTOS task.

#### multiplexing_loop()
```cpp
void multiplexing_loop();
```
FreeRTOS task callback called at 500Hz (2ms period):
1. Runs anti-poison state machine on display
2. Runs multiplexing update on display

### Configuration Methods

These setter methods are called from Python configuration:

```cpp
void set_num_tubes(uint8_t num);          // Set number of tubes (2-6)
void set_sr_data_pin(uint8_t pin);        // Shift register data pin
void set_sr_clock_pin(uint8_t pin);       // Shift register clock pin
void set_sr_latch_pin(uint8_t pin);       // Shift register latch pin
void set_blank_pin(uint8_t pin);          // Shift register blank pin
void set_anode_1_pin(uint8_t pin);        // Anode 1 (tubes 1&3 in 2-anode config)
void set_anode_2_pin(uint8_t pin);        // Anode 2 (tubes 2&4 in 2-anode config)
void set_anode_3_pin(uint8_t pin);        // Anode 3 (for 3-anode configurations)
```

### Display Control Methods

#### test()
```cpp
void test();
```
Test mode for debugging display hardware. (Implementation TBD)

#### set_digit()
```cpp
void set_digit(uint8_t tube_index, uint8_t digit);
```
Sets the digit value for a specific tube.
- `tube_index`: 0-indexed (0 to num_tubes_-1)
- Internally converts to 1-based indexing for NixieDisplay

#### set_digits()
```cpp
void set_digits(const uint8_t *digits, uint8_t count);
```
Sets digits for multiple tubes from an array.
- `digits`: Pointer to array of digit values
- `count`: Number of elements to process

#### set_all_tubes()
```cpp
void set_all_tubes(uint8_t digit);
```
Sets the same digit for all tubes simultaneously.

#### display_on()
```cpp
void display_on();
```
Turns on the display:
1. Calls `display_->on()` to initialize state
2. Starts the FreeRTOS multiplexing task

#### display_off()
```cpp
void display_off();
```
Turns off the display:
1. Calls `display_->off()` to blank all tubes
2. Stops the FreeRTOS multiplexing task

### Anti-Poison Methods

#### start_anti_poison()
```cpp
void start_anti_poison(bool randomize, bool soft_start);
```
Starts the anti-poison routine on the display.
- `randomize`: Whether to randomize digit sequence
- `soft_start`: Whether to gradually transition tubes

#### is_anti_poison_active()
```cpp
bool is_anti_poison_active();
```
Checks if the anti-poison routine is currently active.

### Task Control Methods

#### start_multiplexing_task()
```cpp
BaseType_t start_multiplexing_task();
```
Creates the FreeRTOS multiplexing task:
- Task name: "nixie_mux_task"
- Stack size: 2KB (2048 bytes)
- Priority: tskIDLE_PRIORITY + 2
- Returns: pdPASS on success, pdFAIL on failure

#### stop_multiplexing_task()
```cpp
void stop_multiplexing_task();
```
Stops the FreeRTOS multiplexing task:
1. Sets `start_multiplexing_task_` flag to false
2. Deletes the task using `vTaskDelete()`

#### get_multiplexing_task_running()
```cpp
bool get_multiplexing_task_running() const;
```
Returns true if the multiplexing task is currently running.

### FreeRTOS Task Details

The `multiplexing_task` runs independently in a dedicated FreeRTOS task:

```cpp
void multiplexing_task(void *pvParameters) {
  NixieDisplayMux *component = static_cast<NixieDisplayMux *>(pvParameters);
  while (true) {
    component->multiplexing_loop();
    vTaskDelay(pdMS_TO_TICKS(2));  // 2ms period = ~500Hz
  }
}
```

**Why a FreeRTOS Task?**
- Ensures consistent 500Hz multiplexing rate independent of main loop
- Prevents timing drift that could cause display artifacts
- Allows display updates without blocking main ESPHome tasks

### Internal Data Members

```cpp
uint8_t num_tubes_{4};                  // Number of tubes (configurable 2-6)
uint8_t sr_data_pin_{27};               // Shift register data pin
uint8_t sr_clock_pin_{26};              // Shift register clock pin
uint8_t sr_latch_pin_{4};               // Shift register latch pin
uint8_t blank_pin_{2};                  // Shift register blank pin
uint8_t anode_1_pin_{5};                // Anode 1 pin
uint8_t anode_2_pin_{16};               // Anode 2 pin
uint8_t anode_3_pin_{17};               // Anode 3 pin

NixieShiftRegister *sr_{nullptr};       // Shift register instance (owned)
NixieDisplay *display_{nullptr};        // Display instance (owned)

bool start_multiplexing_task_{false};   // Task running flag
```

### ESPHome Configuration Example

```yaml
esphome:
  name: nixie_clock
  platform: ESP32
  board: esp32dev

nixie_display_mux:
  num_tubes: 4
  sr_data_pin: 27
  sr_clock_pin: 26
  sr_latch_pin: 4
  blank_pin: 2
  anode_1_pin: 5
  anode_2_pin: 16
  anode_3_pin: 17

sensor:
  - platform: time
    name: "Current Time"
```

### Generated Code Flow

1. Python config validation in `__init__.py` validates pin numbers and tube count
2. `to_code()` generates C++ calls to setter methods
3. During ESPHome setup, `setup()` creates hardware instances and starts FreeRTOS task
4. Main loop handles task cleanup if needed
5. FreeRTOS task runs at 500Hz, calling `multiplexing_loop()` for display updates