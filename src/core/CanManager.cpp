#include "core/CanManager.hpp"

#include <Arduino.h>
#include <driver/twai.h>
#include <string.h>

namespace sema {

static twai_timing_config_t timingFor(uint32_t speed) {
  switch (speed) {
    case 125000: return TWAI_TIMING_CONFIG_125KBITS();
    case 250000: return TWAI_TIMING_CONFIG_250KBITS();
    case 1000000: return TWAI_TIMING_CONFIG_1MBITS();
    default: return TWAI_TIMING_CONFIG_500KBITS();
  }
}

void CanManager::apply(const CanConfig& cfg) {
  cfg_ = cfg;
  ready_ = false;
  if (!cfg_.enabled) {
    return;
  }

  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(
      static_cast<gpio_num_t>(cfg_.txPin), static_cast<gpio_num_t>(cfg_.rxPin),
      TWAI_MODE_NORMAL);
  const twai_timing_config_t t = timingFor(cfg_.speed);
  const twai_filter_config_t f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g, &t, &f) == ESP_OK) {
    twai_start();
    ready_ = true;
  }
}

bool CanManager::send(uint32_t id, const uint8_t* data, uint8_t dlc, bool extd) {
  if (!ready_ || dlc > 8) {
    return false;
  }
  twai_message_t msg = {};
  msg.identifier = id;
  msg.extd = extd ? 1 : 0;
  msg.data_length_code = dlc;
  if (data != nullptr && dlc > 0) {
    memcpy(msg.data, data, dlc);
  }
  return twai_transmit(&msg, pdMS_TO_TICKS(1000)) == ESP_OK;
}

bool CanManager::receive(uint32_t& id, uint8_t* data, uint8_t& dlc, bool& extd) {
  if (!ready_) {
    return false;
  }
  twai_message_t msg;
  if (twai_receive(&msg, pdMS_TO_TICKS(0)) != ESP_OK) {
    return false;
  }
  id = msg.identifier;
  extd = (msg.extd == 1);
  dlc = msg.data_length_code;
  if (data != nullptr && dlc > 0) {
    memcpy(data, msg.data, dlc);
  }
  return true;
}

}  // namespace sema
