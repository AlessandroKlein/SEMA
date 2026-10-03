#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Sensor analógico genérico (ADC interno)
// =============================================================================
// D-0051 / README §32-33, §37. Lee un pin ADC y aplica escala/offset (D-0055),
// p. ej. batería con divisor resistivo.

namespace sema {

class AdcSensor : public Sensor {
public:
  AdcSensor(const char* id, uint8_t pin, const char* channel,
            const char* unit, float scale, float offset);

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
  float offset_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
