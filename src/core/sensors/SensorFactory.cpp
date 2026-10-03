#include "core/sensors/SensorFactory.hpp"

#include "core/sensors/AdcSensor.hpp"
#include "core/sensors/Aht20Sensor.hpp"
#include "core/sensors/Bh1750Sensor.hpp"
#include "core/sensors/Bme280Sensor.hpp"
#include "core/sensors/Bmp280Sensor.hpp"
#include "core/sensors/Ds18b20Sensor.hpp"
#include "core/sensors/PcntSensor.hpp"
#include "core/sensors/Pms5003Sensor.hpp"
#include "core/sensors/Scd30Sensor.hpp"
#include "core/sensors/Sht31Sensor.hpp"
#include "core/sensors/Sht40Sensor.hpp"
#include "core/sensors/Veml6075Sensor.hpp"

namespace sema {

Sensor* SensorFactory::create(const SensorSpec& spec) {
  // Los drivers retienen el puntero al id; el catálogo (config.sensors) vive
  // durante todo el runtime (se crea una vez al arrancar y no se reasigna).
  if (spec.model == "BME280") {
    return new Bme280Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "SHT40") {
    return new Sht40Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "SHT31") {
    return new Sht31Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "BMP280") {
    return new Bmp280Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "DS18B20") {
    return new Ds18b20Sensor(spec.id.c_str(), spec.pin);
  }
  if (spec.model == "BH1750") {
    return new Bh1750Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "AHT20") {
    return new Aht20Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "ADC") {
    return new AdcSensor(spec.id.c_str(), spec.pin, spec.channel.c_str(),
                         spec.unit.c_str(), spec.scale, spec.offset);
  }
  if (spec.model == "PCNT") {
    return new PcntSensor(spec.id.c_str(), spec.pin, spec.channel.c_str(),
                          spec.unit.c_str(), spec.scale);
  }
  if (spec.model == "VEML6075") {
    return new Veml6075Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "SCD30") {
    return new Scd30Sensor(spec.id.c_str(), spec.sda, spec.scl);
  }
  if (spec.model == "PMS5003") {
    return new Pms5003Sensor(spec.id.c_str(), spec.rxPin, spec.txPin);
  }
  return nullptr;
}

}  // namespace sema
