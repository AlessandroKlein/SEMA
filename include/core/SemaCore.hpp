#pragma once

#include "core/CapabilityManager.hpp"
#include "core/ConfigManager.hpp"
#include "core/alarms/RuleEngine.hpp"
#include "core/events/EventLog.hpp"
#include "core/EventBus.hpp"
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
  EventLog& eventLog() { return eventLog_; }
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
  RuleEngine rules_;
  EventLog eventLog_;
  Watchdog watchdog_;
  HealthMonitor health_;
  std::vector<DetectedDevice> detectedDevices_;
  std::vector<Sensor*> ownedSensors_;  // sensores creados por la factoría (D-0042)
  Scheduler scheduler_;
  WiFiManager wifi_;
  HttpServer http_;
};

}  // namespace sema
