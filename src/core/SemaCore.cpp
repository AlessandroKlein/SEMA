#include "core/SemaCore.hpp"

#include <Arduino.h>

#include "core/sensors/Bme280Sensor.hpp"

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
  history_.begin();

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

  // Fase 2 — Sensor Engine: registra drivers y arranca la lectura periódica.
  static Bme280Sensor bme280("EXT", 21, 22);
  sensors_.registerSensor(&bme280);
  sensors_.beginAll();

  wifi_.begin(config_.get().network.mode,
              config_.get().network.ssid,
              config_.get().network.password,
              config_.get().network.hostname);

  http_.begin(*this);

  scheduler_.add("core.heartbeat", 5000, []() {
    // Heartbeat periódico del Core. Aquí se integrará el Health Monitor (D-0020).
  });

  scheduler_.add("sensors.read", 10000, [this]() {
    sensors_.readAll();
    for (const Measurement& m : sensors_.measurements()) {
      history_.append(m);
    }
  });

  modules_.enableAll();

  Serial.printf("Sensores: %u registrados, %u activos\n",
                static_cast<unsigned>(sensors_.count()),
                static_cast<unsigned>(sensors_.onlineCount()));
  Serial.printf("Web local: http://%s/\n", wifi_.localIP().c_str());
  Serial.printf("Módulos registrados: %u\n", static_cast<unsigned>(modules_.count()));
}

void SemaCore::loop() {
  modules_.loopAll();
  wifi_.loop();
  http_.loop();
  scheduler_.run();
}

}  // namespace sema
