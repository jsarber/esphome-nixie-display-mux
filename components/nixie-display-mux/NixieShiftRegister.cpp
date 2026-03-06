#include "NixieShiftRegister.h"

/**
 * @brief GPIO-based setup
 */
NixieShiftRegister::NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin) {
  // set attributes
  this->shift_register_count_ = shiftRegisterCount;
  this->data_pin_ = dataPin;
  this->clock_pin_ = clockPin;
  this->latch_pin_ = latchPin;

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);

  // init data - use stack allocation to avoid heap fragmentation
  // Initialize all 32 bits to 0 to prevent garbage values in shift register
  for (int i = 0; i < 32; i++) {
    this->data_[i] = 0;
  }
};

void NixieShiftRegister::set(uint8_t pin, byte dataIn) {
  data_[pin] = dataIn;
}
  
void NixieShiftRegister::update() {
  byte result = 0;
  
  digitalWrite(latch_pin_, LOW);
  for(int i = shift_register_count_ * 8 - 1; i >= 0 ; i--) {
    result |= data_[i] << i % 8;
    if(i % 8 == 0) {
      shiftOut(data_pin_, clock_pin_, MSBFIRST, result);
      result = 0;
    }
  }
  digitalWrite(latch_pin_, HIGH);
}