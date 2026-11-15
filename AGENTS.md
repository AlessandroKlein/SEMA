# AGENTS.md — Memoria de arranque de SEMA

> **Propósito:** este archivo es la **memoria persistente** del proyecto. Todo agente
> o chat nuevo **lo lee primero** y no necesita que se le re-explique el proyecto.
> Fue creado para que un chat nuevo tenga un único punto de entrada.

---

## 1. Qué es SEMA (una línea)

**SEMA (Sistema de Estación Meteorológica Autónoma)** es firmware C/C++ para ESP32
(PlatformIO + framework Arduino) que construye estaciones meteorológicas/ambientales
modulares: el hardware define las capacidades y una **web local configura todo**
(sensores, buses, pines, alarmas, publicadores, calibración) sin recompilar.

Principio: **simple por fuera, modular por dentro.** Firmware actual: **v1.107.0**
(`include/core/Version.hpp`), `hw_version = rev0`, `config_schema_version = 1`,
`protocol_version = 1`.

---

## 2. Repositorios

| Repo | URL | Rol |
|------|-----|-----|
| Código (este) | `git@github.com:AlessandroKlein/SEMA.git` | Firmware SEMA |
| Documentación | `git@github.com:AlessandroKlein/Docs.git` | Wiki unificada (MkDocs → GitHub Pages) |
| Base reutilizable | `git@github.com:AlessandroKlein/ESP32-Base.git` | Librería PlatformIO (WiFi/Ethernet/pines/web/NVS/runtime) sin lo meteorológico; SEMA la consume por `lib_deps` git+tag |
| Servidor central | `git@github.com:AlessandroKlein/Webs.git` | Servidor central multi-proyecto (fuera del alcance del firmware; SEMA solo se conecta con `server_key`) |

- Sistema operativo de trabajo: **Windows**; `git` y `gh` instalados y logueados
  (se puede commit/push/release sin credenciales adicionales).
- El "Servidor Central" **no** es parte de SEMA; SEMA publica por HTTP/MQTT con
  `server_key` y `X-API-Key` local.

---

## 3. Equipo multi-agente (teamagent)

### 3.1 Modelo de orquestación

El **chat principal es el orquestador**: recibe la idea del usuario, la **rearma**
incorporando los datos faltantes (hardware, versión, alcance, referencias al repo)
y la pasa **completa y con contexto** al agente de rol correspondiente.

> El chat principal **no implementa**; delega en el agente adecuado y luego integra
> el resultado. Cada agente recibe: (a) su prompt de sistema, (b) la tarea concreta,
> (c) los datos relevantes ya resueltos.

Herramientas de despacho: `subagent` (tarea autocontenida) / `subagent_fork`
(tarea que continúa el contexto de esta conversación). Se despachan **por tarea**,
no se dejan agentes ociosos.

### 3.2 Asignación de modelos por rol

| Modelo | Roles |
|--------|-------|
| DeepSeek-V4-Pro | Backend Architect, Frontend Engineer, Code Reviewer, Researcher |
| DeepSeek-V4.1-Flash | Documentador, Release Manager, Traductor, Tester (QA Automation) |
| DeepSeek-V4-Pro / DeepSeek Account (modlens vision) | Tester Visual / UI Reviewer |

### 3.3 Prompts de sistema (verbatim)

**Investigador (Researcher) — DeepSeek-V4-Pro**
> "Actúas como Investigador Técnico Principal. Tu objetivo es analizar requerimientos técnicos, buscar mejores prácticas de arquitectura y resolver bloqueos de implementación complejos. Analiza los pro y contra de cada solución antes de proponer código final. Entrega análisis claros, estructurados y justificados."

**Backend Engineer — DeepSeek-V4-Pro**
> "Actúas como Desarrollador Backend Senior. Tu responsabilidad es diseñar e implementar APIs, lógica de negocio y estructuras de datos eficientes. Prioriza la seguridad, el rendimiento y la mantenibilidad. Genera código modular dividiendo las interfaces y lógica en sus archivos `.hpp` o `.cpp` / módulos correspondientes con documentación inline precisa."

**Frontend Engineer — DeepSeek-V4-Pro**
> "Actúas como Desarrollador Frontend Senior. Tu función es construir interfaces de usuario limpias, funcionales y reactivas. Garantiza una correcta integración con las APIs del backend y mantén una estructura modular y reutilizable."

**Tester & QA — DeepSeek-V4-Pro (modlens vision) / DeepSeek-V4.1-Flash**
> "Actúas como Ingeniero de QA y Pruebas Automáticas. Tu rol es diseñar casos de prueba unitarios, de integración y validar interfaces visuales. Analiza capturas de pantalla o fragmentos de código para identificar inconsistencias, edge cases y posibles errores de ejecución."

**Revisión de Código (Code Reviewer) — DeepSeek-V4-Pro**
> "Actúas como Revisor de Código Principal (*Code Reviewer*). Analizas el código producido por los agentes de Backend y Frontend antes de su aprobación. Evalúas legibilidad, seguridad, optimizaciones de memoria y adherencia a los estándares del proyecto. Si detectas fallos, proporciona sugerencias concretas de corrección."

**Documentador — DeepSeek-V4.1-Flash**
> "Actúas como Especialista en Documentación Técnica. Tu tarea es mantener actualizada la documentación del proyecto, incluyendo comentarios claros en el código, especificaciones de APIs y archivos `README.md`. Explica de forma sencilla la función de cada elemento sin redundancias."

**Gestor de Lanzamientos (Release Manager) — DeepSeek-V4.1-Flash**
> "Actúas como Release Manager. Te encargas de rastrear commits, gestionar la versión del software, estructurar el archivo `CHANGELOG.md` y verificar los requerimientos para el empaquetado y despliegue del proyecto."

**Traductor — DeepSeek-V4.1-Flash**
> "Actúas como Traductor Técnico. Tu rol es traducir fielmente cadenas de localización (i18n), archivos de documentación y mensajes del sistema entre diferentes idiomas, conservando la terminología técnica adecuada."

### 3.4 Mapeo de roles → este proyecto (ESP32 C/C++)

| Rol genérico | Qué toca en SEMA |
|--------------|------------------|
| Backend Architect | `src/core/**`, `include/core/**` (SemaCore, módulos, sensores, API REST `/api/v1/*`, publicadores, storage) |
| Frontend Engineer | `src/core/web/HttpServer.cpp`, `src/core/web/GridstackAssets.h`, dashboard web servido desde PROGMEM |
| Code Reviewer | Revisa diffs de `.hpp`/`.cpp` antes de integrar; seguridad, memoria, adherencia a `docs/REGLAS-DE-TRABAJO.md` |
| Researcher | Investiga librerías ESP32/PlatformIO, decisiones de arquitectura, bloqueos |
| Tester | `pio run` (validación), `pio test`, smoke de la API, scripts de QA |
| Tester Visual | Capturas del dashboard web / UI del dispositivo |
| Documentador | `README.md`, `docs/*`, `CHANGELOG.md`, comentarios inline, y repo `Docs.git` (MkDocs, subagente dedicado) |
| Release Manager | Bump de versión, `firmware_manifest.json` + SHA-256, tag + `gh release` |
| Traductor | i18n de la web (si aplica), docs en `es`/`en` |

> **Documentador dedicado de `Docs.git`**: el repo de documentación unificada
> (`git@github.com:AlessandroKlein/Docs.git`, MkDocs → GitHub Pages) lo mantiene un
> **subagente Documentador exclusivo**, que solo documenta el código y el repo Docs.
> Todo cambio de código con comportamiento público se refleja en `Docs.git` vía ese
> subagente, no editándolo a mano desde el orquestador.

---

## 4. Hardware: placas, memoria y variantes

### 4.1 Placas soportadas

| Env (`platformio.ini`) | Board | Flash | Ethernet | Pines |
|------------------------|-------|-------|----------|-------|
| `esp32doit-devkit-v1` | ESP32-WROOM clásico | 4 MB | LAN8720A nativa (RMII) | configurables (`SEMA_PINS_FROM_FILE=0`) |
| `esp32-s3-devkitc-1` | ESP32-S3 | 8 MB | W5500 por SPI (sin MAC nativa) | configurables (`=0`) |
| `esp32-wroom-32u` | ESP32-WROOM-32U | 16 MB | LAN8720A nativa (RMII) | **fijos** (`SEMA_PINS_FROM_FILE=1`, PCB) |
| `esp32-wroom-32u-32mb` | ESP32-WROOM-32U | 32 MB | LAN8720A nativa (RMII) | **fijos** (`=1`, PCB) |
| `esp32-p4` *(env deshabilitado)* | ESP32-P4 (RISC-V) | 32 MB | W5500 por SPI | **fijos** (`=1`) — requiere PlatformIO `espressif32 ≥ 7.2` (no disponible hoy) |
| `demo` | extiende `esp32doit-devkit-v1` | 4 MB | — | `-D SEMA_DEMO=1` (valores ficticios) |

Memorias contempladas por el usuario: **4 / 8 / 16 / 32 MB**. Implementadas las
cuatro (`partitions_4mb.csv`, `partitions_8mb.csv`, `partitions_16mb.csv`,
`partitions_32mb.csv`).

Identidades para OTA/manifest (`include/hw/HwProfile.hpp`): `esp32-wroom-4mb`,
`esp32-s3-8mb`, `esp32-wroom32u-16mb`, `esp32-wroom32u-32mb`, `esp32-p4-32mb`.

> ⚠️ **Decisión pendiente (Ethernet WROOM32U)**: la PCB WROOM-32U real usa W5500
> en VSPI (18/19/23, CS=GPIO5) compartiendo bus con SD (CS=GPIO4) y LoRa, pero el
> perfil de firmware aún define `SEMA_NATIVE_ETH 1` (LAN8720A RMII) con SPI en
> 14/12/15. Falta consolidar W5500 al bus SPI compartido (driver Arduino Ethernet)
> o añadir un segundo bus SPI. Ver `docs/PCB-V1-CORRECCIONES.md` §8.

### 4.2 Dos variantes del firmware

| Variante | Build flag | Qué es |
|----------|-----------|--------|
| **Modular** (por defecto) | `SEMA_PINS_FROM_FILE=0` | El usuario configura pines/buses/sensores desde la web |
| **PCB fijo (producción en masa)** | `SEMA_PINS_FROM_FILE=1` | Pines y elementos fijos definidos en `include/hw/HwProfile.hpp`; no configurables por web |

Mismo código fuente, dos perfiles por build flags. La única fuente de verdad del
hardware es `include/hw/HwProfile.hpp` (board, features, transceiver, origen de pines).

> **Compatibilidad dual (PCB ↔ modular)**: toda feature o dato de compatibilidad
> (sensores, buses, pines, flags) debe funcionar en **ambas variantes**. Lo que sea
> específico de una variante se expone detrás de un flag de compilación
> (`-D SEMA_USE_*=0/1`) para poder deshabilitarlo, nunca se hardcodea a una sola.

---

## 5. Reglas de código y organización

1. **Una clase por archivo**: `.hpp` en `include/core/`, `.cpp` en `src/core/`.
2. **Separar cada dominio en su archivo**: WiFi (`core/network/WiFiManager.*`),
   web (`core/web/HttpServer.*`), sensores (`core/sensors/*`), publicadores
   (`core/publishers/*`), storage (`core/storage/*`), alarmas (`core/alarms/*`), etc.
   No mezclar wifi con web ni sensores con publicadores.
3. **Comentar cada elemento**: comentar el **por qué**, no el qué; `// TODO:` / `// FIXME:` con fecha y autor.
4. **Optimizar siempre**: SEMA corre en ESP32 con RAM/Flash limitadas; cada feature
   es compilación condicional (`-D SEMA_USE_*=0/1`) para reducir firmware. No agregar
   código muerto ni deuda.
5. **No inventar APIs/librerías/flags**: verificar contra el código real o la
   documentación oficial de PlatformIO/ESP32. Ante la duda, preguntar.
6. **No hardcodear secretos** (tokens/claves): NVS o `.env`, nunca en el fuente.
7. **No refactorizar código que no se toca** (salvo cleanup local < 10 líneas).

Reglas completas y de mayor prioridad: `docs/REGLAS-DE-TRABAJO.md` (este archivo
manda sobre cualquier petición en conflicto). Flujo de versionado: `docs/VERSIONADO.md`.

---

## 6. Estado actual y cómo continuar

- **Estado/continuación** (leer de principio a fin antes de seguir):
  [`docs/CONTINUACION.md`](docs/CONTINUACION.md) — resume hecho, pendientes y flujo.
- **Pendientes reales**: [`docs/PENDIENTES.md`](docs/PENDIENTES.md).
- **Guía de inicio**: [`docs/GUIA.md`](docs/GUIA.md).
- **Pines por board**: [`docs/PINES-POR-BOARD.md`](docs/PINES-POR-BOARD.md).
- **Decisiones D-0001…D-0060**: [`docs/DUDAS-Y-DECISIONES.md`](docs/DUDAS-Y-DECISIONES.md).
- **Ideas futuras/backlog**: [`docs/FUTURO.md`](docs/FUTURO.md).

### Flujo obligatorio por cambio de código (resumen de REGLAS-DE-TRABAJO)

```text
1. pio run -e esp32doit-devkit-v1        # compilar/validar → SUCCESS
2. bump MINOR (feat) / PATCH (fix) en include/core/Version.hpp
3. CHANGELOG.md (nueva entrada)
4. docs/MEJORAS.md + docs/IMPLEMENTACION.md (cabecera Versión)
5. (si aplica) Get-FileHash firmware.bin → firmware_manifest.json (SHA-256 real)
6. git add -A && git commit (Conventional Commits en inglés)
7. git tag -a vX.Y.Z -m "..." && git push origin main --tags
8. gh release create vX.Y.Z --repo AlessandroKlein/SEMA --notes-file <tmp.md> firmware.bin
9. Actualizar repo Docs (python -m mkdocs build → SUCCESS → push)
```

Docs-only **no** genera release (ver `docs/VERSIONADO.md`).

---

## 7. Regla de actualización de este archivo

`AGENTS.md` se actualiza cuando cambia algo que un chat nuevo necesita saber para
arrancar: nueva board, nueva variante, nuevo repo, cambio de flujo o de equipo.
Es docs-only (sin release).

---

## 8. Operación del teamagent (cómo trabaja el chat principal)

- El **equipo de 9 roles ya está activo** (§3): no hay que "crear" agentes uno a
  uno. El usuario solo **manda la idea/datos** y el orquestador **completa lo que
  falta** (hardware, versión, alcance, referencias al repo) y **despacha la tarea
  completa al rol** que corresponda con `subagent`/`subagent_fork`.
- Se despacha **por tarea**, no se mantienen agentes ociosos. El resultado del
  agente lo **integra el orquestador** (compila, verifica, commitea).

### 8.1 Modo pausa / reanudación

El usuario puede **pausar en cualquier momento** y retomar otro día:

| El usuario dice | Qué hace el orquestador |
|-----------------|-------------------------|
| `pausa` / `pausá` | Detiene el trabajo, **guarda el estado en los archivos de memoria** (este `AGENTS.md`, `docs/CONTINUACION.md`, `docs/PENDIENTES.md`) y queda a la espera. |
| `continuá` / `seguí` | **Lee de nuevo** `AGENTS.md` + `docs/CONTINUACION.md` + `docs/PENDIENTES.md`, retoma desde el último estado guardado sin re-explicar el proyecto. |

> La memoria persistente es la garantía de reanudación: **cada cierre de tarea deja
> los pendientes actualizados** en `docs/PENDIENTES.md` y `docs/CONTINUACION.md`, así
> un chat nuevo (o el mismo, otro día) arranca leyendo y sigue sin perder contexto.
