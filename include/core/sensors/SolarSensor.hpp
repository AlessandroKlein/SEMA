#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver de radiación solar (piranómetro) analógico
// =============================================================================
// Salida analógica proporcional a la irradiancia; calibración por
// `scale`/`offset` (sensibilidad del piranómetro).

namespace sema {

class SolarSensor : public Sensor {
public:
  SolarSensor(const char* id, uint8_t pin, float scale, float offset);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(Measurement out[], uint8_t max) override;
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
