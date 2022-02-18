#include "frigidaire.h"
#include "esphome/core/log.h"

namespace esphome {
namespace frigidaire {

static const char *TAG = "frigidaire.climate";

// extern size_t FRIGIDAIRE_STATE_LENGTH;

const uint32_t FRIGIDAIRE_CARRIER_FREQUENCY = 38000;

const uint16_t FRIGIDAIRE_HEADER_MARK = 9065;
const uint16_t FRIGIDAIRE_HEADER_SPACE = 4415;
const uint16_t FRIGIDAIRE_BIT_MARK = 725;
const uint16_t FRIGIDAIRE_ONE_SPACE = 1580;
const uint16_t FRIGIDAIRE_ZERO_SPACE = 490;
const uint32_t FRIGIDAIRE_SMALLGAP_SPACE = 19950;
const uint32_t FRIGIDAIRE_LARGEGAP_SPACE = 39930;

const uint8_t POWER_BYTE0 = 0b00010000;
const uint8_t POWER_BYTE2 = 0b00000010;

const uint8_t MODE_MASK_BYTE0 = 0b01100000;
const uint8_t MODE_COOL_BYTE0 = 0b00000000;
const uint8_t MODE_ECO_BYTE0 = 0b00100000;
const uint8_t MODE_FANONLY_BYTE0 = 0b01000000;

const uint8_t FAN_MASK_BYTE0 = 0b00001100;
const uint8_t FAN_AUTO_BYTE0 = 0b00000000;
const uint8_t FAN_LOW_BYTE0 = 0b00001000;
const uint8_t FAN_MED_BYTE0 = 0b00000100;
const uint8_t FAN_HIGH_BYTE0 = 0b00001100;

const uint8_t TEMP_MASK_BYTE1  = 0b11110000;
const uint8_t TEMP_MASK_BYTE3  = 0b00110000;
const uint8_t TEMP_MASK_BYTE10 = 0b00001111;

const uint8_t TEMP_F_OFFSET = 60;
const uint8_t TEMP_TABLE_F_SIZE = 31;
const uint8_t CMD_TEMP_TABLE_F_BYTE1[TEMP_TABLE_F_SIZE] = {
    0b00000000, 0b00000000, 0b10000000, 0b10000000, 0b01000000, 0b01000000, 0b11000000, 0b11000000,
    0b00100000, 0b10100000, 0b10100000, 0b01100000, 0b01100000, 0b11100000, 0b11100000, 0b00010000,
    0b00010000, 0b10010000, 0b01010000, 0b01010000, 0b11010000, 0b11010000, 0b00110000, 0b00110000,
    0b10110000, 0b10110000, 0b01110000, 0b01110000, 0b01110000, 0b01110000, 0b01110000,
};
const uint8_t CMD_TEMP_IS_F_BYTE3 = 0b00010000;
const uint8_t CMD_TEMP_TABLE_F_BYTE3[TEMP_TABLE_F_SIZE] = {
    0b00110000, 0b00010000, 0b00010000, 0b00110000, 0b00010000, 0b00110000, 0b00010000, 0b00110000,
    0b00010000, 0b00010000, 0b00110000, 0b00010000, 0b00110000, 0b00010000, 0b00110000, 0b00010000,
    0b00110000, 0b00010000, 0b00010000, 0b00110000, 0b00010000, 0b00110000, 0b00010000, 0b00110000,
    0b00010000, 0b00110000, 0b00010000, 0b00010000, 0b00110000, 0b00010000, 0b00110000,
};
const uint8_t CMD_TEMP_TABLE_F_BYTE10[TEMP_TABLE_F_SIZE] = {
    0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00001001, 0b00001001, 0b00000101, 0b00000101,
};

const uint8_t TEMP_C_OFFSET = 16;
const uint8_t TEMP_TABLE_C_SIZE = 17;
const uint8_t CMD_TEMP_TABLE_C_BYTE1[TEMP_TABLE_C_SIZE] = {
    0b00000000, 0b10000000, 0b01000000, 0b11000000, 0b00100000, 0b10100000, 0b01100000, 0b11100000, 0b00010000,
    0b10010000, 0b01010000, 0b11010000, 0b00110000, 0b10110000, 0b01110000, 0b01110000, 0b01110000};
const uint8_t CMD_TEMP_TABLE_C_BYTE10[TEMP_TABLE_C_SIZE] = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001001, 0b00000101};

#define BASE_STATE { 0x90, 0x00, 0x06, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05 }

const uint8_t REVERSE_NIBBLE[] = {0b0000, 0b1000, 0b0100, 0b1100, 0b0010, 0b1010, 0b0110, 0b1110,
                                  0b0001, 0b1001, 0b0101, 0b1101, 0b0011, 0b1011, 0b0111, 0b1111};

const uint8_t FIRST_MESSAGE_BYTE0 = 0b10000000;

void FrigidaireClimate::transmit_state() {
  uint8_t remote_state[] = BASE_STATE;

  switch (this->mode) {
    case climate::CLIMATE_MODE_COOL:
      remote_state[0] |= MODE_COOL_BYTE0;
      break;
    case climate::CLIMATE_MODE_HEAT_COOL:
      remote_state[0] |= MODE_ECO_BYTE0;
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
      remote_state[0] |= MODE_FANONLY_BYTE0;
      break;
    case climate::CLIMATE_MODE_OFF:
      remote_state[0] ^= POWER_BYTE0;
      remote_state[2] ^= POWER_BYTE2;
      break;
    default:
      break;
  }

  // Temperature
  if (this->unit_ == UNIT_C) {
    auto temp =
        (uint8_t) lroundf(clamp(this->target_temperature, this->temperature_min_C_(), this->temperature_max_C_()));
    remote_state[1] |= CMD_TEMP_TABLE_C_BYTE1[temp - TEMP_C_OFFSET];
    remote_state[10] |= CMD_TEMP_TABLE_C_BYTE10[temp - TEMP_C_OFFSET];
  } else {  // UNIT_F
    float temp_F = (this->target_temperature) * 1.8 + 32.0;
    auto temp = (uint8_t) lroundf(clamp(temp_F, this->temperature_min_F_(), this->temperature_max_F_()));
    remote_state[1] |= CMD_TEMP_TABLE_F_BYTE1[temp - TEMP_F_OFFSET];
    remote_state[3] |= CMD_TEMP_TABLE_F_BYTE3[temp - TEMP_F_OFFSET];
    remote_state[10] |= CMD_TEMP_TABLE_F_BYTE10[temp - TEMP_F_OFFSET];
  }

  // Fan speed
  switch (this->fan_mode.value()) {
    case climate::CLIMATE_FAN_HIGH:
      remote_state[0] |= FAN_HIGH_BYTE0;
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      remote_state[0] |= FAN_MED_BYTE0;
      break;
    case climate::CLIMATE_FAN_LOW:
      remote_state[0] |= FAN_LOW_BYTE0;
      break;
    case climate::CLIMATE_FAN_AUTO:
      remote_state[0] |= FAN_AUTO_BYTE0;
      break;
    default:
      break;
  }

  for (int i = 0; i < 4; ++i) {
    remote_state[7] += REVERSE_NIBBLE[(remote_state[i] & 0xF0) >> 4];
  }
  for (int i = 4; i < 7; ++i) {
    remote_state[7] += REVERSE_NIBBLE[remote_state[i] & 0x0F];
  }
  remote_state[7] += 10;
  remote_state[7] %= 16;
  remote_state[7] &= 0x0F;
  remote_state[7] = REVERSE_NIBBLE[remote_state[7]];

  ESP_LOGI(TAG, "Encoding: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X",
           remote_state[0], remote_state[1], remote_state[2], remote_state[3], remote_state[4], remote_state[5],
           remote_state[6], remote_state[7], remote_state[8], remote_state[9], remote_state[10], remote_state[11],
           remote_state[12], remote_state[13], remote_state[14], remote_state[15]);

  // Send code
  auto transmit = this->transmitter_->transmit();
  auto dst = transmit.get_data();

  dst->reserve(279);
  dst->set_carrier_frequency(38000);

  dst->item(FRIGIDAIRE_HEADER_MARK, FRIGIDAIRE_HEADER_SPACE);
  for (int i = 0; i < 4; ++i) {
    encodeByte(dst, remote_state[i]);
  }

  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ONE_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_SMALLGAP_SPACE);

  for (int i = 4; i < 8; ++i) {
    encodeByte(dst, remote_state[i]);
  }

  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_LARGEGAP_SPACE);
  dst->item(FRIGIDAIRE_HEADER_MARK, FRIGIDAIRE_HEADER_SPACE);

  for (int i = 8; i < 12; ++i) {
    encodeByte(dst, remote_state[i]);
  }

  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ONE_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE);
  dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_SMALLGAP_SPACE);

  for (int i = 12; i < 16; ++i) {
    encodeByte(dst, remote_state[i]);
  }

  dst->mark(FRIGIDAIRE_BIT_MARK);

  transmit.perform();
}

void FrigidaireClimate::encodeByte(remote_base::RemoteTransmitData *dst, const uint8_t &data) {
  for (uint8_t mask = 1U << 7; mask; mask >>= 1) {
    if (data & mask)
      dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ONE_SPACE);
    else
      dst->item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE);
  }
}

bool FrigidaireClimate::readByte(remote_base::RemoteReceiveData *data, uint8_t &dst) {
  //  ESP_LOGI(TAG, "byte begin");
  for (uint8_t mask = 1U << 7; mask; mask >>= 1) {
    if (data->expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ONE_SPACE)) {
      dst |= mask;
    } else if (!data->expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE)) {
      return false;
    }
  }
  //  ESP_LOGI(TAG, "byte success, %u", dst);
  return true;
}

bool FrigidaireClimate::on_receive(remote_base::RemoteReceiveData data) {
  //   Validate header
  if (!data.expect_item(FRIGIDAIRE_HEADER_MARK, FRIGIDAIRE_HEADER_SPACE)) {
    //    ESP_LOGI(TAG, "Header Fail %i %i", data.peek(), data.peek(1));
    return false;
  }

  uint8_t byte_idx = 0;
  uint8_t first_byte = 0;
  readByte(&data, first_byte);

  bool is_first_message = (first_byte & FIRST_MESSAGE_BYTE0);

  if (is_first_message) {
    for (int i = 1; i < FRIGIDAIRE_STATE_LENGTH; ++i)
      this->received_state[i] = 0;
    this->received_state[0] = first_byte;
    byte_idx = 1;
    //    ESP_LOGI(TAG, "first");
  } else if (this->messages_received == 1) {
    this->received_state[8] = first_byte;
    byte_idx = 9;
    //    ESP_LOGI(TAG, "second");
  } else {
    //    ESP_LOGI(TAG, "Second message without first!");
    return false;
  }

  for (int i = 0; i < 3; ++i) {
    if (!readByte(&data, this->received_state[byte_idx++])) {
      //      ESP_LOGI(TAG, "Byte %u-%u fail", i, byte_idx-1);
      return false;
    }
  }

  if (!data.expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE)) {
    //    ESP_LOGI(TAG, "gap1");
    return false;
  }
  if (!data.expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ONE_SPACE)) {
    //    ESP_LOGI(TAG, "gap2");
    return false;
  }
  if (!data.expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_ZERO_SPACE)) {
    //    ESP_LOGI(TAG, "gap3");
    return false;
  }
  if (!data.expect_item(FRIGIDAIRE_BIT_MARK, FRIGIDAIRE_SMALLGAP_SPACE)) {
    //    ESP_LOGI(TAG, "gap4 %i %i", data.peek(), data.peek(1));
    return false;
  }

  for (int i = 0; i < 4; ++i) {
    if (!readByte(&data, this->received_state[byte_idx++])) {
      //      ESP_LOGI(TAG, "Byte5-8 fail");
      return false;
    }
  }

  //   Validate footer
  if (!data.expect_mark(FRIGIDAIRE_BIT_MARK)) {
    //    ESP_LOGI(TAG, "Footer Fail %i", data.peek());
    //    ESP_LOGI(TAG, "but continuing anyway...");
    //    return false;
  }

  if (is_first_message) {
    this->messages_received = 1;
    //    ESP_LOGI(TAG, "MSG 1 received");
    return false;
  } else {
    this->messages_received = 0;
    ESP_LOGI(TAG, "MSG 2 received");
    ESP_LOGI(TAG, "Got: %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X   %02X %02X %02X %02X",
             this->received_state[0], this->received_state[1], this->received_state[2], this->received_state[3],
             this->received_state[4], this->received_state[5], this->received_state[6], this->received_state[7],
             this->received_state[8], this->received_state[9], this->received_state[10], this->received_state[11],
             this->received_state[12], this->received_state[13], this->received_state[14], this->received_state[15]);
    return process_received_state();
  }
}

uint8_t FrigidaireClimate::calc_checksum(uint8_t *dst) {
  uint8_t result = 0;
  for (int i = 0; i < 4; ++i) {
    result += REVERSE_NIBBLE[(dst[i] & 0xF0) >> 4];
  }
  for (int i = 4; i < 7; ++i) {
    result += REVERSE_NIBBLE[dst[i] & 0x0F];
  }
  result += 10;
  result %= 16;
  result &= 0x0F;
  result = REVERSE_NIBBLE[result];
  return result;
}

bool FrigidaireClimate::process_received_state(){
  uint8_t temp = calc_checksum(&(this->received_state[0]));
  if (temp != this->received_state[7]){
    return false;
  }
  temp = calc_checksum(&(this->received_state[8]));
  if (temp != this->received_state[15]){
    return false;
  }

  if (!(POWER_BYTE0 & this->received_state[0]) && !(POWER_BYTE2 & this->received_state[2])){
    this->mode = climate::CLIMATE_MODE_OFF;
    this->publish_state();
    return true;
  }


  // Set received mode
  auto mode = this->received_state[0] & MODE_MASK_BYTE0;
  ESP_LOGV(TAG, "Mode: %02X", mode);
  switch (mode) {
    case MODE_FANONLY_BYTE0:
      this->mode = climate::CLIMATE_MODE_FAN_ONLY;
      break;
    case MODE_COOL_BYTE0:
      this->mode = climate::CLIMATE_MODE_COOL;
      break;
    case MODE_ECO_BYTE0:
      this->mode = climate::CLIMATE_MODE_HEAT_COOL;
      break;
  }

  // Set received temp
  if (CMD_TEMP_IS_F_BYTE3 & this->received_state[3]) { // If Fahrenheit

    const uint8_t tbyte1 = TEMP_MASK_BYTE1 & this->received_state[1];
    const uint8_t tbyte3 = TEMP_MASK_BYTE3 & this->received_state[3];
    const uint8_t tbyte10 = TEMP_MASK_BYTE10 & this->received_state[10];

    for (int i = 0; i < TEMP_TABLE_F_SIZE; i++) {

      if ((tbyte1 == CMD_TEMP_TABLE_F_BYTE1[i]) &&
          (tbyte3 == CMD_TEMP_TABLE_F_BYTE3[i]) &&
          (tbyte10 == CMD_TEMP_TABLE_F_BYTE10[i]))
      {
        this->target_temperature = (i + TEMP_F_OFFSET - 32.0) / 1.8;
        ESP_LOGI(TAG, "found f %u", i);
        break;
      }

    }

  } else {  // If Celsius

    const uint8_t tbyte1 = TEMP_MASK_BYTE1 & this->received_state[1];
    const uint8_t tbyte10 = TEMP_MASK_BYTE10 & this->received_state[10];

    for (int i = 0; i < TEMP_TABLE_C_SIZE; i++) {
      if ((tbyte1 == CMD_TEMP_TABLE_C_BYTE1[i]) &&
          (tbyte10 == CMD_TEMP_TABLE_C_BYTE10[i]))
      {
        this->target_temperature = i + TEMP_C_OFFSET;
        ESP_LOGI(TAG, "found c %u", i);
        break;
      }
    }
  }

  // Set received fan speed
  auto fan = this->received_state[0] & FAN_MASK_BYTE0;
  ESP_LOGVV(TAG, "Fan: %02X", fan);
  switch (fan) {
    case FAN_HIGH_BYTE0:
      this->fan_mode = climate::CLIMATE_FAN_HIGH;
      break;
    case FAN_MED_BYTE0:
      this->fan_mode = climate::CLIMATE_FAN_MEDIUM;
      break;
    case FAN_LOW_BYTE0:
      this->fan_mode = climate::CLIMATE_FAN_LOW;
      break;
    case FAN_AUTO_BYTE0:
    default:
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      break;
  }

  this->publish_state();
  return true;
}

}  // namespace frigidaire
}  // namespace esphome
