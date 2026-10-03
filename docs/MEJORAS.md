# Mejoras y roadmap — SEMA

> **Tipo:** Roadmap | **Estado:** Planificación | **Fecha:** 2026-10-03 | **Versión:** 0.29.0

Estado del desarrollo de SEMA. Las 9 fases provienen de `README.md` §104.
Estado de cada ítem: ✅ hecho · 🔄 en curso · ⬜ pendiente.

---

## 1. Fases de desarrollo

### Fase 1 — Core
```text
ESP32 · Web · Configuración · NVS · Diagnóstico
```
🔄 en curso (v0.8.0): ConfigManager + NVS, Storage API, Capability Manager, Task,
Scheduler y Web/REST `/api/v1`. Falta diagnóstico, WebSocket y mDNS.

### Fase 2 — Sensores básicos
```text
DS18B20 · AHT20/AHT21/AHT30 · SHT31/SHT40 · BME280 · BMP280 · BH1750
```
🔄 en curso (v0.26.0): Sensor Engine + BME280, SHT40, DS18B20, BH1750 y AHT20.

### Fase 3 — Expansión
```text
MCP23017 · 74HC595 · 74HC165 · ADS1115
```
⬜ pendiente.

### Fase 4 — Meteorología
```text
Viento · Lluvia · Radiación · UV · Rayos
```
⬜ pendiente.

### Fase 5 — Calidad ambiental
```text
CO₂ · PM · CO
```
⬜ pendiente.

### Fase 6 — Industrial
```text
RS485 · Modbus · CAN
```
⬜ pendiente.

### Fase 7 — Comunicaciones remotas
```text
LoRa · Zigbee · Ethernet · MQTT · Servidor central
```
⬜ pendiente.

### Fase 8 — Energía
```text
Panel solar · Batería · Medición energética · Deep Sleep
```
⬜ pendiente.

### Fase 9 — Plataforma distribuida
```text
Múltiples SEMA · Nodos remotos · Servidor central · Históricos · Mapas · Alertas
```
⬜ pendiente.

---

## 2. Definition of Done (`README.md` §103)

Ver [`IMPLEMENTACION.md`](IMPLEMENTACION.md) §11. La lista completa de requisitos
se va marcando conforme se implementa cada fase.

---

## 3. Ítems pendientes y bloqueos

| Ítem | Estado | Notas / vía de solución |
|------|--------|--------------------------|
| Esqueleto modular del Core (registry, event bus, config) | 🔄 | Base de la Fase 1 |
| Clonar y publicar wiki SEMA en `AlessandroKlein/Docs` | ⬜ | Al publicar la release de planificación |
| `firmware_manifest.json` (SHA-256 del binario) | ⬜ | Se crea en la primera release con artefacto `.bin` |

---

## 4. Regla de actualización

Este archivo se actualiza en el mismo ciclo que el código (`REGLAS-DE-TRABAJO.md`
§2): cada ítem que cambia de estado se refleja aquí y en el repo `Docs`.
