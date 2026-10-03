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
  // TODO(README §12): soportar varios DS18B20 por bus (ROM 28-XXXXXXXXXXXX).
  ok_ = ds_->getDeviceCount() > 0;
  return ok_;
}

bool Ds18b20Sensor::healthy() const { return ok_; }

uint8_t Ds18b20Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  ds_->requestTemperatures();  // ~750 ms en resolución por defecto
  const float t = ds_->getTempCByIndex(0);
  // TODO(D-0044): sustituir por epoch UTC real vía NTP/RTC.
  const uint32_t ts = millis() / 1000;

  out[0].sensorId = id_;
  out[0].channelId = "temperature";
  out[0].measurement = "temperature";
  out[0].value = t;
  out[0].unit = "degC";
  out[0].quality = (t == DEVICE_DISCONNECTED_C) ? Quality::SensorDisconnected
                                                : Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
