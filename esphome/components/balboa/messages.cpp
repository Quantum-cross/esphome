//
// Created by robert on 6/29/24.
//

#include "messages.h"
#include "crc8.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace balboa {

ChanAssignResp::ChanAssignResp(RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  ESP_LOGV(TAG, "chan resp: %s", format_hex(msg.payload).c_str());
  assigned_channel = msg.payload[0];
  client_hash = msg.payload[1];
  client_hash <<= 8;
  client_hash |= msg.payload[2];
}

StatusUpdate::StatusUpdate(RawMessage msg) : InputSpaMessage(msg) {
  //  ESP_LOGV(TAG, "Status Update Payload: %s", format_hex(msg.payload).c_str());
  //  ESP_LOGV(TAG, "checksum: %d, expected: %d\n", msg.calculate_checksum(), msg.checksum);
  if (!is_valid()) {
    return;
  }

  uint8_t *ptr = msg.payload.data();

  spa_state = static_cast<SpaState>(*ptr++);
  init_mode = static_cast<InitMode>(*ptr++);
  current_temperature_raw = *ptr++;
  time_hour = *ptr++;
  time_min = *ptr++;
  heating_mode = static_cast<HeatingMode>(*ptr++);
  reminder_type = static_cast<ReminderType>(*ptr++);
  sensor_a_raw = *ptr++;
  sensor_b_raw = *ptr++;
  temp_scale_halved = *ptr & 1u;
  clock_is_24_hour = *ptr & (1u << 1);
  filter_cycle_1_enabled = *ptr & (1u << 3);
  filter_cycle_2_enabled = *ptr & (1u << 4);
  panel_locked = *ptr & (1u << 5);
  ptr++;
  temperature_high_range = *ptr & (1u << 2);
  needs_heating = *ptr & (1u << 3);
  heating_state = static_cast<HeatingState>((*ptr >> 4) & 0b11);
  ptr++;
  pump_states[0] = static_cast<PumpState>(*ptr & 0b11);
  pump_states[1] = static_cast<PumpState>((*ptr >> 2) & 0b11);
  pump_states[2] = static_cast<PumpState>((*ptr >> 4) & 0b11);
  pump_states[3] = static_cast<PumpState>((*ptr >> 6) & 0b11);
  ptr++;
  pump_states[4] = static_cast<PumpState>(*ptr & 0b11);
  pump_states[5] = static_cast<PumpState>((*ptr >> 2) & 0b11);
  ptr++;
  circulation_pump_state = *ptr & (1u << 1);
  blower_state = *ptr & (3u << 2);
  ptr++;
  light_states[0] = *ptr & 3u;
  light_states[1] = *ptr & (3u << 2);
  ptr += 4;
  ptr++;
  notification_state = *ptr & (1u << 5);
  ptr += 2;
  sensor_ab_temp_on = *ptr & (1u << 1);
  timeouts_state = *ptr & (1u << 2);
  settings_locked = *ptr & (1u << 3);
}

FilterCyclesResp::FilterCyclesResp(RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();

  cycle_1_start_hour = *ptr++;
  cycle_1_start_minute = *ptr++;
  cycle_1_duration_hour = *ptr++;
  cycle_1_duration_minute = *ptr++;
  cycle_2_enabled = *ptr & 0x80;
  cycle_2_start_hour = *ptr & 0x7f;
  ptr++;
  cycle_2_start_minute = *ptr++;
  cycle_2_duration_hour = *ptr++;
  cycle_2_duration_minute = *ptr;
}

InformationResp::InformationResp(RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();

  memcpy(software_id, ptr, 4);
  ptr += 4;
  memcpy(model_number, ptr, 8);
  ptr += 8;
  current_config_setup_number = *ptr++;
  memcpy(&configuration_signature, ptr, 4);
  ptr += 4;
  memcpy(&dip_switch_settings, ptr, 2);
}

AdditionalInfoResp::AdditionalInfoResp(RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();
  ptr++;
  low_range_min_temp_raw = *ptr++;
  low_range_max_temp_raw = *ptr++;
  high_range_min_temp_raw = *ptr++;
  high_range_max_temp_raw = *ptr++;
  ptr++;
  for (auto i = 0u; i < 6; i++) {
    pump_available[i] = *ptr & (1u << i);
  }
}

PreferencesResp::PreferencesResp(esphome::balboa::RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();
  ptr++;
  reminders_enabled = *ptr++;
  ptr++;
  temperature_scale_halved = *ptr++;
  clock_is_24_hour = *ptr++;
  cleanup_cycle_length_in_half_hours = *ptr++;
  dolphin_address = *ptr++;
  ptr++;
  m8_ai_enabled = *ptr;
}

FaultLogResp::FaultLogResp(esphome::balboa::RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();
  total_entries = *ptr++;
  entry_number = *ptr++;
  message_code = static_cast<FaultCode>(*ptr++);
  days_ago = *ptr++;
  time_hour = *ptr++;
  time_minute = *ptr++;
  ptr++;
  set_temperature_raw = *ptr++;
  sensor_a_temperature_raw = *ptr++;
  sensor_b_temperature_raw = *ptr;
}

GFCITestResp::GFCITestResp(esphome::balboa::RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  passed = msg.payload[0];
}

ConfigResp::ConfigResp(esphome::balboa::RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();
  pumps_installed[0] = static_cast<PumpType>((*ptr) & 0b11);
  pumps_installed[1] = static_cast<PumpType>((*ptr) >> 2 & 0b11);
  pumps_installed[2] = static_cast<PumpType>((*ptr) >> 4 & 0b11);
  pumps_installed[3] = static_cast<PumpType>((*ptr) >> 6 & 0b11);
  ptr++;
  pumps_installed[4] = static_cast<PumpType>((*ptr) & 0b11);
  pumps_installed[5] = static_cast<PumpType>((*ptr) >> 6 & 0b11);
  ptr++;
  lights_installed[0] = (*ptr) & 0b11;
  lights_installed[1] = (*ptr) >> 6 & 0b11;
  ptr++;
  blower_installed = (*ptr) & 0b11;
  circulation_pump_installed = (*ptr) >> 7 & 0x01;
  ptr++;
  aux_1_installed = (*ptr) & 0x01;
  aux_2_installed = (*ptr) >> 1 & 0x01;
  mister_installed = (*ptr) >> 4 & 0b11;
}

WifiModuleConfigResp::WifiModuleConfigResp(esphome::balboa::RawMessage msg) : InputSpaMessage(msg) {
  if (!is_valid()) {
    return;
  }
  uint8_t *ptr = msg.payload.data();
  ptr += 3;
  memcpy(mac_address.data(), ptr, 6);
  ptr += 6;
  memcpy(mac_oui.data(), ptr, 3);
  ptr += 3;
  memcpy(mac_nic.data(), ptr, 3);
}

bool RawMessage::verify_payload_length() const { return payload.size() == len - 5; }

bool RawMessage::verify_checksum() const { return checksum == calculate_checksum(); }
uint8_t RawMessage::calculate_checksum() const {
  std::vector<uint8_t> csum_data;
  csum_data.push_back(len);
  csum_data.push_back(channel);
  csum_data.push_back(broadcast);
  csum_data.push_back(type);
  for (auto v : payload) {
    csum_data.push_back(v);
  }
  return crc8(csum_data.data(), csum_data.size());
}

bool RawMessage::validate() const { return verify_payload_length() && verify_checksum(); }

InputSpaMessage::InputSpaMessage(RawMessage msg) : valid(msg.validate()) {}
bool InputSpaMessage::is_valid() const { return valid; }

ChanAssignReq::ChanAssignReq(uint8_t device_type, uint16_t client_hash)
    : device_type(device_type), client_hash(client_hash) {}

ChanAssignReq::pack_t ChanAssignReq::pack() const {
  auto ret = pack_t();
  ret[0] = device_type;
  ret[1] = client_hash >> 8;
  ret[2] = client_hash & 0xff;
  return ret;
}

FaultLogReq::FaultLogReq(uint8_t entry) : requested_entry(entry) {}
SettingsReq::pack_t FaultLogReq::pack() const { return {0x20, requested_entry, 0x00}; }

}  // namespace balboa
}  // namespace esphome
