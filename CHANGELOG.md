# Changelog

Todos los cambios relevantes de este proyecto se documentan en este archivo.

El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto adhiere a [Versionado Semántico](https://semver.org/lang/es/).
Ver también [`docs/VERSIONADO.md`](docs/VERSIONADO.md).

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

[0.5.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.5.0
[0.4.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.4.0
[0.3.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.3.0
[0.2.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.2.0
[0.1.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.1.0
