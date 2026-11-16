# Fase 1 — Modbus TCP, BACnet/IP y transceivers (investigación)

> Fecha: 2026-11-15 · Objetivo: sumar **Modbus TCP** y **BACnet/IP** sobre la capa
> Ethernet existente + confirmar compatibilidad de transceivers RS-485/CAN.
> Target: ESP32 (clásico/S3/C6/WROOM-32U), PlatformIO `espressif32` 8.0.0
> (Arduino core 3.x / ESP-IDF 5.5). Estado real relevado: `platformio.ini` ya define
> `SEMA_USE_MODBUS=1`, `SEMA_MODBUS_ISOLATED=1`, `SEMA_USE_CAN=1`, `SEMA_USE_ETHERNET=1`;
> la red (`EthernetManager`), `ModbusManager` (RTU) y `CanManager` (TWAI) viven hoy en
> `ESP32-Base` (`lib_deps = .../ESP32-Base.git#v0.5.0`), no en SEMA.

## Resumen ejecutivo

| Área | Recomendación | Registro PlatformIO | Viabilidad |
|------|---------------|---------------------|-----------|
| Modbus TCP servidor | `emelianov/modbus-esp8266` (clase `ModbusIP`) | `emelianov/modbus-esp8266@^4.x` | ✅ Alta, trivial |
| Modbus TCP (alt.) | `arduino-libraries/ArduinoModbus` (clase `ModbusTCPServer`) | `arduino-libraries/ArduinoModbus@^1.0.x` | ✅ Alta |
| BACnet/IP | `bacnet-stack/bacnet-stack` vía port `vitaly.ruhl/ESP32 BACnet Stack` | `vitaly.ruhl/ESP32 BACnet Stack@^0.34.0` | ⚠️ Viable, **licencia GPL** es el bloqueo real |
| RS-485 | Sin librería nueva (manejo de DE/RE ya existe) | — | ✅ Ya cubierto |
| CAN | Sin librería nueva (TWAI ya existe) | — | ✅ Ya cubierto |

---

## 1. Modbus TCP (servidor/esclavo)

**Recomendación: [`emelianov/modbus-esp8266`](https://github.com/emelianov/modbus-esp8266)**
(clase **`ModbusIP`** = servidor/esclavo TCP).

- Es **servidor** nato: expone holding registers, coils, discrete inputs e input
  registers vía *callbacks* (`addHreg`, `addCoil`, `onSetHreg`, `onGetCoil`…), ideal
  para exponer las magnitudes de SEMA dinámicamente.
- Unifica en **una sola librería** RTU + IP + TCP-master
  ([ref DeepWiki](https://deepwiki.com/emelianov/modbus-esp8266/4-modbustcp-and-network-implementations)),
  así podría incluso reemplazar al actual `4-20ma/ModbusMaster` (RTU **master**).
- Es C++ puro sobre las abstracciones `Stream`/`Client` de Arduino → **agnóstico de
  core**: funciona en ESP32 core 3.x (sin código board-specific), igual que en
  ESP8266. Registro: `emelianov/modbus-esp8266` (última 4.x; confirmar con
  `pio pkg search modbus-esp8266`).

**Alternativa oficial: [`ArduinoModbus`](https://github.com/arduino-libraries/ArduinoModbus)**
(clase [`ModbusTCPServer`](https://deepwiki.com/arduino-libraries/ArduinoModbus/5.2-tcp-server-api)).

- Soporta **servidor TCP** (`ModbusTCPServer`), además de `ModbusTCPClient`/
  `ModbusRTUServer`/`ModbusRTUClient`. Transporte inyectable (`EthernetClient`/
  `WiFiClient`), así que corre sobre la lwIP del ESP32 (Ethernet o WiFi).
- Más simple y "oficial", pero menos flexible (registros estáticos, sin callbacks
  por-registro). Registro: `arduino-libraries/ArduinoModbus` (última 1.0.x).

**Ojo:** el actual `4-20ma/ModbusMaster` es **solo RTU master** — no sirve para
servidor TCP; no es un candidato.

**RAM/Flash:** ambos son mínimos (<50 KB flash; RAM = tamaño de las tablas de
registros que declares). Irrelevante frente a los ~320 KB de RAM del ESP32.

**Puente (bridge):** añadir `SEMA_USE_MODBUS_TCP` independiente; el manager debe
recibir un `Client&` para ser ortogonal al transporte (Ethernet/WiFi), igual que
`EthernetManager` ya hace con la web.

---

## 2. BACnet/IP (el punto difícil)

**Stack: [`bacnet-stack/bacnet-stack`](https://github.com/bacnet-stack/bacnet-stack)** (librería C).

- **Licencia: GPL v2** (con opción de licencia comercial). Hay una
  [discusión abierta sobre la excepción de linking (#582)](https://github.com/bacnet-stack/bacnet-stack/discussions/582).
  **Esto es el bloqueo real**: linkear GPL en el firmware fuerza a distribuir ese
  build bajo GPL. Decisión de negocio del usuario: (a) aceptar GPL en la variante
  BACnet, (b) comprar licencia comercial, o (c) diferir BACnet. Técnicamente no hay
  conflicto con Apache-2.0 (ESP-IDF) ni LGPL (Arduino core), solo obliga GPL al
  binario combinado.

**Port ESP32/Arduino existente: [`vitaly.ruhl/ESP32 BACnet Stack`](https://registry.platformio.org/libraries/vitaly.ruhl/ESP32%20BACnet%20Stack)
v0.34.0** — "BACnet/IP client core + **ESP32 server demo para Arduino**", ya en el
registro PlatformIO (repo [`vitalyruhl/esp32-bacnet-stack`](https://github.com/vitalyruhl/esp32-bacnet-stack)).
Es el punto de arranque directo. Alternativa: [`KaiDroste/bacnet-stack-esp32`](https://github.com/KaiDroste/bacnet-stack-esp32).

**Memoria (viabilidad en ESP32 ~320 KB RAM / 4–16 MB flash):**
- Un servidor BACnet/IP mínimo (Device + 10–30 objetos analog/binary) es **viable**.
  Estima (a medir, no pude abrir datasheets): flash ~150–400 KB, RAM estática
  ~10–30 KB + búfer APDU (1476 B por defecto en Ethernet, configurable a 480 B en
  BACnet/IP puro). La RAM no es el problema; el *footprint* de flash en boards de
  4 MB sí hay que vigilarlo con el resto del firmware.
- **Integración:** core 3.x = ESP-IDF 5.5 con lwIP → el stack C se linkea usando
  BSD sockets (`socket`/`recvfrom`/`sendto`). No hay que "portar" sockets, solo
  cablear el evento BACnet al loop. Más natural en ESP-IDF puro, pero Arduino core
  3.x lo permite igualmente (es el mismo lwIP debajo).

**Recomendación:** viable y factible, pero **condicionado a resolver la licencia
GPL**. Arrancar del port `vitaly.ruhl` detrás de `SEMA_USE_BACNET` (compilación
condicional), con el server mínimo (Device + objetos) como PoC antes de expandir.

---

## 3. Transceivers RS-485 (Modbus RTU)

| Transceiver | V | Half-duplex | DE/RE | Aislamiento | Encapsulado |
|-------------|---|-------------|-------|-------------|-------------|
| **MAX3485ED** | 3.3 V | sí | DE + /RE (separados) | no | SOIC-8 |
| **SN65HVD75DR** | 3.3 V | sí | DE + /RE | no | SOIC-8 |
| **TD501D485H** | 3.3 V (o 5 V) | sí | DE/RE (var. `-A` = auto-dirección) | **sí** (galvánico) | módulo |
| **ADM2483BRWZ** | 3.3/5 V lógica + bus | sí | DE + /RE | **sí** (iCoupler) | SOIC-16 |

**Confirmado:**
- Los cuatro son **half-duplex** con dirección por **DE/RE** (driver/receiver
  enable). `MAX3485ED` y `SN65HVD75DR` son **pin-compatibles** (pinout estándar
  SOIC-8: RO/RE/DE/DI/GND/A/B/VCC).
- `ADM2483BRWZ` ([Analog Devices](https://www.analog.com/en/products/adm2483.html))
  y `TD501D485H` ([MORNSUN](http://www.mornsun-power.com/uploads/pdf/TDx01D485H-E.pdf))
  añaden **aislamiento galvánico** (por eso SEMA ya distingue
  `SEMA_MODBUS_ISOLATED`); no cambian el firmware, cambian la PCB (masas separadas).
- Variante **`TD501D485H-A`** ([auto-dirección](https://www.mornsun-power.com/public/uploads/pdf/TDx01D485H-A.pdf)):
  no necesita DE/RE.

**Conclusión:** el soporte en firmware **se reduce a manejar el pin DE/RE**, que SEMA
**ya hace** (`SEMA_PIN_MODBUS_DERE`, `HwProfile.hpp`). Para añadir MAX3485ED y
ADM2483BRWZ alcanza con: (1) ampliar el enum de transceiver y (2) documentar el
mapeo de pines; **cero librería nueva**. Para las variantes auto-dirección, el GPIO
DE/RE queda opcional.

---

## 4. Transceivers CAN

| Transceiver | V | Pin 8 (RS) | Compatible TWAI (3.3 V) |
|-------------|---|-----------|--------------------------|
| **SN65HVD234DR** | 3.3 V | RS = **standby** (listen-only baja corriente) | ✅ |
| **SN65HVD233DR** | 3.3 V | RS = **standby** (listen-only baja corriente) | ✅ |

**Corrección de la premisa:** ambos `SN65HVD233` y `SN65HVD234` usan el pin 8 (RS)
como **standby/listen-only** (alto = modo baja corriente), **no** slope control.
El *slope control* es del **SN65HVD230** (RS→GND con resistor) y el *sleep* del
**SN65HVD231** ([familia SN65HVD23x, TI](https://www.ti.com/lit/ds/symlink/sn65hvd233-q1.pdf)).
La diferencia 233↔234 es menor (corriente de standby / soporte de loopback) e
**irrelevante** para operación normal de SEMA.

**Conclusión:** ambos son 3.3 V, pin-compatibles y **directamente compatibles con el
TWAI** del ESP32 (ya cableado en `CanManager`, `SEMA_PIN_CAN_TX/RX`). El firmware **no
necesita** manejar RS para operar: alcanza con un GPIO **opcional** configurable
(default LOW = modo normal; HIGH = standby para ahorro). Cero librería nueva.

---

## 5. Veredicto y riesgos

1. **BACnet/licencia GPL** (ALTO): único bloqueo real de Fase 1. Resolver antes de
   escribir código (aceptar GPL / licencia comercial / diferir).
2. **Core 3.x no verificado** (MEDIO): ni `emelianov` ni `ArduinoModbus` ni el port
   BACnet declaran CI sobre core 3.x; son software puro sobre abstracciones Arduino,
   pero hay que **compilar** los 5 envs contra core 3.x antes de dar por hecho.
3. **RAM/flash BACnet** (MEDIO): estimado viable, pero **medir** el footprint real en
   `esp32doit-devkit-v1` (4 MB) antes de sumarlo al firmware completo.
4. **Flags ortogonales** (BAJO): respetar `SEMA_USE_MODBUS_TCP` / `SEMA_USE_BACNET`
   independientes y transporte-agnósticos (`Client&`), para no acoplar a Ethernet.
