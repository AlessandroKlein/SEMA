#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver S12SD (sensor UV analógico, ADC interno)
// =============================================================================
// Módulo UV analógico (p. ej. S12SD): salida de tensión proporcional al índice
// UV (~0.1 V por índice UV). Calibración lineal por config (`scale`/`offset`),
// igual que SOLAR/CO. La sensibilidad exacta depende del full-scale del módulo
// y de si lleva divisor resistivo; se calibra desde la web.

namespace sema {

class S12sdSensor : public base::Sensor {
public:
  S12sdSensor(const char* id, uint8_t pin, float scale, float offset);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(base::Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t pin_;
  float scale_;
  float offset_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
