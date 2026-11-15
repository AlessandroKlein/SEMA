#include "core/sensors/SensorFactory.hpp"

#include "core/sensors/SensorFlags.hpp"

#if SEMA_USE_SENSOR_ADC
#include "core/sensors/AdcSensor.hpp"
#endif
#if SEMA_USE_SENSOR_ADS1115
#include "core/sensors/Ads1115Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_AHT20
#include "core/sensors/Aht20Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_AS3935
#include "core/sensors/As3935Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_BH1750
#include "core/sensors/Bh1750Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_BME280
#include "core/sensors/Bme280Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_BMP280
#include "core/sensors/Bmp280Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_CCS811
#include "core/sensors/Ccs811Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_CO
#include "core/sensors/CoSensor.hpp"
#endif
#if SEMA_USE_SENSOR_DS18B20
#include "core/sensors/Ds18b20Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_HDC1080
#include "core/sensors/Hdc1080Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_PCNT
#include "core/sensors/PcntSensor.hpp"
#endif
#if SEMA_USE_SENSOR_PMS5003
#include "core/sensors/Pms5003Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_S12SD
#include "core/sensors/S12sdSensor.hpp"
#endif
#if SEMA_USE_SENSOR_SCD30
#include "core/sensors/Scd30Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_SGP30
#include "core/sensors/Sgp30Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_SHT31
#include "core/sensors/Sht31Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_SHT40
#include "core/sensors/Sht40Sensor.hpp"
#endif
#if SEMA_USE_SENSOR_SOLAR
#include "core/sensors/SolarSensor.hpp"
#endif
#if SEMA_USE_SENSOR_VEML6075
#include "core/sensors/Veml6075Sensor.hpp"
#endif

namespace sema {

base::Sensor* SensorFactory::create(const base::SensorSpec& spec) {
  // Los drivers retienen el puntero al id; el catálogo (config.sensors) vive
  // durante todo el runtime (se crea una vez al arrancar y no se reasigna).
#if SEMA_USE_SENSOR_BME280
  if (spec.model == "BME280") {
    return new Bme280Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_SHT40
  if (spec.model == "SHT40") {
    return new Sht40Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_SHT31
  if (spec.model == "SHT31") {
    return new Sht31Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_BMP280
  if (spec.model == "BMP280") {
    return new Bmp280Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_DS18B20
  if (spec.model == "DS18B20") {
    return new Ds18b20Sensor(spec.id.c_str(), spec.pin, spec.rom.c_str());
  }
#endif
#if SEMA_USE_SENSOR_BH1750
  if (spec.model == "BH1750") {
    return new Bh1750Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_AHT20
  if (spec.model == "AHT20") {
    return new Aht20Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "AHT10") {
    return new Aht20Sensor(spec.id.c_str(), spec.sda, spec.scl, "AHT10");
  }
#endif
#if SEMA_USE_SENSOR_ADC
  if (spec.model == "ADC") {
    return new AdcSensor(spec.id.c_str(), spec.pin, spec.channel.c_str(),
                         spec.unit.c_str(), spec.scale, spec.offset);
  }
#endif
#if SEMA_USE_SENSOR_PCNT
  if (spec.model == "PCNT") {
    return new PcntSensor(spec.id.c_str(), spec.pin, spec.channel.c_str(),
                          spec.unit.c_str(), spec.scale);
  }
#endif
#if SEMA_USE_SENSOR_VEML6075
  if (spec.model == "VEML6075") {
    return new Veml6075Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_SCD30
  if (spec.model == "SCD30") {
    return new Scd30Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_SGP30
  if (spec.model == "SGP30") {
    return new Sgp30Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_PMS5003
  if (spec.model == "PMS5003") {
    return new Pms5003Sensor(spec.id.c_str(), spec.rxPin, spec.txPin);
  }
#endif
#if SEMA_USE_SENSOR_AS3935
  if (spec.model == "AS3935") {
    return new As3935Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
#if SEMA_USE_SENSOR_ADS1115
  if (spec.model == "ADS1115") {
    return new Ads1115Sensor(spec.id.c_str(), spec.sda, spec.scl, spec.pin,
                             spec.channel.c_str(), spec.unit.c_str(),
                             spec.scale, spec.offset);
  }
#endif
#if SEMA_USE_SENSOR_CO
  if (spec.model == "CO") {
    return new CoSensor(spec.id.c_str(), spec.pin, spec.scale, spec.offset);
  }
#endif
#if SEMA_USE_SENSOR_SOLAR
  if (spec.model == "SOLAR") {
    return new SolarSensor(spec.id.c_str(), spec.pin, spec.scale, spec.offset);
  }
#endif
#if SEMA_USE_SENSOR_S12SD
  if (spec.model == "S12SD") {
    return new S12sdSensor(spec.id.c_str(), spec.pin, spec.scale, spec.offset);
  }
#endif
#if SEMA_USE_SENSOR_CCS811
  if (spec.model == "CCS811") {
    return new Ccs811Sensor(spec.id.c_str(), spec.sda, spec.scl, spec.address);
  }
#endif
#if SEMA_USE_SENSOR_HDC1080
  if (spec.model == "HDC1080") {
    return new Hdc1080Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
#endif
  return nullptr;
}

}  // namespace sema
