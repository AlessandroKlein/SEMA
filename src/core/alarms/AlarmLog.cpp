#include "core/alarms/AlarmLog.hpp"

namespace sema {

AlarmLog::AlarmLog(EventBus& bus, size_t maxEntries) : maxEntries_(maxEntries) {
  bus.subscribe(EventType::Alarm, [this](const Event& e) { onEvent(e); });
}

void AlarmLog::onEvent(const Event& e) {
  events_.push_back(e);
  if (events_.size() > maxEntries_) {
    events_.pop_front();
  }
}

}  // namespace sema
