#pragma once

#include <cstdint>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — Zigbee (RF-BM-2652P2 / CC2652P2) vía ZNP por UART
// =============================================================================
// El CC2652P2 corre como Zigbee Network Processor (ZNP). El ESP32 habla el
// protocolo MT (Monitor & Test) por UART: envía AF_DATA_REQUEST y recibe
// AF_INCOMING_MSG.

namespace sema {

class ZigbeeManager {
public:
  void apply(const ZigbeeConfig& cfg);
  void loop();  // parsea tramas entrantes (llamar desde el loop)

  bool send(uint16_t destination, const uint8_t* data, uint8_t len);

  bool ready() const { return ready_; }
  const ZigbeeConfig& config() const { return cfg_; }
  bool available() const { return hasMessage_; }
  uint16_t lastSrc() const { return lastSrc_; }
  void takeMessage(uint8_t* out, uint8_t maxLen, uint8_t& len);

private:
  void sendFrame(uint8_t cmd0, uint8_t cmd1, const uint8_t* payload, uint8_t len);
  void handleFrame(uint8_t cmd0, uint8_t cmd1, const uint8_t* payload, uint8_t len);

  ZigbeeConfig cfg_;
  bool ready_ = false;
  uint8_t transId_ = 0;

  uint8_t rxBuf_[256];
  uint16_t rxPos_ = 0;

  bool hasMessage_ = false;
  uint16_t lastSrc_ = 0;
  uint8_t lastLen_ = 0;
  uint8_t lastMsg_[128];
};

}  // namespace sema
