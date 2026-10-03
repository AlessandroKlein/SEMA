#include "core/PowerManager.hpp"

#include "driver/gpio.h"
#include "esp_sleep.h"

namespace sema {

PowerManager& PowerManager::instance() {
  static PowerManager mgr;
  return mgr;
}

void PowerManager::sleep(uint64_t seconds) {
  // Wake por timer RTC (D-0021).
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
  esp_deep_sleep_start();
}

void PowerManager::enableRainWakeup(uint8_t pin) {
  // Wake por GPIO (D-0022): pluviómetro de cangilones. Requiere pin RTC-capable
  // (p. ej. GPIO4/5/…); nivel HIGH en el pulso.
  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(pin), 1);
}

uint32_t PowerManager::wakeReason() const {
  return static_cast<uint32_t>(esp_sleep_get_wakeup_cause());
}

}  // namespace sema
