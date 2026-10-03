# Dudas y decisiones — SEMA

> **Tipo:** Convención | **Estado:** Planificación | **Fecha:** 2026-10-03 | **Versión:** 0.2.0

Registro de decisiones de arquitectura (ADR) y dudas pendientes. Formato basado en
`ESTANDAR-DOCUMENTACION.md` §6. Las decisiones firmes pueden migrar a `docs/adr/`.

---

## 1. Decisiones

### D-0001 — Plataforma configurable, no una estación fija

- **Decisión:** diseñar SEMA como plataforma configurable para construir distintas
  estaciones, no como una estación con sensores fijos.
- **Motivo:** `README.md` §102. Un mismo firmware debe servir a instalaciones muy
  diferentes cambiando solo configuración.
- **Consecuencia:** toda magnitud se abstrae del modelo de sensor y del hardware.

### D-0002 — Separación Hardware / Sensor / Servicio

- **Decisión:** separar tres capas: Hardware (buses/expansores), Sensors
  (magnitudes) y Services (web/API/MQTT/storage).
- **Motivo:** `README.md` §4, §46, §78. Evita acoplar un sensor a un GPIO o bus.
- **Consecuencia:** el código de aplicación trabaja con magnitudes, no con modelos.

### D-0003 — Módulos opcionales fuera del Core

- **Decisión:** LoRa, Zigbee, RS485, CAN, MQTT, SD y OTA son módulos opcionales.
- **Motivo:** `README.md` §52. Una instalación sin LoRa no carga lógica LoRa.
- **Consecuencia:** el Core no debe depender de ellos; se registran por capacidad.

### D-0004 — Web local autónoma (Servidor Central opcional)

- **Decisión:** la interfaz web completa vive en el ESP32; el Servidor Central es
  opcional.
- **Motivo:** `README.md` §224, §230-231. La estación debe funcionar sin Internet.
- **Consecuencia:** REST/WebSocket locales son requisito del Core, no del central.

### D-0005 — Versionado SemVer con prefijo `v` en tags

- **Decisión:** firmas `0.x` durante el desarrollo inicial; prefijo `v` minúscula
  solo en tags/releases.
- **Motivo:** `docs/VERSIONADO.md`.
- **Consecuencia:** constantes del firmware sin `v`; CHANGELOG sin `v`.

---

## 2. Dudas pendientes

| # | Duda | Estado |
|---|------|--------|
| Q-0001 | Definir el conjunto mínimo de endpoints REST de la Fase 1 | abierta |
| Q-0002 | Definir el esquema JSON de configuración (schema 1) | abierta |
| Q-0003 | Confirmar bibliotecas exactas de drivers por sensor | abierta |

---

## 3. Regla de actualización

Cada decisión nueva se registra aquí con motivo y consecuencias; cuando queda
cerrada, se refleja en [`IMPLEMENTACION.md`](IMPLEMENTACION.md) y en el repo `Docs`.
