#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "NixieDefines.h"
#include "NixieDisplay.h"

#include "NixieShiftRegister.h"

namespace esphome {
namespace nixie_display_mux {

class NixieDisplayMux : public Component {
 public:
  // Constructor
  // Default constructor for NixieDisplayMux component
  NixieDisplayMux() = default;

  // Destructor
  // Cleans up shift register and display instances
  ~NixieDisplayMux();

  // Override Component methods
  void setup() override;     // Initialize component, create shift register and display, start FreeRTOS task
  void loop() override;      // Main loop - handles task cleanup when not running
  void multiplexing_loop();  // FreeRTOS task callback - runs multiplexing and anti-poison

  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  // ==================== Configuration Methods ====================
  // Set number of nixie tubes (2-6)
  void set_num_tubes(uint8_t num) { this->num_tubes_ = num; }

  // Set shift register data pin
  void set_sr_data_pin(uint8_t pin) { this->sr_data_pin_ = pin; }

  // Set shift register clock pin
  void set_sr_clock_pin(uint8_t pin) { this->sr_clock_pin_ = pin; }

  // Set shift register latch pin
  void set_sr_latch_pin(uint8_t pin) { this->sr_latch_pin_ = pin; }

  // Set shift register blank pin
  void set_blank_pin(uint8_t pin) { this->blank_pin_ = pin; }

  // Set anode 1 pin (controls tubes 1&3 in 2-anode configuration)
  void set_anode_1_pin(uint8_t pin) { this->anode_1_pin_ = pin; }

  // Set anode 2 pin (controls tubes 2&4 in 2-anode configuration)
  void set_anode_2_pin(uint8_t pin) { this->anode_2_pin_ = pin; }

  // Set anode 3 pin (for 3-anode configurations)
  void set_anode_3_pin(uint8_t pin) { this->anode_3_pin_ = pin; }

  // ==================== Display Control Methods ====================
  // Test mode - useful for debugging display hardware
  void test();

  // Set digit value for a specific tube (0-indexed)
  void set_digit(uint8_t tube_index, uint8_t digit);

  // Set digits for multiple tubes from an array
  void set_digits(const uint8_t *digits, uint8_t count);

  // Set the same digit for all tubes
  void set_all_tubes(uint8_t digit);

  // Turn display on - initializes multiplexing
  void display_on();

  // Turn display off - stops multiplexing
  void display_off();

  // ==================== Anti-Poison Routine ====================
  // Start anti-poison routine to cycle through all digits
  // Prevents tube degradation by displaying each digit position periodically
  void start_anti_poison(bool randomize, bool soft_start);

  // Check if anti-poison routine is currently active
  bool is_anti_poison_active();

  // ==================== Neon Indicator Control ====================
  // Enable or disable neon indicator lights
  void neons_on(bool enable);

  // ==================== Task Control ====================
  // Start the FreeRTOS multiplexing task
  // Returns pdPASS on success, pdFAIL on failure
  BaseType_t start_multiplexing_task();

  // Stop the FreeRTOS multiplexing task
  void stop_multiplexing_task();

  // Check if multiplexing task is currently running
  bool get_multiplexing_task_running() const { return this->start_multiplexing_task_; }

 protected:
  bool start_multiplexing_task_{false};

  // Configuration
  uint8_t num_tubes_ = NIXIE_TUBES;
  uint8_t sr_data_pin_ = SHIFT_REGISTER_DATA;
  uint8_t sr_clock_pin_ = SHIFT_REGISTER_CLK;
  uint8_t sr_latch_pin_ = SHIFT_REGISTER_LATCH;
  uint8_t blank_pin_ = SHIFT_REGISTER_BLANK;
  uint8_t anode_1_pin_ = NIXIE_ANODE_1;
  uint8_t anode_2_pin_ = NIXIE_ANODE_2;
  uint8_t anode_3_pin_ = NIXIE_ANODE_3;

  // Your existing library instances
  NixieShiftRegister *sr_{nullptr};
  NixieDisplay *display_{nullptr};
};

extern TaskHandle_t nixie_display_mux_task_handle;

}  // namespace nixie_display_mux
}  // namespace esphome