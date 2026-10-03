#pragma once

#include <cstdint>

#include "core/Measurement.hpp"
#include "core/Time.hpp"

// =============================================================================
// SEMA — Interfaz de un driver de sensor
// =============================================================================
// D-0002 / D-0043 / README §7. El driver produce mediciones canónicas (D-0044);
// la aplicación trabaja con magnitudes, nunca con el modelo físico directamente.

namespace sema {

class Sensor {
public:
  virtual ~Sensor() = default;

  virtual const char* id() const = 0;        // id lógico base (p. ej. "EXT")
  virtual const char* model() const = 0;     // modelo físico (p. ej. "BME280")
  virtual const char* interface() const = 0; // "I2C", "SPI", "UART", …

  virtual bool begin() = 0;   // inicializa y detecta el hardware
  virtual uint8_t measure(Measurement out[], uint8_t max) = 0;  // devuelve cantidad
  virtual bool healthy() const = 0;
};

}  // namespace sema
