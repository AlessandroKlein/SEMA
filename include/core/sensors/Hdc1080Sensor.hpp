#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver HDC1080 (temperatura y humedad) sobre I²C
// =============================================================================
// Implementación I²C directa (sin librería), dirección 0x40. Registro 0x00 =
// temperatura (big-endian 16 bits), 0x01 = humedad. Conversiones del datasheet:
//   T  = (raw / 65536) * 165 - 40   [degC]
//   RH = (raw / 65536) * 100        [percent]

namespace sema {

class Hdc1080Sensor : public Sensor {
public:
  Hdc1080Sensor(const char* id, uint8_t sda, uint8_t scl);

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
