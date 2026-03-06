#include "NixieDisplay.h"

#ifdef USE_ESPHOME_FREERTOS_TASK
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#endif

// Constructor: 2-anode configuration with shift register (optional)
// Used for multiplexed nixie tube displays where anodes are shared between tubes
// Tube mapping for 2-anode configuration: Anode 1 controls tubes 1&3, Anode 2 controls tubes 2&4
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2) {
  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Constructor: 2-anode with shift register for cathode control
// t_sr pointer is not owned by NixieDisplay (managed by NixieDisplayMux)
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, NixieShiftRegister* t_sr)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2), sr_(t_sr) {
  pinMode(this->anode_1_, OUTPUT);
  pinMode(this->anode_2_, OUTPUT);

  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Constructor: 3-anode configuration with shift register (optional)
// Provides individual anode control for up to 6 tubes
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2), anode_3_(t_anode_3) {
  pinMode(this->anode_1_, OUTPUT);
  pinMode(this->anode_2_, OUTPUT);
  pinMode(this->anode_3_, OUTPUT);

  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Constructor: 3-anode with shift register blank pin and optional shift register
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3,
                           uint8_t t_sr_blank)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2), anode_3_(t_anode_3),
      sr_blank_(t_sr_blank) {
  pinMode(this->sr_blank_, OUTPUT);
  pinMode(this->anode_1_, OUTPUT);
  pinMode(this->anode_2_, OUTPUT);
  pinMode(this->anode_3_, OUTPUT);

  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Constructor: 3-anode with shift register (pointer not owned)
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3,
                           NixieShiftRegister* t_sr)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2), anode_3_(t_anode_3), sr_(t_sr) {
  pinMode(this->anode_1_, OUTPUT);
  pinMode(this->anode_2_, OUTPUT);
  pinMode(this->anode_3_, OUTPUT);

  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Constructor: 3-anode with shift register blank pin and shift register
NixieDisplay::NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3,
                           uint8_t t_sr_blank, NixieShiftRegister* t_sr)
    : numTubes_(t_numTubes), anode_1_(t_anode_1), anode_2_(t_anode_2), anode_3_(t_anode_3),
      sr_blank_(t_sr_blank), sr_(t_sr) {
  pinMode(this->sr_blank_, OUTPUT);
  pinMode(this->anode_1_, OUTPUT);
  pinMode(this->anode_2_, OUTPUT);
  pinMode(this->anode_3_, OUTPUT);

  for (uint8_t i = 0; i < this->numTubes_; i++) {
    tubeArray[i] = {(uint8_t)(i + 1), (uint8_t)(i + 1), false};
  }
  for (uint8_t i = this->numTubes_; i < 6; i++) {
    tubeArray[i] = {0, 0, false};
  }
}

// Destructor
// Clean up resources (sr_ is owned by NixieDisplayMux, not deleted here)
NixieDisplay::~NixieDisplay() {
  // sr_ is a pointer passed from NixieDisplayMux, do not delete it here
  // NixieDisplayMux owns the sr_ pointer and is responsible for deletion
}

// Set the digit value displayed on a specific tube
// t_tube is 1-indexed (tubes are numbered 1-6)
// t_digit is the value to display (0-9, with special characters possible depending on tube type)
// When anti-poison is enabled, this function is bypassed to prevent overwriting anti-poison digits
void NixieDisplay::set(uint8_t t_tube, uint8_t t_digit) {
  // Don't update display while anti-poison is active to prevent overwriting anti-poison digits
  tubeArray[t_tube - 1].set_value(t_digit);
}

// Get the current digit value displayed on a specific tube
// t_tube is 1-indexed (tubes are numbered 1-6)
// Returns the current digit value, or 0 if invalid tube index
uint8_t NixieDisplay::get(uint8_t tube) {
  return tubeArray[tube - 1].get_value();
}

// Main update loop - controls nixie tube multiplexing at 50Hz (20ms period)
// Uses instance timer instead of static to avoid race conditions with FreeRTOS tasks
// State machine:
//   State 0 (OFF): All tubes off, sr_blank LOW, blank period
//   State 1 (ON): Display current tube with anode active, sr_blank HIGH
//   State 2 (BLANK): Prepare cathode for next tube, update shift register, sr_blank LOW
// Only updates when 1000us have passed since last update
void NixieDisplay::update() {
  // Use instance timer instead of static to avoid race conditions with FreeRTOS task
  unsigned long current_time = micros();

  if (current_time - this->nixie_timer_ > 1000) {
    this->nixie_timer_ = current_time;
    uint16_t next_delay = this->updateNixies();

    // Only update shift register during the BLANK period (state 2)
    // This prevents ghosting and reduces CPU load
    if (this->nixie_state_ == 2) {
      sr_->update();
    }
  }
}

// Turn on the nixie display
// Sets state to BLANK (2) and triggers initial update
// Enables shift register output (sr_blank HIGH) for display updates
void NixieDisplay::on() {
  this->nixie_state_ = 2;
  NixieDisplay::updateNixies();
  sr_->update();
}

// Turn off the nixie display
// Sets state to OFF (0) and triggers update to blank all tubes
// Disables shift register output (sr_blank LOW)
void NixieDisplay::off() {
  this->nixie_state_ = 0;
  NixieDisplay::updateNixies();
  sr_->update();
}

// Toggle neon indicator lights based on multiplexer selection
// n_mux determines which group of neons is controlled:
//   - Each multiplexer group controls a subset of neon indicators
//   - Alternates between on/off states for visual effect
// This is a stub implementation pending full neon indicator feature
void NixieDisplay::toggleNeons(uint8_t n_mux) {
  // Stub implementation - to be completed when neon indicators are added
  // n_mux determines which neon group to toggle
  // Neon groups provide visual status feedback for the display
}

// Toggle anode pins based on mux selection
// tmux determines which anode is active for the 2-anode multiplexing scheme:
//   - 0: All anodes OFF (blank period)
//   - 1: Anode 1 active (controls tubes 1 and 3 in 2-anode config)
//   - 2: Anode 2 active (controls tubes 2 and 4 in 2-anode config)
//   - 3: Anode 3 active (for 3-anode configurations with up to 6 tubes)
void NixieDisplay::toggleAnode(uint8_t tmux) {
  switch (tmux) {
    case 0:
      digitalWrite(sr_blank_, LOW);
      digitalWrite(anode_1_, LOW);
      digitalWrite(anode_2_, LOW);
      digitalWrite(anode_3_, LOW);
    break;
    case 1:
      digitalWrite(anode_1_, HIGH);
    break;
    case 2:
      digitalWrite(anode_2_, HIGH);
    break;
    case 3:
      digitalWrite(anode_3_, HIGH);
    break;
  }
}

// Set cathode for a specific tube via shift register
// tmux is the tube number (0-indexed, corresponding to tube index)
// Iterates through all shift register bits and sets the cathode pin
// for the specified tube while clearing all other cathodes
void NixieDisplay::toggleCathode(uint8_t tmux) {
  for (int i = 0; i < SHIFT_REGISTER_COUNT * 8; i++) {
    if(i >= SR_OFFSET) {
      if((i == PIN_MAP[get(tmux+(this->numTubes_ / 2))] + SR_OFFSET)) {
        sr_->set(i, 1);
      } else {
        sr_->set(i, 0);
      }
    } else if(i < SR_OFFSET) {
      if(i == PIN_MAP[get(tmux)]) {
        sr_->set(i, 1);
      } else {
        sr_->set(i, 0);
      }
    }
  }
}

// Update nixie multiplexing state machine
// Returns the delay (in microseconds) to wait before next state
// The state machine implements 50Hz multiplexing (20ms period):
//   - State 0 (OFF): Blank period, all anodes off, sr_blank LOW
//   - State 1 (ON): Display current tube, anode active, sr_blank HIGH
//   - State 2 (BLANK): Prepare next tube, update shift register, sr_blank LOW
// The t_mux_ variable tracks which tube is being displayed and alternates
// between anodes based on the 2-anode multiplexing scheme.
uint16_t NixieDisplay::updateNixies() {
  // Nixie Multiplexing function for 2-anode 4-tube configuration:
  // - Anode 1 (pin 5): Controls tubes 1 and 3
  // - Anode 2 (pin 16): Controls tubes 2 and 4
  // Each tube has its own cathode pin (0-9 for digits 0-9)
  // State machine cycles through 3 states:
  // State 0: OFF - blank period, no anodes active (sr_blank LOW)
  // State 1: ON - anode active, displays the tube (sr_blank HIGH)
  // State 2: BLANK - prepares cathode for next tube, sr updates (sr_blank LOW)
  // The shift register only updates during the BLANK period to avoid ghosting.

  switch(this->nixie_state_) {
    case 0:
      // OFF - all tubes off
      toggleAnode(0);
      return NIXIE_BLANK_DELAY;
    break;
    case 1:
      // ON - display current tube with anode active
      this->nixie_state_ = 2;
      toggleAnode(t_mux_);
      if(t_mux_ < (this->numTubes_ / 2)) {
        t_mux_++;
      } else {
        t_mux_ = 1;
      }
      digitalWrite(sr_blank_, HIGH);
      return NIXIE_ON_DELAY;
    break;
    case 2:
      // BLANK - prepare cathode for next tube, update shift register
      this->nixie_state_ = 1;
      digitalWrite(sr_blank_, LOW);
      toggleAnode(0);
      toggleCathode(t_mux_);
      return NIXIE_BLANK_DELAY;
    break;
    default:
      // Should never reach here, but return a safe value for safety
      return NIXIE_BLANK_DELAY;
  }
}

// Update neon indicator lights
// Runs periodically to toggle neon states based on timing
// Manages neon blink, fade, and multiplexing effects
// This is a stub implementation pending full neon indicator feature
void NixieDisplay::updateNeons() {
  // Stub implementation - to be completed when neon indicators are added
  // Handles:
  // - Blinking states based on neon_blink_ flag
  // - Fading effects controlled by fade_state_ and fade_timer_
  // - Multiplexing between neon groups via neon_mux_
}

// ==================== Anti-Poison Routine Implementation ====================
// Anti-poison (anti-fatigue) routine cycles through all digit positions to prevent
// nixie tube degradation. Each tube displays digits 0-9 in sequence over 30 seconds.

// Start the anti-poison routine
// Parameters:
//   randomize - whether to randomize the digit sequence (currently uses sequential order)
//   soft_start - whether to gradually transition tubes to anti-poison mode
// After initialization, the routine cycles through digits for each tube and automatically
// stops when the displayed digit matches the anti-poison digit (within tolerance).
void NixieDisplay::startAntiPoison(bool randomize, bool soft_start) {
  this->anti_poison_enabled_ = true;
  this->ap_random_ = randomize;
  this->ap_soft_start_ = soft_start;

  // Generate anti-poison arrays (digits 0-9 for each tube)
  // Sequential order: 0,1,2,3,4,5,6,7,8,9
  for (uint8_t t = 0; t < this->numTubes_; t++) {
    for (uint8_t d = 0; d < 10; d++) {
      this->ap_array[t][d] = d;
    }
  }

  // Initialize AP state to TIME for all tubes (0 = showing time digit)
  for (uint8_t t = 0; t < this->numTubes_; t++) {
    this->ap_state_[t] = 0;  // TIME mode
  }

  // Reset counters
  this->ap_current_digit_ = 0;
  this->ap_loop_ = 0;
  this->soft_start_progress_ = 0;
  this->soft_stop_progress_ = 0;

  // Initialize timing check to allow immediate execution
  this->ap_last_check_ = 0;

  ESP_LOGI("nixie_display", "Anti-poison routine started (random=%d, soft_start=%d)",
           randomize, soft_start);
}

// Main anti-poison routine - called periodically by update()
// Runs every 100ms to cycle through digit positions
// During soft start: transitions tubes one by one to anti-poison mode
// When all tubes match their displayed digit: exits anti-poison mode automatically
void NixieDisplay::antiPoison() {
  unsigned long current_time = millis();

  if (current_time - ap_last_check_ < AP_TIME_CHECK_DELAY)
    return;

  ap_last_check_ = current_time;

  if (!anti_poison_enabled_)
    return;

  uint8_t new_ap_digit = (ap_current_digit_ + 1) % 10;

  // Soft start: enable tubes one by one
  if (ap_soft_start_ && ap_loop_ < numTubes_) {
    if (ap_state_[ap_loop_] == 0) {
      ap_state_[ap_loop_] = 1;
    }

    ap_loop_++;  // <-- missing increment
  }

  // Normal anti-poison cycling
  else if (ap_loop_ < ap_max_loops_) {
    ap_current_digit_ = new_ap_digit;
    ap_loop_++;
  }

  else {
    uint8_t tubes_still_in_ap = 0;

    for (uint8_t t = 0; t < numTubes_; t++) {
      if (ap_state_[t] == 1) {
        tubes_still_in_ap++;

        uint8_t ap_digit = ap_array[t][ap_current_digit_];
        uint8_t time_digit = tubeArray[t].get_value();

        int diff = abs((int)ap_digit - (int)time_digit);

        if (diff <= AP_MATCH_TOLERANCE) {
          ap_state_[t] = 0;
        }
      }
    }

    if (tubes_still_in_ap == 0) {
      anti_poison_enabled_ = false;
      for (uint8_t t = 0; t < numTubes_; t++)
        ap_state_[t] = 0;
      return;
    }

    if (ap_loop_ >= ap_max_loops_) {
      setAntiPoisonEnabled(false);
      ap_loop_ = 0;
    } else {
      ap_loop_++;
    }

    ap_current_digit_ = new_ap_digit;
  }
}

// Get the anti-poison digit for a specific tube
// tube_index is 0-indexed (0 to numTubes_-1)
// Returns the digit that should be displayed according to anti-poison state:
//   - If tube is in TIME mode: returns the actual time digit
//   - If tube is in ANTI_POISON mode: returns the anti-poison digit
// Returns 0 if tube_index is out of range
uint8_t NixieDisplay::getAntiPoisonDigit(uint8_t tube_index) {
  if (tube_index >= this->numTubes_) {
    return 0;
  }

  uint8_t ap_digit = this->ap_array[tube_index][this->ap_current_digit_];
  uint8_t actual_digit = this->tubeArray[tube_index].get_value();

  // Return the digit based on current state
  // State 0 = TIME mode (show actual digit)
  // State 1 = ANTI_POISON mode (show anti-poison digit)
  return this->ap_state_[tube_index] == 1 ? ap_digit : actual_digit;
}

// Enable or disable the anti-poison routine
// Set to true to start cycling through digits, false to disable
void NixieDisplay::setAntiPoisonEnabled(bool enabled) {
  this->anti_poison_enabled_ = enabled;
}

// Check if the anti-poison routine is currently active
// Returns true if anti-poison is enabled, false otherwise
bool NixieDisplay::isAntiPoisonActive() {
  return this->anti_poison_enabled_;
}