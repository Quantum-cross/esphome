import esphome.codegen as cg
from esphome.components import uart

import esphome.config_validation as cv
from esphome.const import CONF_ID

CONF_BALBOA_ID = "balboa_id"

balboa_ns = cg.esphome_ns.namespace("balboa")
BALBOA = balboa_ns.class_("Balboa", cg.Component, uart.UARTDevice)

DEPENDENCIES = ["uart"]

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BALBOA),
        }
    ).extend(uart.UART_DEVICE_SCHEMA),
)
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "balboa",
    require_rx=True,
    require_tx=True,
    baud_rate=115200,
    data_bits=8,
    parity="NONE",
    stop_bits=1,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
