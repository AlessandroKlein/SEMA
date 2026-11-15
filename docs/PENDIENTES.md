# Pendientes — SEMA

> **Tipo:** Registro de pendientes | **Fecha:** 2026-11-14 | **Firmware:** v1.106.0

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
| Repo base reutilizable | 🔄 en curso (Fase 1, pausada) | Repo `AlessandroKlein/ESP32-Base` creado; F1 iniciada (commit `221dfb3`: esqueleto + módulos copiados, **sin renombrar** a `base::`). Re-despachar F1 para: rename namespace + `include/base/` + resolver 7 acoples + lado SEMA. Quedan F2/F3. Plan: `docs/REPO-BASE-PLAN.md`. |
| Documentador dedicado de `Docs.git` | ✅ regla permanente | Docs.git sincronizado v1.103.0 → v1.106.0 (commit `08a1bac6`). |
| Compatibilidad dual PCB↔modular con flags | ✅ regla permanente | Registrada en AGENTS.md §4.2. |
| Ethernet W5500 (consolidar bus SPI) | ⬜ pendiente | PCB WROOM-32U usa W5500 en VSPI 18/19/23 (CS=GPIO5) pero el firmware define `SEMA_NATIVE_ETH 1` (RMII). Decidir: Arduino Ethernet en bus compartido vs 2º bus SPI. Ver `docs/PCB-V1-CORRECCIONES.md` §8. |

---

## 3. Regla de actualización

Este archivo se actualiza al **retirar** un ítem (cuando se implementa y pasa al
`CHANGELOG`) o al **agregar** nuevos pendientes. No requiere release
(docs-only, ver `VERSIONADO.md`).
