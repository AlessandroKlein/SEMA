#include <Arduino.h>

#include "core/SemaCore.hpp"
#include "core/SemaGlue.hpp"

// =============================================================================
// SEMA — Sistema de Estación Meteorológica Autónoma
// =============================================================================
// main.cpp se mantiene deliberadamente pequeño (DESIGN-SYSTEM.md §102).
// Registra el glue de dominio en base::coreHooks() y arranca el Core genérico
// de ESP32-Base; la lógica específica de SEMA vive en SemaGlue.cpp.

void setup() {
  sema::registerDomainHooks();
  base::SemaCore::instance().setup();
}

void loop() {
  base::SemaCore::instance().loop();
}
