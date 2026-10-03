#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver SHT31 (temperatura y humedad) sobre I²C
// =============================================================================
// D-0043 / README §9-10. Otro modelo de temperatura/humedad detrás de la misma
// interfaz Sensor (catálogo configurable, D-0042).

namespace sema {

class Sht31Sensor : public Sensor {
public:
  Sht31Sensor(const char* id, uint8_t sda, uint8_t scl);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t sda_;
  uint8_t scl_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
