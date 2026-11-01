"""Evidence-labelled passive EHS observations; no writable entity schema."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor, text_sensor
from esphome.const import CONF_ID

ns = cg.esphome_ns.namespace("samsung_hvac").namespace("profiles").namespace("ehs")
Device = ns.class_("Device")
Field = ns.enum("Field", is_class=True)
NUMERIC = {
    "dhw_temperature": Field.DHW_CURRENT,
    "dhw_target_temperature": Field.DHW_TARGET,
    "water_outlet_temperature": Field.OUTLET_CURRENT,
    "water_outlet_target_temperature": Field.OUTLET_TARGET,
    "water_return_temperature": Field.RETURN,
    "water_law_shift": Field.WATER_LAW_SHIFT,
    "water_law_target_temperature": Field.WATER_LAW_TARGET,
    "water_flow": Field.FLOW,
    "outdoor_temperature": Field.OUTDOOR,
    "error_code": Field.ERROR_CODE,
}
BINARY = {
    "power_state": Field.POWER,
    "dhw_power_state": Field.DHW_POWER,
    "water_pump_state": Field.PUMP,
    "defrosting": Field.DEFROST,
}
TEXT = {"operation_mode": Field.MODE, "dhw_mode": Field.DHW_MODE}
ZONE_SCHEMA = cv.Schema(
    {
        cv.Required("number"): cv.int_range(min=1, max=2),
        cv.Optional("power_state"): binary_sensor.binary_sensor_schema(),
    }
)


def _validate(config):
    if config["address"] >> 16 != 0x20:
        raise cv.Invalid("EHS requires an explicit indoor/hydro address in 0x20xxxx")
    if "outdoor_address" in config and config["outdoor_address"] >> 16 != 0x10:
        raise cv.Invalid("EHS outdoor_address requires an outdoor address in 0x10xxxx")
    for key in ("outdoor_temperature", "error_code"):
        if key in config and "outdoor_address" not in config:
            raise cv.Invalid(f"{key} requires outdoor_address")
    numbers = [zone["number"] for zone in config["zones"]]
    if len(numbers) != len(set(numbers)):
        raise cv.Invalid("Duplicate zone number on an EHS device")
    return config


def _numeric_schema(key):
    if key == "error_code":
        return sensor.sensor_schema(accuracy_decimals=0)
    if key == "water_flow":
        return sensor.sensor_schema(
            unit_of_measurement="L/min",
            accuracy_decimals=1,
            device_class="volume_flow_rate",
            state_class="measurement",
        )
    return sensor.sensor_schema(
        unit_of_measurement="°C",
        accuracy_decimals=1,
        device_class="temperature",
        state_class="measurement",
    )


DEVICE_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(Device),
            cv.Required("profile"): cv.one_of("ehs", lower=True),
            cv.Required("address"): cv.int_range(min=0, max=0xFFFFFF),
            cv.Optional("outdoor_address"): cv.int_range(min=0, max=0xFFFFFF),
            cv.Optional("zones", default=[]): cv.All(
                cv.ensure_list(ZONE_SCHEMA), cv.Length(max=2)
            ),
            **{cv.Optional(key): _numeric_schema(key) for key in NUMERIC},
            **{
                cv.Optional(key): binary_sensor.binary_sensor_schema() for key in BINARY
            },
            **{cv.Optional(key): text_sensor.text_sensor_schema() for key in TEXT},
        }
    ),
    _validate,
)


async def to_code(parent, config):
    var = cg.new_Pvariable(config[CONF_ID], config["address"])
    cg.add(parent.add_profile(var))
    if "outdoor_address" in config:
        cg.add(var.set_outdoor_address(config["outdoor_address"]))
    zones = {zone["number"]: zone for zone in config["zones"]}
    cg.add(var.set_zones(1 in zones, 2 in zones))
    for key, field in NUMERIC.items():
        if key in config:
            entity = await sensor.new_sensor(config[key])
            cg.add(var.set_sensor(field, entity))
    for key, field in BINARY.items():
        if key in config:
            entity = await binary_sensor.new_binary_sensor(config[key])
            cg.add(var.set_binary_sensor(field, entity))
    for key, field in TEXT.items():
        if key in config:
            entity = await text_sensor.new_text_sensor(config[key])
            cg.add(var.set_text_sensor(field, entity))
    for number, zone in zones.items():
        if "power_state" in zone:
            entity = await binary_sensor.new_binary_sensor(zone["power_state"])
            cg.add(
                var.set_binary_sensor(
                    Field.ZONE1_POWER if number == 1 else Field.ZONE2_POWER, entity
                )
            )
