#pragma once

// =============================================================================
// SEMA — Versiones del firmware
// =============================================================================
// Convención de numeración: ver docs/VERSIONADO.md
//
//   SEMA_FW_VERSION             → versión del firmware (SemVer, sin prefijo "v")
//   SEMA_HW_VERSION             → revisión del hardware (acumulativa)
//   SEMA_CONFIG_SCHEMA_VERSION  → esquema del JSON de configuración (entero)
//   SEMA_PROTOCOL_VERSION       → protocolo con el servidor central (entero)
//
// El prefijo "v" minúscula se reserva para tags de git y releases de GitHub.
//
// ⚠️ FUENTE DE VERDAD: estos mismos valores se inyectan en la compilación por
// `-D` en `platformio.ini` (build_flags de cada env), porque ESP32-Base no ve
// este header (la librería compila en unidades ajenas a `include/` del proyecto
// y usa los fallbacks de `base/board_config.h`). Mantener AMBOS en sincronía al
// cambiar de versión; si divergen, el banner de arranque muestra el valor de
// platformio.ini (el que realmente llega al compilador).

#define SEMA_FW_VERSION              "1.113.0"
#define SEMA_HW_VERSION              "rev0"
#define SEMA_CONFIG_SCHEMA_VERSION   1
#define SEMA_PROTOCOL_VERSION        1
