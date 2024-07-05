//
// Created by robert on 6/29/24.
//

#ifndef ESPHOME_DEV_MESSAGES_H
#define ESPHOME_DEV_MESSAGES_H

#include <array>
#include <cstdint>
#include <vector>
#include "esphome/core/log.h"
namespace esphome {
namespace balboa {
extern const char *const TAG;

enum SpaState {
  Running = 0x00,
  Initializing = 0x01,
  HoldMode = 0x05,
  ABTempsOn = 0x14,
  TestMode = 0x17,
};
enum InitMode {
  Idle = 0x00,
  PrimingMode = 0x01,
  InitFault = 0x02,
  Reminder = 0x03,
  Stage1 = 0x04,
  Stage3 = 0x05,
  Stage2 = 0x42,
};
enum HeatingMode {
  Ready = 0x00,
  Rest = 0x01,
  ReadyInRest = 0x03,
};
enum HeatingState {
  HeaterOff = 0x00,
  Heating = 0x01,
  HeatWaiting = 0x02,
};
enum ReminderType {
  None = 0x00,
  CleanFilter = 0x04,
  CheckPH = 0x0a,
  CheckSanitizer = 0x09,
  ReminderFault = 0x1e,
};
enum PumpState {
  PumpOff = 0x00,
  PumpLow = 0x01,
  PumpHigh = 0x02,
};
enum PumpType {
  NoPump = 0x00,
  OneSpeedPump = 0x01,
  TwoSpeedPump = 0x02,
};

enum class FaultCode {
  SensorSyncFailure = 15,
  LowFlow,
  FlowFailure,
  SettingsReset,
  PrimingMode,
  ClockFailed,
  SettingsReset2,
  ProgramMemoryFailed,
  SensorSyncFailure2 = 26,
  HeaterIsDry,
  HeaterMaybeDry,
  WaterTooHot,
  HeaterTooHot,
  SensorAFailed,
  SensorBFailed,
  PumpStuckOn = 34,
  HotFault,
  GFCITestFailed,
  HoldMode,
};

enum class MessageType : uint8_t {
  NewClientCTS = 0x00,
  ChanAssignReq = 0x01,
  ChanAssignResp = 0x02,
  ChanAssignAck = 0x03,
  ExistingClientReq = 0x04,
  ExistingClientResp = 0x05,
  ClearToSend = 0x06,
  NothingToSend = 0x07,
  ToggleItemReq = 0x11,
  StatusUpdate = 0x13,
  SetTempReq = 0x20,
  SetTimeReq = 0x21,
  SettingsReq = 0x22,
  FilterCyclesResp = 0x23,
  InformationResp = 0x24,
  AdditionalInfoResp = 0x25,
  PreferencesResp = 0x26,
  SetPreferencesReq = 0x27,
  FaultLogResp = 0x28,
  ChangeSetupReq = 0x2a,
  GFCITestResp = 0x2b,
  LockReq = 0x2d,
  ConfigResp = 0x2e,
  SetWifiSettingsReq = 0x92,
  WifiModuleConfigResp = 0x94,
  ToggleTestSettingReq = 0xe0,
};

struct RawMessage {
  uint8_t len;
  uint8_t channel;
  uint8_t broadcast;
  uint8_t type;
  std::vector<uint8_t> payload;
  uint8_t checksum;
  bool validate() const;

 private:
  bool verify_payload_length() const;
  bool verify_checksum() const;
  uint8_t calculate_checksum() const;
};

class InputSpaMessage {
 private:
  bool valid;

 protected:
  explicit InputSpaMessage(RawMessage msg);

 public:
  bool is_valid() const;
};

struct ChanAssignReq {
  static constexpr auto msg_t = MessageType::ChanAssignReq;
  using pack_t = std::array<uint8_t, 3>;
  explicit ChanAssignReq(uint8_t device_type, uint16_t client_hash = 0xbeef);
  uint8_t device_type;
  uint16_t client_hash;
  pack_t pack() const;
};

struct ChanAssignResp : InputSpaMessage {
  explicit ChanAssignResp(RawMessage msg);
  uint8_t assigned_channel{};
  uint16_t client_hash{};
};

struct SimpleOutMessage {
  using pack_t = std::array<uint8_t, 0>;
  static constexpr pack_t pack() { return {}; };
};

struct ChanAssignAck : SimpleOutMessage {
  static constexpr auto msg_t = MessageType::ChanAssignAck;
};

struct StatusUpdate : InputSpaMessage {
  explicit StatusUpdate(RawMessage msg);

  SpaState spa_state;
  InitMode init_mode;
  uint8_t current_temperature_raw;
  uint8_t time_hour;
  uint8_t time_min;
  HeatingMode heating_mode;
  ReminderType reminder_type;
  uint8_t sensor_a_raw;
  uint8_t sensor_b_raw;
  bool temp_scale_halved;
  bool clock_is_24_hour;
  bool filter_cycle_1_enabled;
  bool filter_cycle_2_enabled;
  bool panel_locked;
  bool temperature_high_range;
  bool needs_heating;
  HeatingState heating_state;
  std::array<PumpState, 6> pump_states{};
  bool circulation_pump_state;
  bool blower_state;
  std::array<bool, 2> light_states{};
  bool notification_state;
  bool sensor_ab_temp_on;
  bool timeouts_state;
  bool settings_locked;

  void dump_config() {
    ESP_LOGCONFIG(TAG, "spa_state: %d", spa_state);
    ESP_LOGCONFIG(TAG, "init_mode: %d", init_mode);
    ESP_LOGCONFIG(TAG, "current_temperature_raw: %d", current_temperature_raw);
    ESP_LOGCONFIG(TAG, "time_hour: %d", time_hour);
    ESP_LOGCONFIG(TAG, "time_min: %d", time_min);
    ESP_LOGCONFIG(TAG, "heating_mode: %d", heating_mode);
    ESP_LOGCONFIG(TAG, "reminder_type: %d", reminder_type);
    ESP_LOGCONFIG(TAG, "sensor_a_raw: %d", sensor_a_raw);
    ESP_LOGCONFIG(TAG, "sensor_b_raw: %d", sensor_b_raw);
    ESP_LOGCONFIG(TAG, "temp_scale_halved: %d", temp_scale_halved);
    ESP_LOGCONFIG(TAG, "clock_is_24_hour: %d", clock_is_24_hour);
    ESP_LOGCONFIG(TAG, "filter_cycle_1_enabled: %d", filter_cycle_1_enabled);
    ESP_LOGCONFIG(TAG, "filter_cycle_2_enabled: %d", filter_cycle_2_enabled);
    ESP_LOGCONFIG(TAG, "panel_locked: %d", panel_locked);
    ESP_LOGCONFIG(TAG, "temperature_high_range: %d", temperature_high_range);
    ESP_LOGCONFIG(TAG, "needs_heating: %d", needs_heating);
    ESP_LOGCONFIG(TAG, "heating_state: %d", heating_state);
    for (auto p : pump_states) {
      ESP_LOGCONFIG(TAG, "pump_states: %d", p);
    }
    ESP_LOGCONFIG(TAG, "circulation_pump_state: %d", circulation_pump_state);
    ESP_LOGCONFIG(TAG, "blower_state: %d", blower_state);
    for (auto l : light_states) {
      ESP_LOGCONFIG(TAG, "light_states: %d", l);
    }
    ESP_LOGCONFIG(TAG, "notification_state: %d", notification_state);
    ESP_LOGCONFIG(TAG, "sensor_ab_temp_on: %d", sensor_ab_temp_on);
    ESP_LOGCONFIG(TAG, "timeouts_state: %d", timeouts_state);
    ESP_LOGCONFIG(TAG, "settings_locked: %d", settings_locked);
  }
};

struct FilterCyclesResp : InputSpaMessage {
  explicit FilterCyclesResp(RawMessage msg);
  uint8_t cycle_1_start_hour;
  uint8_t cycle_1_start_minute;
  uint8_t cycle_1_duration_hour;
  uint8_t cycle_1_duration_minute;
  bool cycle_2_enabled;
  uint8_t cycle_2_start_hour;
  uint8_t cycle_2_start_minute;
  uint8_t cycle_2_duration_hour;
  uint8_t cycle_2_duration_minute;
};

struct InformationResp : InputSpaMessage {
  explicit InformationResp(RawMessage msg);
  char software_id[4]{};
  char model_number[8]{};
  uint8_t current_config_setup_number;
  uint32_t configuration_signature;
  uint16_t dip_switch_settings;
};

struct AdditionalInfoResp : InputSpaMessage {
  explicit AdditionalInfoResp(RawMessage msg);
  uint8_t low_range_min_temp_raw;
  uint8_t low_range_max_temp_raw;
  uint8_t high_range_min_temp_raw;
  uint8_t high_range_max_temp_raw;
  std::array<bool, 6> pump_available{};
};

struct PreferencesResp : InputSpaMessage {
  explicit PreferencesResp(RawMessage msg);
  bool reminders_enabled;
  bool temperature_scale_halved;
  bool clock_is_24_hour;
  uint8_t cleanup_cycle_length_in_half_hours;
  uint8_t dolphin_address;
  uint8_t m8_ai_enabled;
};
struct FaultLogResp : InputSpaMessage {
  explicit FaultLogResp(RawMessage msg);
  uint8_t total_entries;
  uint8_t entry_number;
  FaultCode message_code;
  uint8_t days_ago;
  uint8_t time_hour;
  uint8_t time_minute;
  uint8_t set_temperature_raw;
  uint8_t sensor_a_temperature_raw;
  uint8_t sensor_b_temperature_raw;
};
struct GFCITestResp : InputSpaMessage {
  explicit GFCITestResp(RawMessage msg);
  bool passed{};
};
struct ConfigResp : InputSpaMessage {
  explicit ConfigResp(RawMessage msg);
  std::array<PumpType, 6> pumps_installed{};
  std::array<bool, 2> lights_installed{};
  bool circulation_pump_installed;
  bool blower_installed;
  bool aux_1_installed;
  bool aux_2_installed;
  bool mister_installed;
};
struct WifiModuleConfigResp : InputSpaMessage {
  explicit WifiModuleConfigResp(RawMessage msg);
  std::array<uint8_t, 6> mac_address{};
  std::array<uint8_t, 3> mac_oui{};
  std::array<uint8_t, 3> mac_nic{};
};
struct NothingToSend : SimpleOutMessage {
  static constexpr auto msg_t = MessageType::NothingToSend;
};

struct SettingsReq {
  static constexpr auto msg_t = MessageType::SettingsReq;
  using pack_t = std::array<uint8_t, 3>;
};

struct AdditionalInfoReq : SettingsReq {
  static constexpr pack_t payload = {0x04, 0x00, 0x00};
  pack_t pack() const { return payload; };
};

struct ConfigurationReq : SettingsReq {
  static constexpr pack_t payload = {0x00, 0x00, 0x01};
  pack_t pack() const { return payload; };
};

struct FaultLogReq : SettingsReq {
  explicit FaultLogReq(uint8_t entry = 0xff);
  pack_t pack() const;
  uint8_t requested_entry;
};

struct FilterCyclesReq : SettingsReq {
  static constexpr pack_t payload = {0x01, 0x00, 0x00};
  pack_t pack() const { return payload; };
};

struct GFCITestReq : SettingsReq {
  static constexpr pack_t payload = {0x80, 0x00, 0x00};
  pack_t pack() const { return payload; };
};

struct InformationReq : SettingsReq {
  static constexpr pack_t payload = {0x02, 0x00, 0x00};
  pack_t pack() const { return payload; };
};

struct PreferencesReq : SettingsReq {
  static constexpr pack_t payload = {0x08, 0x00, 0x00};
  pack_t pack() const { return payload; };
};

}  // namespace balboa
}  // namespace esphome

#endif  // ESPHOME_DEV_MESSAGES_H
