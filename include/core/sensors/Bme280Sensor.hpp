#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver BME280 (temperatura, humedad y presión) sobre I²C
// =============================================================================
// D-0043. Envuelve una librería mantenida detrás de la interfaz Sensor, de modo
// que el Core nunca depende del modelo físico.

namespace sema {

class Bme280Sensor : public Sensor {
public:
  Bme280Sensor(const char* id, uint8_t sda, uint8_t scl);

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
