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

#define SEMA_FW_VERSION              "1.36.0"
#define SEMA_HW_VERSION              "rev0"
#define SEMA_CONFIG_SCHEMA_VERSION   1
#define SEMA_PROTOCOL_VERSION        1
