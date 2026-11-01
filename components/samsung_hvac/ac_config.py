"""Device-scoped, passive AC entity schemas and code generation."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, climate, sensor, text_sensor
from esphome.const import CONF_ID

ns = cg.esphome_ns.namespace("samsung_hvac").namespace("profiles")
AcDevice = ns.class_("AcDevice")
AcClimate = ns.class_("AcClimate", climate.Climate)
Family = ns.enum("Family", is_class=True)
Field = ns.enum("Field", is_class=True)
FAMILIES = {"nasa_ac": Family.NASA_AC, "non_nasa_ac": Family.NON_NASA_AC}
FANS = {"auto": 0, "low": 1, "medium": 2, "high": 3, "turbo": 4, "fresh": 5}
PRESETS = {
    "none": 0,
    "sleep": 1,
    "quiet": 2,
    "fast": 3,
    "long_reach": 6,
    "eco": 7,
    "windfree": 9,
}
NUMERIC = {
    "target_temperature": Field.TARGET,
    "room_temperature": Field.ROOM,
    "indoor_eva_in_temperature": Field.PIPE_IN,
    "indoor_eva_out_temperature": Field.PIPE_OUT,
    "outdoor_temperature": Field.OUTDOOR,
    "error_code": Field.ERROR_CODE,
    "humidity": Field.HUMIDITY,
}
BINARY = {
    "power_state": Field.POWER,
    "filter_warning": Field.FILTER_WARNING,
    "defrosting": Field.DEFROSTING,
    "silence_state": Field.SILENCE,
    "automatic_cleaning_state": Field.CLEANING,
    "vertical_swing_state": Field.VERTICAL_SWING,
    "horizontal_swing_state": Field.HORIZONTAL_SWING,
}
TEXT = {
    "operation_mode": Field.MODE,
    "fan_mode": Field.FAN,
    "preset_state": Field.PRESET,
}


def _capabilities(config):
    nasa = config["profile"] == "nasa_ac"
    if nasa:
        if config["address"] >> 16 != 0x20:
            raise cv.Invalid("NASA AC requires a 24-bit indoor address in 0x20xxxx")
        if "room_temperature" in config:
            raise cv.Invalid(
                "NASA room_temperature is unavailable: message meaning is unresolved"
            )
        if "fresh" in config["fan_modes"]:
            raise cv.Invalid("fresh fan mode is unsupported for NASA AC")
    else:
        if config["address"] > 0x7F:
            raise cv.Invalid(
                "Non-NASA AC requires an indoor address from 0x00 through 0x7F"
            )
        for key in (
            "outdoor_address",
            "outdoor_temperature",
            "error_code",
            "humidity",
            "presets",
            "preset_state",
            "filter_warning",
            "defrosting",
            "silence_state",
            "automatic_cleaning_state",
        ):
            if key in config:
                raise cv.Invalid(f"{key} is unsupported for non_nasa_ac")
    if "outdoor_address" in config and config["outdoor_address"] >> 16 != 0x10:
        raise cv.Invalid("NASA outdoor_address requires an outdoor address in 0x10xxxx")
    for key in ("outdoor_temperature", "error_code"):
        if key in config and "outdoor_address" not in config:
            raise cv.Invalid(f"{key} requires an explicit outdoor_address")
    if "fan_mode" in config and not config["fan_modes"]:
        raise cv.Invalid("fan_mode requires explicit fan_modes capabilities")
    if "preset_state" in config and not config.get("presets"):
        raise cv.Invalid("preset_state requires explicit presets capabilities")
    for direction in ("vertical", "horizontal"):
        if f"{direction}_swing_state" in config and not config[f"{direction}_swing"]:
            raise cv.Invalid(
                f"{direction}_swing_state requires {direction}_swing: true"
            )
    return config


def _internal_climate(config):
    if not config["internal"]:
        raise cv.Invalid(
            "Passive climate must be internal: true; public controls are unavailable"
        )
    return config


CLIMATE_SCHEMA = cv.All(
    climate.climate_schema(AcClimate).extend(
        {cv.Optional("internal", default=True): cv.boolean}
    ),
    _internal_climate,
)
DEVICE_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(AcDevice),
            cv.Required("profile"): cv.enum(FAMILIES, lower=True),
            cv.Required("address"): cv.int_range(min=0, max=0xFFFFFF),
            cv.Optional("outdoor_address"): cv.int_range(min=0, max=0xFFFFFF),
            cv.Optional("fan_modes", default=[]): cv.All(
                cv.ensure_list(cv.one_of(*FANS, lower=True)), cv.Unique()
            ),
            cv.Optional("presets"): cv.All(
                cv.ensure_list(cv.one_of(*PRESETS, lower=True)), cv.Unique()
            ),
            cv.Optional("vertical_swing", default=False): cv.boolean,
            cv.Optional("horizontal_swing", default=False): cv.boolean,
            cv.Optional("climate"): CLIMATE_SCHEMA,
            **{
                cv.Optional(key): sensor.sensor_schema(
                    unit_of_measurement="°C",
                    accuracy_decimals=1,
                    device_class="temperature",
                    state_class="measurement",
                )
                if "temperature" in key
                else sensor.sensor_schema(
                    unit_of_measurement="%",
                    accuracy_decimals=0,
                    device_class="humidity",
                    state_class="measurement",
                )
                if key == "humidity"
                else sensor.sensor_schema(accuracy_decimals=0)
                for key in NUMERIC
            },
            **{
                cv.Optional(key): binary_sensor.binary_sensor_schema() for key in BINARY
            },
            **{cv.Optional(key): text_sensor.text_sensor_schema() for key in TEXT},
        }
    ),
    _capabilities,
)


def validate_devices(config):
    expected = "nasa_ac" if config["protocol"] == "nasa" else "non_nasa_ac"
    seen = set()
    for device in config["devices"]:
        if device["profile"] != expected:
            raise cv.Invalid("Device profile must match the bus protocol")
        if device["address"] in seen:
            raise cv.Invalid("Duplicate device address on the same bus")
        seen.add(device["address"])
    return config


async def to_code(parent, config):
    var = cg.new_Pvariable(config[CONF_ID], config["profile"], config["address"])
    cg.add(parent.add_profile(var))
    if "outdoor_address" in config:
        cg.add(var.set_outdoor_address(config["outdoor_address"]))
    cg.add(var.set_fans(sum(1 << FANS[value] for value in config["fan_modes"])))
    cg.add(
        var.set_presets(sum(1 << PRESETS[value] for value in config.get("presets", [])))
    )
    cg.add(var.set_swing(config["vertical_swing"], config["horizontal_swing"]))
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
    if "climate" in config:
        entity = await climate.new_climate(config["climate"])
        cg.add(entity.set_room_supported(config["profile"] == "non_nasa_ac"))
        cg.add(var.set_climate(entity))
