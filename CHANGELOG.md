# Changelog

Todos los cambios relevantes de este proyecto se documentan en este archivo.

El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto adhiere a [Versionado Semántico](https://semver.org/lang/es/).
Ver también [`docs/VERSIONADO.md`](docs/VERSIONADO.md).

## [1.69.0] - 2026-10-07

### Changed

- Registros de desplazamiento (cascada) deshabilitados por defecto (flag SEMA_USE_SHIFT, beta).

## [1.68.0] - 2026-10-07

### Added

- microSD (SPI) configurable desde la web: habilitar/deshabilitar + chip-select.
- Historico guardado en microSD; sin SD las graficas quedan vacias (sin gastar memoria interna).

## [1.67.0] - 2026-10-07

### Added

- Watchdog jerarquico por tarea: cada tarea late con su timeout y se expone en /api/v1/health.

## [1.66.0] - 2026-10-07

### Changed

- Pines de buses por board: Ethernet RMII (WROOM) vs W5500 SPI (S3).
- MicroSD documentada por SPI (no SDMMC) con su chip-select.

## [1.65.0] - 2026-10-07

### Added

- Zona horaria en timestamps (configTzTime + epoch local).
- Config fail-safe: respaldo config_bak y restauracion automatica.

## [1.64.0] - 2026-10-07

### Added

- Registros de desplazamiento en cascada: agregar varios 74HC595/74HC165, cada uno con LATCH y sus 8 pines.
- Tooltips (?) de expansores con tipo de datos y voltajes.

## [1.63.0] - 2026-10-07

### Changed

- Ethernet LAN8720A: pines RMII oficiales Espressif documentados en HwProfile y visibles en Pines de buses.

## [1.62.0] - 2026-10-07

### Changed

- MCP23017 -> MCP23S17 (SPI) con CS y configuracion de los 16 pines.
- Registro de desplazamiento: usa MOSI/SCLK del bus SPI y solo LATCH por chip.

## [1.61.0] - 2026-10-07

### Changed

- Expansores separados: MCP23017 (direccion), 74HC595/74HC165 (tipo+pines) y GPIO, cada uno con su entrada.

## [1.60.0] - 2026-10-07

### Changed

- ADS1115: el canal A0-A3 se elige desde cada sensor analogico (FUENTE=ADS1115), no desde el chip.

## [1.59.0] - 2026-10-07

### Added

- ADS1115: canal A0-A3 y magnitud (que mide) configurables.

## [1.58.0] - 2026-10-07

### Added

- DS18B20: lectura por direccion ROM (getTempC por DeviceAddress).

## [1.57.0] - 2026-10-07

### Changed

- DS18B20: un unico pin de bus 1-Wire y direccion ROM por sensor.

## [1.56.0] - 2026-10-07

### Added

- Fuente ADC (ESP32 nativo o ADS1115) para sensores analogicos.
- Multiples DS18B20 (agregar/eliminar con nombre).

## [1.55.0] - 2026-10-07

### Changed

- Sensores especificos: Veleta, Bateria, Anemometro y Pluviometro (cada uno con su entrada/canal).

## [1.54.0] - 2026-10-07

### Changed

- Sensores con descripcion (tooltip ?) y nombres aclarados (veleta, pluviometro, anemometro).

## [1.53.0] - 2026-10-07

### Fixed

- Switches de sensores (toggle) ahora conmutan (label).

### Added

- Seccion Expansores y salidas: registro 74HC595/74HC165 + GPIO (nativo o MCP23017).
- Endpoint POST /api/v1/config/io.

## [1.52.0] - 2026-10-07

### Changed

- Sensores: deshabilitados por defecto (seguridad), toggle con estilo y selector de direccion I2C.
- Guardar solo los sensores habilitados (evita error de tamano NVS).

## [1.51.0] - 2026-10-07

### Added

- Pagina /config/sensors: configurar habilitacion y pines de los 17 sensores soportados.
- Endpoint POST /api/v1/config/sensors.

## [1.50.0] - 2026-10-07

### Added

- Dashboard publico sin sesion (solo tema + login, sin menu ni edicion).
- Footer con nombre/version/board (adaptable).
- Ruta GET /login.

## [1.49.0] - 2026-10-07

### Changed

- Layout: posiciones preservadas al recargar (sin compact).
- Botones de edicion/borrar solo visibles en modo Editar.
- Graficas con altura fija.

### Debug

- Log serial de fallo al guardar config (putString).

## [1.48.0] - 2026-10-07

### Fixed

- Tarjetas duplicadas por colision de clave (mkey incluye measurement).
- Guardar layout en clave NVS separada (arregla 500 por tamano).
- Graficas demo con historico ficticio + refresco mas lento (60s).

## [1.47.0] - 2026-10-07

### Fixed

- Dashboard: arreglo del bug que dejaba la pagina en Cargando (IIFE de veleta huérfana).
- OTA: barra de progreso y redireccion al dashboard.

## [1.46.0] - 2026-10-07

### Fixed

- Buffer JSON ampliado (8 KB) en endpoints (posible truncado de mediciones demo).
- /config/sensors muestra los pines (I2C, Modbus, CAN, LoRa, Zigbee, Ethernet).

## [1.45.0] - 2026-10-07

### Fixed

- Guardar login/estacion: PUT /api/v1/config usa webAuthed() (antes authorized()).
- Tarjeta de reloj (hora NTP) en el dashboard.

## [1.44.0] - 2026-10-07

### Added

- Modo demo (SEMA_DEMO): valores ficticios para probar la web sin sensores.
- Pagina /config/sensors (listado + origen de pines).
- Enlace mDNS en /config/network y OTA check via firmware_manifest.json.

### Changed

- mDNS: hostname saneado (corroborado contra ESPmDNS core 3.x).

## [1.43.0] - 2026-10-07

### Changed

- CSS unificado: dashboard usa las mismas variables/navbar que /config.
- mDNS: saneamiento del hostname (solo alfanumerico y guiones).

## [1.42.0] - 2026-10-07

### Fixed

- Autenticacion de las paginas /config/*: acceso web unificado (sin login configurado = abierto).
- Paginas de config ahora muestran y guardan valores correctamente.

### Changed

- NTP/zona horaria con listados (offsets + servidores + personalizado).
- Dashboard: navbar compartida + boton tema en el header.

## [1.41.0] - 2026-10-07

### Changed

- Rutas /config/network, /config/security, /config/system, /config/wind.
- Dashboard limpio (solo grid + graficos).
- CSS centrado + navbar mejorada.

### Added

- Config NTP (servidor + zona horaria) en /config/system.
- Comprobar actualizacion desde GitHub.

## [1.40.0] - 2026-10-07

### Added

- Multi-pagina: rutas /network, /security, /system con navbar compartida.
- IP estatica/DNS/gateway en la config de red.
- Claves API adicionales con nombre (generar/revocar).

## [1.39.0] - 2026-10-07

### Fixed

- Dashboard: sin parpadeo al refrescar (construccion unica + actualizacion de valores).
- Escaneo WiFi: reintento y respuesta valida ante fallo.

## [1.38.0] - 2026-10-07

### Fixed

- Dashboard Gridstack: arreglo de superposicion al anadir/eliminar tarjetas (removeAll + compact).

## [1.37.0] - 2026-10-07

### Added

- Escaneo WiFi + autocompletado de SSID (GET /api/v1/wifi/scan).
- Login por usuario/contrasena (security.username/password).
- Navbar y auto-reinicio al guardar la red.

## [1.36.0] - 2026-10-07

### Fixed

- Config: buffer JSON ampliado (16 KB) — arregla el error al guardar.

### Changed

- Red WiFi separada de la config de estacion/seguridad (POST /api/v1/config/network).

## [1.35.0] - 2026-10-07

### Added

- Verificación de integridad OTA (cabecera X-SHA256).

## [1.34.0] - 2026-10-07

### Added

- Tema claro/oscuro persistente en el dashboard (botón 🌓).

## [1.33.0] - 2026-10-06

### Added

- Export CSV del histórico (`/api/v1/history?format=csv` + botón en el dashboard).
- Retención por tiempo del histórico (`storage.retention_days`).

## [1.32.0] - 2026-10-06

### Added

- Gráficos del dashboard: series múltiples, ejes con tiempo/unidad y rangos 1h/24h/7d.
- Documentación `docs/ETHERNET-Y-BUILDFLAGS.md` (Ethernet dual-chip + build_flags).

### Changed

- `/api/v1/history` acepta hasta 3000 registros.

## [1.31.0] - 2026-10-06

### Added

- Dashboard: añadir/eliminar tarjetas y tarjetas de gráficos (última hora) por magnitud.

## [1.30.0] - 2026-10-06

### Changed

- Gridstack.js servido localmente desde LittleFS (sin CDN): dashboard offline.
- `data/` con `gridstack-all.min.js` + `gridstack.min.css` (flashear con `uploadfs`).

## [1.29.0] - 2026-10-06

### Added

- Dashboard con Gridstack.js (layout editable, global, guardado en config).
- Calibración de la veleta WH-SP-WD por tabla de 8 resistencias (web).
- `POST /api/v1/wind/resistors` y `POST /api/v1/dashboard/layout`.

### Changed

- Dirección de viento calculada por tabla de resistencias (16 posiciones: 8 directas + 8 en paralelo).

## [1.28.0] - 2026-10-06

### Added

- Unidades métricas/imperiales (config `system.units`).
- Magnitudes derivadas: punto de rocío, índice de calor, sensación térmica, QNH, VPD, AQI, altitud barométrica, tasa de lluvia, dirección de viento.
- Calibración de norte de la veleta (`POST /api/v1/wind/north`).
- `sensors[].enabled` para apagar sensores desde la web.

### Changed

- `/api/v1/sensors` devuelve magnitudes derivadas + conversión de unidades.

## [1.27.0] - 2026-10-06

### Added

- Binarios por board + `firmware_manifest.json` multi-chip (OTA por chip).
- Tablas de particiones 4/8/16 MB.
- Board futura ESP32-WROOM-32U (16 MB) en `platformio.ini` y `HwProfile.hpp`.

### Changed

- `/api/v1/system` expone `board` y `flash_mb`.

## [1.26.0] - 2026-10-06

### Added

- Perfil de hardware centralizado (`include/hw/HwProfile.hpp`) con `build_flags`: features (`SEMA_USE_*`), transceiver Modbus (`SEMA_MODBUS_ISOLATED`) y origen de pines (`SEMA_PINS_FROM_FILE`).

### Changed

- Compilación condicional de los buses; W5500 en periférico SPI dedicado (CS propio), separado del SPI de LoRa.

## [1.25.0] - 2026-10-06

### Changed

- Ethernet W5500 (ESP32-S3): de pila separada a driver ESP-IDF `esp_eth` integrado a lwIP. El WebServer ahora sirve sobre W5500.

## [1.24.0] - 2026-10-06

### Added

- Ethernet con dos variantes por `build_flags`: LAN8720A nativo (`BOARD_ESP32_WROOM`) y W5500 SPI (`BOARD_ESP32_S3`).

## [1.23.0] - 2026-10-06

### Added

- Zigbee (RF-BM-2652P2 / CC2652P2, ZNP por UART) — config `zigbee` + API `/api/v1/zigbee`.

## [1.22.0] - 2026-10-06

### Added

- LoRa (SX1262) envío/recepción de paquetes (config `lora` + API `/api/v1/lora`).

## [1.21.0] - 2026-10-06

### Added

- CAN 2.0 (TWAI) envío/recepción de tramas (config `can` + API `/api/v1/can`).

## [1.20.0] - 2026-10-06

### Added

- RS485/Modbus RTU maestro (config `modbus` + API `/api/v1/modbus`).

## [1.19.0] - 2026-10-06

### Added

- Sensor SOLAR (radiación solar, piranómetro analógico) — 17º tipo de sensor.

## [1.18.0] - 2026-10-06

### Added

- Sensor CO (monóxido de carbono, analógico) — 16º tipo de sensor.

## [1.17.0] - 2026-10-06

### Added

- Shift registers 74HC595/74HC165 (config `shift_register` + API `/api/v1/shift`).

## [1.16.0] - 2026-10-06

### Security

- Cookie de sesión con `SameSite=Strict` (mitiga CSRF contra `PUT /config`).

## [1.15.0] - 2026-10-06

### Added

- Sensor ADS1115 (ADC externo 16 bits, 4 canales) sobre I²C (15º tipo de sensor).

## [1.14.0] - 2026-10-06

### Added

- Expansor MCP23017 en GPIO (`expander_addr`): 16 GPIO extra por I²C.

## [1.13.0] - 2026-10-06

### Added

- Sensor AS3935 (detección de rayos) sobre I²C (14º tipo de sensor).

## [1.12.0] - 2026-10-06

### Added

- Expiración de sesión del login web (1 h, deslizante).

## [1.11.0] - 2026-10-06

### Added

- Autenticación MQTT (usuario/contraseña) desde configuración (`mqtt_user`/`mqtt_pass`).

## [1.10.0] - 2026-10-06

### Added

- Sensor SGP30 (eCO₂ y TVOC) sobre I²C (13º tipo de sensor).

## [1.9.0] - 2026-10-06

### Added

- Hot reload de sensores: `PUT /config` re-crea el catálogo de sensores sin reinicio.

## [1.8.0] - 2026-10-06

### Added

- Hot reload de publicadores: `PUT /config` re-aplica webhook y MQTT sin reinicio.

## [1.7.0] - 2026-10-06

### Added

- GPIO standalone configurable (D-0050): entradas/salidas digitales por config +
  API `/api/v1/gpio` (GET lee, POST escribe) y hot reload.

## [1.6.0] - 2026-10-06

### Added

- Rate limiting en el login (D-0048): bloquea tras 5 intentos fallidos durante 60 s.

## [1.5.0] - 2026-10-04

### Added

- Edición de configuración desde la UI: formulario en el dashboard (nombre, WiFi,
  hostname, claves) + botón cerrar sesión.

## [1.4.0] - 2026-10-04

### Added

- Gráfico de temperatura en el dashboard (canvas, últimas 100 muestras).

## [1.3.0] - 2026-10-04

### Added

- Dashboard avanzado: sección de histórico en la web (últimas 20 mediciones con
  fecha/hora).

## [1.2.0] - 2026-10-04

### Added

- Re-aplicar configuración sin reinicio: reglas y calibración se re-aplican al
  hacer `PUT /config` (sensores/publicadores aún requieren reinicio).

## [1.1.0] - 2026-10-04

### Added

- Rotación del `EventLog` (D-0057): el archivo de eventos conserva los últimos N al
  superar 2× el límite, en lugar de crecer sin límite.

## [1.0.0] - 2026-10-04

### Primera versión estable

Base completa de SEMA, lista para su alcance principal:

- **Core modular**: `SemaCore`, `EventBus`, `ConfigManager` (schema=1), Storage
  (NVS + LittleFS), `Scheduler`, `CapabilityManager`, `Watchdog`, `HealthMonitor`,
  `PowerManager`.
- **Sensores (12 tipos / 5 interfaces)**: I²C (BME280, BMP280, SHT40, SHT31, AHT20,
  BH1750, VEML6075, SCD30), 1-Wire (DS18B20 multi-dispositivo), ADC, PCNT
  (lluvia/viento), UART (PMS5003). Catálogo configurable por factoría (D-0042) y
  Board Profile (D-0050) para PCBs.
- **Datos**: flujo MEDIR→VALIDAR→PROCESAR→ALMACENAR→PUBLICAR, derivadas (punto de
  rocío, índice de calor, presión de vapor, humedad absoluta), histórico con
  rotación y NTP/RTC (epoch UTC).
- **Web/API**: dashboard con login por sesión, REST `/api/v1` completa, WebSocket,
  eventos/alarmas persistentes.
- **Operación**: OTA con rollback, mDNS, reconexión WiFi, energía (deep sleep +
  wake), backup/restore, diagnóstico y Health Monitor.

## [0.56.0] - 2026-10-03

### Added

- Login web con sesión (cookie): página de acceso para el dashboard y protección
  de `GET /config` y `GET /backup` (claves). Sin claves configuradas queda abierto.

## [0.55.0] - 2026-10-03

### Added

- NTP/RTC (D-0044): sincronización de reloj por NTP y timestamps epoch UTC reales
  en las mediciones (`nowEpoch()` con fallback a uptime).

## [0.54.0] - 2026-10-03

### Added

- Board Profile (D-0050): `include/core/BoardProfile.hpp` con `SEMA_FIXED_HARDWARE`
  y pines fijos para PCB personalizada (deshabilita la configuración de pines por web).

## [0.53.0] - 2026-10-03

### Added

- Rotación del histórico (D-0057): `HistoryStore` conserva la mitad más reciente y
  reescribe al alcanzar el límite (ya no descarta las mediciones nuevas).

## [0.52.0] - 2026-10-03

### Added

- Cálculos derivados adicionales (D-0044/§83): presión de vapor (hPa) y humedad
  absoluta (g/m³).

## [0.51.0] - 2026-10-03

### Added

- Detección 1-Wire (`README.md` §12): `Ds18b20Sensor` lee todos los DS18B20 del
  bus (multi-dispositivo, ids `SOIL`, `SOIL_1`, …).

## [0.50.0] - 2026-10-03

### Added

- Driver `Pms5003Sensor` (PM1/PM2.5/PM10) por UART (Fase 5); `SensorSpec` con
  pines `rx`/`tx`.

## [0.49.0] - 2026-10-03

### Added

- Driver `Scd30Sensor` (CO₂, temperatura y humedad) sobre I²C (Fase 5).

## [0.48.0] - 2026-10-03

### Added

- Driver `Veml6075Sensor` (radiación UV: UVA/UVB/índice UV) sobre I²C (Fase 4).

## [0.47.0] - 2026-10-03

### Added

- Sensor `PcntSensor` (conteo de pulsos por PCNT, D-0051/§21): pluviómetro,
  anemómetro, etc. Integrado en la factoría (`model: "PCNT"`).

## [0.46.0] - 2026-10-03

### Added

- Respaldo/restauración (D-0044/D-0046): `GET/POST /api/v1/backup` con artefacto
  autodescriptivo (config + metadatos).

## [0.45.0] - 2026-10-03

### Added

- Calibración configurable (D-0055): `calibrations` en `schema=1` (gain/offset/rango
  por canal).

## [0.44.0] - 2026-10-03

### Added

- Reglas de alarma configurables (D-0059): `rules` en `schema=1`; `Rule` ahora usa
  `String` (sin riesgo de punteros colgantes).

## [0.43.0] - 2026-10-03

### Added

- Publicadores configurables (D-0010): `publishers` en `schema=1` (webhook URL,
  MQTT host/puerto/topic).

## [0.42.0] - 2026-10-03

### Added

- `/api/v1/diagnostics` enriquecido: firmware/hw, salud, tareas, módulos, eventos
  y heap.

## [0.41.0] - 2026-10-03

### Added

- Health Monitor (D-0020): estado de salud agregado (`HEALTHY`/`DEGRADED`/`ERROR`)
  y `/api/v1/health` con métricas (liveness + sensores + heap).

## [0.40.0] - 2026-10-03

### Added

- `security.server_key` (D-0048): clave API del Servidor Central → SEMA para
  configuración de riesgo por API; la web local usa `api_key`.

## [0.39.0] - 2026-10-03

### Added

- Drivers `Sht31Sensor` y `Bmp280Sensor` (I²C) + entradas en la factoría; completa
  el catálogo de Fase 2 (`README.md` §9-10).

## [0.38.0] - 2026-10-03

### Added

- Catálogo de sensores configurable (D-0042): `sensors` en `schema=1` y
  `SensorFactory`; la configuración define qué drivers se instancian.

## [0.37.0] - 2026-10-03

### Added

- Política de reconexión WiFi con backoff exponencial (`README.md` §183).

## [0.36.0] - 2026-10-03

### Added

- Watchdog jerárquico (D-0019): task watchdog del ESP32 para el bucle principal.

## [0.35.0] - 2026-10-03

### Added

- Dashboard web (UI): página HTML servida en `/` que consume la API local y
  muestra estado + mediciones con auto-refresco.

## [0.34.0] - 2026-10-03

### Added

- Wake-up por GPIO de lluvia (D-0022): `PowerManager::enableRainWakeup()`.
- `energy.rain_pin` en `schema=1`.

## [0.33.0] - 2026-10-03

### Added

- `PowerManager` (D-0054): perfiles energéticos y deep sleep con wake por timer RTC.
- `GET /api/v1/energy` (perfil y motivo de wake).

## [0.32.0] - 2026-10-03

### Added

- mDNS (`README.md` §61): la estación se resuelve como `<hostname>.local`.
- `WiFiManager::mdnsStarted()`.

## [0.31.0] - 2026-10-03

### Added

- `EventLog` generalizado (D-0008/§205): registra y persiste todos los tipos de
  evento; evento de arranque `system/boot`.
- `GET /api/v1/events` (todos los eventos); `/api/v1/alarms` filtra alarmas.

## [0.30.0] - 2026-10-03

### Added

- Sensor analógico `AdcSensor` (ADC interno, D-0051/§32-33): escala/offset.
- Medición de tensión de batería (D-0021/§37) con divisor resistivo.

## [0.29.0] - 2026-10-03

### Added

- OTA con rollback (D-0049): `POST /api/v1/ota` (multipart/form-data, protegido)
  actualiza el firmware sobre las particiones OTA redundantes mediante `Update`.

## [0.28.0] - 2026-10-03

### Added

- `GET /api/v1/capabilities` (lista de capacidades de la plataforma, D-0051).
- `GET /api/v1/network` (modo, conexión, IP, RSSI).

## [0.27.0] - 2026-10-03

### Added

- `GET /api/v1/sensors` expone el catálogo (`catalog`: id, model, interface,
  healthy) junto a las mediciones (README §48).

## [0.26.0] - 2026-10-03

### Added

- Driver `Aht20Sensor` (temperatura y humedad) sobre I²C (`README.md` §9-10).
- Dependencia Adafruit AHTx0.

## [0.25.0] - 2026-10-03

### Added

- Persistencia de alarmas (D-0041/§87): `AlarmLog` guarda los eventos `Alarm` en
  LittleFS (JSONL) y los recarga al arrancar.

## [0.24.0] - 2026-10-03

### Added

- Driver `Bh1750Sensor` (luminosidad, lux) sobre I²C (`README.md` §13).

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

[1.69.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.69.0
[1.68.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.68.0
[1.67.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.67.0
[1.66.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.66.0
[1.65.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.65.0
[1.64.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.64.0
[1.63.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.63.0
[1.62.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.62.0
[1.61.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.61.0
[1.60.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.60.0
[1.59.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.59.0
[1.58.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.58.0
[1.57.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.57.0
[1.56.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.56.0
[1.55.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.55.0
[1.54.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.54.0
[1.53.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.53.0
[1.52.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.52.0
[1.51.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.51.0
[1.50.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.50.0
[1.49.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.49.0
[1.48.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.48.0
[1.47.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.47.0
[1.46.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.46.0
[1.45.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.45.0
[1.44.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.44.0
[1.43.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.43.0
[1.42.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.42.0
[1.41.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.41.0
[1.40.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.40.0
[1.39.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.39.0
[1.38.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.38.0
[1.37.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.37.0
[1.36.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.36.0
[1.35.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.35.0
[1.34.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.34.0
[1.33.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.33.0
[1.32.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.32.0
[1.31.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.31.0
[1.30.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.30.0
[1.29.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.29.0
[1.28.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.28.0
[1.27.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.27.0
[1.26.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.26.0
[1.25.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.25.0
[1.24.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.24.0
[1.23.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.23.0
[1.22.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.22.0
[1.21.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.21.0
[1.20.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.20.0
[1.19.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.19.0
[1.18.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.18.0
[1.17.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.17.0
[1.16.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.16.0
[1.15.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.15.0
[1.14.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.14.0
[1.13.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.13.0
[1.12.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.12.0
[1.11.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.11.0
[1.10.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.10.0
[1.9.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.9.0
[1.8.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.8.0
[1.7.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.7.0
[1.6.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.6.0
[1.5.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.5.0
[1.4.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.4.0
[1.3.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.3.0
[1.2.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.2.0
[1.1.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.1.0
[1.0.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v1.0.0
[0.56.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.56.0
[0.55.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.55.0
[0.54.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.54.0
[0.53.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.53.0
[0.52.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.52.0
[0.51.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.51.0
[0.50.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.50.0
[0.49.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.49.0
[0.48.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.48.0
[0.47.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.47.0
[0.46.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.46.0
[0.45.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.45.0
[0.44.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.44.0
[0.43.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.43.0
[0.42.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.42.0
[0.41.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.41.0
[0.40.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.40.0
[0.39.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.39.0
[0.38.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.38.0
[0.37.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.37.0
[0.36.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.36.0
[0.35.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.35.0
[0.34.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.34.0
[0.33.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.33.0
[0.32.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.32.0
[0.31.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.31.0
[0.30.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.30.0
[0.29.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.29.0
[0.28.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.28.0
[0.27.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.27.0
[0.26.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.26.0
[0.25.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.25.0
[0.24.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.24.0
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
