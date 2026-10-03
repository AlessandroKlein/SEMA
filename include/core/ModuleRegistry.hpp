#pragma once

#include <cstddef>
#include <vector>

#include "core/Module.hpp"

// =============================================================================
// SEMA — Registro de módulos
// =============================================================================
// DESIGN-SYSTEM.md §8 y §105. Descubre, registra y conduce el ciclo de vida de
// los módulos (instalar → configurar → habilitar → arrancar).

namespace sema {

class ModuleRegistry {
public:
  bool registerModule(Module& module);
  Module* find(const char* id) const;

  void enableAll();
  void loopAll();

  size_t count() const { return modules_.size(); }

private:
  std::vector<Module*> modules_;
};

}  // namespace sema
