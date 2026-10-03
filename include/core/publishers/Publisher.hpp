#pragma once

#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Interfaz de publicador externo
// =============================================================================
// D-0010 / README §174. Cada servicio externo (ThingSpeak, Windy, MQTT, HTTP
// genérico, …) es un publicador independiente que consume el Modelo Canónico,
// sin acoplarse a la adquisición ni al resto del sistema.

namespace sema {

class Publisher {
public:
  virtual ~Publisher() = default;

  virtual const char* id() const = 0;
  virtual bool enabled() const = 0;         // false → no publica
  virtual bool publish(const Measurement& m) = 0;
};

}  // namespace sema
