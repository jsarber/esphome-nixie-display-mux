#include "NixieTube.h"

/**
 * @brief Constructor for NixieTube with explicit parameters
 * @param t_nixieIDX Tube display index (1-6)
 * @param t_nixieValue Digit value to display (0-9)
 * @param t_nixieLit Whether the tube is currently lit/active
 */
NixieTube::NixieTube(uint8_t t_nixieIDX, uint8_t t_nixieValue, bool t_nixieLit) : nixieIDX(t_nixieIDX), nixieValue(t_nixieValue), nixieLit{t_nixieLit} {}

/**
 * @brief Returns the tube's display index
 * @return uint8_t - The nixie tube index (1-6)
 */
uint8_t NixieTube::get_idx() {
    return nixieIDX;
}

/**
 * @brief Returns the current digit value displayed on this tube
 * @return uint8_t - The digit value (0-9)
 */
uint8_t NixieTube::get_value() {
    return nixieValue;
}

/**
 * @brief Sets the digit value to display on this tube
 * @param t_nixieValue Digit to display (0-9)
 */
void NixieTube::set_value(uint8_t t_nixieValue) {
    nixieValue = t_nixieValue;
}

/**
 * @brief Returns whether the tube is currently lit/active
 * @return bool - true if tube is lit, false otherwise
 */
bool NixieTube::nixie_lit() {
    return nixieLit;
}

/**
 * @brief Toggles the lit state between true and false
 *
 * Behavior:
 * - If nixieLit is true, sets it to false
 * - If nixieLit is false, sets it to true
 */
void NixieTube::toggle_nixie_lit() {
    if(nixieLit) {
    nixieLit = false;
    } else {
    nixieLit = true;
    }
}
// }; 