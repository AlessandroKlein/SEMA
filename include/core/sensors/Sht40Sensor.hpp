#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver SHT40 (temperatura y humedad) sobre I²C
// =============================================================================
// D-0002 / D-0043. Otro modelo para las mismas magnitudes que el BME280,
// encapsulado detrás de la misma interfaz Sensor.

namespace sema {

class Sht40Sensor : public Sensor {
public:
  Sht40Sensor(const char* id, uint8_t sda, uint8_t scl);

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
