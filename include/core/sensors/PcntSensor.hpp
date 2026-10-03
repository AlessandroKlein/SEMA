#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Sensor de conteo de pulsos (PCNT)
// =============================================================================
// D-0051 / README §21. Usa el contador de pulsos hardware del ESP32 para medir
// pluviómetros de cangilón, anemómetros, etc. value = pulsos · scale.

namespace sema {

class PcntSensor : public Sensor {
public:
  PcntSensor(const char* id, uint8_t pin, const char* channel,
             const char* unit, float scale);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t pin_;
  const char* channel_;
  const char* unit_;
  float scale_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
