#include "core/runtime/Task.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace sema {

Task::Task(TaskFunction fn, const char* name, uint32_t stackBytes, uint32_t priority, void* arg)
    : fn_(fn), name_(name), stack_(stackBytes), priority_(priority), arg_(arg) {}

Task::~Task() {
  stop();
}

bool Task::start() {
  if (handle_ != nullptr) {
    return false;
  }
  // xTaskCreate usa afinidad AUTO (tskNO_AFFINITY) por defecto (D-0053).
  const BaseType_t result = xTaskCreate(fn_, name_, stack_, arg_, priority_, &handle_);
  return result == pdPASS;
}

void Task::stop() {
  if (handle_ != nullptr) {
    vTaskDelete(static_cast<TaskHandle_t>(handle_));
    handle_ = nullptr;
  }
}

}  // namespace sema
