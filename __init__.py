import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import (
    CONF_DATA_RATE,
    CONF_ID,
    CONF_MODE,
    CONF_NUMBER,
)

CODEOWNERS = ["@jsarber"]
DEPENDENCIES = []
AUTO_LOAD = ["spi"]

nixie_display_mux_ns = cg.esphome_ns.namespace("nixie_display_mux")

NixieDisplayMux = nixie_display_mux_ns.class_(
    "NixieDisplayMux",
    cg.Component
)

CONF_NUM_TUBES = "num_tubes"
CONF_SR_DATA_PIN = "sr_data_pin"
CONF_SR_CLOCK_PIN = "sr_clock_pin"
CONF_SR_LATCH_PIN = "sr_latch_pin"
CONF_BLANK_PIN = "blank_pin"
CONF_ANODE_1_PIN = "anode_1_pin"
CONF_ANODE_2_PIN = "anode_2_pin"
CONF_ANODE_3_PIN = "anode_3_pin"

# SPI defaults (same as sn74hc595: MSBFIRST, CPOL=0, CPHA=0)
CONF_SPI_DATA_RATE_DEFAULT = 4000000  # 4MHz

MULTIPLEXING_FREQUENCY_HZ = 50  # 50Hz for 20ms multiplexing period

# Configuration for typed schema (GPIO vs SPI)
TYPE_GPIO = "gpio"
TYPE_SPI = "spi"

# Common configuration for both types
_COMMON_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(NixieDisplayMux),
    cv.Optional(CONF_NUM_TUBES, default=4): cv.int_range(min=2, max=6),
    cv.Optional(CONF_SR_DATA_PIN, default=27): cv.uint8_t,
    cv.Optional(CONF_SR_CLOCK_PIN, default=26): cv.uint8_t,
    cv.Optional(CONF_SR_LATCH_PIN, default=4): cv.uint8_t,
    cv.Optional(CONF_BLANK_PIN, default=2): cv.uint8_t,
    cv.Optional(CONF_ANODE_1_PIN, default=5): cv.uint8_t,
    cv.Optional(CONF_ANODE_2_PIN, default=16): cv.uint8_t,
    cv.Optional(CONF_ANODE_3_PIN, default=17): cv.uint8_t,
})

# GPIO-specific configuration
_GPIO_SCHEMA = _COMMON_SCHEMA.extend({
    cv.Required(CONF_SR_DATA_PIN): cv.uint8_t,
    cv.Required(CONF_SR_CLOCK_PIN): cv.uint8_t,
}).extend(cv.COMPONENT_SCHEMA)

# SPI-specific configuration (extends SPI device schema)
_SPI_SCHEMA = _COMMON_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(NixieDisplayMux),
}).extend(cv.COMPONENT_SCHEMA).extend(
    spi.spi_device_schema(cs_pin_required=False)
)

# Create typed schema with GPIO as default
CONFIG_SCHEMA = cv.typed_schema(
    {
        TYPE_GPIO: _GPIO_SCHEMA,
        TYPE_SPI: _SPI_SCHEMA,
    },
    default_type=TYPE_GPIO,
)


async def to_code(config):
    """Generate C++ code for NixieDisplayMux component.

    Creates the NixieDisplayMux instance, registers it as a component, and
    configures all pins (shift register, anodes, blank pin).
    Also defines USE_ESPHOME_FREERTOS_TASK for FreeRTOS task support.
    """
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # FreeRTOS is available on ESP32/ESP32-based platforms
    # Use defines to indicate FreeRTOS task usage
    cg.add_define("USE_ESPHOME_FREERTOS_TASK")

    # Configuration
    cg.add(var.set_num_tubes(config[CONF_NUM_TUBES]))
    cg.add(var.set_sr_data_pin(config[CONF_SR_DATA_PIN]))
    cg.add(var.set_sr_clock_pin(config[CONF_SR_CLOCK_PIN]))
    cg.add(var.set_sr_latch_pin(config[CONF_SR_LATCH_PIN]))
    cg.add(var.set_blank_pin(config[CONF_BLANK_PIN]))
    cg.add(var.set_anode_1_pin(config[CONF_ANODE_1_PIN]))
    cg.add(var.set_anode_2_pin(config[CONF_ANODE_2_PIN]))
    cg.add(var.set_anode_3_pin(config[CONF_ANODE_3_PIN]))

    # Handle SPI settings if using SPI mode
    if config.get("type") == TYPE_SPI:
        await spi.register_spi_device(var, config)


def nixie_display_mux_pin_schema(pin_type: str):
    """Create pin schema for pins connected to shift registers.

    Args:
        pin_type: Either "gpio" or "spi" to determine pin class

    Returns:
        Pin schema configuration
    """
    if pin_type == TYPE_SPI:
        return cv.All(
            cv.GenerateID().optional(),
            cv.use_id(spi.SPIDevice),
        )
    else:
        import esphome.pins as pins
        return pins.gpio_output_pin_schema


# Register pin schemas for shift register pins
from esphome import pins as pins_module

CONF_SR_DATA = "sr_data"
CONF_SR_CLOCK = "sr_clock"
CONF_SR_LATCH = "sr_latch"
CONF_BLANK = "blank"
CONF_ANODE_1 = "anode_1"
CONF_ANODE_2 = "anode_2"
CONF_ANODE_3 = "anode_3"

pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_SR_DATA,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_SR_CLOCK,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_SR_LATCH,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_BLANK,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_ANODE_1,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_ANODE_2,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)
pins_module.PIN_SCHEMA_REGISTRY.register(
    CONF_ANODE_3,
    nixie_display_mux_pin_schema(TYPE_GPIO),
    None,
)