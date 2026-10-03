#include "core/storage/HistoryStore.hpp"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace sema {

namespace {

bool parseMeasurement(const String& line, Measurement& m) {
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, line)) {
    return false;
  }
  m.timestamp = doc["ts"] | 0;
  m.sensorId = doc["sensor"] | "";
  m.channelId = doc["channel"] | "";
  m.measurement = doc["measurement"] | "";
  m.value = doc["value"] | 0.0f;
  m.unit = doc["unit"] | "";
  m.quality = parseQuality(doc["quality"] | "VALID");
  m.sequence = doc["seq"] | 0;
  return true;
}

}  // namespace

bool HistoryStore::begin(const char* path) {
  path_ = path;
  if (!LittleFS.begin(true)) {
    return false;
  }

  // Cuenta las entradas ya existentes (una línea JSON por medición).
  count_ = 0;
  File f = LittleFS.open(path_, "r");
  if (f) {
    while (f.available()) {
      f.readStringUntil('\n');
      ++count_;
    }
    f.close();
  }
  return true;
}

bool HistoryStore::append(const Measurement& m) {
  if (count_ >= maxEntries_) {
    // TODO(D-0057): rotación por niveles (alta resolución + agregados) en lugar
    // de simplemente descartar.
    return false;
  }

  DynamicJsonDocument doc(256);
  doc["ts"] = m.timestamp;
  doc["sensor"] = m.sensorId;
  doc["channel"] = m.channelId;
  doc["measurement"] = m.measurement;
  doc["value"] = m.value;
  doc["unit"] = m.unit;
  doc["quality"] = qualityName(m.quality);
  doc["seq"] = m.sequence;

  String line;
  serializeJson(doc, line);

  File f = LittleFS.open(path_, "a");
  if (!f) {
    return false;
  }
  f.println(line);
  f.close();

  ++count_;
  return true;
}

bool HistoryStore::readRecent(std::deque<Measurement>& out, size_t maxCount) {
  out.clear();

  File f = LittleFS.open(path_, "r");
  if (!f) {
    return false;
  }

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      continue;
    }
    Measurement m;
    if (parseMeasurement(line, m)) {
      out.push_back(m);
      if (out.size() > maxCount) {
        out.pop_front();
      }
    }
  }
  f.close();
  return true;
}

}  // namespace sema
