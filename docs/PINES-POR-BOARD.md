# Pines por board — SEMA

> **Tipo:** Referencia de hardware | **Fecha:** 2026-10-08 | **Firmware:** v1.95.0

Tabla de pines definitiva por board. Los pines **reservados** por el hardware no
aparecen en los selectores de la web. Los buses **compartidos** (I²C / SPI) se
pueden usar con varios dispositivos del mismo bus.

---

## 1. ESP32 WROOM (esp32doit-devkit-v1, 4 MB) — Ethernet LAN8720A (RMII)

### Pines reservados por Ethernet RMII (no configurables)

| Pin | Función RMII |
|-----|--------------|
| IO0  | EMAC_REF_CLK (50 MHz) |
| IO13 | EMAC_RX_ER |
| IO18 | RMII_MDIO |
| IO19 | EMAC_TXD0 |
| IO21 | EMAC_TX_EN |
| IO22 | EMAC_TXD1 |
| IO23 | RMII_MDC |
| IO25 | EMAC_RXD0 |
| IO26 | EMAC_RXD1 |
| IO27 | EMAC_CRS_DV |

### Conflictos reales en WROOM

| Bus / función | Pines | Estado |
|---------------|-------|--------|
| I²C (SDA/SCL) | 21 / 22 | ⚠️ choca con RMII (TX_EN=21, TXD1=22) |
| VSPI (SCK/MISO/MOSI) | 18 / 19 / 23 | ⚠️ choca con RMII (MDIO, TXD0, MDC) |
| HSPI (SCK/MISO/MOSI) | 14 / 12 / 13 | ⚠️ MOSI=13 choca con RMII RX_ER |

### Conclusión WROOM

Con **Ethernet nativo activo**, los buses I²C/VSPI/HSPI estándar quedan
comprometidos por RMII. **Recomendación:**

- Si usás Ethernet RMII en WROOM → **no usar** LoRa/SD por SPI estándar; usar
  **I²C en pines libres** (p. ej. SDA=16, SCL=17) y **SPI por software** en pines
  libres (2/4/5/12/14/15/16/17/32/33) si hace falta.
- Si necesitás LoRa + SD por SPI + Ethernet → **usar ESP32-S3** (W5500 por SPI,
  FSPI libre para LoRa/SD).

### Pines libres (WROOM, con RMII)

`2, 4, 5, 12, 14, 15, 16, 17, 32, 33` (y `34/35/36/39` solo entrada).

---

## 2. ESP32-S3 (esp32-s3-devkitc-1, 8 MB) — Ethernet W5500 (SPI)

| Función | Pines |
|---------|-------|
| FSPI (LoRa + SD, SCK/MISO/MOSI/CS) | 12 / 13 / 11 / 10 |
| W5500 SPI host2 (SCK/MISO/MOSI/CS/RST/IRQ) | 18 / 19 / 21 / 5 / — / 4 |
| I²C (SDA/SCL) | configurable (default 21/22) |
| UART (Modbus/Zigbee) | configurable (default 16/17) |

En S3 no hay MAC Ethernet nativa, así que **FSPI queda libre** para LoRa + SD
(CS separados), y el W5500 usa un periférico SPI distinto.

---

## 3. Defaults vigentes (config web, `SEMA_PINS_FROM_FILE=0`)

| Función | Default |
|---------|---------|
| I²C SDA/SCL | 21 / 22 |
| Modbus RX/TX | 16 / 17 |
| CAN TX/RX | 5 / 4 |
| LoRa CS/RST/DIO1/BUSY | 10 / 14 / 26 / 27 |
| Zigbee RX/TX | 18 / 19 |
| SD SPI CS/MOSI/MISO/SCK | 4 / 23 / 19 / 18 |
| Ethernet RMII MDC/MDIO | 23 / 18 |

> ⚠️ En WROOM los defaults de SD SPI (23/19/18) y Ethernet MDC/MDIO (23/18) se
> solapan; se resuelve **deshabilitando SD/LoRa SPI cuando hay Ethernet RMII**
> o moviendo SD/LoRa a SPI por software en pines libres.

---

## 4. MCP23S17 (expansor GPIO SPI)

El MCP23S17 es un **chip expansor SPI**: cuando está habilitado, agrega **16 GPIO**
(A0-A7, B0-B7) que se pueden usar como pines de otros chips (p. ej. el CS de la
microSD, CS de LoRa, etc.).

- Su **CS** (chip select) se elige de los pines libres.
- Sus 16 pines son **independientes** del GPIO nativo del ESP32.

---

## 5. Regla de actualización

Este archivo se actualiza cuando cambia el hardware (nueva board, nuevo
transceiver) o se resuelve un solape. Es **docs-only** (sin release).
