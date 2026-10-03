#pragma once

#include <deque>

#include "core/EventBus.hpp"

// =============================================================================
// SEMA — Registro de alarmas (persistente)
// =============================================================================
// D-0041 / README §87. Buffer acotado de eventos `Alarm` alimentado desde el
// Event Bus y persistido en LittleFS (JSONL) para sobrevivir a reinicios.

namespace sema {

class AlarmLog {
public:
  explicit AlarmLog(EventBus& bus, size_t maxEntries = 50);

  void begin(const char* path = "/alarms.jsonl");  // monta y carga existentes
  const std::deque<Event>& events() const { return events_; }

private:
  void onEvent(const Event& e);
  bool parseEvent(const String& line, Event& e);

  std::deque<Event> events_;
  size_t maxEntries_;
  String path_;
};

}  // namespace sema
