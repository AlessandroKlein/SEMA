#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver SGP30 (eCO₂ y TVOC) sobre I²C
// =============================================================================
// Calidad de aire interior (FUTURO §5). Mide CO₂ equivalente y compuestos
// orgánicos volátiles totales.

namespace sema {

class Sgp30Sensor : public Sensor {
public:
  Sgp30Sensor(const char* id, uint8_t sda, uint8_t scl);

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
