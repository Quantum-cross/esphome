//
// Created by robert on 6/29/24.
//

#ifndef ESPHOME_DEV_BALBOA_H
#define ESPHOME_DEV_BALBOA_H

#include <queue>
#include <memory>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/balboa/messages.h"

namespace esphome {
namespace balboa {

enum class ReadState {
  Unsynchronized,
  ReadStart,
  ReadLen,
  ReadChannel,
  ReadBroadcast,
  ReadType,
  ReadPayload,
  ReadChecksum,
  ReadEnd,
};

enum class RegistrationState { Unregistered, WaitingForCTS, WaitingForResponse, Registered };

class Balboa : public Component, public uart::UARTDevice {
 public:
  void loop() override;
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 private:
  bool decode(char c);
  ReadState readState = ReadState::Unsynchronized;
  RegistrationState regState = RegistrationState::Unregistered;
  void reset_message();
  void process_message();
  RawMessage message;
  optional<unsigned char> assigned_channel;

  //  template<size_t N>
  //  bool send_msg(MessageType type, std::array<uint8_t, N> payload);

  std::queue<std::vector<uint8_t>> output_msg_queue;

  template<typename T> typename std::vector<uint8_t> prepare_msg(T msg);

  template<typename T> bool send_msg(T msg);

  template<typename T> bool queue_msg(T msg);

  void handleNewClientCTS();
  void handleChanAssignResp();
  void handleClearToSend();
  void handleStatusUpdate();
  void handleFilterCyclesResp();
  void handleInformationResp();
  void handleAdditionalInfoResp();
  void handlePreferencesResp();
  void handleFaultLogResp();
  void handleGFCITestResp();
  void handleConfigResp();
  void handleWifiModuleConfigResp();
  void handleExistingClientReq();

  bool initial_data = false;

  int debug_count = 0;

  std::vector<uint8_t> debug_bytes;
  std::shared_ptr<StatusUpdate> latest_status_update;
  std::shared_ptr<FilterCyclesResp> latest_filter_cycles_response;
  std::shared_ptr<InformationResp> latest_information_response;
  std::shared_ptr<WifiModuleConfigResp> latest_wifi_module_config_response;
  std::shared_ptr<ConfigResp> latest_config_response;
  std::shared_ptr<GFCITestResp> latest_gfci_test_response;
  std::shared_ptr<FaultLogResp> latest_fault_log_response;
  std::shared_ptr<PreferencesResp> latest_preferences_response;
  std::shared_ptr<AdditionalInfoResp> latest_additional_info_response;
  int messages_processed;
  int new_client_cts_received;
  int channel_assign_req_sent;
  int messages_rejected;
  int ChanAssignResp_received;
  int ExistingClientReq_received;
  int ExistingClientResp_received;
  int ClearToSend_received;
  int StatusUpdate_received;
  int FilterCyclesResp_received;
  int InformationResp_received;
  int AdditionalInfoResp_received;
  int PreferencesResp_received;
  int FaultLogResp_received;
  int GFCITestResp_received;
  int ConfigResp_received;
  int WifiModuleConfigResp_received;
};

}  // namespace balboa
}  // namespace esphome

#endif  // ESPHOME_DEV_BALBOA_H
