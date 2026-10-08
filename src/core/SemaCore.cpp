#include "core/SemaCore.hpp"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include "core/BoardProfile.hpp"
#include "core/publishers/HttpPublisher.hpp"
#include "core/publishers/MqttPublisher.hpp"
#include "core/sensors/AdcSensor.hpp"
#include "core/sensors/Aht20Sensor.hpp"
#include "core/sensors/Bh1750Sensor.hpp"
#include "core/sensors/Bme280Sensor.hpp"
#include "core/sensors/Ds18b20Sensor.hpp"
#include "core/sensors/Sht40Sensor.hpp"
#include "core/sensors/SensorFactory.hpp"

namespace sema {

// Mapea la zona horaria IANA (config) a una cadena POSIX TZ que entiende newlib.
static const char* posixTz(const String& tz) {
  if (tz == "America/Argentina/Buenos_Aires" || tz == "America/Sao_Paulo") return "<-03>3";
  if (tz == "America/Santiago") return "<-04>4<-03>,M9.1.0/24,M4.1.0/24";
  if (tz == "America/Bogota") return "<-05>5";
  if (tz == "America/Mexico_City") return "<-06>6";
  if (tz == "America/New_York") return "EST5EDT,M3.2.0,M11.1.0";
  if (tz == "America/Los_Angeles") return "PST8PDT,M3.2.0,M11.1.0";
  if (tz == "Europe/Madrid" || tz == "Europe/Berlin") return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (tz == "Europe/London") return "GMT0BST,M3.5.0/1,M10.5.0";
  return "UTC0";
}

SemaCore& SemaCore::instance() {
  static SemaCore core;
  return core;
}

SemaCore::SemaCore()
    : store_(),
      config_(store_),
      webhook_("webhook", ""),
      mqtt_("mqtt", "", 1883, ""),
      rules_(events_),
      eventLog_(events_) {}

void SemaCore::setup() {
  Serial.begin(115200);
  delay(200);

  store_.begin("sema");
  history_.begin();
  eventLog_.begin();

  // Contador de reinicios (persistido en NVS).
  {
    uint32_t boots = 0;
    store_.getUInt("boots", boots);
    ++boots;
    store_.putUInt("boots", boots);
    restartCount_ = boots;
  }

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
  history_.setRetentionSeconds(config_.get().storage.retentionDays * 86400);
  // Histórico en microSD (SPI). Solo se guarda si la SD está habilitada y conectada;
  // si no, las gráficas quedan vacías para no gastar memoria interna.
  if (config_.get().storage.sdEnabled) {
    const bool sd = history_.enableSd(config_.get().storage.sdCsPin);
    Serial.printf("MicroSD: %s (CS=%u)\n",
                  sd ? "conectada" : "no detectada",
                  static_cast<unsigned>(config_.get().storage.sdCsPin));
  }

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
  Serial.printf("Wake reason: %u\n", PowerManager::instance().wakeReason());
  if (config_.get().energy.rainPin != 0) {
    PowerManager::instance().enableRainWakeup(config_.get().energy.rainPin);
  }
  Serial.printf("Capacidades: ADC=%d PCNT=%d DualCore=%d CAN=%d\n",
                caps.has(Capability::Adc) ? 1 : 0,
                caps.has(Capability::Pcnt) ? 1 : 0,
                caps.has(Capability::DualCore) ? 1 : 0,
                caps.has(Capability::Can) ? 1 : 0);

  // Detección I²C (D-0058): escanea el bus y sugiere modelos por dirección.
  Wire.begin(SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
  I2cScanner::scan(detectedDevices_);
  for (const DetectedDevice& d : detectedDevices_) {
    Serial.printf("I²C 0x%02X → %s\n", d.address,
                  d.model.length() > 0 ? d.model.c_str() : "desconocido");
  }

  applySensors();
  derived_.configure(config_.get().system);

  applyCalibrations();
  gpio_.apply(config_.get().gpio);
#if SEMA_USE_SHIFT
  shift_.apply(config_.get().shiftRegisters);
#endif

  // Bus SPI compartido (W5500 + LoRa): inicializar una única vez.
#if SEMA_USE_LORA || SEMA_USE_ETHERNET
  SPI.begin(SEMA_SPI_SCK, SEMA_SPI_MISO, SEMA_SPI_MOSI);
#endif

#if SEMA_USE_MODBUS
  modbus_.apply(config_.get().modbus);
#endif
#if SEMA_USE_CAN
  can_.apply(config_.get().can);
#endif
#if SEMA_USE_LORA
  lora_.apply(config_.get().lora);
#endif
#if SEMA_USE_ZIGBEE
  zigbee_.apply(config_.get().zigbee);
#endif
#if SEMA_USE_ETHERNET
  ethernet_.apply(config_.get().ethernet);
#endif

  wifi_.begin(config_.get().network.mode,
              config_.get().network.ssid,
              config_.get().network.password,
              config_.get().network.hostname,
              config_.get().network.ip,
              config_.get().network.gateway,
              config_.get().network.subnet,
              config_.get().network.dns,
              config_.get().network.mdns);

  // Sincronización NTP (D-0044): configura la zona horaria local; nowEpoch()
  // devuelve la época local y cae a uptime hasta sincronizar.
  configTzTime(posixTz(config_.get().system.timezone),
               config_.get().system.ntpServer.c_str(), "time.nist.gov");

  http_.begin(*this);

  // Publicadores (D-0010): webhook HTTP y MQTT, configurados desde la config.
  publishers_.registerPublisher(&webhook_);
  publishers_.registerPublisher(&mqtt_);
  applyPublishers();

  applyRules();

  // Suscriptor de alarmas (D-0045): por ahora registra en serial.
  events_.subscribe(EventType::Alarm, [](const Event& e) {
    Serial.printf("[ALARM] %s → %s = %d\n", e.correlationId.c_str(), e.source.c_str(), e.value);
  });

  // Watchdog jerárquico por tarea: cada tarea registra su timeout y late en cada
  // ejecución. Si una tarea se estanca, HealthMonitor degrada el estado.
  health_.registerTask("core.heartbeat", 30000);
  health_.registerTask("sensors.read", 60000);

  scheduler_.add("core.heartbeat", 5000, [this]() {
    health_.tick();  // Health Monitor (D-0020)
    health_.taskHeartbeat("core.heartbeat");
  });

  scheduler_.add("sensors.read", 10000, [this]() {
    sensors_.readAll();
    health_.setSensorStats(sensors_.onlineCount(), sensors_.count());
    health_.taskHeartbeat("sensors.read");
    http_.broadcastMeasurements(sensors_.measurements());
    for (const Measurement& m : sensors_.measurements()) {
      history_.append(m);
      publishers_.publishAll(m);
    }
    rules_.evaluate(sensors_.measurements());
  });

  modules_.enableAll();

  // Watchdog jerárquico (D-0019): reinicia el SoC si el loop se bloquea.
  watchdog_.begin(10);

  // Health Monitor (D-0020).
  health_.begin();

  // Evento de arranque (D-0008/§205).
  Event boot;
  boot.timestampMs = millis();
  boot.type = EventType::System;
  boot.severity = Severity::Info;
  boot.source = "core";
  boot.correlationId = "boot";
  events_.publish(boot);

  Serial.printf("Sensores: %u registrados, %u activos\n",
                static_cast<unsigned>(sensors_.count()),
                static_cast<unsigned>(sensors_.onlineCount()));
  Serial.printf("Web local: http://%s/\n", wifi_.localIP().c_str());
  if (wifi_.mdnsStarted()) {
    Serial.printf("mDNS: http://%s.local/\n", config_.get().network.hostname.c_str());
  }
  Serial.printf("Módulos registrados: %u\n", static_cast<unsigned>(modules_.count()));
}

void SemaCore::applyCalibrations() {
  sensors_.clearCalibrations();
  if (config_.get().calibrations.empty()) {
    Calibration tempCal;
    tempCal.enabled = true;
    tempCal.gain = 1.0f;
    tempCal.offset = 0.0f;
    tempCal.hasRange = true;
    tempCal.min = -40.0f;
    tempCal.max = 85.0f;
    sensors_.setCalibration("EXT:temperature", tempCal);
    sensors_.setCalibration("INT:temperature", tempCal);
  } else {
    for (const CalibrationSpec& spec : config_.get().calibrations) {
      Calibration cal;
      cal.enabled = true;
      cal.gain = spec.gain;
      cal.offset = spec.offset;
      cal.hasRange = spec.hasRange;
      cal.min = spec.min;
      cal.max = spec.max;
      sensors_.setCalibration(spec.sensorId + ":" + spec.channelId, cal);
    }
  }
}

void SemaCore::applyRules() {
  rules_.clear();
  if (config_.get().rules.empty()) {
    rules_.addRule({"high_temp", "EXT", "temperature", RuleOp::Gt, 40.0f});
  } else {
    for (const RuleSpec& spec : config_.get().rules) {
      Rule r;
      r.id = spec.name;
      r.sensorId = spec.sensorId;
      r.channelId = spec.channelId;
      r.op = parseRuleOp(spec.op.c_str());
      r.threshold = spec.value;
      rules_.addRule(r);
    }
  }
}

void SemaCore::applySensors() {
  derived_.configure(config_.get().system);

  // Libera los sensores config-driven previos y reinicia el registro.
  sensors_.clear();
  for (Sensor* s : ownedSensors_) {
    delete s;
  }
  ownedSensors_.clear();

  // D-0050: con SEMA_FIXED_HARDWARE=1 se usa el catálogo fijo (pines de
  // BoardProfile.hpp) y se ignora sensors[] de la configuración.
  if (SEMA_FIXED_HARDWARE == 1 || config_.get().sensors.empty()) {
    // Catálogo por defecto con los pines del Board Profile.
    static Bme280Sensor bme280("EXT", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
    static Sht40Sensor sht40("INT", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
    static Ds18b20Sensor ds18b20("SOIL", SEMA_PIN_ONEWIRE, nullptr);  // pull-up 4,7 kΩ (README §12)
    static Bh1750Sensor bh1750("LUX", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
    static Aht20Sensor aht20("AUX", SEMA_PIN_I2C_SDA, SEMA_PIN_I2C_SCL);
    // Batería (ADC interno): divisor 11:1 para 12 V (README §37).
    static AdcSensor battery("BATT", SEMA_PIN_BATTERY_ADC, "voltage", "V",
                             3.3f * 11.0f / 4095.0f, 0.0f);
    sensors_.registerSensor(&bme280);
    sensors_.registerSensor(&sht40);
    sensors_.registerSensor(&ds18b20);
    sensors_.registerSensor(&bh1750);
    sensors_.registerSensor(&aht20);
    sensors_.registerSensor(&battery);
  } else {
    // D-0042: el catálogo de sensores viene de la configuración.
    for (const SensorSpec& spec : config_.get().sensors) {
      if (!spec.enabled) {
        continue;  // sensor apagado desde la web
      }
      Sensor* sensor = SensorFactory::create(spec);
      if (sensor != nullptr) {
        sensors_.registerSensor(sensor);
        ownedSensors_.push_back(sensor);
      } else {
        Serial.printf("Sensor desconocido: %s (modelo %s)\n",
                      spec.id.c_str(), spec.model.c_str());
      }
    }
  }
  sensors_.beginAll();
}

void SemaCore::applyGpio() {
  gpio_.apply(config_.get().gpio);
}

void SemaCore::applyShift() {
#if SEMA_USE_SHIFT
  shift_.apply(config_.get().shiftRegisters);
#endif
}

#if SEMA_USE_MODBUS
void SemaCore::applyModbus() {
  modbus_.apply(config_.get().modbus);
}
#endif

#if SEMA_USE_CAN
void SemaCore::applyCan() {
  can_.apply(config_.get().can);
}
#endif

#if SEMA_USE_LORA
void SemaCore::applyLora() {
  lora_.apply(config_.get().lora);
}
#endif

#if SEMA_USE_ZIGBEE
void SemaCore::applyZigbee() {
  zigbee_.apply(config_.get().zigbee);
}
#endif

#if SEMA_USE_ETHERNET
void SemaCore::applyEthernet() {
  ethernet_.apply(config_.get().ethernet);
}
#endif

void SemaCore::applyPublishers() {
  webhook_.setUrl(config_.get().publishers.webhookUrl.c_str());
  mqtt_.configure(config_.get().publishers.mqttHost.c_str(),
                  config_.get().publishers.mqttPort,
                  config_.get().publishers.mqttTopic.c_str(),
                  config_.get().publishers.mqttUser.c_str(),
                  config_.get().publishers.mqttPass.c_str());
}

void SemaCore::loop() {
  watchdog_.feed();
  modules_.loopAll();
  wifi_.loop();
  http_.loop();
#if SEMA_USE_ZIGBEE
  zigbee_.loop();
#endif
#if SEMA_USE_ETHERNET
  ethernet_.loop();
#endif
  // Retención por tiempo: poda el histórico periódicamente (cada hora).
  {
    static uint32_t lastAgg = 0;
    const uint32_t now = nowEpoch();
    if (now - lastAgg >= 3600) {
      lastAgg = now;
      // Agregación por niveles: lo más viejo que la retención se baja a promedios
      // horarios (en el archivo de agregados) en lugar de descartarse.
      const uint32_t cutoff = now - config_.get().storage.retentionDays * 86400;
      history_.aggregate(3600, cutoff);
    }
  }
  scheduler_.run();
}

}  // namespace sema
