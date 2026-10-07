# Mejoras opcionales y futuras — SEMA

> **Tipo:** Catálogo de ideas opcionales | **Fecha:** 2026-10-03 | **Firmware:** v1.53.0

Registro de ítems **opcionales** (no forman parte de la Definition of Done §103).
Sirve para tener todo anotado por si en el futuro se quiere implementar. Organizado
por área; cada ítem es una idea, no un compromiso.

---

## 1. Seguridad

| Ítem | Nota |
|------|------|
| TLS/HTTPS (certificado) | Cifrar la web local y la API |
| OTA con firma/checksum | Verificar integridad antes de aplicar |
| Tokens/API keys revocables | Más de una clave por actor, con rotación |

## 2. Dashboard y web

| Ítem | Nota |
|------|------|
| Páginas por sección | Sensores, red, alarmas, energía, OTA |
| Multi-idioma (es/en) | Selección de idioma en la web |
| Tema claro/oscuro persistente | Guardar preferencia |

## 3. Almacenamiento y datos

| Ítem | Nota |
|------|------|
| Agregación por niveles | Alta resolución reciente + resumen a largo plazo |
| Export CSV/JSON | Descargar el histórico en un archivo |
| Retención por tiempo | Descartar por antigüedad, no solo por cantidad |

## 4. Comunicaciones

| Ítem | Nota |
|------|------|
| LoRaWAN (red) | Protocolo de red sobre LoRa |
| Red Zigbee multi-dispositivo | Sensores/actuadores Zigbee remotos |
| BLE | Configuración por Bluetooth |
| CoAP / MQTT-SN | Protocolos ligeros |

## 5. Energía

| Ítem | Nota |
|------|------|
| Gestión de panel solar | MPPT, voltaje de carga |
| Batería LiFePO4 / Li-ion | Curvas de descarga |
| Perfiles energéticos por horario | Ahorro programado |
| Wake por RTC (alarmas de hora) | Despertar a horas fijas |

## 6. Fiabilidad y operación

| Ítem | Nota |
|------|------|
| RTC hardware (DS3231) con batería | Hora real sin NTP |
| Config de respaldo (fail-safe) | Recuperar si la config se corrompe |
| Watchdog jerárquico por tarea | Un watchdog por cada tarea crítica |

## 7. Plataforma

| Ítem | Nota |
|------|------|
| Multi-estación | Varias SEMA en un dashboard |
| Zona horaria en timestamps | Hoy los timestamps son UTC |
| Health por sensor | Estado individual en la API |

---

## Regla de actualización

Este archivo se actualiza al **retirar** un ítem (cuando se implementa y pasa al
`CHANGELOG`) o al **agregar** nuevas ideas opcionales. No requiere release
(docs-only, ver `VERSIONADO.md`).

Ver también: [`CONTINUACION.md`](CONTINUACION.md) (pendientes activos) y
[`MEJORAS.md`](MEJORAS.md) (Definition of Done).
