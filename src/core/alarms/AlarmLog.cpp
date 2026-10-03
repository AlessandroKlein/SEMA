#include "core/alarms/AlarmLog.hpp"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace sema {

AlarmLog::AlarmLog(EventBus& bus, size_t maxEntries) : maxEntries_(maxEntries) {
  bus.subscribe(EventType::Alarm, [this](const Event& e) { onEvent(e); });
}

void AlarmLog::begin(const char* path) {
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

void AlarmLog::onEvent(const Event& e) {
  events_.push_back(e);
  if (events_.size() > maxEntries_) {
    events_.pop_front();
  }

  DynamicJsonDocument doc(256);
  doc["ts"] = e.timestampMs;
  doc["source"] = e.source;
  doc["rule"] = e.correlationId;
  doc["severity"] = severityName(e.severity);
  doc["value"] = e.value;
  String line;
  serializeJson(doc, line);

  // TODO(D-0057): rotación del archivo (por tamaño/cantidad) en lugar de crecer.
  File f = LittleFS.open(path_, "a");
  if (f) {
    f.println(line);
    f.close();
  }
}

bool AlarmLog::parseEvent(const String& line, Event& e) {
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, line)) {
    return false;
  }
  e.timestampMs = doc["ts"] | 0;
  e.source = doc["source"] | "";
  e.correlationId = doc["rule"] | "";
  e.severity = parseSeverity(doc["severity"] | "WARNING");
  e.type = EventType::Alarm;
  e.value = doc["value"] | 0;
  return true;
}

}  // namespace sema
