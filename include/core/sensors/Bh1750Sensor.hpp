#pragma once

#include <cstdint>

#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Driver BH1750 (luminosidad, lux) sobre I²C
// =============================================================================
// D-0043 / README §13. Implementación I²C directa mínima (modo continuo alta
// resolución); puede sustituirse por una librería mantenida sin cambiar la
// interfaz Sensor.

namespace sema {

class Bh1750Sensor : public Sensor {
public:
  Bh1750Sensor(const char* id, uint8_t sda, uint8_t scl);

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
