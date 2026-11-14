# PCB v1 — Revisión de pines y correcciones (rumbo a v2 SMD)

> **Fecha:** 2026-11-14 · **Base:** ESP32 devkit v1 (WROOM, 4 MB) · **Firmware:** v1.103.0
>
> **Hardware real confirmado por el usuario (m00216):** la placa usa **W5500 (SPI
> Ethernet, CS=GPIO5)** + **VSPI (SCK18/MOSI23/MISO19)** + **microSD (CS=GPIO4)** +
> **I2C (SDA21/SCL22)** + **UART (TX17/RX16)**. **No usa Ethernet RMII nativo.**

Revisión de la asignación de pines de tu PCB v1 contra (a) las reglas de oro del
ESP32-WROOM y (b) las reservas reales de la placa (bus SPI VSPI para SD/W5500 y
sensores). Los errores marcados **ERROR** son bloqueantes para que el sensor
funcione; los **WARNING** funcionan pero con limitaciones.

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
   chip-select: **GPIO5 = W5500** y **GPIO4 = microSD**. Los pines de HSPI
   (GPIO 12/13/14/15) **están libres** en esta placa (no hay RMII ni LoRa en HSPI).
5. **Ethernet RMII (LAN8720A) NO aplica a esta placa**: se usa W5500 por SPI, así
   que los pines RMII (GPIO 0, 25, 26, 27) quedan libres para otros usos.

> ⚠️ **Hallazgo de firmware (a corregir en la integración):** `include/hw/HwProfile.hpp`
> para `BOARD_ESP32_WROOM` y `BOARD_ESP32_WROOM32U` define `SEMA_NATIVE_ETH 1`
> (LAN8720A RMII) y el bus SPI LoRa/SD en GPIO 14/12/15. **Tu placa real usa W5500
> (SPI) en VSPI 18/19/23**, así que hay que cambiar `SEMA_NATIVE_ETH 0` y los pines
> SPI a 18/19/23 para que Ethernet y SD funcionen en tu PCB.

---

## 2. Revisión por sensor (mapa final resuelto para v2)

| # | Sensor | Pin v1 | Veredicto | Pin v2 (resuelto) |
|---|--------|--------|-----------|-------------------|
| 1 | BH1750 (luz, I2C) | I2C | ✅ OK | SDA 21 / SCL 22. Dirección `0x23`. |
| 2 | MQ-7 (CO, ADC) | GPIO34 | ✅ OK (ADC1_CH6) | Mantener. Ver §3 (heater + RL). |
| 3 | S12SD (UV, ADC) | GPIO35 | ✅ OK (ADC1_CH7) | Mantener GPIO35. |
| 4 | PMS5003 (UART) | set GPIO15, reset GPIO2 | ❌ ERROR | **RX 16 / TX 17 / SET 33 / RESET 32** (corregido). |
| 5 | WH-SP-WS01 (anemómetro) | GPIO36 | ✅ OK | Mantener GPIO36 (pulsos PCNT, solo-entrada OK). Solo era la **etiqueta** invertida. |
| 6 | WH-SP-WD (veleta) | GPIO26 | ❌ ERROR | **GPIO39** (ADC1_CH3). GPIO26 era ADC2 + LoRa `DIO1`. |
| 7 | WH-SP-RG (pluviómetro) | GPIO25 | ⚠️ WARNING | **GPIO13** (pulsos PCNT). GPIO25 era ADC2. |
| 8 | CCS811-HDC1080 (I2C) | I2C | ✅ OK | SDA 21 / SCL 22. `0x5A`/`0x5B` + `0x40`. |
| 9 | BME280 + AHT10/AHT20 (I2C) | I2C | ✅ OK | SDA 21 / SCL 22. `0x76` + `0x38`. |
| 10 | DS18B20 (1-Wire) | GPIO37 | ❌ ERROR | **GPIO14** (salida, 1-Wire). El único libre salida-capable sin strapping. |

> **Pines libres para sensores (confirmados por el usuario):** GPIO 14, 12, 13, 15,
> 2, 39. Asignados: **14 → DS18B20**, **39 → veleta**, **13 → pluviómetro**. Sobran
> como reserva GPIO 12 (MTDI strapping), GPIO 15 (MTDO strapping) y GPIO 2
> (strapping + LED onboard) — no usarlos salvo necesidad extrema.

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

### DS18B20 (1-Wire) — mover de GPIO37 a GPIO14
- 1-Wire es **bidireccional open-drain**: el maestro debe poder bajar la línea.
  GPIO34–39 son solo-entrada, por eso GPIO37 **no funciona**.
- **GPIO14** es el único pin libre salida-capable sin strapping (GPIO12/15/2 son
  strapping y quedan como reserva). No es ADC2 ni RMII en esta placa (VSPI usa
  18/19/23). Pull-up de 4,7 kΩ a 3V3.

---

## 4. Intercambio de etiquetas WS01 / WD

Tu descripción tiene **invertidas** las dos estaciones de viento:

| Modelo real | Magnitud | Tipo de señal | Tu etiqueta actual |
|-------------|----------|---------------|--------------------|
| **WH-SP-WS01** | Velocidad del viento (anemómetro, cazoletas) | **Pulsos** (reed/hall) → PCNT | Lo marcaste como "dirección" |
| **WH-SP-WD** | Dirección del viento (veleta) | **Analógico** resistivo → ADC + tabla | Lo marcaste como "velocidad" |

- La veleta (WH-SP-WD) es un divisor resistivo: va a **ADC1** (GPIO39) y se
  decodifica con una tabla de resistencias. SEMA ya modela `"WH-SP-WD"` como veleta.
- El anemómetro (WH-SP-WS01, GPIO36) y el pluviómetro (WH-SP-RG, GPIO13) son
  **pulsos de reed**: van a **PCNT**, no a ADC. El capacitor de 1 µF a GND es para
  debounce del reed (correcto).

---

## 5. Presupuesto de pines (actualizado a W5500 + VSPI)

Tu placa usa **W5500 por SPI (VSPI)** en vez de RMII, lo que **libera** los pines
RMII (0, 25, 26, 27) y deja el bus SPI compartido entre SD (CS4) y W5500 (CS5):

| Recurso | Pines | Estado en tu placa |
|---------|-------|--------------------|
| SPI VSPI (SD + W5500) | 18 (SCK), 23 (MOSI), 19 (MISO) + CS4/CS5 | ✅ Usado |
| I2C (todos los sensores I2C) | 21 (SDA), 22 (SCL) | ✅ Usado |
| UART (PMS5003 + serial) | 16 (RX), 17 (TX) | ✅ Usado |
| HSPI (GPIO 12/13/14/15) | libre | ✅ Reasignado a sensores |
| ADC1 (34, 35, 36, 39) | entrada analógica/pulsos | ✅ MQ-7, S12SD, anemómetro, veleta |
| GPIO14 | salida (DS18B20) | ✅ 1-Wire |
| Strapping (0, 2, 5, 12, 15) | reserva | ⚠️ 5 = CS W5500 (OK por diseño); 2/12/15 como reserva |

**Conclusión:** con W5500 + VSPI no falta ningún pin. El DS18B20 va a **GPIO14**
(salida), la veleta a **GPIO39** (ADC1) y el pluviómetro a **GPIO13** (pulso). No
es necesario soltar buses ni pasarse a otra board.

---

## 6. Recomendaciones de diseño para la v2 (SMD)

1. **Bus I2C único** (SDA=GPIO21, SCL=GPIO22) con pull-ups de 4,7 kΩ. Todos los
   sensores I2C conviven en un solo bus: BH1750 `0x23`, BME280 `0x76`, AHT20 `0x38`,
   CCS811 `0x5A`, HDC1080 `0x40`.
2. **Entradas ADC al exterior** (veleta, UV, CO): resistor serie (1k) + capacitor
   0,1 µF a GND + TVS/ESD para proteger contra descargas.
3. **Sensores de pulsos** (anemómetro GPIO36, pluviómetro GPIO13): pull-up 10k a
   3V3 + cap 1 µF a GND (debounce).
4. **MQ-7**: etapa de heater dedicada (MOSFET + pin de control) y conector de 5 V.
5. **DS18B20**: GPIO14 (salida) + pull-up 4,7 kΩ.
6. **Pines de depuración**: dejar GPIO1/GPIO3 (UART0) libres para serial de log.
7. **No tocar los strapping** (GPIO2, GPIO12, GPIO15) salvo que no quede alternativa.

---

## 7. Nota sobre ESP32-P4 (nuevo en la lista)

El **ESP32-P4 es RISC-V y no tiene WiFi/Bluetooth** (ni MAC Ethernet nativa): se
usa para edge-AI/HMI con mucha RAM/CPU y Ethernet por SPI (W5500). En SEMA es un
perfil **experimental**: la web local de configuración depende de WiFi, así que el
P4 solo aplica como nodo de cómputo/concentrador con Ethernet. Queda registrado
como `esp32-p4` en `platformio.ini` con la salvedad de que el core Arduino para P4
está madurando.

---

## 8. Acciones de firmware derivadas de este mapa (pendiente de integración)

1. **`include/hw/HwProfile.hpp`**: `SEMA_NATIVE_ETH` debe ser `0` (W5500) para
   `BOARD_ESP32_WROOM`/`BOARD_ESP32_WROOM32U` (hoy es `1` = RMII).
2. **Bus SPI LoRa/SD**: cambiar de GPIO 14/12/15 a **18/23/19** (VSPI) para que SD
   y LoRa compartan bus con el W5500 (cada uno con su CS: SD=4, W5500=5).
3. **Pines de sensores fijos** (si `SEMA_PINS_FROM_FILE=1`): DS18B20→14, veleta→39,
   pluviómetro→13, anemómetro→36, S12SD→35, MQ-7→34, PMS5003→16/17/33/32.
4. **Identidad 32 MB** (bug ya detectado): el env `esp32-wroom-32u-32mb` hereda
   `SEMA_FLASH_MB 16`; corregir a 32 y `SEMA_BOARD_ID "esp32-wroom32u-32mb"`.
