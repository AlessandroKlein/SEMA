#pragma once

#include <cstdint>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — CAN 2.0 (TWAI) envío/recepción de tramas
// =============================================================================

namespace sema {

class CanManager {
public:
  void apply(const CanConfig& cfg);

  bool send(uint32_t id, const uint8_t* data, uint8_t dlc, bool extd);
  bool receive(uint32_t& id, uint8_t* data, uint8_t& dlc, bool& extd);

  bool ready() const { return ready_; }
  const CanConfig& config() const { return cfg_; }

private:
  CanConfig cfg_;
  bool ready_ = false;
};

}  // namespace sema
