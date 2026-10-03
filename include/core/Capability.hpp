#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Capacidades de la plataforma
// =============================================================================
// D-0016 / D-0051. Enumeración de capacidades que un Board/Chip Profile puede
// declarar. El código pregunta capacidades en lugar de preguntar por modelo.

namespace sema {

enum class Capability : uint8_t {
  WiFi,
  Bluetooth,
  Ethernet,
  Adc,
  Dac,
  Pcnt,
  LedcPwm,
  I2c,
  Spi,
  Uart,
  Can,
  Psram,
  RtcGpio,
  DeepSleep,
  DualCore,
  Ieee802154,

  Count  // número de capacidades
};

}  // namespace sema
