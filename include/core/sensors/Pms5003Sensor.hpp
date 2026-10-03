#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver PMS5003 (material particulado PM1/PM2.5/PM10) por UART
// =============================================================================
// D-0043 / README §13 (Fase 5). Sensor láser de partículas, interfaz UART.

namespace sema {

class Pms5003Sensor : public Sensor {
public:
  Pms5003Sensor(const char* id, uint8_t rxPin, uint8_t txPin);

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t rx_;
  uint8_t tx_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
