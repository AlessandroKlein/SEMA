# Guía de inicio — SEMA

> **Tipo:** Guía | **Estado:** Activa | **Fecha:** 2026-10-03 | **Firmware:** v1.69.0

Documentación de referencia para **entender y usar SEMA sin conocer el proyecto
ni el código de antemano**. Se recomienda leer de principio a fin; los apartados
son progresivos.

---

## 1. ¿Qué es SEMA?

**SEMA** (Sistema de Estación Meteorológica Autónoma) es una plataforma
**modular** para construir estaciones meteorológicas y ambientales basadas en el
microcontrolador **ESP32**.

La idea central es muy simple:

> **El hardware define las capacidades. La configuración web define cómo se utilizan.**

Es decir: conectás los sensores al ESP32 (hardware), y después **desde una página
web** decidís qué hace cada uno (qué mide, con qué nombre, cada cuánto, a dónde
se envía) — **sin recompilar ni tocar código**.

> **Simple por fuera, modular por dentro.** Por fuera: una estación que se
> configura desde el navegador. Por dentro: módulos independientes (sensores,
> alarmas, publicadores, almacenamiento…) que el Core orquesta.

### ¿Qué puede hacer hoy (v0.40.0)?

- Leer **sensores** de temperatura, humedad, presión, luminosidad y tensión de
  batería por **I²C, 1-Wire y ADC**.
- **Detectar** automáticamente qué hay conectado en el bus I²C.
- Guardar **histórico** y **eventos/alarmas** en la memoria flash (sobreviven a
  un reinicio).
- Publicar las mediciones por **HTTP (webhook) y MQTT**.
- Exponer una **API REST** y un **dashboard web** en tiempo real (WebSocket).
- **Actualizarse por OTA** (sin cable) y **dormir** (deep sleep) para ahorrar
  batería, despertando por temporizador o por un pluviómetro.

---

## 2. Principios de diseño

Cinco reglas resumen la filosofía (detalle en `Arquitectura.md`):

```text
SENSOR ≠ FUNCIÓN      → un mismo sensor puede medir distintas cosas
GPIO   ≠ SENSOR       → un pin no es un sensor; es un recurso asignable
BUS    ≠ SENSOR       → el bus (I²C/1-Wire) es transporte, no identidad
MODELO ≠ MAGNITUD     → dos modelos distintos pueden medir la misma magnitud
HARDWARE ≠ CONFIG     → el hardware da capacidades; la config decide el uso
```

Consecuencias prácticas:

- **El Core nunca depende de módulos opcionales**: si un sensor falla, el resto
  sigue midiendo.
- **Los sensores son intercambiables**: BME280, SHT40, SHT31, AHT20 y BMP280
  miden lo mismo (temperatura/humedad/presión) tras la misma interfaz.
- **El catálogo de sensores sale de la configuración**, no del código (v0.38.0).

---

## 3. Hardware

### Placa

- **ESP32 DOIT DevKit V1** (plataforma `esp32doit-devkit-v1`), 4 MB de flash.
- Partición **OTA redundante**: hay dos copias del firmware (A/B); si una
  actualización falla, se vuelve a la anterior.

### Pines usados por defecto

| Función | Pin | Nota |
|---------|-----|------|
| I²C SDA | GPIO 21 | Sensores I²C |
| I²C SCL | GPIO 22 | Sensores I²C |
| 1-Wire | GPIO 4 | DS18B20 con pull-up de 4,7 kΩ |
| ADC batería | GPIO 34 | Divisor resistivo (11:1 para 12 V) |

### Pines fijos para PCB personalizada

Por defecto, los pines de los sensores son **configurables desde la web** (sección
`sensors` de la configuración). Para una **PCB personalizada** podés fijarlos en
tiempo de compilación y deshabilitar la configuración por web:

1. Editá `include/core/BoardProfile.hpp`.
2. Poné `SEMA_FIXED_HARDWARE` en `1`.
3. Ajustá los pines (`SEMA_PIN_I2C_SDA`, `SEMA_PIN_I2C_SCL`, `SEMA_PIN_ONEWIRE`,
   `SEMA_PIN_BATTERY_ADC`) a tu diseño.

Con `SEMA_FIXED_HARDWARE = 1`, el catálogo `sensors[]` de la configuración **se
ignora** y se usa el catálogo fijo con esos pines (D-0050).

### Sensores soportados (catálogo configurable)

| Modelo | Magnitud | Bus | Dirección/pin |
|--------|----------|-----|---------------|
| BME280 | temperatura, humedad, presión | I²C | 0x76 / 0x77 |
| BMP280 | temperatura, presión | I²C | 0x76 / 0x77 |
| SHT40 | temperatura, humedad | I²C | 0x44 |
| SHT31 | temperatura, humedad | I²C | 0x44 |
| AHT20 | temperatura, humedad | I²C | 0x38 |
| BH1750 | luminosidad (lux) | I²C | 0x23 |
| DS18B20 | temperatura | 1-Wire | GPIO 4 |
| ADC (genérico) | tensión/otro | ADC | GPIO configurable |
| PCNT (genérico) | lluvia/viento (pulsos) | GPIO (PCNT) | pin configurable |
| VEML6075 | UVA/UVB/índice UV | I²C | 0x10 |
| SCD30 | CO₂, temperatura, humedad | I²C | 0x61 |
| PMS5003 | PM1/PM2.5/PM10 | UART | RX/TX configurables |

---

## 4. Compilar y flashear

### Requisitos

- [PlatformIO](https://platformio.org/) (IDE o CLI). El proyecto usa el framework
  **Arduino**.
- Un ESP32 DOIT DevKit V1 y un cable USB.

### Compilar

```bash
pio run -e esp32doit-devkit-v1
```

El resultado (firmware) queda en `.pio/build/esp32doit-devkit-v1/firmware.bin`.

### Flashear por USB

```bash
pio run -e esp32doit-devkit-v1 -t upload
```

### Flashear por OTA (sin cable)

Una vez la estación está en la red, se puede actualizar enviando `firmware.bin`
al endpoint `POST /api/v1/ota` (ver §7). Cada release de GitHub incluye el
`firmware.bin` y su hash SHA-256 en `firmware_manifest.json`.

---

## 5. Primer arranque

1. Al encender, el ESP32 carga la configuración guardada; si no hay, usa valores
   por defecto.
2. Se conecta a la **WiFi** en modo `STA` (o abre un punto de acceso en modo `AP`).
3. **mDNS** publica el nombre `sema-001.local` (configurable).
4. Abrí el dashboard en el navegador:

```text
http://sema-001.local/
```

Verás el **nombre de la estación, versión, uptime y las mediciones en vivo**
(se actualizan cada 5 segundos).

Si configuraste `security.api_key` o `security.server_key`, el dashboard pide
**login** (página de acceso con la clave). Sin claves, queda abierto para la
primera configuración.

### Por consola (Serial 115200)

Al arrancar imprime, entre otras cosas:

```text
SEMA v0.40.0 (hw rev0, schema 1, protocol 1)
Estación: Estación Norte (SEMA-001)
Web local: http://192.168.1.10/
mDNS: http://sema-001.local/
```

---

## 6. Configuración

La configuración es un **JSON versionado** (`schema=1`) que se guarda en NVS y se
edita desde la web o la API. Ejemplo completo:

```json
{
  "schema_version": 1,
  "station": { "id": "SEMA-001", "name": "Estación Norte" },
  "network": {
    "mode": "STA",
    "ssid": "MiRed",
    "password": "clave",
    "hostname": "sema-001",
    "mdns": true
  },
  "system": { "timezone": "America/Argentina/Buenos_Aires", "log_level": "INFO" },
  "storage": { "backend": "littlefs", "retention_days": 30 },
  "security": {
    "api_key": "clave-web-local",
    "server_key": "clave-servidor-central"
  },
  "publishers": {
    "webhook_url": "",
    "mqtt_host": "",
    "mqtt_port": 1883,
    "mqtt_topic": "sema/measurement"
  },
  "rules": [
    { "name": "high_temp", "sensor_id": "EXT", "channel_id": "temperature", "op": "gt", "value": 40.0 }
  ],
  "calibrations": [
    { "sensor_id": "EXT", "channel_id": "temperature", "gain": 1.0, "offset": 0.0, "has_range": true, "min": -40.0, "max": 85.0 }
  ],
  "energy": { "rain_pin": 0 },
  "sensors": [
    { "id": "EXT",  "model": "BME280",  "sda": 21, "scl": 22 },
    { "id": "SOIL", "model": "DS18B20", "pin": 4 },
    { "id": "BATT", "model": "ADC", "pin": 34, "channel": "voltage", "unit": "V", "scale": 0.00886 }
  ]
}
```

### Secciones

| Sección | Para qué |
|---------|----------|
| `station` | Identidad de la estación |
| `network` | WiFi (modo, credenciales, hostname, mDNS) |
| `system` | Zona horaria y nivel de log |
| `storage` | Backend y retención del histórico |
| `security` | Claves de acceso (`api_key`, `server_key`) |
| `publishers` | Webhook URL y MQTT (host/puerto/topic) |
| `rules` | Reglas de alarma (nombre, sensor, canal, operador, umbral) |
| `calibrations` | Calibración por canal (gain/offset/rango) |
| `energy` | Pin del pluviómetro para wake-up |
| `sensors` | **Catálogo de sensores** (si está vacío, se usa el catálogo por defecto) |

### `sensors` (catálogo configurable)

Cada sensor es un objeto `{ id, model, ... }`. Según el modelo, se usan distintos
campos:

- `BME280`, `BMP280`, `SHT40`, `SHT31`, `AHT20`, `BH1750` → `sda`, `scl`.
- `DS18B20` → `pin`.
- `ADC` → `pin`, `channel`, `unit`, `scale`, `offset` (calibración: `value = raw·scale + offset`).

---

## 7. API REST

Base: `/api/v1`. Las operaciones de **escritura/riesgo** exigen el header
`X-API-Key` con la clave `api_key` (web local) **o** `server_key` (Servidor
Central). Si no hay claves configuradas, se permite (primera configuración).

| Método | Endpoint | Descripción | Auth |
|--------|----------|-------------|:----:|
| GET | `/status` | Nombre, firmware, uptime | — |
| GET | `/health` | Estado de salud | — |
| GET | `/system` | Info del sistema | — |
| GET | `/config` | Configuración actual (JSON) | — |
| PUT | `/config` | Aplica nueva configuración | ✔ |
| GET | `/backup` | Respaldo (config + metadatos) | — |
| POST | `/backup` | Restaura un respaldo | ✔ |
| GET | `/sensors` | Catálogo + mediciones actuales | — |
| GET | `/history` | Histórico reciente | — |
| GET | `/alarms` | Solo eventos de alarma | — |
| GET | `/events` | Todos los eventos | — |
| GET | `/capabilities` | Capacidades de la placa | — |
| GET | `/network` | Modo, IP, RSSI | — |
| GET | `/energy` | Perfil energético y wake reason | — |
| GET | `/diagnostics` | Diagnóstico | — |
| POST | `/restart` | Reinicia el ESP32 | ✔ |
| POST | `/ota` | Actualiza el firmware (multipart) | ✔ |

### Ejemplos

```bash
# Leer el estado
curl http://sema-001.local/api/v1/status

# Leer mediciones
curl http://sema-001.local/api/v1/sensors

# Cambiar configuración (requiere X-API-Key)
curl -X PUT http://sema-001.local/api/v1/config \
     -H "Content-Type: application/json" \
     -H "X-API-Key: clave-web-local" \
     -d '{"schema_version":1,"station":{"id":"SEMA-001","name":"Nueva"}}'
```

---

## 8. Dashboard y WebSocket

- **Dashboard**: página servida en `/` que consume la API y muestra estado +
  mediciones con auto-refresco.
- **WebSocket**: `/ws` (puerto 81) difunde las mediciones en tiempo real a los
  clientes conectados.

---

## 9. Cómo funciona (flujo de datos)

Cada ciclo de lectura (cada 10 s por defecto) sigue esta cadena:

```text
MEDIR → VALIDAR → PROCESAR → ALMACENAR → PUBLICAR
```

1. **MEDIR** — el `SensorManager` lee todos los sensores registrados.
2. **VALIDAR** — se aplica calibración (offset/gain/rango) y se marca la calidad.
3. **PROCESAR** — el `DerivedEngine` calcula magnitudes derivadas (punto de rocío,
   sensación térmica).
4. **ALMACENAR** — cada medición se agrega al `HistoryStore` (LittleFS, JSONL).
5. **PUBLICAR** — los `PublisherManager` (HTTP/MQTT) envían la medición; el motor
   de reglas evalúa umbrales y puede disparar **alarmas**; el WebSocket difunde.

Las **alarmas** y otros eventos se registran en el `EventLog` (persistente) y se
consultan por `/api/v1/alarms` o `/api/v1/events`.

---

## 10. Estructura del código

```text
SEMA/
├── include/core/         ← cabeceras/interfaces del Core
│   ├── SemaCore.hpp      ← orquestador (inicializa servicios y conduce el loop)
│   ├── ConfigManager.hpp ← configuración versionada + catálogo de sensores
│   ├── EventBus.hpp      ← bus de eventos tipado
│   ├── Measurement.hpp   ← modelo canónico de medición
│   ├── Capability.hpp    ← capacidades de la placa (bitmask)
│   ├── PowerManager.hpp  ← energía (perfiles, deep sleep, wake)
│   ├── Watchdog.hpp      ← task watchdog
│   ├── sensors/          ← Sensor.hpp, SensorManager, SensorFactory, drivers
│   ├── alarms/           ← RuleEngine, reglas
│   ├── events/           ← EventLog
│   ├── publishers/       ← PublisherManager, HTTP, MQTT
│   ├── network/          ← WiFiManager
│   ├── storage/          ← KeyValueStore, NvsStore, HistoryStore
│   └── web/              ← HttpServer (REST + dashboard + WebSocket)
├── src/                  ← implementaciones (espejo de include/)
│   └── main.cpp          ← muy pequeño: setup() y loop()
├── partitions.csv        ← tabla de particiones OTA
├── platformio.ini        ← configuración de build + librerías
└── docs/                 ← documentación
```

### Componentes clave

| Componente | Responsabilidad |
|------------|-----------------|
| `SemaCore` | Inicializa y conecta todos los servicios; conduce el bucle |
| `ConfigManager` | Carga/valida/guarda la configuración (NVS, transaccional) |
| `SensorManager` | Registra sensores, lee todos, aplica calibración |
| `SensorFactory` | Crea un driver según `model` (catálogo configurable) |
| `EventBus` | Publica/suscribe eventos tipados |
| `RuleEngine` | Evalúa reglas (umbrales) y dispara alarmas |
| `EventLog` | Persiste eventos en LittleFS |
| `HistoryStore` | Persiste mediciones en LittleFS |
| `PublisherManager` | Envía mediciones (HTTP webhook, MQTT) |
| `HttpServer` | REST + dashboard + WebSocket |
| `WiFiManager` | WiFi (STA/AP), mDNS, reconexión |
| `PowerManager` | Perfiles energéticos, deep sleep, wake |
| `Watchdog` | Reinicia si el bucle se bloquea |

---

## 11. Cómo extender SEMA

### Agregar un sensor nuevo

1. Creá `include/core/sensors/MiSensor.hpp` y `src/core/sensors/MiSensor.cpp`
   implementando la interfaz `Sensor` (`id()`, `model()`, `interface()`,
   `begin()`, `measure()`, `healthy()`).
2. Registrá el modelo en `SensorFactory::create()` (un `if` más).
3. Listo: ahora se puede usar desde la configuración con `"model": "MiSensor"`.

### Agregar un endpoint

1. Declará `void onXyz();` en `HttpServer.hpp`.
2. Implementalo en `HttpServer.cpp` (construir JSON y `server_.send(...)`).
3. Registralo en `HttpServer::begin()` con `server_.on("/api/v1/xyz", HTTP_GET, ...)`.

### Agregar una regla de alarma

En `SemaCore::setup()` (o desde configuración, en una iteración posterior):

```cpp
rules_.addRule({"high_temp", "EXT", "temperature", RuleOp::Gt, 40.0f});
```

Cuando `EXT:temperature > 40` se publica un evento `Alarm`.

---

## 12. Flujo de desarrollo y releases

Cada cambio sigue este ciclo (ver `REGLAS-DE-TRABAJO.md` y `VERSIONADO.md`):

```text
código → pio run (compilar) → bump de versión → CHANGELOG → commit
       → tag → push → release en GitHub (firmware.bin + SHA-256) → actualizar wiki
```

- **Versionado SemVer**: `MAJOR.MINOR.PATCH`; tags con prefijo `v` minúscula
  (`v0.40.0`); en firmware el string es sin `v` (`0.40.0`).
- **Conventional Commits** para los mensajes.
- El `firmware_manifest.json` guarda la versión y el hash SHA-256 del binario.

---

## 13. Glosario

| Término | Significado |
|---------|-------------|
| **Core** | Núcleo que orquesta los módulos; no depende de opcionales |
| **HAL** | Capa de abstracción de hardware (perfiles de placa/chip) |
| **Magnitud** | Lo que se mide (temperatura, humedad, presión, luz…) |
| **Canal** | Una magnitud concreta de un sensor (p. ej. `EXT:temperature`) |
| **Schema** | Versión del formato de configuración (hoy `1`) |
| **mDNS** | Nombre local `.local` sin necesidad de IP |
| **OTA** | Actualización de firmware por red |
| **Deep sleep** | Modo de bajo consumo; despierta por timer o GPIO |

---

## Lecturas relacionadas

- [`README.md`](../README.md) — especificación completa (433 secciones).
- [`Arquitectura.md`](Arquitectura.md) — arquitectura y principios.
- [`Decisiones.md`](DUDAS-Y-DECISIONES.md) — decisiones de diseño (D-0001…D-0060).
- [`IMPLEMENTACION.md`](IMPLEMENTACION.md) — especificación técnica.
- [`MEJORAS.md`](MEJORAS.md) — roadmap y Definition of Done.
- [`CONTINUACION.md`](CONTINUACION.md) — estado actual, pendientes y flujo para retomar.
- [`FUTURO.md`](FUTURO.md) — ideas opcionales y futuras.
