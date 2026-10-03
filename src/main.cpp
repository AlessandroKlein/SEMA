#include <Arduino.h>

#include "core/SemaCore.hpp"

// =============================================================================
// SEMA — Sistema de Estación Meteorológica Autónoma
// =============================================================================
// main.cpp se mantiene deliberadamente pequeño (DESIGN-SYSTEM.md §102).
// Su única responsabilidad es arrancar el Core; la lógica vive en módulos
// registrados sobre ModuleRegistry y coordinados por SemaCore.

void setup() {
  sema::SemaCore::instance().setup();
}

void loop() {
  sema::SemaCore::instance().loop();
}
