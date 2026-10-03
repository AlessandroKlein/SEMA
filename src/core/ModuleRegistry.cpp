#include "core/ModuleRegistry.hpp"

#include <cstring>

namespace sema {

bool ModuleRegistry::registerModule(Module& module) {
  if (find(module.id()) != nullptr) {
    return false;  // id ya registrado
  }
  modules_.push_back(&module);
  return true;
}

Module* ModuleRegistry::find(const char* id) const {
  for (Module* m : modules_) {
    if (std::strcmp(m->id(), id) == 0) {
      return m;
    }
  }
  return nullptr;
}

void ModuleRegistry::enableAll() {
  for (Module* m : modules_) {
    if (m->state() == ModuleState::Available) {
      m->install();
      m->configure();
      m->enable();
      m->start();
    }
  }
}

void ModuleRegistry::loopAll() {
  for (Module* m : modules_) {
    const ModuleState s = m->state();
    if (s == ModuleState::Enabled || s == ModuleState::Running) {
      m->loop();
    }
  }
}

}  // namespace sema
