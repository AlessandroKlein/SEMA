#pragma once

#include <Arduino.h>
#include <cstdint>

// =============================================================================
// SEMA — Storage API (abstracción del Core)
// =============================================================================
// D-0003 / D-0046. El Core conoce el concepto de almacenamiento sin depender de
// un medio concreto: NVS, Flash partition, LittleFS, SD o externo son backends.
// La disponibilidad de SD nunca es requisito.

namespace sema {

// Almacenamiento clave-valor mínimo. Es la frontera que usan los servicios del
// Core (ConfigManager, logs, histórico) para no acoplarse al backend.
class KeyValueStore {
public:
  virtual ~KeyValueStore() = default;

  virtual bool begin(const char* name) = 0;

  virtual bool getString(const char* key, String& out) = 0;
  virtual bool putString(const char* key, const char* value) = 0;

  virtual bool getUInt(const char* key, uint32_t& out) = 0;
  virtual bool putUInt(const char* key, uint32_t value) = 0;

  virtual bool clear() = 0;
};

}  // namespace sema
