#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver ADS1115 (ADC externo 16 bits, 4 canales) sobre I²C
// =============================================================================
// `pin` del SensorSpec es el canal (0..3); `channel`/`unit`/`scale`/`offset`
// definen la magnitud medida.

namespace sema {

class Ads1115Sensor : public Sensor {
public:
  Ads1115Sensor(const char* id, uint8_t sda, uint8_t scl, uint8_t channel,
                const char* name, const char* unit, float scale, float offset);

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
  uint8_t channel_;
  String name_;
  String unit_;
  float scale_;
  float offset_;
  bool ok_ = false;
  uint32_t sequence_ = 0;
};

}  // namespace sema
