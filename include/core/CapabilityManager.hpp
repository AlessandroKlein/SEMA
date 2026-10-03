#pragma once

#include <cstdint>

#include "core/Capability.hpp"

// =============================================================================
// SEMA — Capability Manager
// =============================================================================
// Punto único de consulta de capacidades (D-0016). La UI y los módulos preguntan
// antes de mostrar o ejecutar una función dependiente del hardware.

namespace sema {

class CapabilityManager {
public:
  static CapabilityManager& instance();

  void set(Capability c, bool available);
  bool has(Capability c) const;

private:
  CapabilityManager() = default;

  uint32_t mask_ = 0;
};

}  // namespace sema
