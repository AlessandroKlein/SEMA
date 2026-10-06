# Ethernet dual-chip y `build_flags` (preprocesador)

> **Tipo:** Referencia técnica | **Fecha:** 2026-10-06 | **Firmware:** v1.31.0

Documento de referencia para entender cómo SEMA soporta **dos variantes de
Ethernet** (según el chip) y cómo se selecciona **todo el hardware** mediante
`build_flags` + directivas del preprocesador (`#if` / `#elif` / `#else` /
`#endif`). Pensado para futuros proyectos y actualizaciones.

---

## 1. Ethernet dual-chip

SEMA tiene **dos caminos de Ethernet** que se eligen en tiempo de compilación
según el chip del ESP32.

| Board (`build_flags`) | Chip Ethernet | Capa física | Protocolo / driver |
|-----------------------|---------------|-------------|--------------------|
| `BOARD_ESP32_WROOM` | MAC nativa | **LAN8720A** (RMII) | `ETH.h` (Arduino core) |
| `BOARD_ESP32_S3` | sin MAC nativa | **W5500** (SPI) | `esp_eth` (ESP-IDF) + lwIP |

### 1.1 `BOARD_ESP32_WROOM` → LAN8720A (RMII, MAC nativa)

Los ESP32 "clásicos" (`ESP32-WROOM-32`, `WROVER`, `D0WDQ6`, `D0WD`, `S0WD`,
`P4`) tienen un **controlador Ethernet MAC** integrado. Solo hay que conectar una
**PHY externa** (LAN8720A) por **RMII** (*Reduced Media Independent Interface*).

- Pines RMII de datos **fijos** del MAC: `TXD0=19`, `TXD1=22`, `TX_EN=21`,
  `RXD0=25`, `RXD1=26`, `CRS_DV=27`.
- Pines configurables: `MDC=23`, `MDIO=18` (bus de gestión MDIO), `PHY_ADDR=1`.
- Reloj de 50 MHz entrando por `GPIO0` (`ETH_CLOCK_GPIO0_IN`).
- Driver: `ETH.begin(...)` del Arduino core (integra con **lwIP**).

### 1.2 `BOARD_ESP32_S3` → W5500 (SPI, sin MAC nativa)

Los ESP32 **sin MAC Ethernet** (`ESP32-S2/S3`, `C2/C3/C5/C6`, `H2`) usan el
módulo **W5500** (WIZnet) por **SPI**. El driver es el de **ESP-IDF**
(`esp_eth_mac_new_w5500`) e **integra el W5500 a lwIP** (la misma pila que WiFi),
por lo que el `WebServer` sirve sobre Ethernet sin parches.

- Bus SPI: `MISO/MOSI/SCK` configurados en `HwProfile.hpp`.
- Dispositivo SPI W5500: `command_bits=16` (dirección) + `address_bits=8`
  (fase de control).
- `esp_eth_mac_new_w5500(&w5500, &mac_cfg)` → `esp_eth_driver_install` →
  `esp_netif_new` + `esp_eth_set_default_handlers` + `esp_netif_attach` →
  `esp_eth_start` (DHCP).

> **Nota de diseño:** el W5500 va en **SPI3** (periférico dedicado) y el LoRa
> (RadioLib) usa el `SPI` de Arduino (SPI2/FSPI). Así no chocan, porque el driver
> `esp_eth` y el `SPI` de Arduino son capas incompatibles sobre el mismo
> periférico. Cada uno se selecciona con su propio **CS**.

### 1.3 Evolución histórica

| Versión | Estado |
|---------|--------|
| v1.24.0 | Ethernet con W5500 por pila separada (`arduino-libraries/Ethernet`): solo conexión/cliente, sin WebServer. |
| v1.25.0 | W5500 por `esp_eth` (ESP-IDF) **integrado a lwIP**: el WebServer sirve sobre Ethernet. |
| v1.26.0 | `HwProfile.hpp` centraliza board + features + pines. |

---

## 2. `build_flags` y directivas del preprocesador

El **hardware se elige en compilación** mediante macros definidas con
`-D` en `platformio.ini` (`build_flags`), y el código usa
`#if defined(...)` / `#elif` / `#else` / `#endif`.

### 2.1 Cómo se define (platformio.ini)

```ini
[env:esp32doit-devkit-v1]
build_flags =
    -D BOARD_ESP32_WROOM        ; variante de silicio
    -D SEMA_USE_ETHERNET=1      ; feature (1 = compilar)
    -D SEMA_USE_LORA=1
    -D SEMA_MODBUS_ISOLATED=1   ; TD501D485H (0 = SN65HVD75DR)
    -D SEMA_PINS_FROM_FILE=0    ; pines por web (1 = fijos en HwProfile)
```

### 2.2 Cómo se consume (código)

```cpp
// include/hw/HwProfile.hpp
#if !defined(BOARD_ESP32_WROOM) && !defined(BOARD_ESP32_S3) && !defined(BOARD_ESP32_WROOM32U)
#error "Define BOARD_ESP32_WROOM, BOARD_ESP32_WROOM32U o BOARD_ESP32_S3"
#endif

#if defined(BOARD_ESP32_S3)
  #define SEMA_NATIVE_ETH 0          // W5500 por SPI
  #define SEMA_SPI_SCK 12
#else
  #define SEMA_NATIVE_ETH 1          // LAN8720A nativo
  #define SEMA_SPI_SCK 18
#endif
```

```cpp
// src/core/EthernetManager.cpp — dos implementaciones
#if SEMA_NATIVE_ETH
  #include <ETH.h>
#else
  #include <esp_eth.h>       // W5500 (ESP-IDF)
  #include <driver/spi_master.h>
#endif

void EthernetManager::apply(const EthernetConfig& cfg) {
#if SEMA_NATIVE_ETH
  ETH.begin(cfg.phyAddr, cfg.powerPin, cfg.mdcPin, cfg.mdioPin,
            ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
#else
  spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO);
  /* ... esp_eth_mac_new_w5500 + netif ... */
#endif
}
```

### 2.3 Flags de feature (compilar o no un módulo)

`#if SEMA_USE_LORA` excluye el módulo **y su librería** del firmware (reduce
flash). Ejemplo en `LoraManager.cpp`:

```cpp
#include "hw/HwProfile.hpp"
#if SEMA_USE_LORA
#include <RadioLib.h>
/* ... implementación ... */
#endif  // SEMA_USE_LORA
```

Si `-D SEMA_USE_LORA=0`, RadioLib **no se linkea** (la única inclusión está
dentro del `#if`).

### 2.4 Origen de pines (`SEMA_PINS_FROM_FILE`)

- `0` (default): los pines se configuran **en runtime** desde la web.
- `1` (PCB): los pines son **fijos** (de `HwProfile.hpp`) y la web los ignora.

```cpp
// ConfigManager.cpp
static void applyHwProfile(Config& c) {
#if SEMA_PINS_FROM_FILE
  c.can.txPin   = SEMA_PIN_CAN_TX;
  c.modbus.rxPin = SEMA_PIN_MODBUS_RX;
  /* ... */
#endif
}
```

### 2.5 Resumen de directivas

| Directiva | Uso |
|-----------|-----|
| `#if defined(MACRO)` | comprueba si una macro está definida (por `-D`). |
| `#if MACRO` / `#if MACRO==1` | comprueba el valor de una macro. |
| `#elif` | rama alternativa (`else if`). |
| `#else` | rama por defecto. |
| `#endif` | cierra el bloque. |
| `#ifndef MACRO` | "si NO está definida". |
| `#define MACRO VAL` | define (en código o por `-D`). |
| `#error "mensaje"` | aborta la compilación (guardia de variantes). |

---

## 3. Archivos clave

| Archivo | Rol |
|---------|-----|
| `platformio.ini` | `build_flags` por entorno + `board_build.partitions` + `filesystem`. |
| `include/hw/HwProfile.hpp` | Única fuente de verdad del hardware (board, features, pines, bus SPI/CS). |
| `include/core/EthernetManager.hpp` / `.cpp` | Abstracción Ethernet (nativo vs W5500). |
| `src/core/ConfigManager.cpp` | `applyHwProfile()` (pines fijos si `SEMA_PINS_FROM_FILE`). |
| `include/core/SemaCore.hpp` / `.cpp` | Compilación condicional de los buses. |
