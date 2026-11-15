#pragma once

// =============================================================================
// SEMA — Board Profile (D-0050) — pines por defecto del catálogo fijo
// =============================================================================
// Define los pines por defecto (`SEMA_PIN_*`) usados por SemaGlue para el
// catálogo de sensores por defecto (BME280/SHT40/DS18B20/BH1750/AHT20/ADC).
// Los pines configurables por web (variante modular) y el catálogo fijo de la
// variante PCB (`SEMA_PINS_FROM_FILE=1`) los gestiona `include/hw/HwProfile.hpp`.
// =============================================================================

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
