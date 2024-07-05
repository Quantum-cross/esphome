//
// Created by robert on 6/29/24.
//

#include "balboa.h"
#include "messages.h"
#include "esphome/core/helpers.h"
#include "crc8.h"

namespace esphome {
namespace balboa {
const char *const TAG = "balboa";

void Balboa::dump_config() {
  ESP_LOGCONFIG(TAG, "In balboa dump_config");
  //  for(auto b : debug_bytes){
  //    ESP_LOGCONFIG(TAG, "byte: %x", b);
  //  }
  ESP_LOGCONFIG(TAG, "reg state: %d", static_cast<int>(regState));
  ESP_LOGCONFIG(TAG, "read state: %d", static_cast<int>(readState));
  if (assigned_channel.has_value()) {
    ESP_LOGCONFIG(TAG, "  Assigned channel: %d", assigned_channel.value());
  } else {
    ESP_LOGCONFIG(TAG, "  Assigned channel: None");
  }
  if (latest_status_update != nullptr && latest_status_update->is_valid()) {
    latest_status_update->dump_config();
  }
  ESP_LOGCONFIG(TAG, "  messages_processed: %d", messages_processed);
  ESP_LOGCONFIG(TAG, "  new_client_cts_received: %d", new_client_cts_received);
  ESP_LOGCONFIG(TAG, "  channel_assign_req_sent: %d", channel_assign_req_sent);
  ESP_LOGCONFIG(TAG, "  messages rejected: %d", messages_rejected);
  ESP_LOGCONFIG(TAG, "  ChanAssignResp rec: %d", ChanAssignResp_received);
  ESP_LOGCONFIG(TAG, "  ExistingClientReq rec: %d", ExistingClientReq_received);
  ESP_LOGCONFIG(TAG, "  ExistingClientResp rec: %d", ExistingClientResp_received);
  ESP_LOGCONFIG(TAG, "  ClearToSend rec: %d", ClearToSend_received);
  ESP_LOGCONFIG(TAG, "  StatusUpdate rec: %d", StatusUpdate_received);
  ESP_LOGCONFIG(TAG, "  FilterCyclesResp rec: %d", FilterCyclesResp_received);
  ESP_LOGCONFIG(TAG, "  InformationResp rec: %d", InformationResp_received);
  ESP_LOGCONFIG(TAG, "  AdditionalInfoResp rec: %d", AdditionalInfoResp_received);
  ESP_LOGCONFIG(TAG, "  PreferencesResp rec: %d", PreferencesResp_received);
  ESP_LOGCONFIG(TAG, "  FaultLogResp rec: %d", FaultLogResp_received);
  ESP_LOGCONFIG(TAG, "  GFCITestResp rec: %d", GFCITestResp_received);
  ESP_LOGCONFIG(TAG, "  ConfigResp rec: %d", ConfigResp_received);
  ESP_LOGCONFIG(TAG, "  WifiModuleConfigResp rec: %d", WifiModuleConfigResp_received);
}

void Balboa::loop() {
  //  if(debug_count < 10){
  //    ESP_LOGW(TAG, "loop %d", debug_count);
  //    ESP_LOGCONFIG(TAG, "loop %d", debug_count++);
  //  }
  if (!initial_data && regState == RegistrationState::Registered) {
    //    queue_msg(AdditionalInfoReq());
    //    queue_msg(ConfigurationReq());
    //    queue_msg(FaultLogReq());
    //    queue_msg(FilterCyclesReq());
    //    queue_msg(GFCITestReq());
    //    queue_msg(InformationReq());
    //    queue_msg(PreferencesReq());
    initial_data = true;
    ESP_LOGD(TAG, "Queued initial data");
  }
  while (this->available()) {
    //    if(debug_bytes.size() < 100){
    //      debug_bytes.push_back(this->peek());
    //    }
    if (decode(this->read())) {
      process_message();
      reset_message();
      messages_processed++;
    }
  }
}

void Balboa::setup() {}

bool Balboa::decode(char c) {
  switch (readState) {
    case ReadState::Unsynchronized:
      if (c == '~') {
        readState = ReadState::ReadStart;
      }
      return false;
    case ReadState::ReadStart:
      if (c == '~') {
        reset_message();
        readState = ReadState::ReadLen;
      } else {
        readState = ReadState::Unsynchronized;
      }
      return false;
    case ReadState::ReadLen:
      message.len = c;
      readState = ReadState::ReadChannel;
      return false;
    case ReadState::ReadChannel:
      message.channel = c;
      readState = ReadState::ReadBroadcast;
      return false;
    case ReadState::ReadBroadcast:
      message.broadcast = c;
      readState = ReadState::ReadType;
      return false;
    case ReadState::ReadType:
      message.type = c;
      if (message.len <= 5) {
        readState = ReadState::ReadChecksum;
      } else {
        readState = ReadState::ReadPayload;
      }
      return false;
    case ReadState::ReadPayload:
      message.payload.push_back(c);
      if (message.payload.size() >= message.len - 5) {
        readState = ReadState::ReadChecksum;
      }
      return false;
    case ReadState::ReadChecksum:
      message.checksum = c;
      readState = ReadState::ReadEnd;
      return false;
    case ReadState::ReadEnd:
      if (c == '~') {
        readState = ReadState::ReadStart;
      }
      return true;
    default:
      readState = ReadState::Unsynchronized;
  }
  return false;
}

void Balboa::reset_message() { message = RawMessage(); }
void Balboa::process_message() {
  const bool is_broadcast = message.channel == 0xff && message.broadcast == 0xaf;
  const bool is_new_client_broadcast = message.channel == 0xfe && message.broadcast == 0xbf;
  const bool channel_matches = assigned_channel.has_value() && message.channel == assigned_channel.value();
  if (!(is_broadcast || is_new_client_broadcast || channel_matches)) {
    messages_rejected++;
    return;
  }
  //  ESP_LOGV(TAG, "t: %d, c: %d, b: %d, l: %d, ps: %d", message.type, message.channel, message.broadcast, message.len,
  //           message.payload.size());
  switch (static_cast<MessageType>(message.type)) {
    case MessageType::NewClientCTS:
      new_client_cts_received++;
      handleNewClientCTS();
      break;
    case MessageType::ChanAssignResp:
      ChanAssignResp_received++;
      handleChanAssignResp();
      break;
    case MessageType::ExistingClientReq:
      ExistingClientReq_received++;
      handleExistingClientReq();
      break;
    case MessageType::ExistingClientResp:
      ExistingClientResp_received++;
      break;
    case MessageType::ClearToSend:
      ClearToSend_received++;
      handleClearToSend();
      break;
    case MessageType::StatusUpdate:
      StatusUpdate_received++;
      handleStatusUpdate();
      break;
    case MessageType::FilterCyclesResp:
      FilterCyclesResp_received++;
      handleFilterCyclesResp();
      break;
    case MessageType::InformationResp:
      InformationResp_received++;
      handleInformationResp();
      break;
    case MessageType::AdditionalInfoResp:
      AdditionalInfoResp_received++;
      handleAdditionalInfoResp();
      break;
    case MessageType::PreferencesResp:
      PreferencesResp_received++;
      handlePreferencesResp();
      break;
    case MessageType::FaultLogResp:
      FaultLogResp_received++;
      handleFaultLogResp();
      break;
    case MessageType::GFCITestResp:
      GFCITestResp_received++;
      handleGFCITestResp();
      break;
    case MessageType::ConfigResp:
      ConfigResp_received++;
      handleConfigResp();
      break;
    case MessageType::WifiModuleConfigResp:
      WifiModuleConfigResp_received++;
      handleWifiModuleConfigResp();
      break;

    case MessageType::ToggleItemReq:
    case MessageType::SetTimeReq:
    case MessageType::SettingsReq:
    case MessageType::SetTempReq:
    case MessageType::NothingToSend:
    case MessageType::SetPreferencesReq:
    case MessageType::ChangeSetupReq:
    case MessageType::LockReq:
    case MessageType::SetWifiSettingsReq:
    case MessageType::ToggleTestSettingReq:
    case MessageType::ChanAssignReq:
    case MessageType::ChanAssignAck:
      // Client messages, don't need to handle these
      break;
    default:
      // Unknown messages
      ESP_LOGD(TAG, "Unknown message: %x", message.type);
      break;
  }
}

void Balboa::handleNewClientCTS() {
  //  ESP_LOGD(TAG, "NewClientCTS %d", static_cast<int>(regState));
  switch (regState) {
    case RegistrationState::Unregistered:
    case RegistrationState::Registered:
      return;
    case RegistrationState::WaitingForCTS:
    case RegistrationState::WaitingForResponse:
      auto req = ChanAssignReq(0x22);
      if (send_msg(req)) {
        channel_assign_req_sent++;
        regState = RegistrationState::WaitingForResponse;
      }
      return;
  }
}

void Balboa::handleChanAssignResp() {
  if (regState != RegistrationState::WaitingForResponse) {
    return;
  }
  //  ESP_LOGV(TAG, "Got chan assign resp");
  auto resp = ChanAssignResp(message);
  //  ESP_LOGV(TAG, "Valid? %d", resp.is_valid());
  if (!resp.is_valid()) {
    return;
  }
  //  ESP_LOGV(TAG, "client hash: %04x, chan: %02x", resp.client_hash, resp.assigned_channel);
  if (resp.client_hash != 0xbeef) {
    return;
  }
  if (resp.assigned_channel >= 0x2F) {
    return;
  }
  assigned_channel = resp.assigned_channel;
  if (send_msg(ChanAssignAck())) {
    regState = RegistrationState::Registered;
  };
}

void Balboa::handleExistingClientReq() {}

void Balboa::handleClearToSend() {
  if (output_msg_queue.empty()) {
    send_msg(NothingToSend());
    return;
  }
  write_array(output_msg_queue.front());
  flush();
  output_msg_queue.pop();
}

void Balboa::handleStatusUpdate() {
  latest_status_update = std::make_shared<StatusUpdate>(message);
  if (regState == RegistrationState::Unregistered && latest_status_update->is_valid()) {
    regState = RegistrationState::WaitingForCTS;
  }
}
void Balboa::handleFilterCyclesResp() { latest_filter_cycles_response = std::make_shared<FilterCyclesResp>(message); }
void Balboa::handleInformationResp() { latest_information_response = std::make_shared<InformationResp>(message); }
void Balboa::handleAdditionalInfoResp() {
  latest_additional_info_response = std::make_shared<AdditionalInfoResp>(message);
}
void Balboa::handlePreferencesResp() { latest_preferences_response = std::make_shared<PreferencesResp>(message); }
void Balboa::handleFaultLogResp() { latest_fault_log_response = std::make_shared<FaultLogResp>(message); }
void Balboa::handleGFCITestResp() { latest_gfci_test_response = std::make_shared<GFCITestResp>(message); }
void Balboa::handleConfigResp() { latest_config_response = std::make_shared<ConfigResp>(message); }
void Balboa::handleWifiModuleConfigResp() {
  latest_wifi_module_config_response = std::make_shared<WifiModuleConfigResp>(message);
}

template<typename T> typename std::vector<uint8_t> Balboa::prepare_msg(T msg) {
  constexpr auto payload_len = std::tuple_size<typename T::pack_t>();
  constexpr size_t total_msg_size = payload_len + 7;
  constexpr uint8_t length_parameter = payload_len + 5;
  constexpr uint8_t csum_length = payload_len + 4;
  const uint8_t out_channel = T::msg_t == MessageType::ChanAssignReq ? 0xfe : assigned_channel.value_or(0xff);
  if (assigned_channel == 0xff) {
    return {};
  }
  std::array<uint8_t, total_msg_size> bytes;
  bytes.at(0) = '~';
  bytes.at(1) = length_parameter;
  bytes.at(2) = out_channel;
  bytes.at(3) = 0xbf;
  bytes.at(4) = static_cast<uint8_t>(T::msg_t);
  int i = 5;
  for (auto byte : msg.pack()) {
    bytes.at(i++) = byte;
  }
  bytes.at(i++) = crc8(bytes.data() + 1, csum_length);
  bytes.at(i) = '~';
  return {bytes.begin(), bytes.end()};
}

template<typename T> bool Balboa::send_msg(T msg) {
  auto bytes = prepare_msg(msg);
  //  ESP_LOGV(TAG, "sending msg: %s", format_hex(bytes).c_str());
  if (!bytes.empty()) {
    write_array(bytes);
    flush();
    return true;
  }
  return false;
}

template<typename T> bool Balboa::queue_msg(T msg) {
  auto bytes = prepare_msg(msg);
  if (!bytes.empty()) {
    output_msg_queue.push(prepare_msg(msg));
    return true;
  }
  return false;
}

}  // namespace balboa
}  // namespace esphome
