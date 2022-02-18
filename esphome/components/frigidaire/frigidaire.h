#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome {
namespace frigidaire {

enum Unit {
  UNIT_C = 0,
  UNIT_F = 1
};

// Temperature
const float FRIGIDAIRE_TEMP_MIN_C = 16.0;
const float FRIGIDAIRE_TEMP_MAX_C = 32.0;
const float FRIGIDAIRE_TEMP_MIN_F = 60.0;
const float FRIGIDAIRE_TEMP_MAX_F = 90.0;

const size_t FRIGIDAIRE_STATE_LENGTH = 16;

class FrigidaireClimate : public climate_ir::ClimateIR {
 public:
  FrigidaireClimate()
      : climate_ir::ClimateIR(temperature_min_C_(), temperature_max_C_(), 1.0f, false, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH}) {}

  void setup() override {
    climate_ir::ClimateIR::setup();
    this->set_supports_heat(false);
  }

  /// Override control to change settings of the climate device.
  void control(const climate::ClimateCall &call) override {
    climate_ir::ClimateIR::control(call);
  }

  void set_unit_of_measurement(Unit unit) { this->unit_ = unit; }

 protected:
  /// Transmit via IR the state of this climate controller.
  void transmit_state() override;
  /// Handle received IR Buffer
  bool on_receive(remote_base::RemoteReceiveData data) override;

  Unit unit_;

  int messages_received = 0;
  uint8_t received_state[FRIGIDAIRE_STATE_LENGTH] = {0};

  float temperature_min_C_() {
    return FRIGIDAIRE_TEMP_MIN_C;
  }
  float temperature_max_C_() {
    return FRIGIDAIRE_TEMP_MAX_C;
  }
  float temperature_min_F_() {
    return FRIGIDAIRE_TEMP_MIN_F;
  }
  float temperature_max_F_() {
    return FRIGIDAIRE_TEMP_MAX_F;
  }

  void encodeByte(remote_base::RemoteTransmitData*, const uint8_t &);
  bool readByte(remote_base::RemoteReceiveData *, uint8_t &);
  bool process_received_state();
  uint8_t calc_checksum(uint8_t *);
};

}  // namespace frigidaire
}  // namespace esphome
