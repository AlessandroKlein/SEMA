#include "core/sensors/Ds18b20Sensor.hpp"

#include <DallasTemperature.h>
#include <OneWire.h>

namespace sema {

Ds18b20Sensor::Ds18b20Sensor(const char* id, uint8_t pin) : id_(id), pin_(pin) {}

Ds18b20Sensor::~Ds18b20Sensor() {
  delete ds_;
  delete oneWire_;
}

const char* Ds18b20Sensor::id() const { return id_; }
const char* Ds18b20Sensor::model() const { return "DS18B20"; }
const char* Ds18b20Sensor::interface() const { return "1-Wire"; }

bool Ds18b20Sensor::begin() {
  oneWire_ = new OneWire(pin_);
  ds_ = new DallasTemperature(oneWire_);
  ds_->begin();
  // Auto-detección (README §12): lee todos los DS18B20 presentes en el bus.
  ok_ = ds_->getDeviceCount() > 0;
  return ok_;
}

bool Ds18b20Sensor::healthy() const { return ok_; }

uint8_t Ds18b20Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  ds_->requestTemperatures();  // ~750 ms en resolución por defecto
  const uint8_t count = ds_->getDeviceCount();
  const uint8_t n = count < max ? count : max;
  // TODO(D-0044): sustituir por epoch UTC real vía NTP/RTC.
  const uint32_t ts = nowEpoch();

  for (uint8_t i = 0; i < n; ++i) {
    const float t = ds_->getTempCByIndex(i);
    out[i].sensorId = (i == 0) ? id_ : (String(id_) + "_" + String((int)i));
    out[i].channelId = "temperature";
    out[i].measurement = "temperature";
    out[i].value = t;
    out[i].unit = "degC";
    out[i].quality = (t == DEVICE_DISCONNECTED_C) ? Quality::SensorDisconnected
                                                  : Quality::Valid;
    out[i].sequence = ++sequence_;
    out[i].timestamp = ts;
  }
  return n;
}

}  // namespace sema
