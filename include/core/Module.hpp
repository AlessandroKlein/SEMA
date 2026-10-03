#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Contrato mínimo de un módulo
// =============================================================================
// Sigue el ciclo de vida del README.md §53 y el contrato del DESIGN-SYSTEM.md
// §224. Cada módulo (obligatorio u opcional) implementa esta interfaz para
// registrarse en el ModuleRegistry y no acoplarse al núcleo.
//
// Los valores del enum van en PascalCase para evitar colisiones con las macros
// del core Arduino/ESP32 (p. ej. `DISABLED`, `ENABLED`).

namespace sema {

// Estado de un módulo (README.md §53).
enum class ModuleState : uint8_t {
  Available,
  Installed,
  Configured,
  Enabled,
  Running,
  Disabled,
  Uninstalled,
  InstallError,
  ConfigError,
  RuntimeError,
  UpdateError
};

class Module {
public:
  virtual ~Module() = default;

  // Identificador estable y versión del módulo (DESIGN-SYSTEM.md §28).
  virtual const char* id() const = 0;
  virtual const char* version() const = 0;

  virtual bool install() { return true; }
  virtual bool configure() { return true; }
  virtual bool enable() {
    state_ = ModuleState::Enabled;
    return true;
  }
  virtual void start() { state_ = ModuleState::Running; }
  virtual void loop() {}
  virtual void stop() { state_ = ModuleState::Enabled; }
  virtual bool disable() {
    state_ = ModuleState::Disabled;
    return true;
  }

  ModuleState state() const { return state_; }

protected:
  ModuleState state_ = ModuleState::Available;
};

}  // namespace sema
