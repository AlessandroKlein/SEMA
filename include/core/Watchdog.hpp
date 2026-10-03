#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Watchdog jerárquico (D-0019)
// =============================================================================
// Task watchdog del ESP32 para el bucle principal: reinicia el SoC si el loop
// se bloquea más del timeout configurado.

namespace sema {

class Watchdog {
public:
  bool begin(uint32_t timeoutSeconds = 10);
  void feed();

private:
  bool started_ = false;
};

}  // namespace sema
