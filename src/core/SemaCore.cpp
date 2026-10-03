#include "core/SemaCore.hpp"

#include <Arduino.h>
#include <Wire.h>

#include "core/publishers/HttpPublisher.hpp"
#include "core/publishers/MqttPublisher.hpp"
#include "core/sensors/Bme280Sensor.hpp"
#include "core/sensors/Ds18b20Sensor.hpp"
#include "core/sensors/Sht40Sensor.hpp"

namespace sema {

SemaCore& SemaCore::instance() {
  static SemaCore core;
  return core;
}

SemaCore::SemaCore() : store_(), config_(store_), rules_(events_), alarmLog_(events_) {}

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

  // Detección I²C (D-0058): escanea el bus y sugiere modelos por dirección.
  Wire.begin(21, 22);
  I2cScanner::scan(detectedDevices_);
  for (const DetectedDevice& d : detectedDevices_) {
    Serial.printf("I²C 0x%02X → %s\n", d.address,
                  d.model.length() > 0 ? d.model.c_str() : "desconocido");
  }

  // Fase 2 — Sensor Engine: registra drivers y arranca la lectura periódica.
  static Bme280Sensor bme280("EXT", 21, 22);
  static Sht40Sensor sht40("INT", 21, 22);
  static Ds18b20Sensor ds18b20("SOIL", 4);  // 1-Wire con pull-up 4,7 kΩ (README §12)
  sensors_.registerSensor(&bme280);
  sensors_.registerSensor(&sht40);
  sensors_.registerSensor(&ds18b20);
  sensors_.beginAll();

  // Calibración de ejemplo (D-0055): límites de temperatura. En una iteración
  // posterior estos valores vendrán de la configuración (schema=1).
  Calibration tempCal;
  tempCal.enabled = true;
  tempCal.gain = 1.0f;
  tempCal.offset = 0.0f;
  tempCal.hasRange = true;
  tempCal.min = -40.0f;
  tempCal.max = 85.0f;
  sensors_.setCalibration("EXT:temperature", tempCal);
  sensors_.setCalibration("INT:temperature", tempCal);

  wifi_.begin(config_.get().network.mode,
              config_.get().network.ssid,
              config_.get().network.password,
              config_.get().network.hostname);

  http_.begin(*this);

  // Publicadores (D-0010): webhook HTTP y MQTT (host/URL vacíos → deshabilitados).
  static HttpPublisher webhook("webhook", "");
  static MqttPublisher mqtt("mqtt", "", 1883, "sema/measurement");
  publishers_.registerPublisher(&webhook);
  publishers_.registerPublisher(&mqtt);

  // Regla de ejemplo (D-0059): alarma si la temperatura exterior supera 40 °C.
  rules_.addRule({"high_temp", "EXT", "temperature", RuleOp::Gt, 40.0f});

  // Suscriptor de alarmas (D-0045): por ahora registra en serial.
  events_.subscribe(EventType::Alarm, [](const Event& e) {
    Serial.printf("[ALARM] %s → %s = %d\n", e.correlationId.c_str(), e.source.c_str(), e.value);
  });

  scheduler_.add("core.heartbeat", 5000, []() {
    // Heartbeat periódico del Core. Aquí se integrará el Health Monitor (D-0020).
  });

  scheduler_.add("sensors.read", 10000, [this]() {
    sensors_.readAll();
    http_.broadcastMeasurements(sensors_.measurements());
    for (const Measurement& m : sensors_.measurements()) {
      history_.append(m);
      publishers_.publishAll(m);
    }
    rules_.evaluate(sensors_.measurements());
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
