#pragma once

#include "core/EventBus.hpp"
#include "core/ModuleRegistry.hpp"
#include "core/Version.hpp"

// =============================================================================
// SEMA — Núcleo de la plataforma
// =============================================================================
// main.cpp se mantiene pequeño (DESIGN-SYSTEM.md §102): el Core es quien
// inicializa, registra, arranca y conduce el bucle de los módulos.

namespace sema {

class SemaCore {
public:
  static SemaCore& instance();

  void setup();
  void loop();

  ModuleRegistry& modules() { return modules_; }
  EventBus& events() { return events_; }

private:
  SemaCore() = default;

  ModuleRegistry modules_;
  EventBus events_;
};

}  // namespace sema
