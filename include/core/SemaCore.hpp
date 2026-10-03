#pragma once

#include "core/CapabilityManager.hpp"
#include "core/ConfigManager.hpp"
#include "core/EventBus.hpp"
#include "core/Measurement.hpp"
#include "core/ModuleRegistry.hpp"
#include "core/Version.hpp"
#include "core/storage/NvsStore.hpp"

// =============================================================================
// SEMA — Núcleo de la plataforma
// =============================================================================
// main.cpp se mantiene pequeño (DESIGN-SYSTEM.md §102): el Core inicializa los
// servicios base (storage, configuración, capacidades, módulos, eventos) y
// conduce el bucle.

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

private:
  SemaCore();

  NvsStore store_;
  ModuleRegistry modules_;
  EventBus events_;
  ConfigManager config_;
};

}  // namespace sema
