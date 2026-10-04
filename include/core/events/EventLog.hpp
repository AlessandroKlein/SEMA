#pragma once

#include <deque>

#include "core/EventBus.hpp"

// =============================================================================
// SEMA — Registro de eventos (persistente)
// =============================================================================
// D-0008 / D-0041 / README §87, §111. Buffer acotado de todos los tipos de
// evento, alimentado desde el Event Bus y persistido en LittleFS (JSONL).

namespace sema {

class EventLog {
public:
  explicit EventLog(EventBus& bus, size_t maxEntries = 100);

  void begin(const char* path = "/events.jsonl");  // monta y carga existentes
  const std::deque<Event>& events() const { return events_; }

private:
  void onEvent(const Event& e);
  bool parseEvent(const String& line, Event& e);
  bool rotate();

  std::deque<Event> events_;
  size_t maxEntries_;
  String path_;
  size_t fileCount_ = 0;
};

}  // namespace sema
