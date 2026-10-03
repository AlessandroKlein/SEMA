#include "core/web/HttpServer.hpp"

#include <ArduinoJson.h>
#include <esp_system.h>

#include "core/SemaCore.hpp"

namespace sema {

void HttpServer::begin(SemaCore& core) {
  core_ = &core;

  server_.on("/api/v1/status", HTTP_GET, [this]() { onStatus(); });
  server_.on("/api/v1/health", HTTP_GET, [this]() { onHealth(); });
  server_.on("/api/v1/system", HTTP_GET, [this]() { onSystem(); });
  server_.on("/api/v1/config", HTTP_GET, [this]() { onConfig(); });
  server_.on("/api/v1/config", HTTP_PUT, [this]() { onConfigPut(); });
  server_.on("/api/v1/restart", HTTP_POST, [this]() { onRestart(); });
  server_.on("/api/v1/diagnostics", HTTP_GET, [this]() { onDiagnostics(); });
  server_.on("/api/v1/sensors", HTTP_GET, [this]() { onSensors(); });
  server_.on("/api/v1/history", HTTP_GET, [this]() { onHistory(); });
  server_.on("/api/v1/alarms", HTTP_GET, [this]() { onAlarms(); });
  server_.onNotFound([this]() { onNotFound(); });

  static const char* kHeaders[] = {"X-API-Key"};
  server_.collectHeaders(kHeaders, 1);

  server_.begin();
  ws_.begin();
}

void HttpServer::loop() {
  server_.handleClient();
  ws_.loop();
}

void HttpServer::broadcastMeasurements(const std::vector<Measurement>& measurements) {
  if (ws_.connectedClients() == 0) {
    return;
  }

  DynamicJsonDocument doc(8192);
  doc["type"] = "measurements";
  JsonArray arr = doc.createNestedArray("data");
  for (const Measurement& m : measurements) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }
  String out;
  serializeJson(doc, out);
  ws_.broadcastTXT(out.c_str());
}

void HttpServer::onStatus() {
  DynamicJsonDocument doc(256);
  doc["station"] = core_->config().get().station.id;
  doc["name"] = core_->config().get().station.name;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["uptime_s"] = millis() / 1000;
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onHealth() {
  DynamicJsonDocument doc(384);
  doc["status"] = "HEALTHY";
  doc["uptime_s"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["sensors"]["total"] = core_->sensors().count();
  doc["sensors"]["online"] = core_->sensors().onlineCount();
  doc["sensors"]["error"] = core_->sensors().count() - core_->sensors().onlineCount();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onSystem() {
  DynamicJsonDocument doc(256);
  doc["id"] = core_->config().get().station.id;
  doc["name"] = core_->config().get().station.name;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["hw"] = SEMA_HW_VERSION;
  doc["config_schema"] = SEMA_CONFIG_SCHEMA_VERSION;
  doc["protocol"] = SEMA_PROTOCOL_VERSION;
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onConfig() {
  String out;
  if (core_->config().toJson(out)) {
    server_.send(200, "application/json", out);
  } else {
    server_.send(500, "application/json", "{\"error\":\"serialization failed\"}");
  }
}

bool HttpServer::authorized() {
  const String key = core_->config().get().security.apiKey;
  if (key.length() == 0) {
    return true;  // sin clave configurada → permitir (primera configuración)
  }
  return server_.hasHeader("X-API-Key") && server_.header("X-API-Key") == key;
}

void HttpServer::onRestart() {
  if (!authorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
  delay(100);
  ESP.restart();
}

void HttpServer::onConfigPut() {
  if (!authorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"body required\"}");
    return;
  }
  const String body = server_.arg("plain");
  if (core_->config().applyJson(body)) {
    server_.send(200, "application/json", "{\"ok\":true}");
  } else {
    server_.send(400, "application/json", "{\"error\":\"invalid config\"}");
  }
}

void HttpServer::onDiagnostics() {
  DynamicJsonDocument doc(1024);
  doc["uptime_s"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["reset_reason"] = static_cast<int>(esp_reset_reason());
  doc["history"]["entries"] = core_->history().count();
  doc["history"]["max"] = core_->history().maxEntries();
  doc["tasks"] = 0;

  JsonArray i2c = doc.createNestedArray("i2c_devices");
  for (const DetectedDevice& d : core_->detectedDevices()) {
    JsonObject o = i2c.createNestedObject();
    char addr[8];
    snprintf(addr, sizeof(addr), "0x%02X", d.address);
    o["address"] = addr;
    o["model"] = d.model;
  }

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onSensors() {
  DynamicJsonDocument doc(2048);

  JsonArray catalog = doc.createNestedArray("catalog");
  std::vector<SensorInfo> info;
  core_->sensors().describe(info);
  for (const SensorInfo& s : info) {
    JsonObject o = catalog.createNestedObject();
    o["id"] = s.id;
    o["model"] = s.model;
    o["interface"] = s.interface;
    o["healthy"] = s.healthy;
  }

  JsonArray arr = doc.createNestedArray("measurements");
  for (const Measurement& m : core_->sensors().measurements()) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onHistory() {
  size_t limit = 50;
  if (server_.hasArg("limit")) {
    const long l = server_.arg("limit").toInt();
    if (l > 0 && l <= 100) {
      limit = static_cast<size_t>(l);
    }
  }

  std::deque<Measurement> items;
  core_->history().readRecent(items, limit);

  DynamicJsonDocument doc(16384);
  JsonArray arr = doc.createNestedArray("history");
  for (const Measurement& m : items) {
    JsonObject o = arr.createNestedObject();
    o["ts"] = m.timestamp;
    o["sensor"] = m.sensorId;
    o["channel"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["seq"] = m.sequence;
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onAlarms() {
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("alarms");
  for (const Event& e : core_->alarms().events()) {
    JsonObject o = arr.createNestedObject();
    o["ts"] = e.timestampMs;
    o["source"] = e.source;
    o["rule"] = e.correlationId;
    o["severity"] = severityName(e.severity);
    o["value"] = e.value;
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onNotFound() {
  server_.send(404, "application/json", "{\"error\":\"not found\"}");
}

}  // namespace sema
