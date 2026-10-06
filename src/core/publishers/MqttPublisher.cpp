#include "core/publishers/MqttPublisher.hpp"

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

namespace sema {

static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);

MqttPublisher::MqttPublisher(const char* id, const char* host, uint16_t port,
                             const char* topic)
    : id_(id), host_(host), port_(port), topic_(topic) {}

void MqttPublisher::configure(const char* host, uint16_t port, const char* topic) {
  host_ = host;
  port_ = port;
  topic_ = topic;
}

const char* MqttPublisher::id() const { return id_; }

bool MqttPublisher::enabled() const { return host_.length() > 0; }

bool MqttPublisher::publish(const Measurement& m) {
  if (!enabled()) {
    return false;
  }

  if (!mqtt.connected()) {
    mqtt.setServer(host_.c_str(), port_);
    mqtt.connect(id_);  // TODO(D-0048): usuario/contraseña desde configuración.
  }
  if (!mqtt.connected()) {
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

  return mqtt.publish(topic_.c_str(), body.c_str());
}

}  // namespace sema
