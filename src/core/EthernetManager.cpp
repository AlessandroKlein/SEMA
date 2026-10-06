#include "core/EthernetManager.hpp"

#include <Arduino.h>

#include "hw/HwProfile.hpp"

#if SEMA_USE_ETHERNET

// Selección de variante por directivas del preprocesador (build_flags).
//   BOARD_ESP32_WROOM → MAC Ethernet nativa + LAN8720A (RMII).
//   BOARD_ESP32_S3    → sin MAC nativa, W5500 SPI vía driver ESP-IDF (esp_eth),
//                       integrado a lwIP (la misma pila que WiFi).
#if defined(BOARD_ESP32_WROOM)
#include <ETH.h>
#elif defined(BOARD_ESP32_S3)
#include <esp_eth.h>
#include <esp_eth_mac.h>
#include <esp_eth_phy.h>
#include <esp_netif.h>
#include <esp_eth_netif_glue.h>
#include <driver/spi_master.h>
#else
#error "Define BOARD_ESP32_WROOM o BOARD_ESP32_S3 en build_flags (platformio.ini)"
#endif

namespace sema {

#if defined(BOARD_ESP32_S3)
static esp_netif_t* g_eth_netif = nullptr;
#endif

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
  // --- 1. Bus SPI ---
  spi_bus_config_t buscfg = {};
  buscfg.miso_io_num = cfg_.misoPin;
  buscfg.mosi_io_num = cfg_.mosiPin;
  buscfg.sclk_io_num = cfg_.sckPin;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 4000;
  if (spi_bus_initialize(static_cast<spi_host_device_t>(SEMA_ETH_SPI_HOST),
                         &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
    return;
  }

  // --- 2. Dispositivo SPI (W5500: address 16 bits + control 8 bits) ---
  spi_device_interface_config_t devcfg = {};
  devcfg.command_bits = 16;
  devcfg.address_bits = 8;
  devcfg.mode = 0;
  devcfg.clock_speed_hz = 20 * 1000 * 1000;  // 20 MHz
  devcfg.spics_io_num = cfg_.csPin;
  devcfg.queue_size = 20;
  spi_device_handle_t spi_hdl = nullptr;
  if (spi_bus_add_device(static_cast<spi_host_device_t>(SEMA_ETH_SPI_HOST),
                         &devcfg, &spi_hdl) != ESP_OK) {
    return;
  }

  // --- 3. MAC W5500 ---
  eth_w5500_config_t w5500 = ETH_W5500_DEFAULT_CONFIG(spi_hdl);
  w5500.int_gpio_num = cfg_.irqPin;
  eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
  mac_cfg.smi_mdc_gpio_num = -1;
  mac_cfg.smi_mdio_gpio_num = -1;
  esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&w5500, &mac_cfg);
  if (mac == nullptr) {
    return;
  }

  // --- 4. PHY W5500 ---
  eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
  phy_cfg.phy_addr = cfg_.phyAddr;
  phy_cfg.reset_gpio_num = cfg_.rstPin;
  esp_eth_phy_t* phy = esp_eth_phy_new_w5500(&phy_cfg);
  if (phy == nullptr) {
    return;
  }

  // --- 5. Driver Ethernet ---
  esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy);
  esp_eth_handle_t eth_handle = nullptr;
  if (esp_eth_driver_install(&eth_cfg, &eth_handle) != ESP_OK) {
    return;
  }

  // --- 6. Interfaz de red lwIP ---
  esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
  g_eth_netif = esp_netif_new(&netif_cfg);
  esp_eth_set_default_handlers(g_eth_netif);
  esp_netif_attach(g_eth_netif, esp_eth_new_netif_glue(eth_handle));

  // --- 7. Arranque (DHCP) ---
  esp_eth_start(eth_handle);
  ready_ = true;
#endif
}

void EthernetManager::loop() {
  // lwIP (nativo y W5500) gestiona DHCP automáticamente; sin mantenimiento.
}

bool EthernetManager::connected() const {
#if defined(BOARD_ESP32_WROOM)
  return ready_ && ETH.linkUp();
#elif defined(BOARD_ESP32_S3)
  if (!ready_ || g_eth_netif == nullptr) {
    return false;
  }
  esp_netif_ip_info_t ip;
  if (esp_netif_get_ip_info(g_eth_netif, &ip) != ESP_OK) {
    return false;
  }
  return ip.ip.addr != 0;
#endif
  return false;
}

String EthernetManager::localIP() const {
#if defined(BOARD_ESP32_WROOM)
  return ETH.localIP().toString();
#elif defined(BOARD_ESP32_S3)
  if (g_eth_netif == nullptr) {
    return String();
  }
  esp_netif_ip_info_t ip;
  if (esp_netif_get_ip_info(g_eth_netif, &ip) != ESP_OK) {
    return String();
  }
  return IPAddress(ip.ip.addr).toString();
#endif
  return String();
}

}  // namespace sema
#endif  // SEMA_USE_ETHERNET
