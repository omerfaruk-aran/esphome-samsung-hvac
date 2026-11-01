"""Bounded, passive NASA scalar observers and explicit custom descriptors."""

import math
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, binary_sensor
from esphome.const import CONF_ID
from .catalog import load_catalog, validate_catalog, NASA_TYPES

CATALOG = load_catalog()
ns = cg.esphome_ns.namespace("samsung_hvac").namespace("advanced")
MessageEntity = ns.class_("MessageEntity")
AMBIGUOUS = {0x4203, 0x4204, 0x42D6}


def finite(value):
    value = cv.float_(value)
    if not math.isfinite(value):
        raise cv.Invalid("Descriptor conversion must be finite")
    return value


RANGE_SCHEMA = cv.Schema(
    {cv.Required("minimum"): cv.uint32_t, cv.Required("maximum"): cv.uint32_t}
)
CUSTOM_SCHEMA = cv.Schema(
    {
        cv.Required("code"): cv.uint16_t,
        cv.Required("value_type"): cv.one_of(
            "enum", "variable", "long_variable", lower=True
        ),
        cv.Required("signed"): cv.boolean,
        cv.Required("unit"): cv.string_strict,
        cv.Required("scale"): finite,
        cv.Required("offset"): finite,
        cv.Required("source_role"): cv.one_of("indoor", "outdoor", lower=True),
        cv.Required("source_url"): cv.string_strict,
        cv.Optional("sentinels", default=[]): cv.All(
            cv.ensure_list(cv.uint32_t), cv.Length(max=4), cv.Unique()
        ),
        cv.Optional("valid_values"): cv.All(
            cv.ensure_list(cv.uint32_t), cv.Length(min=1, max=16), cv.Unique()
        ),
        cv.Optional("raw_range"): RANGE_SCHEMA,
    }
)


def selected(config):
    if "catalog" in config:
        name = config["catalog"]
        if name not in CATALOG:
            raise cv.Invalid("Unknown catalog descriptor")
        result = CATALOG[name]
        if result["access"] != "passive":
            raise cv.Invalid("Named descriptor is unresolved and disabled")
        if result["protocol"] != "nasa":
            raise cv.Invalid("Generic observers support NASA scalars only")
        return result
    custom = config["custom"]
    entry = dict(CATALOG["nasa_target"])
    entry.update(
        id="custom",
        code=int(custom["code"]),
        value_type=custom["value_type"],
        payload_length=NASA_TYPES[custom["value_type"]],
        source_role=custom["source_role"],
        signedness="signed" if custom["signed"] else "unsigned",
        unit=custom["unit"],
        scale=float(custom["scale"]),
        offset=float(custom["offset"]),
        sentinels=[int(value) for value in custom["sentinels"]],
        sentinel_status="specified" if custom["sentinels"] else "unknown",
        valid_values=[int(value) for value in custom["valid_values"]]
        if "valid_values" in custom
        else None,
        raw_range={key: int(value) for key, value in custom["raw_range"].items()}
        if "raw_range" in custom
        else None,
        access="disabled",
        evidence_level="unknown",
        source_url=[custom["source_url"]],
        device_scope=["custom"],
        test_reference=["user_configuration"],
        notes="Explicit custom observation. Meaning and device support remain UNKNOWN.",
    )
    try:
        result = validate_catalog({"version": 1, "messages": [entry]})["custom"]
    except ValueError as error:
        raise cv.Invalid(str(error)) from error
    if result["code"] in AMBIGUOUS and (
        result["unit"]
        or result["scale"] != 1
        or result["offset"] != 0
        or result["signedness"] != "unsigned"
    ):
        raise cv.Invalid(
            "Ambiguous IDs permit only raw unsigned unitless scale1/offset0 observations"
        )
    return result


def _validate(config):
    descriptor = selected(config)
    role = 0x20 if descriptor["source_role"] == "indoor" else 0x10
    if config["source_address"] >> 16 != role:
        raise cv.Invalid("Observer source role does not match source_address")
    boolean = descriptor["unit"] == "boolean"
    if boolean and "binary_sensor" not in config:
        raise cv.Invalid("Boolean descriptor requires binary_sensor")
    if not boolean and "sensor" not in config:
        raise cv.Invalid("Numeric descriptor requires sensor")
    if not boolean:
        entity = config["sensor"]
        if entity.get("unit_of_measurement", descriptor["unit"]) != descriptor["unit"]:
            raise cv.Invalid("Entity unit must match the descriptor unit")
        if descriptor["unit"]:
            entity["unit_of_measurement"] = descriptor["unit"]
    return config


SCHEMA = cv.All(
    cv.Schema(
        {
            cv.Required(CONF_ID): cv.declare_id(MessageEntity),
            cv.Required("source_address"): cv.int_range(min=0, max=0xFFFFFF),
            cv.Optional("catalog"): cv.string_strict,
            cv.Optional("custom"): CUSTOM_SCHEMA,
            cv.Optional("sensor"): sensor.sensor_schema(accuracy_decimals=1),
            cv.Optional("binary_sensor"): binary_sensor.binary_sensor_schema(),
        }
    ),
    cv.has_exactly_one_key("catalog", "custom"),
    cv.has_exactly_one_key("sensor", "binary_sensor"),
    _validate,
)


async def to_code(parent, config):
    descriptor = selected(config)
    var = cg.new_Pvariable(
        config[CONF_ID],
        config["source_address"],
        descriptor["code"],
        descriptor["signedness"] == "signed",
        cg.RawExpression(repr(float(descriptor["scale"]))),
        cg.RawExpression(repr(float(descriptor["offset"]))),
    )
    cg.add(parent.add_message(var))
    for index, value in enumerate(descriptor["sentinels"]):
        cg.add(var.set_sentinel(index, value))
    for index, value in enumerate(descriptor["valid_values"] or []):
        cg.add(var.set_valid_value(index, value))
    if descriptor["raw_range"]:
        cg.add(
            var.set_raw_range(
                descriptor["raw_range"]["minimum"], descriptor["raw_range"]["maximum"]
            )
        )
    if "sensor" in config:
        entity = await sensor.new_sensor(config["sensor"])
        cg.add(var.set_sensor(entity))
    else:
        entity = await binary_sensor.new_binary_sensor(config["binary_sensor"])
        cg.add(var.set_binary_sensor(entity))
