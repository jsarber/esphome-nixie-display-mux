#ifndef NixieShiftRegister_h
#define NixieShiftRegister_h
#include "Arduino.h"

/**
 * @brief Manages output to multiple shift registers for nixie tube cathode control
 *
 * Uses a fixed-size stack-based array to avoid heap fragmentation on embedded systems.
 * Supports up to 4 cascaded shift registers (32 bits total).
 */
class NixieShiftRegister {
 public:
  /**
   * @brief Constructor without blank pin support
   * @param shiftRegisterCount Number of cascaded shift registers (max 4)
   * @param dataPin Pin connected to shift register data output
   * @param clockPin Pin connected to shift register clock
   * @param latchPin Pin connected to shift register latch/strobe
   */
  NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);

  /**
   * @brief Constructor with blank pin support
   * @param shiftRegisterCount Number of cascaded shift registers (max 4)
   * @param dataPin Pin connected to shift register data output
   * @param clockPin Pin connected to shift register clock
   * @param latchPin Pin connected to shift register latch/strobe
   * @param blankPin Pin connected to shift register blank (optional)
   */
  NixieShiftRegister(uint8_t shiftRegisterCount, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin, uint8_t blankPin);

  /**
   * @brief Set the data bit at the specified position
   * @param pin Bit position index (0-31)
   * @param dataIn Byte value (0-255) to store at that position
   */
  void set(uint8_t pin, byte dataIn);

  /**
   * @brief Transfer all stored data to the physical shift registers via SPI-like serial output
   *
   * Operation:
   * 1. Drives latch pin LOW
   * 2. Sends data in 8-bit chunks using shiftOut() with MSBFIRST ordering
   * 3. Sends shiftRegisterCount bytes total
   * 4. Drives latch pin HIGH to capture data
   */
  void update();

 private:
  uint8_t shift_register_count_;
  uint8_t data_pin_;
  uint8_t clock_pin_;
  uint8_t latch_pin_;
  // Fixed-size array to avoid heap allocation - max 4 shift registers * 8 bits
  bool data_[32];
};

#endif  // NixieShiftRegister_h