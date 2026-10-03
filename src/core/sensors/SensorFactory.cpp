#include "core/sensors/SensorFactory.hpp"

#include "core/sensors/AdcSensor.hpp"
#include "core/sensors/Aht20Sensor.hpp"
#include "core/sensors/Bh1750Sensor.hpp"
#include "core/sensors/Bme280Sensor.hpp"
#include "core/sensors/Ds18b20Sensor.hpp"
#include "core/sensors/Sht40Sensor.hpp"

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
  return nullptr;
}

}  // namespace sema
