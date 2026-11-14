# PCB v1 — Revisión de pines y correcciones (rumbo a v2 SMD)

> **Fecha:** 2026-11-14 · **Base:** ESP32 devkit v1 (WROOM, 4 MB) · **Firmware:** v1.104.0
>
> **Hardware real confirmado por el usuario:** la placa usa **W5500 (SPI Ethernet,
> CS=GPIO5)** + **VSPI (SCK18/MOSI23/MISO19)** + **microSD (CS=GPIO4)** + **I2C
> (SDA21/SCL22)** + **UART (TX17/RX16)**. **No usa Ethernet RMII nativo.** La base
> es un **ESP32 devkit v1** (módulo WROOM, 4 MB), no un WROOM-32U.

Revisión de la asignación de pines de tu PCB v1 contra (a) las reglas de oro del
ESP32-WROOM y (b) las reservas reales de la placa (bus SPI VSPI para SD/W5500).
Los errores marcados **ERROR** son bloqueantes; los **WARNING** funcionan con
limitaciones.

---

## 1. Reglas de oro del ESP32-WROOM que afectan tu placa

1. **ADC1 (GPIO32–39)** funciona con WiFi activo. **ADC2** (GPIO 0, 2, 4, 12, 13,
   14, 15, 25, 26, 27) **no se puede leer mientras WiFi está encendido** — las
   lecturas devuelven ruido/0.
2. **GPIO34–39 son solo-entrada** (sin pull-up interno y **no pueden "escribir"**).
   Sirven para ADC, pulsos o entradas digitales, **no** para 1-Wire ni salidas.
3. **Pines de strapping (boot):** GPIO0 (modo boot), GPIO2 (debe estar ALTO al
   arrancar), GPIO5 (SDIO), GPIO12 (VDD_SDIO, debe estar BAJO), GPIO15 (debe estar
   BAJO al arrancar). Usarlos para otra cosa puede impedir el arranque.
4. **Bus SPI (VSPI)** reserva **GPIO 18 (SCK) / 23 (MOSI) / 19 (MISO)** con dos
   chip-select: **GPIO5 = W5500** y **GPIO4 = microSD**.
5. **Ethernet RMII (LAN8720A) NO aplica a esta placa**: se usa W5500 por SPI.

> ⚠️ **Hallazgo de firmware (pendiente de decisión):** `include/hw/HwProfile.hpp`
> para `BOARD_ESP32_WROOM` define `SEMA_NATIVE_ETH 1` (LAN8720A RMII) y el bus SPI
> LoRa/SD en GPIO 14/12/15. **Tu placa real usa W5500 (SPI) en VSPI 18/19/23**, así
> que hay que consolidar W5500 al bus SPI compartido (driver Arduino Ethernet) o
> añadir un segundo bus SPI. Ver §8.

---

## 2. Mapa final de pines (v2 SMD, corregido 2026-11-14)

| # | Sensor | Pin v2 | Tipo | Estado |
|---|--------|--------|------|--------|
| 1 | BH1750 (luz) | SDA 21 / SCL 22 | I2C `0x23` | ✅ OK |
| 2 | MQ-7 (CO) | GPIO34 | ADC1_CH6 | ✅ OK (ver §3 heater) |
| 3 | S12SD (UV) | GPIO35 | ADC1_CH7 | ✅ OK |
| 4 | PMS5003 (partículas) | RX 16 / TX 17 / SET 33 / RESET 32 | UART | ✅ OK |
| 5 | DS18B20 (1-Wire) | **GPIO27** | 1-Wire | ✅ corregido (estaba siempre aquí) |
| 6 | WH-SP-RG (pluviómetro) | **GPIO13** | PCNT (pulsos) | ✅ OK |
| 7 | WH-SP-WS01 (anemómetro) | **GPIO39** | ADC1_CH3 | ⚠️ ver §4 |
| 8 | CCS811-HDC1080 | SDA 21 / SCL 22 | I2C `0x5A`/`0x40` | ✅ OK |
| 9 | BME280 + AHT20 | SDA 21 / SCL 22 | I2C `0x76`/`0x38` | ✅ OK |

> **Pines libres (confirmados por el usuario):** GPIO **25, 26, 14, 12, 15, 2**.
> Como reserva quedan los strapping GPIO12 (MTDI), GPIO15 (MTDO) y GPIO2
> (strapping + LED onboard) — no usarlos salvo necesidad extrema. GPIO25/26 son
> ADC2 (no legibles con WiFi activo); sirven como GPIO digital de propósito general.

---

## 3. Correcciones específicas (detalle)

### MQ-7 (monóxido de carbono) — GPIO34
- La lectura ADC está bien, pero **el MQ-7 necesita un calefactor de 5 V** que
  consume ~1,4 A: no alimentarlo desde el 3V3 del ESP32. Manejar el heater con un
  MOSFET/transistor + pin de control (no está en tu lista: agregarlo).
- Requiere **precalentado 24–48 h** antes de calibrar y respuesta **logarítmica**
  (no lineal): usar una tabla de calibración, no `scale` lineal.
- La resistencia de carga (RL) forma el divisor con la resistencia interna del
  sensor; fijarla (10k típico) y calibrar contra gas de referencia.

### S12SD (UV) — GPIO35
- Salida analógica ~0,1 V por índice UV (GUVA-S12SD). Con ADC de 12 bits y
  atenuación de 11 dB (0–3,3 V), `uv_index = raw * (3.3/4095) / 0.1`.
- GPIO35 es ADC1_CH7 (funciona con WiFi). El divisor de referencia puede ajustarse
  con `scale`/`offset` en la config.

### PMS5003 (partículas) — UART
- RX 16 / TX 17 (UART 3,3 V). SET (standby) en GPIO33, RESET en GPIO32.
- Requiere alimentación estable (el ventilador consume picos); un capacitor de
  ~100 µF cerca del módulo evita reinicios.

### DS18B20 (1-Wire) — GPIO27 (no 14, no 37)
- 1-Wire es **bidireccional open-drain**: el maestro debe poder bajar la línea.
  GPIO34–39 son solo-entrada, por eso GPIO37 **no funciona**.
- **GPIO27** es un GPIO de propósito general (no strapping, no solo-entrada) y era
  el pin original del sensor. Pull-up de 4,7 kΩ a 3V3.

---

## 4. Viento: WS01 (anemómetro) en GPIO39 — a verificar

Tu mensaje asigna **WH-SP-WS01 (anemómetro) → GPIO39 (ADC1_CH3)**. Dos cosas a
confirmar porque afectan el tipo de sensor en el firmware:

1. **Anemómetro = pulsos, no analógico (normalmente).** El anemómetro de cazoletas
   (velocidad del viento) suele ser un reed/hall que emite **pulsos** → se lee por
   **PCNT**, no por ADC. La veleta (dirección) sí es un divisor resistivo → **ADC**.
   - Si tu WS01 es de salida **analógica (0–5 V)**, está bien en GPIO39/ADC.
   - Si es de **pulsos (reed/hall)**, debe ir a PCNT (p. ej. GPIO36 o un libre).
2. **¿Dónde quedó la veleta (WH-SP-WD, dirección)?** No aparece en tu lista. Antes
   estaba en GPIO39; si ahora GPIO39 es el anemómetro, la veleta necesita otro pin
   ADC1 (GPIO34/35/36/39 están tomados; quedan solo-entrada 34–39 → solo GPIO36
   libre como ADC1).

> En SEMA, `WH-SP-WS01` se modela como **PCNT** (velocidad) y `WH-SP-WD` como
> **ADC** (dirección). Verificar contra el datasheet real del módulo.

---

## 5. Presupuesto de pines (W5500 + VSPI)

| Recurso | Pines | Estado |
|---------|-------|--------|
| SPI VSPI (SD + W5500) | 18 (SCK), 23 (MOSI), 19 (MISO) + CS4/CS5 | ✅ Usado |
| I2C (todos los sensores I2C) | 21 (SDA), 22 (SCL) | ✅ Usado |
| UART (PMS5003 + serial) | 16 (RX), 17 (TX) | ✅ Usado |
| ADC1 (34, 35, 39) | MQ-7, S12SD, WS01 | ✅ Usado |
| 1-Wire | 27 (DS18B20) | ✅ Usado |
| PCNT | 13 (pluviómetro) | ✅ Usado |
| Libres | 25, 26, 14, 12, 15, 2 | ✅ 6 pines |

---

## 6. Recomendaciones de diseño para la v2 (SMD)

1. **Bus I2C único** (SDA=GPIO21, SCL=GPIO22) con pull-ups de 4,7 kΩ: BH1750
   `0x23`, BME280 `0x76`, AHT20 `0x38`, CCS811 `0x5A`, HDC1080 `0x40`.
2. **Entradas ADC al exterior** (UV, CO, viento): resistor serie (1k) + capacitor
   0,1 µF a GND + TVS/ESD para proteger contra descargas.
3. **Sensores de pulsos** (pluviómetro GPIO13, y anemómetro si es PCNT): pull-up
   10k a 3V3 + cap 1 µF a GND (debounce).
4. **MQ-7**: etapa de heater dedicada (MOSFET + pin de control) y conector de 5 V.
5. **DS18B20**: GPIO27 + pull-up 4,7 kΩ.
6. **Pines de depuración**: dejar GPIO1/GPIO3 (UART0) libres para serial de log.
7. **No tocar los strapping** (GPIO2, GPIO12, GPIO15) salvo que no quede alternativa.

---

## 7. Nota sobre ESP32-P4 (nuevo en la lista)

El **ESP32-P4 es RISC-V y no tiene WiFi/Bluetooth** (ni MAC Ethernet nativa): se
usa para edge-AI/HMI con mucha RAM/CPU y Ethernet por SPI (W5500). En SEMA es un
perfil **experimental**: la web local de configuración depende de WiFi, así que el
P4 solo aplica como nodo de cómputo/concentrador con Ethernet. El env `esp32-p4`
quedó **deshabilitado** en `platformio.ini`: la plataforma `espressif32 7.1.3`
instalada no trae el board P4 ni el core RISC-V (requiere `espressif32 ≥ 7.2`).
El código ya está preparado (`BOARD_ESP32_P4` en `HwProfile.hpp`).

---

## 8. Acciones de firmware derivadas de este mapa

1. **Ethernet W5500 (decisión pendiente):** `include/hw/HwProfile.hpp` define
   `SEMA_NATIVE_ETH 1` (RMII) para `BOARD_ESP32_WROOM`. Para que el W5500 del PCB
   funcione hay que **consolidar W5500 al bus VSPI compartido** (driver Arduino
   Ethernet) o **añadir un segundo bus SPI**. Hoy no se puede solo poner
   `SEMA_NATIVE_ETH 0` porque el driver ESP-IDF de W5500 y el SPI de Arduino (SD/
   LoRa) pelearían por los mismos pines 18/19/23.
2. **Pines de sensores** (si se hace variante `SEMA_PINS_FROM_FILE=1`): DS18B20→27,
   pluviómetro→13, anemómetro→39, S12SD→35, MQ-7→34, PMS5003→16/17/33/32.
3. **Identidad 32 MB**: ya corregida en v1.104.0 (`esp32-wroom32u-32mb`), pero la
   placa real es devkit v1 (4 MB), no WROOM-32U — el env `esp32-wroom-32u*` no
   aplica a este PCB.
