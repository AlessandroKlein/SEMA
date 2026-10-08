#pragma once

#include "hw/HwProfile.hpp"

#include "core/CapabilityManager.hpp"
#include "core/ConfigManager.hpp"
#include "core/derived/DerivedCalculator.hpp"
#include "core/alarms/RuleEngine.hpp"
#include "core/events/EventLog.hpp"
#include "core/EventBus.hpp"
#include "core/GpioManager.hpp"
#include "core/ShiftRegisterManager.hpp"
#if SEMA_USE_MODBUS
#include "core/ModbusManager.hpp"
#endif
#if SEMA_USE_CAN
#include "core/CanManager.hpp"
#endif
#if SEMA_USE_LORA
#include "core/LoraManager.hpp"
#endif
#if SEMA_USE_ZIGBEE
#include "core/ZigbeeManager.hpp"
#endif
#if SEMA_USE_ETHERNET
#include "core/EthernetManager.hpp"
#endif
#include "core/HealthMonitor.hpp"
#include "core/Measurement.hpp"
#include "core/ModuleRegistry.hpp"
#include "core/sensors/I2cScanner.hpp"
#include "core/sensors/SensorManager.hpp"
#include "core/Scheduler.hpp"
#include "core/Version.hpp"
#include "core/network/WiFiManager.hpp"
#include "core/PowerManager.hpp"
#include "core/Watchdog.hpp"
#include "core/publishers/HttpPublisher.hpp"
#include "core/publishers/MqttPublisher.hpp"
#include "core/publishers/PublisherManager.hpp"
#include "core/storage/HistoryStore.hpp"
#include "core/storage/NvsStore.hpp"
#include "core/web/HttpServer.hpp"

// =============================================================================
// SEMA — Núcleo de la plataforma
// =============================================================================
// main.cpp se mantiene pequeño (DESIGN-SYSTEM.md §102): el Core inicializa los
// servicios base (storage, configuración, capacidades, red, web, scheduler,
// módulos, eventos) y conduce el bucle.

namespace sema {

class SemaCore {
public:
  static SemaCore& instance();

  void setup();
  void loop();

  // Re-aplican configuración no hardware-dependiente (sin reinicio).
  void applyRules();
  void applyCalibrations();
  void applyGpio();
  void applyPublishers();
  void applySensors();
  void applyShift();
#if SEMA_USE_MODBUS
  void applyModbus();
#endif
#if SEMA_USE_CAN
  void applyCan();
#endif
#if SEMA_USE_LORA
  void applyLora();
#endif
#if SEMA_USE_ZIGBEE
  void applyZigbee();
#endif
#if SEMA_USE_ETHERNET
  void applyEthernet();
#endif

  ModuleRegistry& modules() { return modules_; }
  EventBus& events() { return events_; }
  ConfigManager& config() { return config_; }
  CapabilityManager& capabilities() { return CapabilityManager::instance(); }
  Scheduler& scheduler() { return scheduler_; }
  WiFiManager& wifi() { return wifi_; }
  PowerManager& power() { return PowerManager::instance(); }
  Watchdog& watchdog() { return watchdog_; }
  HealthMonitor& health() { return health_; }
  SensorManager& sensors() { return sensors_; }
  HistoryStore& history() { return history_; }
  PublisherManager& publishers() { return publishers_; }
  RuleEngine& rules() { return rules_; }
  DerivedCalculator& derived() { return derived_; }
  EventLog& eventLog() { return eventLog_; }
  GpioManager& gpio() { return gpio_; }
  ShiftRegisterManager& shift() { return shift_; }
  uint32_t restartCount() const { return restartCount_; }
#if SEMA_USE_MODBUS
  ModbusManager& modbus() { return modbus_; }
#endif
#if SEMA_USE_CAN
  CanManager& can() { return can_; }
#endif
#if SEMA_USE_LORA
  LoraManager& lora() { return lora_; }
#endif
#if SEMA_USE_ZIGBEE
  ZigbeeManager& zigbee() { return zigbee_; }
#endif
#if SEMA_USE_ETHERNET
  EthernetManager& ethernet() { return ethernet_; }
#endif
  const std::vector<DetectedDevice>& detectedDevices() const { return detectedDevices_; }

private:
  SemaCore();

  NvsStore store_;
  ModuleRegistry modules_;
  EventBus events_;
  ConfigManager config_;
  SensorManager sensors_;
  HistoryStore history_;
  PublisherManager publishers_;
  HttpPublisher webhook_;
  MqttPublisher mqtt_;
  RuleEngine rules_;
  DerivedCalculator derived_;
  EventLog eventLog_;
  GpioManager gpio_;
  ShiftRegisterManager shift_;
#if SEMA_USE_MODBUS
  ModbusManager modbus_;
#endif
#if SEMA_USE_CAN
  CanManager can_;
#endif
#if SEMA_USE_LORA
  LoraManager lora_;
#endif
#if SEMA_USE_ZIGBEE
  ZigbeeManager zigbee_;
#endif
#if SEMA_USE_ETHERNET
  EthernetManager ethernet_;
#endif
  Watchdog watchdog_;
  HealthMonitor health_;
  uint32_t restartCount_ = 0;
  std::vector<DetectedDevice> detectedDevices_;
  std::vector<Sensor*> ownedSensors_;  // sensores creados por la factoría (D-0042)
  Scheduler scheduler_;
  WiFiManager wifi_;
  HttpServer http_;
};

}  // namespace sema
