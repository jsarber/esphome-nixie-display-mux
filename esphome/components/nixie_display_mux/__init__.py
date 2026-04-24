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
CONF_SR_OFFSET = "sr_offset"
CONF_TUBE_TYPE = "tube_type"
CONF_POWER_BOARD = "power_board"

# SPI defaults (same as sn74hc595: MSBFIRST, CPOL=0, CPHA=0)
CONF_SPI_DATA_RATE_DEFAULT = 4000000  # 4MHz

MULTIPLEXING_FREQUENCY_HZ = 50  # 50Hz for 20ms multiplexing period

# GPIO-specific configuration
TYPE_GPIO = "gpio"
TYPE_SPI = "spi"

# Tube type presets: timing constants and neon settings
TUBE_PRESETS = {
    "in12a": {
        CONF_NUM_TUBES: 6,
        "blank_delay": 500,
        "on_delay": 3500,
        "neon_enabled": True,
    },
    "z5660m": {
        CONF_NUM_TUBES: 4,
        "blank_delay": 500,
        "on_delay": 3000,
        "neon_enabled": True,
    },
    "z573m": {
        CONF_NUM_TUBES: 4,
        "blank_delay": 1000,
        "on_delay": 1000,
        "neon_enabled": False,
    },
}

# Power board presets: GPIO pin assignments
POWER_BOARD_PRESETS = {
    "v30": {
        CONF_SR_DATA_PIN: 12,
        CONF_SR_CLOCK_PIN: 13,
        CONF_SR_LATCH_PIN: 14,
        CONF_BLANK_PIN: 21,
        CONF_ANODE_1_PIN: 15,
        CONF_ANODE_2_PIN: 16,
        CONF_ANODE_3_PIN: 17,
        "sr_offset": 15,
    },
    "v31": {
        CONF_SR_DATA_PIN: 27,
        CONF_SR_CLOCK_PIN: 26,
        CONF_SR_LATCH_PIN: 22,
        CONF_BLANK_PIN: 21,
        CONF_ANODE_1_PIN: 5,
        CONF_ANODE_2_PIN: 16,
        CONF_ANODE_3_PIN: 17,
        "sr_offset": 16,
    },
    "v32": {
        CONF_SR_DATA_PIN: 27,
        CONF_SR_CLOCK_PIN: 26,
        CONF_SR_LATCH_PIN: 4,
        CONF_BLANK_PIN: 2,
        CONF_ANODE_1_PIN: 5,
        CONF_ANODE_2_PIN: 16,
        CONF_ANODE_3_PIN: 17,
        "sr_offset": 16,
    },
}

# Common configuration for both types - pins have no fixed defaults (resolved from presets)
_COMMON_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(NixieDisplayMux),
    cv.Optional(CONF_TUBE_TYPE, default="z573m"): cv.one_of(*TUBE_PRESETS.keys(), lower=True),
    cv.Optional(CONF_POWER_BOARD, default="v30"): cv.one_of(*POWER_BOARD_PRESETS.keys(), lower=True),
    cv.Optional(CONF_NUM_TUBES): cv.int_range(min=2, max=6),
    cv.Optional(CONF_SR_DATA_PIN): cv.uint8_t,
    cv.Optional(CONF_SR_CLOCK_PIN): cv.uint8_t,
    cv.Optional(CONF_SR_LATCH_PIN): cv.uint8_t,
    cv.Optional(CONF_BLANK_PIN): cv.uint8_t,
    cv.Optional(CONF_ANODE_1_PIN): cv.uint8_t,
    cv.Optional(CONF_ANODE_2_PIN): cv.uint8_t,
    cv.Optional(CONF_ANODE_3_PIN): cv.uint8_t,
})

# GPIO-specific configuration
_GPIO_SCHEMA = _COMMON_SCHEMA.extend(cv.COMPONENT_SCHEMA)

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

    Resolves pin defaults from tube_type and power_board presets, merges with
    YAML overrides, and generates compile-time defines for timing and neon settings.
    """
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # FreeRTOS is available on ESP32/ESP32-based platforms
    cg.add_define("USE_ESPHOME_FREERTOS_TASK")

    # Resolve pin defaults from power board preset, merge with YAML overrides
    pb = POWER_BOARD_PRESETS[config[CONF_POWER_BOARD]]
    tube = TUBE_PRESETS[config[CONF_TUBE_TYPE]]

    num_tubes = config.get(CONF_NUM_TUBES, tube[CONF_NUM_TUBES])
    sr_data_pin = config.get(CONF_SR_DATA_PIN, pb[CONF_SR_DATA_PIN])
    sr_clock_pin = config.get(CONF_SR_CLOCK_PIN, pb[CONF_SR_CLOCK_PIN])
    sr_latch_pin = config.get(CONF_SR_LATCH_PIN, pb[CONF_SR_LATCH_PIN])
    blank_pin = config.get(CONF_BLANK_PIN, pb[CONF_BLANK_PIN])
    anode_1_pin = config.get(CONF_ANODE_1_PIN, pb[CONF_ANODE_1_PIN])
    anode_2_pin = config.get(CONF_ANODE_2_PIN, pb[CONF_ANODE_2_PIN])
    anode_3_pin = config.get(CONF_ANODE_3_PIN, pb[CONF_ANODE_3_PIN])

    # Apply configuration via setter methods
    cg.add(var.set_num_tubes(num_tubes))
    cg.add(var.set_sr_data_pin(sr_data_pin))
    cg.add(var.set_sr_clock_pin(sr_clock_pin))
    cg.add(var.set_sr_latch_pin(sr_latch_pin))
    cg.add(var.set_blank_pin(blank_pin))
    cg.add(var.set_anode_1_pin(anode_1_pin))
    cg.add(var.set_anode_2_pin(anode_2_pin))
    cg.add(var.set_anode_3_pin(anode_3_pin))

    # Generate compile-time defines for tube type selection
    tube_define = {
        "z5660m": "TUBE_Z5660M",
        "in12a": "TUBE_IN12A_6",
        "z573m": None,  # default, no define needed
    }
    tube_define_name = tube_define.get(config[CONF_TUBE_TYPE])
    if tube_define_name:
        cg.add_define(tube_define_name)

    # Generate compile-time define for power board version (controls SR_OFFSET)
    power_board_define = f"POWER_BOARD_{config[CONF_POWER_BOARD].upper()}"
    cg.add_define(power_board_define)

    # Generate compile-time defines for timing and neon (override tube header values)
    cg.add_define("NIXIE_BLANK_DELAY", tube["blank_delay"])
    cg.add_define("NIXIE_ON_DELAY", tube["on_delay"])
    if tube["neon_enabled"]:
        cg.add_define("NEONS_ENABLED")

    # Handle SPI settings if using SPI mode
    if config.get("type") == TYPE_SPI:
        await spi.register_spi_device(var, config)


def nixie_display_mux_pin_schema(pin_type: str):
    """Create pin schema for pins connected to shift registers.

    Args:
        pin_type: Either "gpio" or "spi" to determine pin class
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
