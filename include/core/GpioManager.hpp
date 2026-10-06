#pragma once

#include <vector>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — Gestor de GPIO standalone (entradas/salidas digitales)
// =============================================================================
// Configura y controla pines GPIO independientes de los sensores (relés,
// interruptores, leds, etc.), definidos desde la configuración.

namespace sema {

class GpioManager {
public:
  void apply(const std::vector<GpioSpec>& specs);
  int read(uint8_t pin) const;
  void write(uint8_t pin, int value);

  const std::vector<GpioSpec>& specs() const { return specs_; }

private:
  std::vector<GpioSpec> specs_;
};

}  // namespace sema
