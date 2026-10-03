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

// Nombre de la capacidad, para serialización/API.
inline const char* capabilityName(Capability c) {
  switch (c) {
    case Capability::WiFi: return "wifi";
    case Capability::Bluetooth: return "bluetooth";
    case Capability::Ethernet: return "ethernet";
    case Capability::Adc: return "adc";
    case Capability::Dac: return "dac";
    case Capability::Pcnt: return "pcnt";
    case Capability::LedcPwm: return "ledc_pwm";
    case Capability::I2c: return "i2c";
    case Capability::Spi: return "spi";
    case Capability::Uart: return "uart";
    case Capability::Can: return "can";
    case Capability::Psram: return "psram";
    case Capability::RtcGpio: return "rtc_gpio";
    case Capability::DeepSleep: return "deep_sleep";
    case Capability::DualCore: return "dual_core";
    case Capability::Ieee802154: return "ieee802154";
    default: return "unknown";
  }
}

}  // namespace sema
