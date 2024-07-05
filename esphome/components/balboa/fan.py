import esphome.codegen as cg
from esphome.components import fan
from esphome.components.balboa import balboa_ns  # , CONF_BALBOA_ID, BALBOA
import esphome.config_validation as cv

DEPENDENCIES = ["balboa"]
CONF_PUMP_ID = "pump_id"

BALBOA_PUMP = balboa_ns.class_("Pump", cg.Component, fan.Fan)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(CONF_PUMP_ID): cv.declare_id(BALBOA_PUMP),
            # cv.GenerateID(CONF_BALBOA_ID): cv.use_id(BALBOA),
        }
    ).extend(fan.FAN_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_PUMP_ID])
    await cg.register_component(var, config)
    await fan.register_fan(var, config)

    # output_ = await cg.get_variable(config[CONF_OUTPUT])
    # cg.add(var.set_output(output_))

    # if CONF_OSCILLATION_OUTPUT in config:
    #     oscillation_output = await cg.get_variable(config[CONF_OSCILLATION_OUTPUT])
    #     cg.add(var.set_oscillating(oscillation_output))

    # if CONF_DIRECTION_OUTPUT in config:
    #     direction_output = await cg.get_variable(config[CONF_DIRECTION_OUTPUT])
    #     cg.add(var.set_direction(direction_output))

    # if CONF_PRESET_MODES in config:
    #     cg.add(var.set_preset_modes(config[CONF_PRESET_MODES]))
