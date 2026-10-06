#pragma once

#include <cstdint>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — Ethernet (LAN8720A nativo o W5500 SPI, según la board)
// =============================================================================
// La variante se elige en compilación con build_flags (-D BOARD_ESP32_WROOM o
// -D BOARD_ESP32_S3). Ver platformio.ini.

namespace sema {

class EthernetManager {
public:
  void apply(const EthernetConfig& cfg);
  void loop();

  bool enabled() const { return cfg_.enabled; }
  bool connected() const;
  String localIP() const;
  const EthernetConfig& config() const { return cfg_; }

private:
  EthernetConfig cfg_;
  bool ready_ = false;
};

}  // namespace sema
