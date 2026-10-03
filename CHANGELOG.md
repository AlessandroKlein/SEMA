# Changelog

Todos los cambios relevantes de este proyecto se documentan en este archivo.

El formato sigue [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/)
y el proyecto adhiere a [Versionado Semántico](https://semver.org/lang/es/).
Ver también [`docs/VERSIONADO.md`](docs/VERSIONADO.md).

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

[0.2.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.2.0
[0.1.0]: https://github.com/AlessandroKlein/SEMA/releases/tag/v0.1.0
