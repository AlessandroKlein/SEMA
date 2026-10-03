#include "core/SemaCore.hpp"

#include <Arduino.h>

namespace sema {

SemaCore& SemaCore::instance() {
  static SemaCore core;
  return core;
}

SemaCore::SemaCore() : store_(), config_(store_) {}

void SemaCore::setup() {
  Serial.begin(115200);
  delay(200);

  store_.begin("sema");

  // Perfil base ESP32 clásico (D-0050/D-0051). En una iteración posterior esto se
  // carga desde el Board/Chip Profile en lugar de declararse aquí.
  CapabilityManager& caps = CapabilityManager::instance();
  caps.set(Capability::WiFi, true);
  caps.set(Capability::Bluetooth, true);
  caps.set(Capability::Adc, true);
  caps.set(Capability::Dac, true);
  caps.set(Capability::Pcnt, true);
  caps.set(Capability::LedcPwm, true);
  caps.set(Capability::I2c, true);
  caps.set(Capability::Spi, true);
  caps.set(Capability::Uart, true);
  caps.set(Capability::Can, true);
  caps.set(Capability::RtcGpio, true);
  caps.set(Capability::DeepSleep, true);
  caps.set(Capability::DualCore, true);

  config_.load();

  Serial.println();
  Serial.printf("SEMA v%s (hw %s, schema %d, protocol %d)\n",
                SEMA_FW_VERSION,
                SEMA_HW_VERSION,
                SEMA_CONFIG_SCHEMA_VERSION,
                SEMA_PROTOCOL_VERSION);
  Serial.printf("Estación: %s (%s)\n",
                config_.get().station.name.c_str(),
                config_.get().station.id.c_str());
  Serial.printf("Config válida: %s\n", config_.valid() ? "sí" : "no");
  Serial.printf("Capacidades: ADC=%d PCNT=%d DualCore=%d CAN=%d\n",
                caps.has(Capability::Adc) ? 1 : 0,
                caps.has(Capability::Pcnt) ? 1 : 0,
                caps.has(Capability::DualCore) ? 1 : 0,
                caps.has(Capability::Can) ? 1 : 0);
  Serial.printf("Módulos registrados: %u\n", static_cast<unsigned>(modules_.count()));

  modules_.enableAll();
}

void SemaCore::loop() {
  modules_.loopAll();
}

}  // namespace sema
