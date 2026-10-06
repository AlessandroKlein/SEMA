#pragma once

#include <Adafruit_MCP23X17.h>
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
  const GpioSpec* find(uint8_t pin) const;

  std::vector<GpioSpec> specs_;
  mutable Adafruit_MCP23X17 mcp_;  // digitalRead() no es const en la librería
  uint8_t mcpAddr_ = 0;
  bool mcpReady_ = false;
};

}  // namespace sema
