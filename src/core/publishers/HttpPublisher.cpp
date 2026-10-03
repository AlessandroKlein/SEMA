#include "core/publishers/HttpPublisher.hpp"

#include <ArduinoJson.h>
#include <HTTPClient.h>

namespace sema {

HttpPublisher::HttpPublisher(const char* id, const char* url) : id_(id), url_(url) {}

const char* HttpPublisher::id() const { return id_; }

bool HttpPublisher::enabled() const { return url_.length() > 0; }

bool HttpPublisher::publish(const Measurement& m) {
  if (!enabled()) {
    return false;
  }

  DynamicJsonDocument doc(256);
  doc["station_id"] = m.stationId;
  doc["sensor_id"] = m.sensorId;
  doc["channel_id"] = m.channelId;
  doc["measurement"] = m.measurement;
  doc["value"] = m.value;
  doc["unit"] = m.unit;
  doc["quality"] = qualityName(m.quality);
  doc["sequence"] = m.sequence;
  doc["timestamp"] = m.timestamp;
  String body;
  serializeJson(doc, body);

  HTTPClient http;
  http.begin(url_);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(2000);  // corto para no bloquear (D-0010)
  const int code = http.POST(body);
  http.end();
  return code > 0 && code < 400;
}

}  // namespace sema
