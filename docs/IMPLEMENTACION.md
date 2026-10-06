# Arquitectura e implementación — SEMA

> **Tipo:** Concepto | **Estado:** Planificación | **Fecha:** 2026-10-03 | **Versión:** 1.16.0

Consolida la arquitectura de SEMA a partir de la especificación del `README.md`.
Es la referencia para implementar el firmware de forma modular. El estado de qué
está hecho y qué falta vive en [`MEJORAS.md`](MEJORAS.md); las decisiones con su
motivo, en [`DUDAS-Y-DECISIONES.md`](DUDAS-Y-DECISIONES.md).

---

## 1. Principios de arquitectura

Reglas que gobiernan todo diseño (`README.md` §2, §4, §102, §105):

```text
SENSOR ≠ FUNCIÓN
GPIO   ≠ SENSOR
BUS    ≠ SENSOR
MODELO ≠ MAGNITUD
HARDWARE ≠ CONFIGURACIÓN
```

- El hardware define las capacidades; la configuración web define cómo se usan.
- Simple por fuera, modular por dentro.
- El Core nunca depende de módulos opcionales.
- Una falla opcional no detiene la adquisición.

---

## 2. Arquitectura conceptual (`README.md` §78)

```text
                    SEMA CORE
                        │
        ┌───────────────┼────────────────┐
        │               │                │
    Hardware         Sensors          Services
        │               │                │
        │               │                ├── Web
        │               │                ├── API
        │               │                ├── MQTT
        │               │                ├── OTA
        │               │                └── Storage
        │               │
        ├── GPIO        ├── Temperature
        ├── I²C         ├── Humidity
        ├── SPI         ├── Pressure
        ├── UART        ├── Light
        ├── ADC         ├── UV
        ├── RS485       ├── CO₂
        ├── CAN         ├── PM
        ├── 1-Wire      ├── Wind
        ├── MCP23017    ├── Rain
        ├── 74HC595     ├── Solar
        ├── 74HC165     ├── Lightning
        └── ADS1115     ├── Soil
                        └── Energy
```

Tres capas desacopladas: el **Hardware** expone buses/expansores, los **Sensors**
exponen magnitudes (no modelos), y los **Services** consumen esas magnitudes.

---

## 2.1 Compatibilidad multigeneración: perfiles y HAL

La compatibilidad entre variantes de ESP32 **no se resuelve con `#ifdef`
repartidos** (ver [`DUDAS-Y-DECISIONES.md`](DUDAS-Y-DECISIONES.md) §2):

```text
SEMA Application
       │
       ▼
SEMA Core
       ├── Capability Manager
       ├── Resource Manager
       ├── Runtime Manager
       └── Hardware Abstraction Layer (HAL)
                    │
                    ▼
             Board/Chip Profile
```

- **Capability Manager** — qué puede hacer la plataforma (ADC, PCNT, PSRAM, SMP,
  Wi-Fi, 802.15.4, CAN/TWAI, …).
- **Resource Manager** — asigna y valida recursos (GPIO, periféricos, canales,
  buses) detectando conflictos.
- **Runtime Manager** — tareas y afinidad (`AUTO` por defecto) sobre FreeRTOS.
- **HAL + Board/Chip Profile** — única frontera con el hardware concreto.

---

## 3. Estructura de carpetas (`README.md` §77)

```text
SEMA/
├── include/
│   └── core/          ← interfaces y cabeceras del Core
├── src/
│   ├── main.cpp       ← pequeño: initialize → register → start → loop
│   ├── core/          ← núcleo (registry, event bus, config, health)
│   ├── config/        ← configuración persistente (NVS) y validación
│   ├── hardware/      ← drivers de GPIO, ADC y periféricos internos
│   ├── buses/         ← I²C, SPI, UART, 1-Wire, RS485, CAN
│   ├── sensors/       ← drivers de sensores (abstracción por magnitud)
│   ├── actuators/     ← salidas digitales/PWM
│   ├── communications/← MQTT, HTTP, WebSocket, publicadores externos
│   ├── storage/       ← LittleFS/SD, histórico, eventos
│   ├── energy/        ← medición de batería/panel, sleep, wake
│   ├── alarms/        ← alarmas y umbrales
│   ├── diagnostics/   ← salud, logs, inventario, métricas
│   ├── web/           ← servidor web local y páginas
│   ├── api/           ← endpoints REST
│   ├── ota/           ← actualización de firmware
│   └── modules/       ← módulos opcionales (LoRa, Zigbee, …)
└── docs/
```

Los detalles podrán evolucionar durante el desarrollo (`README.md` §77).

---

## 4. Módulos del Core (`README.md` §52, §200)

```text
CORE
 ├── Sensor Engine (medición + validación + calibración)
 ├── Configuration (persistencia, esquema versionado, migraciones)
 ├── Capability Manager / Resource Manager / Runtime Manager / HAL
 ├── Event Bus / Event Manager
 ├── Scheduler (tareas por capacidades)
 ├── Storage API (NVS / Flash / LittleFS / SD / externo)
 ├── Web Server / API / WebSocket
 └── Diagnostics (health, watchdog, observabilidad)

CAPACIDADES / INTERFACES (implementaciones, no módulos que contaminan el Core)
 ├── MQTT, HTTP, LoRa, Zigbee, RS485, CAN, OTA
 ├── Weather, Air Quality, Lightning, Soil, Energy
```

El Core conoce el concepto de almacenamiento y de cada capacidad, pero las
implementaciones concretas se registran por interfaz (`DUDAS-Y-DECISIONES.md`
D-0003).

---

## 5. Ciclo de vida de módulos (`README.md` §53)

```text
AVAILABLE → INSTALLED → CONFIGURED → ENABLED → RUNNING
RUNNING   → DISABLED  → UNINSTALLED

Errores: INSTALL_ERROR · CONFIG_ERROR · RUNTIME_ERROR · UPDATE_ERROR
```

---

## 6. Flujo de datos (`README.md` §201, §222)

```text
MEDIR → VALIDAR → PROCESAR → ALMACENAR → PUBLICAR → SINCRONIZAR → DORMIR
                                                                      │
                                              DESPERTAR POR EVENTO ←──┘
```

```text
Hardware → Measurement Engine → Canonical Data → Storage / API / Publishers
```

- **Storage** → histórico local.
- **API** → JSON (`/api/*`).
- **Publishers** → MQTT, HTTP, servicios cloud.
- Todo queda gobernado por el **System Core** (config, health, watchdog, NTP,
  seguridad, OTA) y el **Power Manager** (active / light sleep / deep sleep).

---

## 7. Event Bus y Event Manager (`README.md` §204-205)

El bus interno desacopla módulos:

```text
RAIN_START → Storage, Alarm, MQTT, Webhook, Dashboard, Wake Manager
```

Eventos con estructura común:

```text
SensorEvent · RainEvent · LightningEvent · BatteryEvent
NetworkEvent · AlarmEvent · SystemEvent · WakeEvent · SleepEvent
```

---

## 8. Scheduler y recursos dinámicos (`README.md` §206-208)

Las tareas se crean **dinámicamente según los módulos habilitados**:

```text
No LoRa → no hay tarea LoRa
No SD   → no hay tarea SD
No PMS5003 → no hay tarea PMS
```

Esto reduce RAM, CPU, consumo y complejidad.

---

## 9. Prioridades y no bloqueo (`README.md` §202-203)

```text
P1 Core / seguridad / watchdog
P2 Adquisición de sensores
P3 Procesamiento y almacenamiento
P4 Alarmas y eventos
P5 API local
P6 Comunicación
P7 Servicios externos
```

Ningún servicio externo puede bloquear `SensorTask`, `MeasurementTask`,
`StorageTask` ni el `Watchdog`.

---

## 10. Niveles de visualización (`README.md` §230)

```text
Nivel 1 — Estación autónoma (web local, sin servidor)
Nivel 2 — Estación + servicios externos (MQTT, ThingSpeak, Windy, …)
Nivel 3 — Sistema centralizado (Servidor Central opcional)
```

Los tres niveles coexisten; el Servidor Central nunca es obligatorio.

---

## 11. Definition of Done (`README.md` §103)

Una versión funcional requiere, como mínimo: Core, configuración persistente, web
local, API, dashboard modular, sistema de módulos y sensores, catálogo, detección
I²C/1-Wire, configuración GPIO/ADC, expansores, RS485/Modbus, CAN, LoRa, Zigbee,
medición energética, almacenamiento, histórico, alarmas, diagnóstico, calibración,
validación de configuración, backup, importación/exportación, OTA, seguridad,
watchdog y documentación. Los módulos aún no implementados quedan en `AVAILABLE`.

---

## 12. Plan de implementación

Las 9 fases de desarrollo (`README.md` §104) y su estado están en
[`MEJORAS.md`](MEJORAS.md).

---

## 13. Especificación técnica de la base del Core

Definiciones técnicas de las cinco piezas base (D-0041, D-0042, D-0044, D-0045,
D-0046), derivadas del contrato de [`DUDAS-Y-DECISIONES.md`](DUDAS-Y-DECISIONES.md).

### 13.1 JSON Schema de configuración v1 (D-0042)

```json
{
  "schema_version": 1,
  "station":  { "id": "SEMA-001", "name": "Estación Norte" },
  "board":    { "profile": "esp32" },
  "network":  { "mode": "STA", "ssid": "", "password": "", "hostname": "sema-001", "mdns": true },
  "time":     { "timezone": "America/Argentina/Buenos_Aires", "ntp": true },
  "sensors":  [],
  "channels": [],
  "buses":    {},
  "storage":  { "backend": "littlefs", "retention_days": 30 },
  "publishers": {},
  "energy":   {},
  "security": { "log_level": "INFO" },
  "modules":  {},
  "runtime":  {}
}
```

En Fase 1 se implementan `station`, `network`, `time`, `storage` y `security`;
el resto son placeholders que se completan en fases posteriores.

### 13.2 Canonical Data Model (D-0044)

```cpp
// include/core/Measurement.hpp
enum class Quality : uint8_t {
  Valid, Invalid, Stale, Timeout, OutOfRange,
  CalibrationError, CommunicationError, SensorDisconnected
};

struct Measurement {
  String stationId, sensorId, channelId, measurement, unit;
  float value;
  Quality quality;
  uint32_t sequence, timestamp;
};
```

JSON equivalente (`README.md` §173):

```json
{
  "station_id": "SEMA-001", "sensor_id": "TEMP_EXT",
  "channel_id": "1", "measurement": "temperature",
  "value": 24.7, "unit": "degC", "quality": "VALID",
  "sequence": 1234, "timestamp": 1790000000
}
```

### 13.3 Event Bus (D-0045)

```cpp
// include/core/EventBus.hpp
enum class EventType : uint8_t {
  Sensor, Rain, Lightning, Battery, Network, Alarm, System, Wake, Sleep
};
enum class Severity : uint8_t {
  Debug, Info, Notice, Warning, Error, Critical
};

struct Event {
  uint32_t id, timestampMs;
  const char* source, *correlationId, *target;
  EventType type;
  Severity severity;
  int32_t value;   // payload numérico simple
};
```

### 13.4 Storage API (D-0046)

```text
Storage API
 ├── NVS        → configuración, identidad, contadores
 ├── Flash/LittleFS/SD → histórico, eventos, logs
```

Interfaz mínima implementada: `include/core/storage/Storage.hpp` (`begin`,
`getString/putString`, `getUInt/putUInt`, `clear`) + `NvsStore` (Preferences).

### 13.5 REST API `/api/v1` (D-0041)

```text
GET  /api/v1/status        resumen general
GET  /api/v1/health        salud (uptime, heap, wifi, sensores)
GET  /api/v1/system        identidad, firmware, versiones
GET  /api/v1/config        configuración (schema=1)
GET  /api/v1/diagnostics   diagnóstico (reset reason, tasks, buses, storage, power)

PUT  /api/v1/config        aplicar configuración (transaccional, autenticado)
POST /api/v1/restart       reinicio (autenticado)
```

WebSocket en `/ws` para datos y eventos en tiempo real.
