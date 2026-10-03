#pragma once

#include <Arduino.h>
#include <cstdint>
#include <vector>

#include "core/storage/Storage.hpp"

// =============================================================================
// SEMA — ConfigManager (configuración transaccional, schema=1)
// =============================================================================
// D-0042 / D-0023 / D-0024. Configuración jerárquica versionada, persistida en
// NVS a través del Storage API. Aplica con validación previa y rollback.

namespace sema {

struct StationConfig {
  String id;
  String name;
};

struct NetworkConfig {
  String mode;      // "STA" | "AP"
  String ssid;
  String password;
  String hostname;
  bool mdns;
};

struct SystemConfig {
  String timezone;
  String logLevel;
};

struct StorageConfig {
  String backend;   // "littlefs" | "flash" | "sd"
  uint32_t retentionDays;
};

struct SecurityConfig {
  String apiKey;     // clave de la web local (D-0048); vacía = sin autenticación
  String serverKey;  // clave del Servidor Central → SEMA (config de riesgo, vía API)
};

struct EnergyConfig {
  uint8_t rainPin = 0;  // GPIO del pluviómetro (D-0022); 0 = deshabilitado
};

struct PublishersConfig {
  String webhookUrl;   // vacío = deshabilitado
  String mqttHost;     // vacío = deshabilitado
  uint16_t mqttPort = 1883;
  String mqttTopic = "sema/measurement";
};

// Especificación de un sensor (D-0042): el catálogo se define por configuración.
struct SensorSpec {
  String id;
  String model;      // "BME280" | "SHT40" | "DS18B20" | "BH1750" | "AHT20" | "ADC"
  uint8_t sda = 21;
  uint8_t scl = 22;
  uint8_t pin = 0;
  String channel;    // para sensores analógicos (p. ej. "voltage")
  String unit;       // unidad (p. ej. "V")
  float scale = 1.0f;
  float offset = 0.0f;
};

// Configuración completa (schema=1).
struct Config {
  uint32_t schemaVersion = 1;
  StationConfig station;
  NetworkConfig network;
  SystemConfig system;
  StorageConfig storage;
  SecurityConfig security;
  EnergyConfig energy;
  PublishersConfig publishers;
  std::vector<SensorSpec> sensors;  // vacío = usar catálogo por defecto
};

class ConfigManager {
public:
  explicit ConfigManager(KeyValueStore& store);

  bool load();                    // carga desde NVS o aplica defaults
  bool save();                    // persiste el estado actual
  bool apply(const Config& next); // valida + aplica + persiste + rollback

  const Config& get() const { return config_; }
  bool valid() const { return valid_; }
  bool toJson(String& out) const { return serialize(out); }
  bool applyJson(const String& json);  // parsea y aplica transaccionalmente

private:
  bool validate(const Config& c) const;
  bool serialize(String& out) const;
  bool parseInto(const String& in, Config& c);
  bool deserialize(const String& in);

  KeyValueStore& store_;
  Config config_;
  Config backup_;
  bool valid_ = false;
};

}  // namespace sema
