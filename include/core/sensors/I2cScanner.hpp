#pragma once

#include <Arduino.h>
#include <cstdint>
#include <vector>

// =============================================================================
// SEMA — Escáner y catálogo I²C
// =============================================================================
// D-0058 / README §44. Detecta dispositivos en el bus I²C y sugiere el modelo
// probable por dirección. La detección nunca modifica configuraciones críticas.

namespace sema {

struct DetectedDevice {
  uint8_t address = 0;
  String model;  // modelo probable ("" si desconocido)
};

class I2cScanner {
public:
  static size_t scan(std::vector<DetectedDevice>& out);
  static String modelForAddress(uint8_t address);
};

}  // namespace sema
