"""Independent Samsung HVAC component foundation; no active bus traffic."""

from esphome import pins
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_FLOW_CONTROL_PIN,
    CONF_ID,
    CONF_NUMBER,
    CONF_PROTOCOL,
    CONF_RX_PIN,
    CONF_TX_PIN,
    CONF_UART_ID,
)
import esphome.final_validate as fv

CODEOWNERS = ["@omerfaruk-aran"]
DEPENDENCIES = ["uart"]
MULTI_CONF = True

ns = cg.esphome_ns.namespace("samsung_hvac")
SamsungHVAC = ns.class_("SamsungHVAC", cg.Component, uart.UARTDevice)
Protocol = ns.enum("Protocol", is_class=True)
PROTOCOLS = {"nasa": Protocol.NASA, "non_nasa": Protocol.NON_NASA}

CONFIG_SCHEMA = cv.All(
    cv.only_on_esp32,
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SamsungHVAC),
            cv.Required(CONF_PROTOCOL): cv.enum(PROTOCOLS, lower=True),
            cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.internal_gpio_output_pin_schema,
        }
    ).extend(cv.COMPONENT_SCHEMA),
)


def _mappings(value):
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from _mappings(child)
    elif isinstance(value, list):
        for child in value:
            yield from _mappings(child)


def _exclusive_uart(config):
    """Reject competing declarative consumers, including legacy and raw actions."""
    full = fv.full_config.get()
    bus = config[CONF_UART_ID]
    for item in _mappings(full):
        if item.get(CONF_UART_ID) == bus and item.get(CONF_ID) != config[CONF_ID]:
            raise cv.Invalid(
                "samsung_hvac requires exclusive ownership of its UART bus"
            )
        action = item.get("uart.write")
        if isinstance(action, dict) and action.get(CONF_ID) == bus:
            raise cv.Invalid("uart.write cannot bypass the samsung_hvac UART owner")
    path = full.get_path_for_id(bus)[:-1]
    hub = full.get_config_for_path(path)
    if not hub[CONF_ID].type.inherits_from(uart.IDFUARTComponent):
        raise cv.Invalid("samsung_hvac requires a native ESP32 UART")
    if CONF_FLOW_CONTROL_PIN in hub:
        raise cv.Invalid(
            "Configure flow_control_pin under samsung_hvac instead of uart"
        )
    owned_pins = {
        hub[key][CONF_NUMBER] for key in (CONF_RX_PIN, CONF_TX_PIN) if key in hub
    }
    for other in full.get("uart", []):
        if other[CONF_ID] == hub[CONF_ID]:
            continue
        other_pins = {
            other[key][CONF_NUMBER]
            for key in (CONF_RX_PIN, CONF_TX_PIN)
            if key in other
        }
        if owned_pins & other_pins:
            raise cv.Invalid(
                "samsung_hvac UART pins must not be shared by another UART bus"
            )
    uart_pins = {
        entry[key][CONF_NUMBER]
        for entry in full.get("uart", [])
        for key in (CONF_RX_PIN, CONF_TX_PIN)
        if key in entry
    }
    direction_pins = set()
    for owner in full["samsung_hvac"]:
        if CONF_FLOW_CONTROL_PIN not in owner:
            continue
        number = owner[CONF_FLOW_CONTROL_PIN][CONF_NUMBER]
        if number in uart_pins or number in direction_pins:
            raise cv.Invalid(
                "samsung_hvac direction pin must be exclusive of UART and other direction pins"
            )
        direction_pins.add(number)
    if hub.get("debug", {}).get("dummy_receiver", False):
        raise cv.Invalid("UART debug dummy_receiver must be disabled for samsung_hvac")
    return config


FINAL_VALIDATE_SCHEMA = cv.All(
    _exclusive_uart,
    uart.final_validate_device_schema(
        "samsung_hvac",
        baud_rate=9600,
        require_rx=True,
        require_tx=True,
        data_bits=8,
        parity="EVEN",
        stop_bits=1,
    ),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_protocol(config[CONF_PROTOCOL]))
    if CONF_FLOW_CONTROL_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_FLOW_CONTROL_PIN])
        cg.add(var.set_flow_control_pin(pin))
