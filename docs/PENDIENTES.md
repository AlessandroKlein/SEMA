# Pendientes — SEMA

> **Tipo:** Registro de pendientes | **Fecha:** 2026-10-07 | **Firmware:** v1.93.0

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

---

## 3. Regla de actualización

Este archivo se actualiza al **retirar** un ítem (cuando se implementa y pasa al
`CHANGELOG`) o al **agregar** nuevos pendientes. No requiere release
(docs-only, ver `VERSIONADO.md`).
