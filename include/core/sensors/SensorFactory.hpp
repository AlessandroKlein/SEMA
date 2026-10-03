#pragma once

#include "core/ConfigManager.hpp"
#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Factoría de sensores (D-0042)
// =============================================================================
// Crea el driver correcto a partir de una especificación de configuración.

namespace sema {

class SensorFactory {
public:
  static Sensor* create(const SensorSpec& spec);
};

}  // namespace sema
