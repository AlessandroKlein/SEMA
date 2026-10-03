#include "core/Scheduler.hpp"

#include <Arduino.h>
#include <utility>

namespace sema {

void Scheduler::add(const char* name, uint32_t intervalMs, std::function<void()> fn) {
  tasks_.push_back({name, intervalMs, 0, std::move(fn)});
}

void Scheduler::run() {
  const uint32_t now = millis();
  for (Task& task : tasks_) {
    if (now - task.lastRun >= task.intervalMs) {
      task.lastRun = now;
      task.fn();
    }
  }
}

}  // namespace sema
