#pragma once

#include <cstdint>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — Gestor de shift registers (74HC595 salida / 74HC165 entrada)
// =============================================================================
// Expansión digital bit-banged de 8 bits por 3 pines (data, clock, latch).

namespace sema {

class ShiftRegisterManager {
public:
  void apply(const ShiftRegisterConfig& cfg);
  void writeByte(uint8_t value);   // 74HC595 (salida)
  uint8_t readByte();              // 74HC165 (entrada)

  bool configured() const { return cfg_.dataPin != 0; }
  bool isOutput() const { return cfg_.type != "74HC165"; }
  const ShiftRegisterConfig& config() const { return cfg_; }

private:
  ShiftRegisterConfig cfg_;
};

}  // namespace sema
