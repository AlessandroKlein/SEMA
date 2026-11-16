# Pendientes — SEMA

> **Tipo:** Registro de pendientes | **Fecha:** 2026-11-15 | **Firmware:** v1.114.0

Resultado de la comprobación de `MEJORAS.md`, `FUTURO.md` y `CONTINUACION.md`
contra el código. Los ítems **no realizados** quedan aquí para retomarlos en otra
instancia.

---

## 1. Resumen de la comprobación

Se verificó que los ítems marcados como **✅ (hecho)** en `MEJORAS.md` §2 (DoD) y
§3 están efectivamente implementados: core, config, web, API, sensores,
expansores (MCP23017/ADS1115/74HC595), buses (RS485/Modbus/CAN/LoRa/Zigbee/
Ethernet), histórico, alarmas, calibración, OTA, seguridad básica, watchdog y
documentación. **No hay falsos positivos.**

Se corrigieron dos inconsistencias de documentación:

- `MEJORAS.md` §1 (Fases): las fases 3–6 y los sensores de meteorología/calidad
  estaban marcados `⬜`/`🔄` pese a estar hechos; ahora figuran `✅`.
- `FUTURO.md` §5: se retiró "Veleta (dirección del viento)" porque ya se
  implementó (WH-SP-WD con calibración por tabla de resistencias, v1.29.0).

---

## 2. Pendientes reales

### 2.1 Parciales (🔄 — en curso, requieren más trabajo)

| Ítem | Estado actual | Qué falta |
|------|---------------|-----------|
| Sistema de módulos | Interfaz + registro | Módulos reales (instalación/desinstalación/permisos). |
| Medición energética | Batería por ADC | Medición de consumo/corriente. |
| Diagnóstico | Health monitor + endpoints | Diagnóstico por sensor más profundo. |
| Seguridad | api_key + server_key + login + rate limiting + keys revocables (`extra_keys`) | TLS/HTTPS, OTA firmado (keys revocables parcial: RBAC completo). |
| Comunicaciones remotas | MQTT/LoRa/Zigbee/Ethernet | Servidor Central (fuera de alcance). |
| Energía | Batería, deep sleep | Gestión de carga/MPPT del panel solar. |

### 2.2 Fuera de alcance

| Ítem | Motivo |
|------|--------|
| Servidor Central (multiestación, mapas, alertas) | Proyecto separado; SEMA solo se conecta con `server_key`. |

### 2.3 Futuras / opcionales (catálogo `FUTURO.md`)

**Seguridad**
- TLS/HTTPS (certificado).
- OTA con firma/checksum.
- Tokens/API keys revocables (parcial: `extra_keys` con revocación; falta RBAC completo).

**Comunicaciones**
- LoRaWAN (red sobre LoRa).
- Red Zigbee multi-dispositivo.
- BLE (configuración por Bluetooth).
- CoAP / MQTT-SN.

**Energía**
- Gestión de carga/MPPT del panel solar.
- Batería LiFePO4 / Li-ion (curvas de descarga).
- Perfiles energéticos por horario.
- Wake por RTC (alarmas de hora).

**Fiabilidad y operación**
- RTC hardware (DS3231) con batería.

**Plataforma**
- Multi-estación en un dashboard.

### 2.4 Nuevos pendientes del usuario (2026-11-14)

| Ítem | Estado | Detalle |
|------|--------|---------|
| Habilitar/deshabilitar elementos en runtime | ✅ hecho (v1.106.0) | Toggle runtime de elementos (sensores, buses, Ethernet, Modbus, CAN, LoRa, Zigbee) persistido en NVS, incluso en PCB. |
| Edición de pines solo en modo AP | ✅ hecho (v1.106.0) | `pinsLocked()`: en STA/PCB los pines quedan solo-lectura; `/api/v1/system` expone `ap_mode`/`pins_from_file`. |
| Repo base reutilizable | ✅ hecho (v1.107.0 → v0.7.0) | `AlessandroKlein/ESP32-Base` (tag `v0.10.0`, commit `a0f9efb`); SEMA consume por `lib_deps` `#v0.10.0`; 65 módulos genéricos movidos; `src/core/SemaGlue.cpp` con 14 hooks. F1+F2+F3 completadas, code review aplicado. v0.2.0…v0.10.0 añaden: expansores I²C (Fase 0a), detección de chip, compat C6, rediseño web, expansores UART/SPI/mux/ADC (Fase 0b), servidor Modbus TCP (Fase 1a), servidor BACnet/IP (Fase 1b), mesh ESP-NOW (Fase 2a) y selector de PHY Ethernet (LAN8720A/W5500), flag SEMA_USE_WEB (Fase 2b), dashboard limpio (sin Gridstack) + config en LittleFS (D-0046) + reloj UTC, y migración ESP-IDF core 3.x. Plan: `docs/REPO-BASE-PLAN.md`. |
| Documentador dedicado de `Docs.git` | ✅ regla permanente | Docs.git sincronizado → v1.107.0 (commit `a454b92`). |
| Compatibilidad dual PCB↔modular con flags | ✅ regla permanente | Registrada en AGENTS.md §4.2. |
| Ethernet W5500 (consolidar bus SPI) | ⬜ pendiente | PCB WROOM-32U usa W5500 en VSPI 18/19/23 (CS=GPIO5) pero el firmware define `SEMA_NATIVE_ETH 1` (RMII). Decidir: Arduino Ethernet en bus compartido vs 2º bus SPI. Ver `docs/PCB-V1-CORRECCIONES.md` §8. |

### 2.5 Expansión ESP32-Base → framework de domótica (2026-11-15)

Objetivo: ampliar SEMA + ESP32-Base hacia un **framework de domótica y
automatización de edificios** (BACnet/IP, Modbus TCP, expansores, multi-ESP32,
mesh, coordinador, válvulas BLE/Zigbee).

**Roadmap acordado:**

| Fase | Alcance | Estado |
|------|---------|--------|
| 0 | Expansores (MCP23S17, PCA9685, PCF8574/8575, TCA9548A, SC16IS752, MAX14830, SC18IS602B, ADS1115 genérico) + flags `SEMA_USE_*` | ✅ (0a GPIO/PWM; 0b UART/SPI/mux/ADC — MAX14830 diferido, sin librería en registry) |
| 1 | Modbus TCP + BACnet/IP + verificar CAN + transceivers RS-485/CAN | ✅ (Modbus TCP v0.6.0 + BACnet/IP v0.7.0; transceivers RS-485/CAN; CAN por TWAI ya existía) |
| 2 | Mesh ESP-NOW sin central + maestro-esclavo (C3/S2/S3; offload web/Ethernet) | ✅ (2a `MeshManager` ✅ v0.8.0; 2b esclavos C3/S2 ✅ v0.9.0 via `SEMA_USE_WEB`) |
| 3 | Coordinador + modelo árbol (Casa→Habitación→Dispositivo→Capacidad, "a qué escucha") | ⬜ |
| 4 | Esclavos radio C6/H2 (Zigbee/Thread/Matter) + válvulas/concentradores BLE (esfuerzo ALTO, ESP-IDF) | ⬜ |

**Decisiones:** mesh = ESP-NOW primero; primer entregable = Fase 0.
**Deuda diferida:** renombrar flags `SEMA_USE_*` → `BASE_USE_*` (mecánico, toca todo).
**Fase 0 desglosada:** 0a = flags + GPIO/PWM (MCP23S17, PCA9685, PCF8574/8575);
0b = UART (SC16IS752, MAX14830, SC18IS602B) + mux TCA9548A + ADS1115 genérico.

**Fase 1 desglosada (alcance confirmado por el usuario 2026-11-15):**
- **Modbus TCP servidor** (holding registers + coils), flag `SEMA_USE_MODBUS_TCP`.
- **BACnet/IP servidor** (objeto Device + analog/binary mapeados a sensores), flag `SEMA_USE_BACNET`. Stack `bacnet-stack` (GPL v2) vía port ESP32 `vitaly.ruhl/ESP32 BACnet Stack@^0.34.0` — **licencia GPL v2 ACEPTADA por el usuario (2026-11-15)**: el firmware que lo vincule queda bajo GPL v2.
- **Transceivers RS-485** (Modbus RTU): MAX3485ED / SN65HVD75DR / TD501D485H / ADM2483BRWZ (a elegir).
- **Transceivers CAN**: SN65HVD234DR / SN65HVD233DR.
- **Principio de ortogonalidad**: Modbus RTU (RS-485) y CAN (no-TCP) se habilitan con flags independientes, sin depender de Ethernet/WiFi; el dispositivo actúa de **puente** entre protocolos.
- **Ethernet común** sigue funcionando con BACnet/IP y Modbus TCP deshabilitados (son capas opcionales sobre la red).

### 2.6 Compatibilidad multi-SoC + detección de chip (2026-11-15)

Objetivo: función en ESP32-Base que **determina el ESP32** (compile-time por
target) y **habilita solo lo compatible** (capability matrix), para soportar los
SoC de la lista del usuario.

**Realidad verificada (espressif32 8.0.0 + arduino core 3.x):** los ~40 SoC de
la lista colapsan a **11 targets IDF** (los 9 de abajo + C2/ESP8684 + S31
pre-release); hoy compilan **6** (C6 validado con env propio):

| Target | WiFi | BLE/BT | 802.15.4 | USB-OTG | Eth MAC | Núcleos | Arq | ¿Compila hoy? |
|---|---|---|---|---|---|---|---|---|
| esp32 (D0WD/U4WDH/PICO) | 4 | BT+BLE | ✗ | ✗ | ✓ | 2 | Xtensa | ✅ |
| esp32s2 | 4 | ✗ | ✗ | ✓ | ✗ | 1 | Xtensa | ✅ |
| esp32s3 | 4 | BLE5 | ✗ | ✓ | ✗ | 2 | Xtensa | ✅ |
| esp32c3 (y ESP8685) | 4 | BLE5 | ✗ | ✗ | ✗ | 1 | RISC-V | ✅ |
| esp32c6 | 6 | BLE5 | ✓ | ✗ | ✗ | 1 | RISC-V | ✅ (env `esp32-c6-devkitc-1` añadido y validado) |
| esp32h2 (H4/H21) | ✗ | BLE5 | ✓ | ✗ | ✗ | 1 | RISC-V | ❌ requiere arduino-esp32 3.x |
| esp32c5 | 6 | BLE5 | ✓ | ✗ | ✗ | 1 | RISC-V | ❌ IDF más nuevo |
| esp32c61 | 6 | BLE5 | ✗ | ✗ | ✗ | 1 | RISC-V | ❌ IDF más nuevo |
| esp32p4 | ✗ | ✗ | ✗ | ✓ | ✓ | 2 | RISC-V | ❌ IDF más nuevo |

Nota: "ESP32-S31NRV16/32 (Pre-release)" no se reconoce como parte oficial
(probable errata); tratar como familia ESP32-S3 hasta confirmar nombre.

**Diseño de la detección:**
- **Compile-time** (núcleo): header `include/base/chip.hpp` mapea
  `CONFIG_IDF_TARGET_ESP32*` → `BASE_CHIP_*` (identidad) + `BASE_HAVE_*`
  (capacidades: WiFi/BLE/802.15.4/USB-OTG/Eth-MAC/dual-core/RISC-V). Los flags
  `SEMA_USE_*` por defecto = `BASE_HAVE_*` (no se habilita Ethernet en un C3 sin
  MAC, salvo que se compile un driver SPI explícito).
- **Runtime** (solo lo que varía por módulo/paquete): `base::chipModel()`,
  `base::chipRevision()`, `base::flashSize()`, `base::psramSize()`,
  `base::coreCount()` — wrappers finos de `ESP.getChipModel()` etc.

**Estado (2026-11-15):** `chip.hpp` + `ChipInfo.hpp` **implementados** en
ESP32-Base v0.3.0 (11 targets). Env `esp32-c6-devkitc-1` **añadido y validado**
(v0.3.1: PowerManager GPIO-wakeup + guard de `ethernet` en `HttpServer::onNetwork`
+ OneWire fijado a commit con soporte C6). Pendiente: envs **S2/C3** para validar
compilación; H2/C5/C61/P4 = sin board en espressif32 8.0.0 (esperar PlatformIO
más nuevo).

---

## 3. Regla de actualización

Este archivo se actualiza al **retirar** un ítem (cuando se implementa y pasa al
`CHANGELOG`) o al **agregar** nuevos pendientes. No requiere release
(docs-only, ver `VERSIONADO.md`).
