# Changelog

Todos los cambios relevantes de este proyecto se documentan en este archivo.

El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto adhiere a [Versionado Semántico](https://semver.org/lang/es/).
Ver también [`docs/VERSIONADO.md`](docs/VERSIONADO.md).

## [0.23.0] - 2026-10-03

### Added

- Autenticación de API (D-0048): `security.api_key` en `schema=1`; las
  operaciones de escritura (`PUT /api/v1/config`, `POST /api/v1/restart`)
  exigen el header `X-API-Key` cuando la clave está configurada.
- Endpoint `POST /api/v1/restart`.

## [0.22.0] - 2026-10-03

### Added

- WebSocket en `/ws` (puerto 81): difunde las mediciones en tiempo real a los
  clientes conectados (D-0041/§226). Dependencia WebSockets.

## [0.21.0] - 2026-10-03

### Added

- Configuración desde la web (D-0042): `PUT /api/v1/config` aplica la
  configuración de forma transaccional (validar → aplicar → persistir → rollback).
- `ConfigManager::applyJson()`.

## [0.20.0] - 2026-10-03

### Added

- Publicador MQTT (D-0010/§56): `MqttPublisher` que publica el Modelo Canónico
  como JSON en un topic. Dependencia PubSubClient.

## [0.19.0] - 2026-10-03

### Added

- `AlarmLog`: registro acotado de alarmas alimentado desde el Event Bus.
- Endpoint `GET /api/v1/alarms` (D-0041) para consultar las alarmas recientes.
- `Event` ahora es propietario de sus cadenas (String), evitando punteros colgantes.

## [0.18.0] - 2026-10-03

### Added

- Motor de reglas y alarmas (D-0059): `RuleEngine` con operadores `>`, `<`, `>=`, `<=`.
- Las reglas publican eventos `Alarm` en el Event Bus (D-0045); suscriptor de
  ejemplo que registra alarmas en serial.

## [0.17.0] - 2026-10-03

### Added

- Infraestructura de publicadores (D-0010): `Publisher`, `PublisherManager`.
- `HttpPublisher` (webhook genérico, D-0047/§113): POST JSON del Modelo Canónico.
- Los publicadores se ejecutan tras el almacenamiento de cada medición.

## [0.16.0] - 2026-10-03

### Added

- Driver `Ds18b20Sensor` (temperatura) sobre 1-Wire (`README.md` §12).
- Dependencias OneWire y DallasTemperature.

## [0.15.0] - 2026-10-03

### Added

- Calibración y validación de rango (D-0055/§84): `Calibration` + `applyCalibration`.
- `SensorManager::setCalibration()` por canal; las mediciones fuera de rango se
  marcan `OUT_OF_RANGE` (D-0056).

## [0.14.0] - 2026-10-03

### Added

- Cálculos derivados (`README.md` §83): `DerivedEngine` con punto de rocío e
  índice de calor, calculados a partir de temperatura y humedad.

## [0.13.0] - 2026-10-03

### Added

- Detección I²C (D-0058): `I2cScanner` con catálogo de modelos por dirección.
- Dispositivos detectados expuestos en `/api/v1/diagnostics` (`i2c_devices`).

## [0.12.0] - 2026-10-03

### Added

- Driver `Sht40Sensor` (temperatura y humedad) sobre I²C (D-0002/D-0043).
- Dependencia Adafruit SHT4x.

## [0.11.0] - 2026-10-03

### Added

- Endpoint `GET /api/v1/history?limit=N` (`README.md` §110) para consultar el histórico.
- `HistoryStore::readRecent()` y `parseQuality()` (deserialización de Quality Flags).

## [0.10.0] - 2026-10-03

### Added

- Histórico persistente (D-0046/D-0057): `HistoryStore` JSONL sobre LittleFS.
- Tabla de particiones propia (`partitions.csv`) con partición `spiffs` de datos.
- Las mediciones se guardan tras cada lectura; `/api/v1/diagnostics` expone el
  conteo de entradas del histórico.

## [0.9.0] - 2026-10-03

### Added

- Fase 2 — Sensor Engine (primera iteración):
  - `Sensor` (interfaz de driver por magnitud) y `SensorManager` (registro + lectura).
  - Driver `Bme280Sensor` (temperatura, humedad, presión) sobre I²C.
  - Endpoint `GET /api/v1/sensors` y contadores reales en `/api/v1/health`.
- Dependencias Adafruit BME280 (+ Unified Sensor, BusIO).

## [0.8.0] - 2026-10-03

### Added

- `Scheduler` (D-0018): tareas periódicas por intervalo.
- `WiFiManager`: conexión STA o Access Point de emergencia (`README.md` §59-60).
- `HttpServer`: REST local `/api/v1/status·health·system·config·diagnostics` (D-0041).
- `NetworkConfig.password` y `ConfigManager::toJson()`.

## [0.7.0] - 2026-10-03

### Added

- `include/core/Measurement.hpp`: Modelo Canónico de Mediciones (D-0044) con
  Quality Flags (D-0056).
- Event Bus tipado (D-0045): `Event` con `id`, `severity`, `correlation_id` y
  `target`; enum `Severity`.
- `docs/IMPLEMENTACION.md` §13: especificación técnica de D-0041, D-0042, D-0044,
  D-0045 y D-0046.

## [0.6.0] - 2026-10-03

### Added

- Núcleo de la Fase 1:
  - `ConfigManager` (schema=1, NVS transaccional con rollback).
  - Storage API + backend `NvsStore` (Preferences).
  - `CapabilityManager` + enumeración `Capability`.
  - `Task` (abstracción de tarea sobre FreeRTOS, afinidad `AUTO`).
- Dependencia `ArduinoJson@6` para serialización de configuración.

## [0.5.0] - 2026-10-03

### Changed

- `docs/DUDAS-Y-DECISIONES.md`: las 20 dudas (Q-0001…Q-0020) resueltas y
  convertidas en decisiones D-0041…D-0060. La sección 5 pasa a ser
  "Resolución de dudas abiertas".
- Contrato arquitectónico completo: 60 decisiones cerradas (D-0001…D-0060).

## [0.4.0] - 2026-10-03

### Added

- Decisiones de implementación D-0041…D-0045 que cierran las primeras dudas:
  - D-0041: endpoints REST mínimos de Fase 1 (`/api/v1/*`).
  - D-0042: JSON Schema de configuración `schema=1`.
  - D-0043: Modelo Canónico de Mediciones.
  - D-0044: estructura del Event Bus.
  - D-0045: esquema de almacenamiento local (Storage API).

## [0.3.0] - 2026-10-03

### Changed

- `docs/DUDAS-Y-DECISIONES.md` llevado a contrato arquitectónico: 40 decisiones
  (D-0001…D-0040) y 20 dudas (Q-0001…Q-0020).
- `docs/IMPLEMENTACION.md` incorpora la compatibilidad por perfiles
  (Capability / Resource / Runtime Manager + HAL) y el Storage API.
- Corrección D-0003: almacenamiento y protocolos como abstracciones del Core.

## [0.2.0] - 2026-10-03

### Added

- Documentación de planificación y arquitectura:
  - `docs/IMPLEMENTACION.md` (arquitectura e implementación).
  - `docs/MEJORAS.md` (roadmap por fases).
  - `docs/DUDAS-Y-DECISIONES.md` (decisiones de arquitectura).
- Núcleo modular del firmware (inicio de la Fase 1):
  - `include/core/Module.hpp` (contrato de módulo y ciclo de vida).
  - `include/core/ModuleRegistry.hpp` (registro de módulos).
  - `include/core/EventBus.hpp` (bus interno de eventos).
  - `include/core/SemaCore.hpp` (orquestador del Core).
- `src/main.cpp` reducido a arrancar el Core.

## [0.1.0] - 2026-10-03

### Added

- Arranque inicial del proyecto **SEMA** (Sistema de Estación Meteorológica Autónoma).
- Estructura base del firmware con PlatformIO (`esp32doit-devkit-v1`, framework Arduino).
- Constantes de versión del firmware en `include/core/Version.hpp`.
- Convenciones de arranque del proyecto:
  - `docs/REGLAS-DE-TRABAJO.md`
  - `docs/ESTANDAR-DOCUMENTACION.md`
  - `docs/VERSIONADO.md`
  - `DESIGN-SYSTEM.md`
  - `SECURITY.md`

[0.23.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.23.0
[0.22.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.22.0
[0.21.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.21.0
[0.20.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.20.0
[0.19.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.19.0
[0.18.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.18.0
[0.17.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.17.0
[0.16.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.16.0
[0.15.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.15.0
[0.14.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.14.0
[0.13.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.13.0
[0.12.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.12.0
[0.11.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.11.0
[0.10.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.10.0
[0.9.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.9.0
[0.8.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.8.0
[0.7.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.7.0
[0.6.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.6.0
[0.5.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.5.0
[0.4.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.4.0
[0.3.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.3.0
[0.2.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.2.0
[0.1.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.1.0
