//
// Created by robert on 7/4/24.
//

#ifndef ESPHOME_DEV_CRC8_H
#define ESPHOME_DEV_CRC8_H

#include <cstdint>
#include <vector>

namespace esphome {
namespace balboa {

uint8_t crc8(std::vector<uint8_t> data);
uint8_t crc8(uint8_t *data, size_t len);

}  // namespace balboa
}  // namespace esphome

#endif  // ESPHOME_DEV_CRC8_H
