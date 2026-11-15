# Plan de Extracción — Repo Base Reutilizable (SEMA)

> **Tipo:** Plan de análisis (solo lectura; no se ejecutó ninguna extracción)
> **Fecha:** 2026-11-14 | **Firmware analizado:** v1.106.0 | **Autor:** Backend Architect

Objetivo: separar de SEMA una **base reutilizable** (WiFi/Ethernet, pines, web,
NVS, runtime) de lo **específico de meteorología** (sensores de clima, Forecast,
veleta/pluviómetro, catálogo de magnitudes).

---

## 1. Inventario: genérico vs. meteorológico

### 1.1 Genérico (va a la base)

| Módulo | Archivos (ruta) |
|--------|-----------------|
| Runtime/orquestación | `src/core/SemaCore.*`, `include/core/SemaCore.hpp` (con los acoples §3) |
| Plugin system | `include/core/Module.hpp`, `src/core/ModuleRegistry.cpp` |
| Eventos | `src/core/EventBus.cpp`, `src/core/events/EventLog.cpp` |
| Scheduler | `src/core/Scheduler.cpp`, `include/core/runtime/Task.hpp` |
| Config (framework) | `src/core/ConfigManager.cpp`, `include/core/ConfigManager.hpp` |
| Storage | `src/core/storage/{NvsStore,HistoryStore}.cpp`, `include/core/storage/*` |
| Red | `src/core/network/WiFiManager.cpp`, `src/core/EthernetManager.cpp` |
| Buses | `src/core/{CanManager,LoraManager,ModbusManager,ZigbeeManager}.cpp` |
| Alarmas | `src/core/alarms/RuleEngine.cpp`, `include/core/alarms/*` |
| Publicadores | `src/core/publishers/*` (HttpPublisher, MqttPublisher, Publisher, PublisherManager) |
| Infraestructura | `Watchdog`, `HealthMonitor`, `PowerManager`, `CapabilityManager`, `Time`, `Calibration`, `GpioManager`, `ShiftRegisterManager` |
| Web (shell) | `src/core/web/HttpServer.cpp` + `src/core/web/GridstackAssets.h` (con los acoples §3) |
| Modelo canónico | `include/core/Measurement.hpp` (calidad, timestamp, channelId, unit) |

### 1.2 Específico de meteorología (queda en SEMA)

| Módulo | Archivos |
|--------|----------|
| Drivers de sensores (17 tipos) | `src/core/sensors/*` (BME280, SHT40, DS18B20, BH1750, VEML6075, SCD30, SGP30, PMS5003, AS3935, ADS1115, CO, SOLAR, S12SD, CCS811, HDC1080, ADC, PCNT) |
| Factoría de sensores | `src/core/sensors/SensorFactory.cpp` + `include/core/sensors/SensorFlags.hpp` |
| Derivadas meteorológicas | `src/core/derived/{DerivedCalculator,DerivedEngine}.cpp` (rocío, índice de calor, sensación térmica, QNH, VPD, tasa de lluvia, dirección de viento) |
| Pronóstico | `src/core/prediction/Forecast.cpp` + `include/core/prediction/Forecast.hpp` |
| Catálogo y etiquetas de magnitudes | `HttpServer.cpp` `TYPES[]` (:1046-1069) e i18n (:312-318) |

---

## 2. Nombre y consumo recomendado

**Nombre sugerido:** `ESP32-Base` (repo `AlessandroKlein/ESP32-Base`), namespace
neutro `base::` (hoy todo es `sema::`).

**Consumo recomendado: (a) PlatformIO `lib_deps` apuntando a git + tag.**

```ini
lib_deps =
    https://github.com/AlessandroKlein/ESP32-Base.git#v1.0.0
```

La base se empaqueta como **librería PlatformIO** (`library.json` + `src/` +
`include/`), no como proyecto. Justificación:

- `git clone ESP32-Base` + `pio run` = proyecto nuevo listo, y `SEMA` solo pinnea
  un tag; subir de versión = cambiar el `#vX.Y.Z` (nada de submodules anidados).
- PlatformIO resuelve la dependencia en Windows sin pasos manuales.
- **Descartado (b) submodule**: añade dos `git` a mantener y rompe el "siempre
  actualizado" salvo `submodule update` manual.
- **Descartado (c) vendorizado**: copia estática que se desincroniza y no es
  "siempre actualizado".

`ESP32-Base` lleva su propio SemVer + tag + `CHANGELOG` (docs-only, sin release de
firmware). El release de firmware sigue siendo de SEMA (§5).

---

## 3. Puntos de acoplamiento y desacople

| # | Acople hoy | Evidencia | Desacople |
|---|-----------|-----------|-----------|
| 1 | Catálogo `TYPES[]` + etiquetas i18n hardcodeados en la web | `HttpServer.cpp:1046-1069`, `:312-318` | Interfaz `CatalogProvider` que SEMA registra en la base; la web renderiza el catálogo dinámicamente (no constantes PROGMEM). |
| 2 | Campos meteorológicos en `SystemConfig` | `ConfigManager.hpp:40-47` (`altitude`, `windNorthOffset`, `windDirectionPin`, `windRpull`, `windResistors[8]`) | `SystemConfig` base queda neutro; SEMA añade su `SystemConfig` extensión vía sub-estructura `weather` inyectada. |
| 3 | Catálogo de sensores por defecto en `applySensors()` | `SemaCore.cpp:271-316` (BME280/SHT40/DS18B20/BH1750/AHT20/batería) | `SemaCore` expone hook `registerDefaultSensors()`; SEMA lo implementa con su catálogo. |
| 4 | Regla por defecto meteorológica | `SemaCore.cpp:254-269` (`high_temp` EXT>40) | `registerDefaultRules()` hook. |
| 5 | Derivadas + Forecast cableadas al core | `SemaCore.hpp:36,102,161`, `SemaCore.cpp:120,194` | Interfaz `DerivedProvider` / `ForecastProvider`; base solo consume `Measurement` sin saber de magnitudes. |
| 6 | `SensorSpec` y `SensorFactory` dentro del core | `ConfigManager.hpp:100-118`, `SensorFactory.cpp` | La base define el contrato `Sensor` + `SensorManager`; la factoría y sus flags quedan en SEMA (registro por callback). |
| 7 | Web/dashboard con tarjetas meteorológicas y demo | `HttpServer.cpp:294,2215,2262` | Separar "shell web" (login, API, WebSocket, OTA) del "contenido del dashboard" (SEMA inyecta tarjetas + demo). |

---

## 4. Plan por fases

### Fase 1 — Extraer la base (mínimo viable)
- Crear `ESP32-Base` como librería PlatformIO con `library.json` + `src/` + `include/`.
- Mover §1.1; renombrar namespace `sema` → `base` (cambio mecánico).
- `HwProfile.hpp` (identidad de board y flags) **se queda en SEMA**; la base
  entrega `board_config.h` con defaults `#ifndef` para que cada proyecto lo llene.
- Verificar con `pio run -e esp32doit-devkit-v1` → SUCCESS.
- **Esfuerzo: grande** (~2-3 días-agente): movimiento mecánico + rename de
  namespace + re-verificación en boards.

### Fase 2 — Desacoplar sensores/pronóstico
- Definir contratos: `Sensor` (ya existe), `SensorManager` genérico, `CatalogProvider`,
  `DerivedProvider`, `ForecastProvider`, hooks `registerDefault*`.
- Mover §1.2 a la capa de dominio SEMA; la web renderiza catálogo/labels por inyección.
- **Esfuerzo: medio** (~1-2 días-agente): diseño de interfaces + mover dominio +
  limpiar acoples 1-7.

### Fase 3 — SEMA consume la base
- `platformio.ini` de SEMA: `lib_deps = .../ESP32-Base.git#vX.Y.Z`.
- `main.cpp` queda fino: registra dominio meteorológico (sensores + derived +
  forecast + catálogo) y arranca el core base.
- Actualizar flujo de release para versionar `ESP32-Base` por separado.
- **Esfuerzo: medio** (~1 día-agente): cableado + release + Docs.

---

## 5. Riesgos

1. **Flag duplicado `SEMA_PINS_FROM_FILE` vs `SEMA_FIXED_HARDWARE`.** `HwProfile.hpp:93-95`
   define `SEMA_PINS_FROM_FILE` (origen de pines), pero `SemaCore.cpp:283` decide el
   catálogo fijo con `SEMA_FIXED_HARDWARE` (de `BoardProfile.hpp:21`). Dos fuentes de
   verdad para "pines fijos": consolidar en UN solo flag antes/ durante la extracción.
2. **Flags de compilación `-D SEMA_USE_*`.** Están en el `platformio.ini` del proyecto;
   la base debe honrarlos por `#ifndef` (patrón ya usado en `SensorFlags.hpp`). Si no,
   la base compila todo y rompe el ahorro de flash.
3. **Sistema de release.** `firmware_manifest.json` + SHA-256 + `gh release` son de
   SEMA (por board) y **no se mueven**; lo nuevo es que `ESP32-Base` versiona por tag
   propio. Riesgo: versiones desincronizadas si no se pineea el tag en `lib_deps`.
4. **Web assets.** `GridstackAssets.h` + `gen_gridstack.py` mezclan shell genérico con
   dashboard meteorológico; al partir, regenerar assets sin perder el shell.
5. **Namespace.** El rename `sema`→`base` es el cambio más ancho (todo `src/` e
   `include/`); si se pospone, la base nace con namespace SEMA y el "reutilizable"
   queda manchado. Hacerlo en Fase 1.
