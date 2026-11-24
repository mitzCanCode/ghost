#ifndef ETHERNET_PROJECT_SHARED_H
#define ETHERNET_PROJECT_SHARED_H

#include <Arduino.h>

enum class DeviceStatus : uint8_t {
  NotApplicable,
  On,
  Off
};

enum class StorageStatus : uint8_t {
  NotApplicable,
  In,
  Out
};

const uint8_t ETH_CS = 10;
const uint8_t SD_CS = 4;
const uint16_t MAX_FRAME = 1520;

#endif  // ETHERNET_PROJECT_SHARED_H
