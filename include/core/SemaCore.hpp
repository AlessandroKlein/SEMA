#pragma once

#include "core/CapabilityManager.hpp"
#include "core/ConfigManager.hpp"
#include "core/EventBus.hpp"
#include "core/Measurement.hpp"
#include "core/ModuleRegistry.hpp"
#include "core/sensors/SensorManager.hpp"
#include "core/Scheduler.hpp"
#include "core/Version.hpp"
#include "core/network/WiFiManager.hpp"
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
  SensorManager& sensors() { return sensors_; }

private:
  SemaCore();

  NvsStore store_;
  ModuleRegistry modules_;
  EventBus events_;
  ConfigManager config_;
  SensorManager sensors_;
  Scheduler scheduler_;
  WiFiManager wifi_;
  HttpServer http_;
};

}  // namespace sema
