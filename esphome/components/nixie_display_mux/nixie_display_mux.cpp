#include "nixie_display_mux.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/defines.h"

namespace esphome {
namespace nixie_display_mux {

static const char *const TAG = "nixie_display_mux";

// FreeRTOS task function for multiplexing
// Runs in a dedicated task to handle high-frequency display updates
// Independent from main loop to ensure consistent 500Hz multiplexing
void multiplexing_task(void *pvParameters) {
  NixieDisplayMux *component = static_cast<NixieDisplayMux *>(pvParameters);
  while (true) {
    component->multiplexing_loop();
    vTaskDelay(pdMS_TO_TICKS(2));  // 2ms multiplexing period (~500Hz, reduces flicker)
  }
}

// Destructor
// Clean up dynamically allocated resources
// Deletes display and shift register instances created in setup()
NixieDisplayMux::~NixieDisplayMux() {
  if (this->display_) {
    delete this->display_;
    this->display_ = nullptr;
  }
  if (this->sr_) {
    delete this->sr_;
    this->sr_ = nullptr;
  }
}

TaskHandle_t nixie_display_mux_task_handle = nullptr;

void NixieDisplayMux::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Nixie Display using original libraries...");

  this->sr_ = new NixieShiftRegister(4, this->sr_data_pin_, this->sr_clock_pin_, this->sr_latch_pin_);

  this->display_ = new NixieDisplay(this->num_tubes_, this->anode_1_pin_, this->anode_2_pin_,
                                     this->anode_3_pin_, this->blank_pin_, this->sr_);

  ESP_LOGCONFIG(TAG, "  Number of tubes: %d", this->num_tubes_);
  ESP_LOGCONFIG(TAG, "  Using NixieDisplay and NixieShiftRegister libraries");
  ESP_LOGCONFIG(TAG, "  Multiplexing frequency: %d Hz (2ms period)", MULTIPLEXING_FREQUENCY_HZ);

  this->set_all_tubes(0);

  BaseType_t create_result = this->start_multiplexing_task();

  if (create_result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create nixie display multiplexing task!");
    this->start_multiplexing_task_ = false;
    return;
  }
}

// Main loop override
// Minimal implementation - only handles task cleanup when not running
// Multiplexing is handled by the dedicated FreeRTOS task
void NixieDisplayMux::loop() {
  // This runs in the main loop - only check task status
  if (!this->start_multiplexing_task_) {
    // Clean up task handle if task was stopped
    if (nixie_display_mux_task_handle != nullptr) {
      vTaskDelete(nixie_display_mux_task_handle);
      nixie_display_mux_task_handle = nullptr;
    }
    return;
  }
  if (this->display_->isAntiPoisonActive()) {
    this->display_->antiPoison();  // Run anti-poison state machine
    for(uint8_t i = 0; i < NIXIE_TUBES; i++) {
      this->set_digit(i,this->display_->getAntiPoisonDigit(i));
    }
  }
}

// FreeRTOS task callback - runs at 500Hz (2ms period)
// Calls anti-poison state machine and display multiplexing
void NixieDisplayMux::multiplexing_loop() {
  if (this->display_) {
    this->display_->update();       // Run multiplexing
  }
}

// Start the FreeRTOS multiplexing task
// Creates a dedicated task for high-frequency display updates (500Hz)
// Returns pdPASS on success, pdFAIL on failure
BaseType_t NixieDisplayMux::start_multiplexing_task() {
  this->start_multiplexing_task_ = true;

  return xTaskCreate(
      multiplexing_task,      // Task function
      "nixie_mux_task",       // Task name
      2048,                   // Stack size (2KB)
      this,                   // Pass component pointer as parameter
      24,   // Priority: highest priority to reduce flicker
      &nixie_display_mux_task_handle  // Task handle
  );
}

// Stop the FreeRTOS multiplexing task
// Deletes the task and cleans up resources
void NixieDisplayMux::stop_multiplexing_task() {
  this->start_multiplexing_task_ = false;
  if (nixie_display_mux_task_handle != nullptr) {
    vTaskDelete(nixie_display_mux_task_handle);
    nixie_display_mux_task_handle = nullptr;
  }
}

// Set digit value for a specific tube
// tube_index is 0-indexed (0 to num_tubes_-1)
// NixieDisplay uses 1-based indexing internally, so we convert
void NixieDisplayMux::set_digit(uint8_t tube_index, uint8_t digit) {
  if (this->display_ && tube_index < this->num_tubes_) {
    uint8_t digit_value = this->display_->get(tube_index + 1);
    if (digit_value != digit) {
      this->display_->set(tube_index + 1, digit);  // NixieDisplay uses 1-based indexing
    }
  }
}

// Set digits for multiple tubes from an array
// digits is a null-terminated or count-limited array of digit values
// Each tube index (0 to count-1) receives the corresponding digit value
void NixieDisplayMux::set_digits(const uint8_t *digits, uint8_t count) {
  if (!this->display_->isAntiPoisonActive()) {
    for (uint8_t i = 0; i < count && i < this->num_tubes_; i++) {
      this->set_digit(i, digits[i]);
    }
  }
}

// Set the same digit for all tubes
// Convenience method to update all tubes simultaneously
void NixieDisplayMux::set_all_tubes(uint8_t digit) {
  for (uint8_t i = 0; i < this->num_tubes_; i++) {
    this->set_digit(i, digit);
  }
}

// Turn display on
// Initializes display and starts multiplexing
void NixieDisplayMux::display_on() {
  if (this->display_) {
    this->display_->on();
    NixieDisplayMux::start_multiplexing_task();
  }
}

// Turn display off
// Stops multiplexing and blanks all tubes
void NixieDisplayMux::display_off() {
  if (this->display_) {
    this->display_->off();
    this->start_multiplexing_task_ = false;
  }
}

// Start anti-poison routine to cycle through all digits
// Prevents tube degradation by displaying each digit position periodically
// Parameters:
//   randomize - whether to randomize digit sequence (currently uses sequential order)
//   soft_start - whether to gradually transition tubes to anti-poison mode
void NixieDisplayMux::start_anti_poison(bool randomize, bool soft_start) {
  if (this->display_) {
    ESP_LOGI(TAG, "Starting anti-poison routine (random=%d, soft_start=%d)", randomize, soft_start);
    this->display_->startAntiPoison(randomize, soft_start);
  }
}

// Check if anti-poison routine is currently active
// Returns true if anti-poison is enabled, false otherwise
bool NixieDisplayMux::is_anti_poison_active() {
  if (this->display_) {
    return this->display_->isAntiPoisonActive();
  }
  return false;
}

}  // namespace nixie_display_mux
}  // namespace esphome
