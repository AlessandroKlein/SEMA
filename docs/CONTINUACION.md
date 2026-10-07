# Continuación — SEMA

> **Tipo:** Guía de continuación | **Fecha:** 2026-10-03 | **Firmware:** v1.70.0 | **Releases:** 89

Documento de **retorno**: resume el estado actual, lo pendiente y el flujo para
retomar el trabajo después de corroborar. Leer de principio a fin antes de seguir.

---

## 1. Estado actual (hecho)

### Planificación y arquitectura

- Contrato arquitectónico completo: decisiones **D-0001…D-0060** en
  [`DUDAS-Y-DECISIONES.md`](DUDAS-Y-DECISIONES.md) (resuelve las 20 dudas Q-0001…Q-0020).
- Docs de trabajo: [`REGLAS-DE-TRABAJO.md`](REGLAS-DE-TRABAJO.md),
  [`ESTANDAR-DOCUMENTACION.md`](ESTANDAR-DOCUMENTACION.md),
  [`VERSIONADO.md`](VERSIONADO.md), [`IMPLEMENTACION.md`](IMPLEMENTACION.md),
  [`MEJORAS.md`](MEJORAS.md) (DoD + roadmap) y [`GUIA.md`](GUIA.md) (guía de inicio).

### Firmware (Core completo, configurable de punta a punta)

- **Core modular**: `SemaCore`, `ModuleRegistry`, `EventBus` tipado, `ConfigManager`
  (schema=1, transaccional), Storage (NVS + LittleFS), `Scheduler`, `CapabilityManager`,
  `Watchdog` (D-0019), `HealthMonitor` (D-0020), `PowerManager` (D-0054).
- **Sensores (17 tipos, 5 interfaces)**:
  - I²C: BME280, BMP280, SHT40, SHT31, AHT20, BH1750 (lux), VEML6075 (UV), SCD30 (CO₂), SGP30 (eCO₂/TVOC), AS3935 (rayos), ADS1115 (ADC ext.).
  - 1-Wire: DS18B20 (multi-dispositivo, auto-detección).
  - ADC: batería (genérico), CO (monóxido), SOLAR (radiación).
  - PCNT: lluvia/viento por pulsos.
  - UART: PMS5003 (PM1/PM2.5/PM10).
  - Catálogo configurable vía [`SensorFactory`](../src/core/sensors/SensorFactory.cpp) (D-0042).
- **Flujo de datos**: MEDIR → VALIDAR → PROCESAR → ALMACENAR → PUBLICAR.
  - Derivadas (D-0044): punto de rocío, índice de calor, presión de vapor, humedad absoluta.
- **API REST completa** (`/api/v1/*`): status, health, system, config (GET/PUT),
  sensors (catálogo+mediciones), history, alarms, events, capabilities, network,
  energy, diagnostics, backup (GET/POST), restart (POST), ota (POST).
- **Dashboard web** en `/`: estado, histórico, gráfico de temperatura, edición de
  config y login/logout + **WebSocket** `/ws` (puerto 81).
- **Eventos/alarmas persistentes** (`EventLog` en LittleFS, con rotación) + reglas y
  calibración configurables (hot reload sin reinicio).
- **Configuración web con auth**: `api_key` (web local) y `server_key` (Servidor Central).
  Todo configurable: `sensors[]`, `publishers`, `rules[]`, `calibrations[]`.
- **NTP/RTC** (epoch UTC), **Board Profile** (pines fijos para PCB), **OTA con
  rollback** (particiones A/B), **mDNS**, **reconexión WiFi** con backoff,
  **deep sleep + wake por lluvia**, **backup/restore**, **diagnóstico enriquecido**.

---

## 2. Qué falta (orden sugerido)

### A. Mejoras de software (no requieren hardware)

| Prioridad | Ítem | Nota |
|:---------:|------|------|
| 1 | **TLS/HTTPS** (opcional) | Cifrar la web local y la API |

### B. Hardware específico (requieren el módulo físico para validar)

| Fase | Ítem | Módulos |
|:----:|------|---------|
| 3 | Expansión | MCP23017, 74HC595, 74HC165, ADS1115 |
| 4 | Meteorología | Rayos (AS3935), radiación (piranómetro por ADC) |
| 5 | Calidad | CO (MQ-7 / MICS-5524) |
| 6 | Industrial | RS485/Modbus RTU, CAN (TWAI) |
| 7 | Comunicaciones | LoRa, Zigbee, Ethernet |
| 8 | Energía | Gestión de panel solar (MPPT/voltaje) |

> **Nota**: el "Servidor Central" está **fuera del alcance de SEMA** (es otro
> proyecto que agrega múltiples proyectos). SEMA solo se conecta con `server_key`.

---

## 3. Cómo continuar (flujo obligatorio por cambio)

Reproducir exactamente este ciclo (ver [`REGLAS-DE-TRABAJO.md`](REGLAS-DE-TRABAJO.md)):

```text
1. pio run -e esp32doit-devkit-v1        # compilar/validar (debe dar SUCCESS)
2. bump MINOR en include/core/Version.hpp (SEMA_FW_VERSION sin "v")
3. CHANGELOG.md (entrada nueva + enlace)
4. docs/MEJORAS.md + docs/IMPLEMENTACION.md (cabecera Versión)
5. (si aplica) docs/GUIA.md (guía)
6. (Get-FileHash firmware.bin).Hash.ToLower() → firmware_manifest.json
7. git add -A && git commit (Conventional Commits)
8. git tag -a vX.Y.Z -m "..."
9. git push origin main --tags
10. gh release create vX.Y.Z --repo AlessandroKlein/SEMA \
      --title "..." --notes-file <tmp.md> firmware.bin
11. Actualizar wiki Docs (Home/Arquitectura/Evolucion/Decisiones/Guia: versión)
12. cd Docs && python -m mkdocs build && git add -A && git commit && git push
```

- **Versionado**: SemVer, tag con `v` minúscula; docs-only **no** genera release.
- **Repos**: código `AlessandroKlein/SEMA` · wiki `AlessandroKlein/Docs`.
- El `firmware_manifest.json` guarda la versión y el SHA-256 del binario.

---

## 4. Checklist de corroboración

Para validar el estado actual antes de continuar:

- [ ] `pio run -e esp32doit-devkit-v1` → **SUCCESS** (Flash ≈63 %, RAM ≈16 %).
- [ ] 52 releases en GitHub (`gh release list --repo AlessandroKlein/SEMA`).
- [ ] `firmware_manifest.json` con `version: 0.52.0` y SHA-256 correcto.
- [ ] Wiki publicada: https://alessandroklein.github.io/Docs/sema/ (Guía de inicio presente).
- [ ] Ambos repos limpios (`git status --short --branch` → `## main...origin/main`).
- [ ] `CHANGELOG.md` completo de v0.2.0 a v0.52.0.
- [ ] (si hay hardware) Flashear `firmware.bin` y verificar `/api/v1/sensors`,
      dashboard en `http://sema-001.local/`, y `PUT /api/v1/config` con `X-API-Key`.

---

## Lecturas de referencia

- [`README.md`](../README.md) — especificación completa (433 secciones).
- [`GUIA.md`](GUIA.md) — guía de inicio (concepto, hardware, config, API, código).
- [`MEJORAS.md`](MEJORAS.md) — DoD (§103) y roadmap por fases.
- [`DUDAS-Y-DECISIONES.md`](DUDAS-Y-DECISIONES.md) — decisiones D-0001…D-0060.
- [`FUTURO.md`](FUTURO.md) — ideas opcionales y futuras (backlog).
