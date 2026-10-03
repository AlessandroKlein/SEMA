# Mejoras y roadmap — SEMA

> **Tipo:** Roadmap | **Estado:** En desarrollo | **Fecha:** 2026-10-03 | **Versión:** 0.49.0

Estado del desarrollo de SEMA. Las 9 fases provienen de `README.md` §104.
Estado de cada ítem: ✅ hecho · 🔄 en curso/parcial · ⬜ pendiente.

---

## 1. Fases de desarrollo

### Fase 1 — Core
```text
ESP32 · Web · Configuración · NVS · Diagnóstico
```
✅ esencial completa (v0.34.0): ConfigManager+NVS, Storage API, Capability/Runtime
Manager, Scheduler, Web/REST `/api/v1`, WebSocket, mDNS, autenticación y OTA.
Diagnóstico parcial.

### Fase 2 — Sensores básicos
```text
DS18B20 · AHT20/AHT21/AHT30 · SHT31/SHT40 · BME280 · BMP280 · BH1750
```
✅ (v0.39.0): BME280, SHT40, SHT31, DS18B20, BH1750, AHT20 y BMP280 + detección I²C.

### Fase 3 — Expansión
```text
MCP23017 · 74HC595 · 74HC165 · ADS1115
```
⬜ pendiente.

### Fase 4 — Meteorología
```text
Viento · Lluvia · Radiación · UV · Rayos
```
🔄 (v0.47.0): pluviómetro/anemómetro por PCNT (conteo de pulsos).

### Fase 5 — Calidad ambiental
```text
CO₂ · PM · CO
```
🔄 (v0.49.0): CO₂ (SCD30).

### Fase 6 — Industrial
```text
RS485 · Modbus · CAN
```
⬜ pendiente.

### Fase 7 — Comunicaciones remotas
```text
LoRa · Zigbee · Ethernet · MQTT · Servidor central
```
🔄 MQTT listo (publicador); LoRa/Zigbee/Ethernet/Servidor central pendientes.

### Fase 8 — Energía
```text
Panel solar · Batería · Medición energética · Deep Sleep
```
🔄 (v0.34.0): batería por ADC, perfiles energéticos, deep sleep (timer RTC) y
wake-up por lluvia. Falta gestión de panel solar.

### Fase 9 — Plataforma distribuida
```text
Múltiples SEMA · Nodos remotos · Servidor central · Históricos · Mapas · Alertas
```
⬜ pendiente.

---

## 2. Definition of Done (`README.md` §103)

| Requisito | Estado |
|-----------|:------:|
| Core funcionando | ✅ |
| Configuración persistente | ✅ |
| Web local | ✅ |
| API | ✅ |
| Dashboard modular | 🔄 (página básica en `/`) |
| Sistema de módulos | 🔄 (interfaz + registro; sin módulos reales) |
| Sistema de sensores | ✅ |
| Catálogo de sensores | ✅ (configurable vía factoría) |
| Detección I²C | ✅ |
| Detección 1-Wire | ⬜ (driver DS18B20; sin auto-detección) |
| Configuración GPIO | ⬜ |
| Configuración ADC | 🔄 (AdcSensor; no configurable por web) |
| MCP23017 / 74HC595 / 74HC165 | ⬜ |
| RS485 / Modbus RTU / CAN | ⬜ |
| LoRa / Zigbee | ⬜ |
| Medición energética | 🔄 (batería por ADC) |
| Almacenamiento | ✅ |
| Histórico | ✅ |
| Alarmas | ✅ |
| Diagnóstico | 🔄 (health monitor + endpoints) |
| Calibración | ✅ (configurable por canal) |
| Validación de configuración | ✅ |
| Backup / Importación / Exportación | ✅ (GET/POST `/backup`) |
| OTA | ✅ |
| Seguridad | 🔄 (api_key web + server_key del Central) |
| Watchdog | ✅ |
| Documentación | ✅ |

---

## 3. Ítems pendientes y bloqueos

| Ítem | Estado | Notas / vía de solución |
|------|--------|--------------------------|
| Servidor Central | — | Fuera del alcance de SEMA (proyecto separado); SEMA se conecta con `server_key` |
| Expansión GPIO (MCP23017/ADS1115) | ⬜ | Fase 3 (requieren hardware) |
| Buses industriales (RS485/CAN) | ⬜ | Fase 6 (requieren hardware) |
| LoRa / Zigbee | ⬜ | Fase 7 (requieren hardware) |
| Sensores CO₂/PM/UV/viento/lluvia | ⬜ | Fases 4/5 (requieren hardware) |

---

## 4. Regla de actualización

Este archivo se actualiza en el mismo ciclo que el código (`REGLAS-DE-TRABAJO.md`
§2): cada ítem que cambia de estado se refleja aquí y en el repo `Docs`.
