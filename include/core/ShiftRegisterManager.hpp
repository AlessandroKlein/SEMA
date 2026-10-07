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
  void apply(const std::vector<ShiftRegisterConfig>& cfg);
  void writeByte(uint8_t value);   // escribe en todos los 74HC595
  uint8_t readByte();              // lee del primer 74HC165

  bool configured() const { return !cfg_.empty(); }
  bool hasOutput() const;
  bool hasInput() const;

private:
  std::vector<ShiftRegisterConfig> cfg_;
};

}  // namespace sema
