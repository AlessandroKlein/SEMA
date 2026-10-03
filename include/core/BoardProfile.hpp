#pragma once

// =============================================================================
// SEMA — Board Profile (D-0050) — pines fijos para PCB personalizada
// =============================================================================
// Define los pines por defecto del catálogo de sensores. Para una PCB propia:
//
//   1. Poné `SEMA_FIXED_HARDWARE` en `1`.
//      → deshabilita la configuración de pines por web: el catálogo `sensors[]`
//        de la configuración se ignora y se usa el catálogo fijo de abajo.
//   2. Ajustá los pines (`SEMA_PIN_*`) a tu diseño.
//
// Con `SEMA_FIXED_HARDWARE = 0` (por defecto) los pines son configurables desde
// la web; los `SEMA_PIN_*` solo se usan como fallback cuando la config no define
// sensores.
// =============================================================================

// 0 = pines configurables desde la web (por defecto).
// 1 = pines fijos (catálogo de sensores NO tomado de la config web).
#ifndef SEMA_FIXED_HARDWARE
#define SEMA_FIXED_HARDWARE 0
#endif

// Pines por defecto del catálogo fijo.
#ifndef SEMA_PIN_I2C_SDA
#define SEMA_PIN_I2C_SDA 21
#endif

#ifndef SEMA_PIN_I2C_SCL
#define SEMA_PIN_I2C_SCL 22
#endif

#ifndef SEMA_PIN_ONEWIRE
#define SEMA_PIN_ONEWIRE 4
#endif

#ifndef SEMA_PIN_BATTERY_ADC
#define SEMA_PIN_BATTERY_ADC 34
#endif
