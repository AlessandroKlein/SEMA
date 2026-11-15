#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver CCS811 (eCO₂ y TVOC) sobre I²C
// =============================================================================
// Calidad de aire interior (FUTURO §5). Mide CO₂ equivalente (eCO₂, derivado de
// compuestos orgánicos volátiles) y TVOC. Dirección I²C 0x5A por defecto
// (0x5B si ADDR está a nivel alto); configurable por `address`.

namespace sema {

class Ccs811Sensor : public base::Sensor {
public:
  Ccs811Sensor(const char* id, uint8_t sda, uint8_t scl, uint8_t address = 0);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(base::Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t sda_;
  uint8_t scl_;
  uint8_t address_;  // 0 = default del driver (0x5A)
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
