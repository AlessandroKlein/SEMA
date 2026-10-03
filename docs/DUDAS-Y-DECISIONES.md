# Dudas y decisiones — SEMA

> **Tipo:** Convención | **Estado:** Planificación | **Fecha:** 2026-10-03 | **Versión:** 0.3.0

Registro de decisiones de arquitectura (ADR) y dudas pendientes. Este documento
es el **contrato arquitectónico** previo al desarrollo del Core de SEMA.
Formato basado en `ESTANDAR-DOCUMENTACION.md` §6.

---

## 1. Resumen de decisiones

| ID | Decisión | Estado |
|----|----------|--------|
| D-0001 | Plataforma configurable, no estación fija | Cerrada |
| D-0002 | Separación Hardware / Sensor / Servicio | Cerrada |
| D-0003 | Almacenamiento y protocolos como abstracciones del Core | Cerrada |
| D-0004 | Web local autónoma; Servidor Central opcional | Cerrada |
| D-0005 | Versionado SemVer con prefijo `v` | Cerrada |
| D-0006 | REST API + WebSocket forman parte del Core | Cerrada |
| D-0007 | Modelo Canónico de Mediciones | Cerrada |
| D-0008 | Event Bus interno | Cerrada |
| D-0009 | Store & Forward para datos | Cerrada |
| D-0010 | Publishers externos desacoplados | Cerrada |
| D-0011 | Servidor Central opcional | Cerrada |
| D-0012 | FreeRTOS como runtime | Cerrada |
| D-0013 | SMP adaptativo | Cerrada |
| D-0014 | Task Affinity `AUTO` por defecto | Cerrada |
| D-0015 | Compatibilidad basada en Board/Chip Profiles | Cerrada |
| D-0016 | Capability Manager | Cerrada |
| D-0017 | Resource Manager y detección de conflictos | Cerrada |
| D-0018 | Event-driven + scheduler híbrido | Cerrada |
| D-0019 | Watchdog jerárquico | Cerrada |
| D-0020 | Health Monitor | Cerrada |
| D-0021 | Deep Sleep + Wake-up Manager | Cerrada |
| D-0022 | GPIO/INT de lluvia como fuente de wake-up | Cerrada |
| D-0023 | Configuración transaccional con rollback | Cerrada |
| D-0024 | Safe Mode / recuperación | Cerrada |
| D-0025 | Calibración independiente del driver | Cerrada |
| D-0026 | Quality Flags para mediciones | Cerrada |
| D-0027 | OpenAPI/JSON Schema para API | Cerrada |
| D-0028 | Identidad única de estación/dispositivo/sensor | Cerrada |
| D-0029 | Sistema de módulos instalables/habilitables | Cerrada |
| D-0030 | Arquitectura Offline-First | Cerrada |
| D-0031 | OTA con rollback | Cerrada |
| D-0032 | Almacenamiento por capas | Cerrada |
| D-0033 | Sistema de eventos y alarmas | Cerrada |
| D-0034 | Diagnóstico y observabilidad | Cerrada |
| D-0035 | Seguridad por roles/capacidades | Cerrada |
| D-0036 | Servidor Central multiestación | Cerrada |
| D-0037 | API de sincronización estación ↔ central | Cerrada |
| D-0038 | Compatibilidad ESP32 single-core y multicore | Cerrada |
| D-0039 | Abstracción RT (no acoplar SEMA a FreeRTOS) | Cerrada |
| D-0040 | Configuración avanzada de Tasks solo en Expert Mode | Cerrada |

---

## 2. Principio clave: compatibilidad por perfiles, no por `#ifdef`

La compatibilidad con `ESP32-WROOM-32E`, `ESP32-WROOM-32UE`, `ESP32`, `ESP32-S3`,
`ESP32-C5`, `ESP32-C6`, `ESP32-P4` y futuras variantes **no se resuelve con
`#ifdef` repartidos por el código**.

```text
SEMA Application
       │
       ▼
SEMA Core
       │
       ├── Capability Manager
       ├── Resource Manager
       ├── Runtime Manager
       └── Hardware Abstraction Layer (HAL)
                    │
                    ▼
             Board/Chip Profile
                    │
          ┌─────────┼─────────┐
          ▼         ▼         ▼
       ESP32      ESP32-S3   ESP32-C6
       Profile    Profile    Profile
```

El código de SEMA pregunta capacidades en lugar de preguntar por modelo:

```text
¿Tengo ADC? ¿Tengo PCNT? ¿Tengo PSRAM? ¿Tengo 2 cores? ¿Tengo SMP?
¿Tengo GPIO RTC? ¿Tengo Wi-Fi? ¿Tengo Bluetooth? ¿Tengo 802.15.4?
¿Tengo CAN/TWAI? ¿Cuántos UART? ¿Cuántos I²C?
```

en lugar de:

```cpp
#ifdef CONFIG_IDF_TARGET_ESP32C6
...
#elif ...
```

Esto es especialmente relevante para **FreeRTOS/SMP/Task Pinning**: SEMA usa
`AUTO` por defecto y consulta las capacidades reales antes de usar afinidad.

---

## 3. Decisiones detalladas

### 3.1 Plataforma y hardware

- **D-0015 — Board/Chip Profiles.** Cada placa/chip tiene un perfil que declara
  sus capacidades; la aplicación nunca usa `#ifdef` por modelo. Ver §2.
- **D-0038 — Single-core y multicore.** El código debe funcionar igual en ESP32
  single-core y multicore; la diferencia es configuración de runtime, no código.
- **D-0016 — Capability Manager.** Punto único de consulta de capacidades
  (`device.sensors.read`, `device.can`, `device.psram`, …). La UI y los módulos
  preguntan antes de mostrar o ejecutar.
- **D-0017 — Resource Manager.** Asigna y valida recursos (GPIO, periféricos,
  canales ADC, buses) detectando conflictos antes de aplicarlos.

### 3.2 Core y runtime

- **D-0012 — FreeRTOS como runtime.** Tasks, colas, semáforos y timers como
  base de concurrencia (`README.md` §202-203).
- **D-0013 — SMP adaptativo.** Se aprovechan los 2 cores cuando existen, sin
  depender de ellos.
- **D-0014 — Task Affinity `AUTO`.** Sin pinning por defecto; solo se fija CPU
  cuando hay una razón documentada.
- **D-0039 — Abstracción RT.** Capa propia de tareas/colas para no acoplar SEMA a
  la API de FreeRTOS; facilita portabilidad y tests.
- **D-0018 — Event-driven + scheduler híbrido.** Eventos para reaccionar
  (`RAIN_START`) y scheduler para tareas periódicas (lecturas, uploads).
- **D-0019 — Watchdog jerárquico.** Un watchdog de sistema + watchdog por tarea.
- **D-0020 — Health Monitor.** Estado/health de cada subsistema, recuperable por
  API y diagnóstico.
- **D-0023 — Configuración transaccional.** Validar → previsualizar → aplicar →
  persistir → verificar; si falla, rollback al estado anterior.
- **D-0024 — Safe Mode.** Si la configuración es inválida o el arranque falla, la
  estación arranca en modo mínimo de recuperación (AP + web básica).

### 3.3 Datos y sensores

- **D-0007 — Modelo Canónico de Mediciones.** Una única representación de la
  medición (`timestamp`, `sensor_id`, `magnitud`, `valor`, `unidad`, `calidad`)
  compartida por adquisición, storage, API y publishers.
- **D-0026 — Quality Flags.** Toda medición lleva estado: `VALID`, `INVALID`,
  `STALE`, `TIMEOUT`, `OUT_OF_RANGE`, `CALIBRATION_ERROR`,
  `COMMUNICATION_ERROR`, `SENSOR_DISCONNECTED`.
- **D-0025 — Calibración independiente del driver.** Offset, gain, multipunto,
  mínimo/máximo y filtro viven en la configuración del sensor, no en el driver.
- **D-0032 — Almacenamiento por capas.** Configuración (NVS), histórico, eventos
  y logs separados; LittleFS/SD según disponibilidad.
- **D-0033 — Eventos y alarmas.** Los eventos generan alarmas con severidad,
  origen, timestamp, estado y acuse (`ack`).
- **D-0028 — Identidad única.** IDs estables para estación, dispositivo, sensor y
  módulo (legibles + técnicos).

### 3.4 Comunicación y distribución

- **D-0006 — REST + WebSocket en el Core.** La estación expone API local y canal
  `/ws` sin depender de ningún servidor.
- **D-0009 — Store & Forward.** Los datos se encolan localmente y se reenvían al
  recuperar la conexión, sin perder mediciones.
- **D-0010 — Publishers desacoplados.** Cada servicio externo (ThingSpeak, Windy,
  Weathercloud, PWSWeather, MQTT, HTTP genérico) es un publicador independiente
  sobre el modelo canónico; nunca bloquea la adquisición.
- **D-0011 / D-0030 — Offline-First.** La estación es autónoma; Internet, cloud,
  MQTT y servidor son complementarios, no requisitos.
- **D-0036 — Servidor Central multiestación.** Componente independiente para
  gestionar N estaciones sin quitarles autonomía.
- **D-0037 — API de sincronización estación ↔ central.** Sincronización
  incremental con identificador de secuencia y Store & Forward.

### 3.5 Módulos, operación y seguridad

- **D-0029 — Módulos instalables/habilitables.** Ciclo de vida
  `AVAILABLE → INSTALLED → CONFIGURED → ENABLED → RUNNING` y estados de error.
- **D-0031 — OTA con rollback.** Doble partición, checksum y vuelta atrás segura.
- **D-0021 — Deep Sleep + Wake-up Manager.** Ciclo dormir/despertar centralizado
  (RTC, GPIO INT, lluvia) según perfil energético.
- **D-0022 — Lluvia como wake-up.** GPIO/INT del pluviómetro despierta la estación
  y suma al contador de pulsos.
- **D-0034 — Diagnóstico y observabilidad.** Logs estructurados
  (`timestamp, level, module, event, message, code`), métricas y health.
- **D-0035 — Seguridad por roles/capacidades.** RBAC + least privilege; API
  autenticada, API Keys revocables, rate limiting.
- **D-0027 — OpenAPI/JSON Schema.** La API se describe con esquema versionado para
  generar clientes y validar.
- **D-0040 — Tasks solo en Expert Mode.** El usuario normal trabaja con magnitudes;
  la configuración de tasks/prioridades queda en modo experto.

---

## 4. Correcciones conceptuales

### D-0003 (corregida) — Almacenamiento y protocolos como abstracciones del Core

**SD no es "un módulo que no carga lógica si no existe".** El almacenamiento es
una abstracción del Core:

```text
Storage API
    │
    ├── Internal NVS
    ├── Flash partition
    ├── LittleFS
    ├── SD
    └── External storage
```

SEMA funciona perfectamente sin SD, pero el Core **sí conoce el concepto de
almacenamiento**. Lo mismo aplica a MQTT, HTTP, LoRa, Zigbee, RS485 y CAN: son
**implementaciones de capacidades/interfaces**, no módulos que contaminan el
núcleo meteorológico.

---

## 5. Dudas abiertas

| # | Duda |
|---|------|
| Q-0001 | Definir endpoints REST mínimos de Fase 1 |
| Q-0002 | Definir JSON Schema de configuración `schema=1` |
| Q-0003 | Seleccionar bibliotecas/drivers definitivos por sensor |
| Q-0004 | Definir estructura exacta del Canonical Data Model |
| Q-0005 | Definir estructura del Event Bus |
| Q-0006 | Definir esquema de almacenamiento local |
| Q-0007 | Definir protocolo estación ↔ Servidor Central |
| Q-0008 | Definir autenticación API y roles |
| Q-0009 | Definir política definitiva de OTA y rollback |
| Q-0010 | Definir perfiles oficiales de placas ESP32 soportadas |
| Q-0011 | Definir matriz de capacidades de GPIO/periféricos por chip |
| Q-0012 | Definir perfiles de FreeRTOS y prioridades de Tasks |
| Q-0013 | Definir qué Tasks pueden usar Task Pinning |
| Q-0014 | Definir política de Deep Sleep por perfil energético |
| Q-0015 | Definir esquema de calibración |
| Q-0016 | Definir Quality Flags |
| Q-0017 | Definir retención de históricos |
| Q-0018 | Definir mecanismo de descubrimiento de sensores |
| Q-0019 | Definir sistema de alarmas/reglas |
| Q-0020 | Definir protocolo de comunicación de módulos externos |

---

## 6. Regla de actualización

Cada decisión nueva se registra aquí con motivo y consecuencias; cuando queda
cerrada, se refleja en [`IMPLEMENTACION.md`](IMPLEMENTACION.md) y en el repo
`Docs`. Las dudas `Q-xxxx` se cierran con una decisión `D-xxxx` antes de
implementar el área afectada.
