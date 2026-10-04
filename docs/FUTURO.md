# Mejoras opcionales y futuras — SEMA

> **Tipo:** Catálogo de ideas opcionales | **Fecha:** 2026-10-03 | **Firmware:** v1.0.0

Registro de ítems **opcionales** (no forman parte de la Definition of Done §103).
Sirve para tener todo anotado por si en el futuro se quiere implementar. Organizado
por área; cada ítem es una idea, no un compromiso.

---

## 1. Seguridad

| Ítem | Nota |
|------|------|
| TLS/HTTPS (certificado) | Cifrar la web local y la API |
| Rate limiting | Limitar intentos de login y de la API |
| Expiración de sesión (login web) | Tiempo máximo de la cookie |
| OTA con firma/checksum | Verificar integridad antes de aplicar |
| Tokens/API keys revocables | Más de una clave por actor, con rotación |

## 2. Dashboard y web

| Ítem | Nota |
|------|------|
| Gráficos de histórico | Visualizar evolución de mediciones |
| Edición de config desde la UI | Formularios en vez de JSON crudo |
| Páginas por sección | Sensores, red, alarmas, energía, OTA |
| Multi-idioma (es/en) | Selección de idioma en la web |
| Tema claro/oscuro persistente | Guardar preferencia |

## 3. Almacenamiento y datos

| Ítem | Nota |
|------|------|
| Rotación del EventLog (D-0057) | El histórico ya rota; falta el de eventos |
| Agregación por niveles | Alta resolución reciente + resumen a largo plazo |
| Export CSV/JSON | Descargar el histórico en un archivo |
| Retención por tiempo | Descartar por antigüedad, no solo por cantidad |

## 4. Comunicaciones

| Ítem | Nota |
|------|------|
| LoRa / LoRaWAN | Nodos remotos de largo alcance |
| Zigbee / 802.15.4 | Red de sensores |
| Ethernet (W5500 / ENC28J60) | Conexión cableada |
| BLE | Configuración por Bluetooth |
| CoAP / MQTT-SN | Protocolos ligeros |

## 5. Sensores adicionales

| Ítem | Nota |
|------|------|
| Rayos (AS3935) | Detección de tormentas (SPI/I²C + IRQ) |
| CO (MQ-7 / MICS-5524) | Monóxido de carbono |
| Radiación solar (piranómetro) | Salida analógica (ya soportado por `ADC`) |
| Calidad de aire (SGP30) | eCO₂ / TVOC |
| Veleta (dirección del viento) | Encoder/ADC (velocidad ya cubierta por PCNT) |

## 6. Energía

| Ítem | Nota |
|------|------|
| Gestión de panel solar | MPPT, voltaje de carga |
| Batería LiFePO4 / Li-ion | Curvas de descarga |
| Perfiles energéticos por horario | Ahorro programado |
| Wake por RTC (alarmas de hora) | Despertar a horas fijas |

## 7. Hardware / expansión

| Ítem | Nota |
|------|------|
| MCP23017 | Expansor I²C de 16 GPIO |
| ADS1115 | ADC externo 16 bits |
| 74HC595 / 74HC165 | Shift registers (salidas/entradas) |
| RS485 / Modbus RTU | Bus industrial |
| CAN (TWAI) | Bus industrial |
| GPIO standalone | Entradas/salidas digitales independientes de sensores |

## 8. Fiabilidad y operación

| Ítem | Nota |
|------|------|
| Re-aplicar config sin reinicio | Hot reload de sensores/reglas/publicadores |
| RTC hardware (DS3231) con batería | Hora real sin NTP |
| Config de respaldo (fail-safe) | Recuperar si la config se corrompe |
| Watchdog jerárquico por tarea | Un watchdog por cada tarea crítica |

## 9. Plataforma

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
