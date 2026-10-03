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
  DynamicJsonDocument doc(1024);
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
  serializeJson(doc, out);
  return true;
}

bool ConfigManager::parseInto(const String& in, Config& c) {
  DynamicJsonDocument doc(1024);
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
