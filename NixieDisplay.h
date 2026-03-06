#ifndef NixieDisplay_h
#define NixieDisplay_h

#include "Arduino.h"
#include "NixieDefines.h"
#include "NixieTube.h"
#include "NixieShiftRegister.h"

// ESPHome includes for defines and helpers
#ifdef USE_ESPHOME_FREERTOS_TASK
#include "esphome/core/defines.h"
#include "esphome/core/helpers.h"
#endif

class NixieDisplay {
 public:
  // Constructors for different anode configurations
  // 2-anode configuration with shift register (optional) - multiplexed tubes
  // Tube mapping: Anode 1 controls tubes 1&3, Anode 2 controls tubes 2&4
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2);

  // 2-anode with shift register for cathode control
  // t_sr pointer is not owned by NixieDisplay (managed by NixieDisplayMux)
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, NixieShiftRegister* t_sr);

  // 3-anode configuration with shift register (optional) - up to 6 tubes
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3);

  // 3-anode with shift register blank pin and optional shift register
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank);

  // 3-anode with shift register (pointer not owned)
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, NixieShiftRegister* t_sr);

  // 3-anode with shift register blank pin and shift register
  NixieDisplay(uint8_t t_numTubes, uint8_t t_anode_1, uint8_t t_anode_2, uint8_t t_anode_3, uint8_t t_sr_blank,
               NixieShiftRegister* t_sr);

  // Set the digit value displayed on a specific tube (1-indexed)
  void set(uint8_t t_tube, uint8_t t_digit);

  // Get the current digit value displayed on a specific tube (1-indexed)
  uint8_t get(uint8_t tube);

  // Main update loop - controls nixie tube multiplexing at 50Hz
  void update();

  // Turn on the nixie display
  void on();

  // Turn off the nixie display
  void off();

  // Display feature control (time set indicator and neon indicators)
  // Enable/disable time set indicator (future implementation)
  void timeSetIndicator(bool t_enable);

  // Enable/disable neon indicator lights (future implementation)
  void neonsOff(bool t_enable);

  ~NixieDisplay();

  // State variables
  bool neons_on_{false};      // Neon indicator lights state
  bool crossfade_{false};     // Crossfade mode between tubes

  // Anti-poison routine (anti-fatigue) for nixie tubes
  // Cycles through all digit positions to prevent tube degradation
  void startAntiPoison(bool randomize, bool soft_start);

  // Main anti-poison routine - called periodically by update()
  void antiPoison();

  // Get the anti-poison digit for a specific tube (0-indexed)
  uint8_t getAntiPoisonDigit(uint8_t tube_index);

  // Enable or disable the anti-poison routine
  void setAntiPoisonEnabled(bool enabled);

  // Check if the anti-poison routine is currently active
  bool isAntiPoisonActive();

 private:
  // ==================== Data Members ====================

  // ==================== Configuration Variables ====================
  uint8_t numTubes_{0};                    // Number of nixie tubes (max 6)
  uint8_t anode_1_{0};                     // Pin for anode 1 (controls tubes 1&3 in 2-anode config)
  uint8_t anode_2_{0};                     // Pin for anode 2 (controls tubes 2&4 in 2-anode config)
  uint8_t anode_3_{0};                     // Pin for anode 3 (for 3-anode configurations)
  bool time_set_{false};                   // Time set indicator state
  uint8_t sr_blank_{0};                    // Shift register blank pin
  NixieTube tubeArray[6];                  // Fixed-size array, no heap allocation (tube index, value, lit)
  NixieShiftRegister* sr_{nullptr};        // Shift register pointer (not owned by this class)
  uint16_t muxDelay_{0};                   // Multiplexing delay value
  byte t_mux_{1};                          // Current multiplexer index for tube selection

  // ==================== Instance Timing Variables ====================
  // Instance timing variables (one per display) - fixes race conditions with FreeRTOS task
  unsigned long nixie_timer_{0};           // Last time updateNixies() was called (micros)
  unsigned long neon_timer_{0};            // Last neon update time
  unsigned long blink_timer_{0};           // Last neon blink time
  unsigned long fade_timer_{0};            // Last fade update time
  uint16_t neon_delay_{2000};              // Neon off delay (milliseconds)
  bool neon_blink_{false};                 // Neon blinking state
  bool fade_state_{false};                 // Current fade state
  byte nixie_state_{1};                    // State machine state (0=OFF, 1=ON, 2=BLANK)
  byte neon_mux_{1};                       // Neon multiplexer index

  // Per-tube fade state (instance variables, not static - fixes race conditions)
  // Each tube has its own state since the 2-anode mux only controls 2 tubes at a time

  // ==================== Anti-Poison Routine Variables ====================
  bool anti_poison_enabled_{false};        // Whether anti-poison routine is currently enabled
  bool ap_random_{true};                   // Whether to randomize digit sequence (sequential mode used)
  bool ap_soft_start_{true};               // Whether to gradually transition tubes to anti-poison mode
  uint8_t ap_array[6][10];                 // Anti-poison digit permutations per tube (tube x digit)
  uint8_t ap_state_[6];                    // AP state for each tube (0=TIME, 1=ANTI_POISON)
  uint8_t ap_current_digit_{0};            // Current digit position in anti-poison array (shared across all tubes)
  uint16_t ap_loop_{0};                     // Current loop count (tracks soft start progress)
  uint16_t ap_max_loops_{AP_MAX_LOOPS_DEFAULT};  // Maximum anti-poison loops before exit check
  uint8_t soft_start_progress_{0};         // Progress through soft-start sequence
  uint8_t soft_stop_progress_{0};          // Progress through soft-stop sequence
  unsigned long ap_last_check_{0};         // Last time anti-poison state was checked (millis)

  // ==================== Private Methods ====================
  // Neon indicator methods (stub - to be implemented)
  void toggleNeons(uint8_t n_mux);         // Toggle neon indicator lights by multiplexer group; n_mux determines which neon group to toggle, alternating on/off states for visual status feedback
  void updateNeons();                      // Update neon indicator lights; handles blinking, fading, and multiplexing between neon groups (stub implementation pending)

  // Anode multiplexing method
  void toggleAnode(uint8_t tmux);          // Toggle anode pins based on mux selection for 2-anode multiplexing; tmux=0 (all OFF), 1 (anode 1 active - tubes 1&3), 2 (anode 2 active - tubes 2&4), 3 (anode 3 active)
  void toggleCathode(uint8_t tube_index);  // Set cathode for a specific tube via shift register; tmux is tube number (0-indexed), sets the specified tube's cathode while clearing all others
  uint16_t updateNixies();                 // Update nixie multiplexing state machine, returns delay in microseconds; implements 50Hz multiplexing with 3-state cycle: OFF (blank period) → ON (display current tube) → BLANK (prepare next tube), cycles through anodes for 2-anode configuration
};

#endif