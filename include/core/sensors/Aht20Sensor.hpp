#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver AHT20 (temperatura y humedad) sobre I²C
// =============================================================================
// D-0043 / README §9-10. Otro modelo para temperatura/humedad, encapsulado
// detrás de la misma interfaz base::Sensor.

namespace sema {

class Aht20Sensor : public base::Sensor {
public:
  Aht20Sensor(const char* id, uint8_t sda, uint8_t scl,
              const char* model = "AHT20");

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(base::Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  const char* model_;
  uint8_t sda_;
  uint8_t scl_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
