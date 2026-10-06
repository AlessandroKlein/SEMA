#pragma once

#include <cstdint>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — LoRa (SX1262) envío/recepción de paquetes
// =============================================================================

namespace sema {

class LoraManager {
public:
  void apply(const LoraConfig& cfg);

  bool send(const uint8_t* data, uint8_t len);
  uint8_t receive(uint8_t* data, uint8_t maxLen);  // bytes leídos (0 = nada)

  bool ready() const { return ready_; }
  const LoraConfig& config() const { return cfg_; }

private:
  LoraConfig cfg_;
  bool ready_ = false;
};

}  // namespace sema
