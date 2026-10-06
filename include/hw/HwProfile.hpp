#pragma once

// =============================================================================
// SEMA — Perfil de hardware (compile-time)
// =============================================================================
// Se selecciona mediante build_flags en platformio.ini. Es la única fuente de
// verdad del hardware: board, features, transceivers, origen de pines y bus SPI.
//
//   Board (variante de silicio):
//     -D BOARD_ESP32_WROOM   ESP32 clásico (MAC Ethernet nativa, SPI VSPI)
//     -D BOARD_ESP32_S3      ESP32-S3 (SPI FSPI, sin MAC Ethernet nativa)
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

#if !defined(BOARD_ESP32_WROOM) && !defined(BOARD_ESP32_S3)
#error "Define BOARD_ESP32_WROOM o BOARD_ESP32_S3 en build_flags (platformio.ini)"
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

// =============================================================================
// Bus SPI y chip-select (CS)
// =============================================================================
// LoRa (RadioLib) usa el objeto SPI de Arduino (FSPI/VSPI). El W5500 usa el
// driver ESP-IDF en un periférico SPI distinto (para no chocar con RadioLib).
// Cada módulo se selecciona con su propio CS.

// --- LoRa (bus SPI de Arduino) ---
#if defined(BOARD_ESP32_WROOM)
#define SEMA_SPI_SCK 18
#define SEMA_SPI_MISO 19
#define SEMA_SPI_MOSI 23
#define SEMA_SPI_CS 5
#elif defined(BOARD_ESP32_S3)
#define SEMA_SPI_SCK 12
#define SEMA_SPI_MISO 13
#define SEMA_SPI_MOSI 11
#define SEMA_SPI_CS 10
#endif

// --- Chip-select de cada dispositivo ---
#define SEMA_CS_LORA 10
#define SEMA_CS_ETHERNET_W5500 5

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

#define SEMA_PIN_ETH_MDC 23
#define SEMA_PIN_ETH_MDIO 18
#define SEMA_PIN_ETH_PHY_ADDR 1
#define SEMA_PIN_ETH_POWER -1

#define SEMA_PIN_ETH_W5500_RST -1
#define SEMA_PIN_ETH_W5500_IRQ 4
