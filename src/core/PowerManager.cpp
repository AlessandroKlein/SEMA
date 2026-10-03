#include "core/PowerManager.hpp"

#include "esp_sleep.h"

namespace sema {

PowerManager& PowerManager::instance() {
  static PowerManager mgr;
  return mgr;
}

void PowerManager::sleep(uint64_t seconds) {
  // Wake por timer RTC (D-0021). Otras fuentes (GPIO/INT lluvia) se añaden luego.
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
  esp_deep_sleep_start();
}

uint32_t PowerManager::wakeReason() const {
  return static_cast<uint32_t>(esp_sleep_get_wakeup_cause());
}

}  // namespace sema
