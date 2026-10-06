#include "core/EthernetManager.hpp"

#include <Arduino.h>

// Selección de variante por directivas del preprocesador (build_flags).
//   BOARD_ESP32_WROOM → MAC Ethernet nativa + LAN8720A (RMII).
//   BOARD_ESP32_S3    → sin MAC nativa, W5500 por SPI.
#if defined(BOARD_ESP32_WROOM)
#include <ETH.h>
#elif defined(BOARD_ESP32_S3)
#include <Ethernet.h>
static byte kMac[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
#else
#error "Define BOARD_ESP32_WROOM o BOARD_ESP32_S3 en build_flags (platformio.ini)"
#endif

namespace sema {

void EthernetManager::apply(const EthernetConfig& cfg) {
  cfg_ = cfg;
  ready_ = false;
  if (!cfg_.enabled) {
    return;
  }

#if defined(BOARD_ESP32_WROOM)
  // LAN8720A por RMII: reloj de 50 MHz entrando por GPIO0.
  ETH.begin(cfg_.phyAddr, cfg_.powerPin, cfg_.mdcPin, cfg_.mdioPin,
            ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  ready_ = true;
#elif defined(BOARD_ESP32_S3)
  Ethernet.init(cfg_.csPin);
  Ethernet.begin(kMac);  // DHCP
  ready_ = true;
#endif
}

void EthernetManager::loop() {
#if defined(BOARD_ESP32_S3)
  if (ready_) {
    Ethernet.maintain();  // renovación DHCP del W5500
  }
#endif
}

bool EthernetManager::connected() const {
#if defined(BOARD_ESP32_WROOM)
  return ready_ && ETH.linkUp();
#elif defined(BOARD_ESP32_S3)
  return ready_ && (Ethernet.linkStatus() == LinkON);
#endif
  return false;
}

String EthernetManager::localIP() const {
#if defined(BOARD_ESP32_WROOM)
  return ETH.localIP().toString();
#elif defined(BOARD_ESP32_S3)
  return Ethernet.localIP().toString();
#endif
  return String();
}

}  // namespace sema
