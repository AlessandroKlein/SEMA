#pragma once

// =============================================================================
// SEMA — Perfil de hardware (compile-time)
// =============================================================================
// Se selecciona mediante build_flags en platformio.ini. Es la única fuente de
// verdad del hardware: board, features, transceivers, origen de pines y bus SPI.
//
//   Board (variante de silicio):
//     -D BOARD_ESP32_WROOM     ESP32 clásico 4 MB (MAC Ethernet nativa, VSPI)
//     -D BOARD_ESP32_WROOM32U  ESP32-WROOM-32U 16 MB (PCB futura)
//     -D BOARD_ESP32_S3        ESP32-S3 8 MB (SPI FSPI, sin MAC nativa)
//
//   Features (0 = no compilar; reduce firmware):
//     -D SEMA_USE_ETHERNET=0  -D SEMA_USE_LORA=0  -D SEMA_USE_MODBUS=0
//     -D SEMA_USE_CAN=0       -D SEMA_USE_ZIGBEE=0
//
//   Transceiver RS485 (Modbus):
//     -D SEMA_MODBUS_ISOLATED=1  → TD501D485H  (aislado)
//     -D SEMA_MODBUS_ISOLATED=0  → SN65HVD75DR (sin aislar)
//
//   Origen de pines:
//     -D SEMA_PINS_FROM_FILE=1   → pines fijos de este archivo (PCB);
//                                  no configurables desde la web.
//     -D SEMA_PINS_FROM_FILE=0   → pines configurables en runtime (web).
// =============================================================================

#if !defined(BOARD_ESP32_WROOM) && !defined(BOARD_ESP32_S3) && !defined(BOARD_ESP32_WROOM32U)
#error "Define BOARD_ESP32_WROOM, BOARD_ESP32_WROOM32U o BOARD_ESP32_S3 en build_flags"
#endif

// --- Identidad de la board (para OTA / manifest) ---
#if defined(BOARD_ESP32_WROOM)
#define SEMA_BOARD_ID "esp32-wroom-4mb"
#define SEMA_FLASH_MB 4
#define SEMA_NATIVE_ETH 1
#elif defined(BOARD_ESP32_WROOM32U)
#define SEMA_BOARD_ID "esp32-wroom32u-16mb"
#define SEMA_FLASH_MB 16
#define SEMA_NATIVE_ETH 1
#elif defined(BOARD_ESP32_S3)
#define SEMA_BOARD_ID "esp32-s3-8mb"
#define SEMA_FLASH_MB 8
#define SEMA_NATIVE_ETH 0
#endif

#define SEMA_ON 1
#define SEMA_OFF 0

// --- Features (por defecto todas activas) ---
#ifndef SEMA_USE_ETHERNET
#define SEMA_USE_ETHERNET SEMA_ON
#endif
#ifndef SEMA_USE_LORA
#define SEMA_USE_LORA SEMA_ON
#endif
#ifndef SEMA_USE_MODBUS
#define SEMA_USE_MODBUS SEMA_ON
#endif
#ifndef SEMA_USE_CAN
#define SEMA_USE_CAN SEMA_ON
#endif
#ifndef SEMA_USE_ZIGBEE
#define SEMA_USE_ZIGBEE SEMA_ON
#endif
// Registro de desplazamiento en cascada (74HC595/74HC165). Beta: deshabilitado
// por defecto hasta completar su driver (cascada, niveles lógicos, etc.).
#ifndef SEMA_USE_SHIFT
#define SEMA_USE_SHIFT SEMA_OFF
#endif

// --- Transceiver RS485 ---
#ifndef SEMA_MODBUS_ISOLATED
#define SEMA_MODBUS_ISOLATED SEMA_ON
#endif
#if SEMA_MODBUS_ISOLATED
#define SEMA_MODBUS_TRANSCEIVER "TD501D485H"   // aislado galvánicamente
#else
#define SEMA_MODBUS_TRANSCEIVER "SN65HVD75DR"  // sin aislamiento
#endif

// --- Origen de pines ---
#ifndef SEMA_PINS_FROM_FILE
#define SEMA_PINS_FROM_FILE SEMA_OFF
#endif

// Modo demo: muestra todas las páginas y genera valores ficticios dentro del
// rango estándar de cada magnitud (para probar la web sin sensores conectados).
#ifndef SEMA_DEMO
#define SEMA_DEMO SEMA_OFF
#endif

// =============================================================================
// Bus SPI y chip-select (CS)
// =============================================================================
// LoRa (RadioLib) usa el objeto SPI de Arduino (FSPI/VSPI). El W5500 usa el
// driver ESP-IDF en un periférico SPI distinto (para no chocar con RadioLib).
// Cada módulo se selecciona con su propio CS.

// --- LoRa (bus SPI de Arduino) ---
#if defined(BOARD_ESP32_S3)
#define SEMA_SPI_SCK 12
#define SEMA_SPI_MISO 13
#define SEMA_SPI_MOSI 11
#define SEMA_SPI_CS 10
#else  // WROOM / WROOM32U (VSPI)
#define SEMA_SPI_SCK 18
#define SEMA_SPI_MISO 19
#define SEMA_SPI_MOSI 23
#define SEMA_SPI_CS 5
#endif

// --- Chip-select de cada dispositivo ---
#define SEMA_CS_LORA 10
#define SEMA_CS_ETHERNET_W5500 5

// --- MicroSD (SPI, no SDMMC) ---
// Los pines SDMMC del ESP32 están compartidos con otras salidas; la microSD se
// conecta por el bus SPI (SEMA_SPI_MOSI/MISO/SCK) con su propio chip-select.
#if defined(BOARD_ESP32_S3)
#define SEMA_PIN_SD_CS 4
#else  // WROOM / WROOM32U
#define SEMA_PIN_SD_CS 4
#endif

// --- W5500 (driver ESP-IDF, periférico SPI dedicado) ---
#if defined(BOARD_ESP32_S3)
#define SEMA_ETH_SPI_HOST 2          // SPI3_HOST (HSPI), separado del SPI de Arduino
#define SEMA_ETH_SPI_SCK 18
#define SEMA_ETH_SPI_MISO 19
#define SEMA_ETH_SPI_MOSI 21
#else
#define SEMA_ETH_SPI_HOST 1          // SPI2_HOST (no usado: WROOM usa LAN8720A)
#define SEMA_ETH_SPI_SCK 14
#define SEMA_ETH_SPI_MISO 12
#define SEMA_ETH_SPI_MOSI 13
#endif

// =============================================================================
// Pines fijos (se usan sólo cuando SEMA_PINS_FROM_FILE == 1)
// =============================================================================
#define SEMA_PIN_CAN_TX 5
#define SEMA_PIN_CAN_RX 4

#define SEMA_PIN_MODBUS_RX 16
#define SEMA_PIN_MODBUS_TX 17
#define SEMA_PIN_MODBUS_DERE 0

#define SEMA_PIN_ZIGBEE_RX 18
#define SEMA_PIN_ZIGBEE_TX 19

#define SEMA_PIN_LORA_RST 14
#define SEMA_PIN_LORA_DIO1 26
#define SEMA_PIN_LORA_BUSY 27

// =============================================================================
// Ethernet LAN8720A (RMII) — pines oficiales Espressif para ESP32 WROOM
// =============================================================================
// MDC y MDIO son configurables por software (ETH.begin). El resto son fijos del
// MAC EMAC interno del ESP32 y NO se pueden reasignar.
#define SEMA_PIN_ETH_MDC 23      // IO23_RMII_MDC
#define SEMA_PIN_ETH_MDIO 18     // IO18_RMII_MDIO
#define SEMA_PIN_ETH_PHY_ADDR 1
#define SEMA_PIN_ETH_POWER -1
// Fijos RMII (hardware EMAC, solo informativo):
#define SEMA_PIN_ETH_TXD0 19     // IO19_RMII_EMAC_TXD0
#define SEMA_PIN_ETH_TXD1 22     // IO22_RMII_EMAC_TXD1
#define SEMA_PIN_ETH_TX_EN 21    // IO21_RMII_EMAC_TX_EN
#define SEMA_PIN_ETH_RXD0 25     // IO25_RMII_EMAC_RXD0
#define SEMA_PIN_ETH_RXD1 26     // IO26_RMII_EMAC_RXD1
#define SEMA_PIN_ETH_CRS_DV 27   // IO27_RMII_EMAC_CRS_DV
#define SEMA_PIN_ETH_RX_ER 13    // IO13_RMII_EMAC_RX_ER
#define SEMA_PIN_ETH_REF_CLK 0   // IO0_RMII_EMAC_REF_CLK (50 MHz, GPIO0_IN)

#define SEMA_PIN_ETH_W5500_RST -1
#define SEMA_PIN_ETH_W5500_IRQ 4
