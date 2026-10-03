#pragma once

#include <deque>

#include "core/EventBus.hpp"

// =============================================================================
// SEMA — Registro reciente de alarmas
// =============================================================================
// D-0041 / README §87. Buffer acotado de eventos `Alarm`, alimentado desde el
// Event Bus para consulta por API.

namespace sema {

class AlarmLog {
public:
  explicit AlarmLog(EventBus& bus, size_t maxEntries = 50);

  const std::deque<Event>& events() const { return events_; }

private:
  void onEvent(const Event& e);

  std::deque<Event> events_;
  size_t maxEntries_;
};

}  // namespace sema
