import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir
from esphome.const import CONF_ID, CONF_UNIT_OF_MEASUREMENT

AUTO_LOAD = ["climate_ir"]
CODEOWNERS = ["@glmnet"]

frigidaire_ns = cg.esphome_ns.namespace("frigidaire")
FrigidaireClimate = frigidaire_ns.class_("FrigidaireClimate", climate_ir.ClimateIR)

Unit = frigidaire_ns.enum("Unit")
UNITS = {
    "C": Unit.UNIT_C,
    "°C": Unit.UNIT_C,
    "° C": Unit.UNIT_C,
    "F": Unit.UNIT_F,
    "°F": Unit.UNIT_F,
    "° F": Unit.UNIT_F,
}

CONFIG_SCHEMA = climate_ir.CLIMATE_IR_WITH_RECEIVER_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(FrigidaireClimate),
        cv.Optional(CONF_UNIT_OF_MEASUREMENT, default="C"): cv.enum(UNITS, upper=True),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await climate_ir.register_climate_ir(var, config)
    cg.add(var.set_unit_of_measurement(config[CONF_UNIT_OF_MEASUREMENT]))
