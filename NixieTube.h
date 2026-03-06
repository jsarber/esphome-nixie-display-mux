#ifndef NixieTube_h
#define NixieTube_h
#include "Arduino.h"

/**
 * @brief Represents a single nixie tube with display index, digit value, and lit state
 *
 * Used by NixieDisplay to manage up to 6 tubes in a nixie display.
 * Each tube stores:
 * - nixieIDX: Display index (1-6)
 * - nixieValue: Current digit to display (0-9)
 * - nixieLit: Whether the tube is currently lit/active
 */
class NixieTube {
  public:
    uint8_t nixieIDX;
    uint8_t nixieValue;
    bool nixieLit;

    /**
     * @brief Default constructor - initializes all values to zero/false
     */
    NixieTube() : nixieIDX(0), nixieValue(0), nixieLit(false) {}

    /**
     * @brief Constructor with explicit parameters
     * @param t_nixieIDX Tube display index (1-6)
     * @param t_nixieValue Digit value to display (0-9)
     * @param t_nixieLit Whether the tube is currently lit/active
     */
    NixieTube(uint8_t t_nixieIDX, uint8_t t_nixieValue, bool t_nixieLit);

    /**
     * @brief Returns the tube's display index
     * @return uint8_t - The nixie tube index (1-6)
     */
    uint8_t get_idx();

    /**
     * @brief Returns the current digit value displayed on this tube
     * @return uint8_t - The digit value (0-9)
     */
    uint8_t get_value();

    /**
     * @brief Sets the digit value to display on this tube
     * @param t_nixieValue Digit to display (0-9)
     */
    void set_value(uint8_t t_nixieValue);

    /**
     * @brief Returns whether the tube is currently lit/active
     * @return bool - true if tube is lit, false otherwise
     */
    bool nixie_lit();

    /**
     * @brief Toggles the lit state between true and false
     *
     * Behavior:
     * - If nixieLit is true, sets it to false
     * - If nixieLit is false, sets it to true
     */
    void toggle_nixie_lit();
};

#endif