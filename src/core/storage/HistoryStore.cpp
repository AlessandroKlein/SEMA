#include "core/storage/HistoryStore.hpp"

#include <ArduinoJson.h>
#include <SD.h>

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
  // El histórico vive en la microSD (SPI). Sin SD activa no se almacena nada:
  // las gráficas quedan vacías para no gastar memoria interna (ver enableSd).
  count_ = 0;
  return true;
}

bool HistoryStore::enableSd(uint8_t csPin) {
  // SD.begin(cs) usa el bus SPI por defecto (VSPI/FSPI) con su chip-select.
  sdEnabled_ = SD.begin(csPin);
  if (sdEnabled_) {
    // Cuenta las entradas ya existentes (una línea JSON por medición).
    count_ = 0;
    File f = SD.open(path_, "r");
    if (f) {
      while (f.available()) {
        f.readStringUntil('\n');
        ++count_;
      }
      f.close();
    }
  }
  return sdEnabled_;
}

bool HistoryStore::rotate() {
  if (!sdEnabled_) {
    return false;
  }
  // D-0057: conserva la mitad más reciente y reescribe el archivo.
  std::deque<String> lines;
  File f = SD.open(path_, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        lines.push_back(line);
      }
    }
    f.close();
  }

  const size_t keep = lines.size() / 2;
  if (keep == lines.size()) {
    return true;  // nada que rotar
  }
  const size_t skip = lines.size() - keep;

  File w = SD.open(path_, "w");  // trunca
  if (!w) {
    return false;
  }
  size_t idx = 0;
  for (const String& line : lines) {
    if (idx++ < skip) {
      continue;
    }
    w.println(line);
  }
  w.close();

  count_ = keep;
  return true;
}

bool HistoryStore::append(const Measurement& m) {
  if (!sdEnabled_) {
    return false;  // sin SD → no se guarda histórico (ahorrar memoria)
  }
  if (count_ >= maxEntries_) {
    if (!rotate()) {
      return false;
    }
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

  File f = SD.open(path_, "a");
  if (!f) {
    return false;
  }
  f.println(line);
  f.close();

  ++count_;
  return true;
}

bool HistoryStore::prune(uint32_t nowEpoch) {
  if (!sdEnabled_ || retentionSeconds_ == 0) {
    return true;
  }
  const uint32_t cutoff = nowEpoch - retentionSeconds_;
  std::deque<String> keep;
  File f = SD.open(path_, "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {
        continue;
      }
      Measurement m;
      if (parseMeasurement(line, m) && m.timestamp >= cutoff) {
        keep.push_back(line);
      }
    }
    f.close();
  }

  File w = SD.open(path_, "w");  // trunca
  if (!w) {
    return false;
  }
  for (const String& line : keep) {
    w.println(line);
  }
  w.close();
  count_ = keep.size();
  return true;
}

bool HistoryStore::readRecent(std::deque<Measurement>& out, size_t maxCount) {
  out.clear();
  if (!sdEnabled_) {
    return false;  // sin SD → sin histórico
  }

  File f = SD.open(path_, "r");
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
