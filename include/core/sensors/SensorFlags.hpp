#pragma once

// =============================================================================
// SEMA — Flags de compilación por sensor
// =============================================================================
// Un flag por modelo. Con `-D SEMA_USE_SENSOR_<MODELO>=0` ese driver se excluye
// del SensorFactory (no se enlaza) y el firmware pesa menos. Por defecto todos
// ENCENDIDOS (= 1), espejando el patrón de include/hw/HwProfile.hpp.
//
//   platformio.ini build_flags:
//     -D SEMA_USE_SENSOR_SCD30=0    → SCD30 no se compila
//
// Los drivers NO usan estos flags internamente: solo se excluyen del factory.
// El ahorro real viene del linker (`--gc-sections`, activo en ESP32 Arduino),
// que descarta el driver si nadie lo referencia.

#ifndef SEMA_USE_SENSOR_BME280
#define SEMA_USE_SENSOR_BME280 1
#endif

#ifndef SEMA_USE_SENSOR_SHT40
#define SEMA_USE_SENSOR_SHT40 1
#endif

#ifndef SEMA_USE_SENSOR_SHT31
#define SEMA_USE_SENSOR_SHT31 1
#endif

#ifndef SEMA_USE_SENSOR_BMP280
#define SEMA_USE_SENSOR_BMP280 1
#endif

#ifndef SEMA_USE_SENSOR_DS18B20
#define SEMA_USE_SENSOR_DS18B20 1
#endif

#ifndef SEMA_USE_SENSOR_BH1750
#define SEMA_USE_SENSOR_BH1750 1
#endif

#ifndef SEMA_USE_SENSOR_AHT20
#define SEMA_USE_SENSOR_AHT20 1
#endif

#ifndef SEMA_USE_SENSOR_ADC
#define SEMA_USE_SENSOR_ADC 1
#endif

#ifndef SEMA_USE_SENSOR_PCNT
#define SEMA_USE_SENSOR_PCNT 1
#endif

#ifndef SEMA_USE_SENSOR_VEML6075
#define SEMA_USE_SENSOR_VEML6075 1
#endif

#ifndef SEMA_USE_SENSOR_SCD30
#define SEMA_USE_SENSOR_SCD30 1
#endif

#ifndef SEMA_USE_SENSOR_SGP30
#define SEMA_USE_SENSOR_SGP30 1
#endif

#ifndef SEMA_USE_SENSOR_PMS5003
#define SEMA_USE_SENSOR_PMS5003 1
#endif

#ifndef SEMA_USE_SENSOR_AS3935
#define SEMA_USE_SENSOR_AS3935 1
#endif

#ifndef SEMA_USE_SENSOR_ADS1115
#define SEMA_USE_SENSOR_ADS1115 1
#endif

#ifndef SEMA_USE_SENSOR_CO
#define SEMA_USE_SENSOR_CO 1
#endif

#ifndef SEMA_USE_SENSOR_SOLAR
#define SEMA_USE_SENSOR_SOLAR 1
#endif

#ifndef SEMA_USE_SENSOR_S12SD
#define SEMA_USE_SENSOR_S12SD 1
#endif

#ifndef SEMA_USE_SENSOR_CCS811
#define SEMA_USE_SENSOR_CCS811 1
#endif

#ifndef SEMA_USE_SENSOR_HDC1080
#define SEMA_USE_SENSOR_HDC1080 1
#endif
