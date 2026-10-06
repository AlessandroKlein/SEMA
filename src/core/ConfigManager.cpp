#include "core/ConfigManager.hpp"

#include <ArduinoJson.h>

namespace sema {

static const char* kConfigKey = "config";

ConfigManager::ConfigManager(KeyValueStore& store) : store_(store) {}

bool ConfigManager::load() {
  String raw;
  if (store_.getString(kConfigKey, raw) && raw.length() > 0) {
    if (deserialize(raw) && validate(config_)) {
      valid_ = true;
      return true;
    }
  }

  // Defaults (schema=1).
  config_ = Config{};
  config_.schemaVersion = 1;
  config_.station.id = "SEMA-001";
  config_.station.name = "Estación Norte";
  config_.network.mode = "STA";
  config_.network.hostname = "sema-001";
  config_.network.mdns = true;
  config_.system.timezone = "America/Argentina/Buenos_Aires";
  config_.system.logLevel = "INFO";
  config_.storage.backend = "littlefs";
  config_.storage.retentionDays = 30;

  valid_ = validate(config_);
  return valid_;
}

bool ConfigManager::save() {
  String raw;
  if (!serialize(raw)) {
    return false;
  }
  return store_.putString(kConfigKey, raw.c_str());
}

bool ConfigManager::apply(const Config& next) {
  if (!validate(next)) {
    return false;
  }
  backup_ = config_;
  config_ = next;
  if (!save()) {
    config_ = backup_;  // rollback
    return false;
  }
  valid_ = true;
  return true;
}

bool ConfigManager::validate(const Config& c) const {
  if (c.schemaVersion != 1) {
    return false;
  }
  if (c.station.id.length() == 0) {
    return false;
  }
  if (c.network.mode != "STA" && c.network.mode != "AP") {
    return false;
  }
  if (c.storage.backend != "littlefs" && c.storage.backend != "flash" &&
      c.storage.backend != "sd") {
    return false;
  }
  return true;
}

bool ConfigManager::serialize(String& out) const {
  DynamicJsonDocument doc(2048);
  doc["schema_version"] = config_.schemaVersion;
  doc["station"]["id"] = config_.station.id;
  doc["station"]["name"] = config_.station.name;
  doc["network"]["mode"] = config_.network.mode;
  doc["network"]["ssid"] = config_.network.ssid;
  doc["network"]["password"] = config_.network.password;
  doc["network"]["hostname"] = config_.network.hostname;
  doc["network"]["mdns"] = config_.network.mdns;
  doc["system"]["timezone"] = config_.system.timezone;
  doc["system"]["log_level"] = config_.system.logLevel;
  doc["storage"]["backend"] = config_.storage.backend;
  doc["storage"]["retention_days"] = config_.storage.retentionDays;
  doc["security"]["api_key"] = config_.security.apiKey;
  doc["security"]["server_key"] = config_.security.serverKey;
  doc["energy"]["rain_pin"] = config_.energy.rainPin;
  doc["publishers"]["webhook_url"] = config_.publishers.webhookUrl;
  doc["publishers"]["mqtt_host"] = config_.publishers.mqttHost;
  doc["publishers"]["mqtt_port"] = config_.publishers.mqttPort;
  doc["publishers"]["mqtt_topic"] = config_.publishers.mqttTopic;
  doc["publishers"]["mqtt_user"] = config_.publishers.mqttUser;
  doc["publishers"]["mqtt_pass"] = config_.publishers.mqttPass;
  JsonArray rules = doc.createNestedArray("rules");
  for (const RuleSpec& r : config_.rules) {
    JsonObject o = rules.createNestedObject();
    o["name"] = r.name;
    o["sensor_id"] = r.sensorId;
    o["channel_id"] = r.channelId;
    o["op"] = r.op;
    o["value"] = r.value;
  }
  JsonArray calibrations = doc.createNestedArray("calibrations");
  for (const CalibrationSpec& c : config_.calibrations) {
    JsonObject o = calibrations.createNestedObject();
    o["sensor_id"] = c.sensorId;
    o["channel_id"] = c.channelId;
    o["gain"] = c.gain;
    o["offset"] = c.offset;
    o["has_range"] = c.hasRange;
    o["min"] = c.min;
    o["max"] = c.max;
  }
  JsonArray sensors = doc.createNestedArray("sensors");
  for (const SensorSpec& s : config_.sensors) {
    JsonObject o = sensors.createNestedObject();
    o["id"] = s.id;
    o["model"] = s.model;
    o["sda"] = s.sda;
    o["scl"] = s.scl;
    o["pin"] = s.pin;
    o["rx"] = s.rxPin;
    o["tx"] = s.txPin;
    o["channel"] = s.channel;
    o["unit"] = s.unit;
    o["scale"] = s.scale;
    o["offset"] = s.offset;
  }
  JsonArray gpio = doc.createNestedArray("gpio");
  for (const GpioSpec& g : config_.gpio) {
    JsonObject o = gpio.createNestedObject();
    o["id"] = g.id;
    o["pin"] = g.pin;
    o["mode"] = g.mode;
    o["initial"] = g.initial;
    o["expander_addr"] = g.expanderAddr;
  }
  doc["shift_register"]["type"] = config_.shiftRegister.type;
  doc["shift_register"]["data_pin"] = config_.shiftRegister.dataPin;
  doc["shift_register"]["clock_pin"] = config_.shiftRegister.clockPin;
  doc["shift_register"]["latch_pin"] = config_.shiftRegister.latchPin;
  serializeJson(doc, out);
  return true;
}

bool ConfigManager::parseInto(const String& in, Config& c) {
  DynamicJsonDocument doc(2048);
  const DeserializationError err = deserializeJson(doc, in);
  if (err) {
    return false;
  }
  c.schemaVersion = doc["schema_version"] | 1;
  c.station.id = doc["station"]["id"] | "SEMA-001";
  c.station.name = doc["station"]["name"] | "Estación Norte";
  c.network.mode = doc["network"]["mode"] | "STA";
  c.network.ssid = doc["network"]["ssid"] | "";
  c.network.password = doc["network"]["password"] | "";
  c.network.hostname = doc["network"]["hostname"] | "sema-001";
  c.network.mdns = doc["network"]["mdns"] | true;
  c.system.timezone = doc["system"]["timezone"] | "America/Argentina/Buenos_Aires";
  c.system.logLevel = doc["system"]["log_level"] | "INFO";
  c.storage.backend = doc["storage"]["backend"] | "littlefs";
  c.storage.retentionDays = doc["storage"]["retention_days"] | 30;
  c.security.apiKey = doc["security"]["api_key"] | "";
  c.security.serverKey = doc["security"]["server_key"] | "";
  c.energy.rainPin = doc["energy"]["rain_pin"] | 0;
  c.publishers.webhookUrl = doc["publishers"]["webhook_url"] | "";
  c.publishers.mqttHost = doc["publishers"]["mqtt_host"] | "";
  c.publishers.mqttPort = doc["publishers"]["mqtt_port"] | 1883;
  c.publishers.mqttTopic = doc["publishers"]["mqtt_topic"] | "sema/measurement";
  c.publishers.mqttUser = doc["publishers"]["mqtt_user"] | "";
  c.publishers.mqttPass = doc["publishers"]["mqtt_pass"] | "";
  c.rules.clear();
  for (JsonObject o : doc["rules"].as<JsonArray>()) {
    RuleSpec r;
    r.name = o["name"] | "";
    r.sensorId = o["sensor_id"] | "";
    r.channelId = o["channel_id"] | "";
    r.op = o["op"] | "gt";
    r.value = o["value"] | 0.0f;
    c.rules.push_back(r);
  }
  c.calibrations.clear();
  for (JsonObject o : doc["calibrations"].as<JsonArray>()) {
    CalibrationSpec cal;
    cal.sensorId = o["sensor_id"] | "";
    cal.channelId = o["channel_id"] | "";
    cal.gain = o["gain"] | 1.0f;
    cal.offset = o["offset"] | 0.0f;
    cal.hasRange = o["has_range"] | false;
    cal.min = o["min"] | 0.0f;
    cal.max = o["max"] | 0.0f;
    c.calibrations.push_back(cal);
  }
  c.sensors.clear();
  for (JsonObject o : doc["sensors"].as<JsonArray>()) {
    SensorSpec s;
    s.id = o["id"] | "";
    s.model = o["model"] | "";
    s.sda = o["sda"] | 21;
    s.scl = o["scl"] | 22;
    s.pin = o["pin"] | 0;
    s.rxPin = o["rx"] | 0;
    s.txPin = o["tx"] | 0;
    s.channel = o["channel"] | "";
    s.unit = o["unit"] | "";
    s.scale = o["scale"] | 1.0f;
    s.offset = o["offset"] | 0.0f;
    c.sensors.push_back(s);
  }
  c.gpio.clear();
  for (JsonObject o : doc["gpio"].as<JsonArray>()) {
    GpioSpec g;
    g.id = o["id"] | "";
    g.pin = o["pin"] | 0;
    g.mode = o["mode"] | "input";
    g.initial = o["initial"] | 0;
    g.expanderAddr = o["expander_addr"] | 0;
    c.gpio.push_back(g);
  }
  c.shiftRegister.type = doc["shift_register"]["type"] | "74HC595";
  c.shiftRegister.dataPin = doc["shift_register"]["data_pin"] | 0;
  c.shiftRegister.clockPin = doc["shift_register"]["clock_pin"] | 0;
  c.shiftRegister.latchPin = doc["shift_register"]["latch_pin"] | 0;
  return true;
}

bool ConfigManager::deserialize(const String& in) {
  return parseInto(in, config_);
}

bool ConfigManager::applyJson(const String& json) {
  Config next;
  if (!parseInto(json, next)) {
    return false;
  }
  return apply(next);
}

}  // namespace sema
