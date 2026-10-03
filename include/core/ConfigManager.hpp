#pragma once

#include <Arduino.h>
#include <cstdint>

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

// Configuración completa (schema=1).
struct Config {
  uint32_t schemaVersion = 1;
  StationConfig station;
  NetworkConfig network;
  SystemConfig system;
  StorageConfig storage;
};

class ConfigManager {
public:
  explicit ConfigManager(KeyValueStore& store);

  bool load();                    // carga desde NVS o aplica defaults
  bool save();                    // persiste el estado actual
  bool apply(const Config& next); // valida + aplica + persiste + rollback

  const Config& get() const { return config_; }
  bool valid() const { return valid_; }

private:
  bool validate(const Config& c) const;
  bool serialize(String& out) const;
  bool deserialize(const String& in);

  KeyValueStore& store_;
  Config config_;
  Config backup_;
  bool valid_ = false;
};

}  // namespace sema
