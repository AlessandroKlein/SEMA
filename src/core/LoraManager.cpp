#include "core/LoraManager.hpp"

#include "hw/HwProfile.hpp"

#if SEMA_USE_LORA
#include <RadioLib.h>

namespace sema {

static Module* g_loraModule = nullptr;
static SX1262* g_loraRadio = nullptr;

void LoraManager::apply(const LoraConfig& cfg) {
  cfg_ = cfg;
  ready_ = false;

  if (g_loraRadio != nullptr) {
    delete g_loraRadio;  // libera también el Module
    g_loraRadio = nullptr;
    g_loraModule = nullptr;
  }
  if (!cfg_.enabled) {
    return;
  }

  g_loraModule = new Module(cfg_.csPin, cfg_.dio1Pin, cfg_.rstPin, cfg_.busyPin);
  g_loraRadio = new SX1262(g_loraModule);
  const int16_t state = g_loraRadio->begin(
      cfg_.frequency, cfg_.bandwidth, cfg_.spreading, cfg_.codingRate,
      RADIOLIB_SX126X_SYNC_WORD_PRIVATE, cfg_.txPower);
  if (state == RADIOLIB_ERR_NONE) {
    g_loraRadio->startReceive();
    ready_ = true;
  }
}

bool LoraManager::send(const uint8_t* data, uint8_t len) {
  if (!ready_ || data == nullptr || len == 0) {
    return false;
  }
  const int16_t state = g_loraRadio->transmit(const_cast<uint8_t*>(data), len);
  if (state == RADIOLIB_ERR_NONE) {
    g_loraRadio->startReceive();
    return true;
  }
  return false;
}

uint8_t LoraManager::receive(uint8_t* data, uint8_t maxLen) {
  if (!ready_ || data == nullptr || g_loraRadio->available() <= 0) {
    return 0;
  }
  if (g_loraRadio->readData(data, maxLen) != RADIOLIB_ERR_NONE) {
    return 0;
  }
  const size_t n = g_loraRadio->getPacketLength();
  return n > maxLen ? maxLen : static_cast<uint8_t>(n);
}

}  // namespace sema
#endif  // SEMA_USE_LORA
