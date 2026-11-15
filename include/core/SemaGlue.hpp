#pragma once

// =============================================================================
// SEMA — Glue de dominio (Fase 2)
// =============================================================================
// Rellena base::coreHooks() con los comportamientos específicos de SEMA:
// catálogo de sensores, magnitudes derivadas, pronóstico y web de dominio.
// main.cpp debe llamar sema::registerDomainHooks() ANTES de base::SemaCore::setup().

namespace sema {
void registerDomainHooks();
}
