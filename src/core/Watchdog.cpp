#include "core/Watchdog.hpp"

#include "esp_task_wdt.h"

namespace sema {

bool Watchdog::begin(uint32_t timeoutSeconds) {
  // Si el core de Arduino ya inicializó el TWDT, ESP_ERR_INVALID_STATE es válido:
  // se conserva su configuración y solo se añade la tarea actual (loopTask).
  const esp_err_t err = esp_task_wdt_init(timeoutSeconds, true);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    return false;
  }
  started_ = (esp_task_wdt_add(NULL) == ESP_OK);
  return started_;
}

void Watchdog::feed() {
  if (started_) {
    esp_task_wdt_reset();
  }
}

}  // namespace sema
