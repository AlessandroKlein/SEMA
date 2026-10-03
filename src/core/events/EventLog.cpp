#include "core/events/EventLog.hpp"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace sema {

EventLog::EventLog(EventBus& bus, size_t maxEntries) : maxEntries_(maxEntries) {
  // Suscripción a todos los tipos de evento (D-0008/§205).
  // TODO: usar un centinela Count en EventType para iterar de forma robusta.
  for (uint8_t i = 0; i <= static_cast<uint8_t>(EventType::Sleep); ++i) {
    bus.subscribe(static_cast<EventType>(i), [this](const Event& e) { onEvent(e); });
  }
}

void EventLog::begin(const char* path) {
  path_ = path;
  if (!LittleFS.begin(true)) {
    return;
  }

  File f = LittleFS.open(path_, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {
        continue;
      }
      Event e;
      if (parseEvent(line, e)) {
        events_.push_back(e);
        if (events_.size() > maxEntries_) {
          events_.pop_front();
        }
      }
    }
    f.close();
  }
}

void EventLog::onEvent(const Event& e) {
  events_.push_back(e);
  if (events_.size() > maxEntries_) {
    events_.pop_front();
  }

  DynamicJsonDocument doc(256);
  doc["ts"] = e.timestampMs;
  doc["type"] = eventTypeName(e.type);
  doc["source"] = e.source;
  doc["rule"] = e.correlationId;
  doc["severity"] = severityName(e.severity);
  doc["value"] = e.value;
  String line;
  serializeJson(doc, line);

  // TODO(D-0057): rotación del archivo (por tamaño/cantidad).
  File f = LittleFS.open(path_, "a");
  if (f) {
    f.println(line);
    f.close();
  }
}

bool EventLog::parseEvent(const String& line, Event& e) {
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, line)) {
    return false;
  }
  e.timestampMs = doc["ts"] | 0;
  e.type = parseEventType(doc["type"] | "system");
  e.source = doc["source"] | "";
  e.correlationId = doc["rule"] | "";
  e.severity = parseSeverity(doc["severity"] | "INFO");
  e.value = doc["value"] | 0;
  return true;
}

}  // namespace sema
