#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

class OneWire;            // <OneWire.h>
class DallasTemperature;  // <DallasTemperature.h>

// =============================================================================
// SEMA — Driver DS18B20 (temperatura) sobre 1-Wire
// =============================================================================
// D-0043 / README §12. Primer sensor en un bus distinto a I²C. El 1-Wire requiere
// pull-up de 4,7 kΩ a 3,3 V en el pin de datos.

namespace sema {

class Ds18b20Sensor : public Sensor {
public:
  Ds18b20Sensor(const char* id, uint8_t pin);
  ~Ds18b20Sensor() override;

  const char* id() const override;
  const char* model() const override;
  const char* interface() const override;

  bool begin() override;
  uint8_t measure(Measurement out[], uint8_t max) override;
  bool healthy() const override;

private:
  const char* id_;
  uint8_t pin_;
  OneWire* oneWire_ = nullptr;
  DallasTemperature* ds_ = nullptr;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
