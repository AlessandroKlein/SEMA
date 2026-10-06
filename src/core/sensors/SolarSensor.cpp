#include "core/sensors/SolarSensor.hpp"

#include <Arduino.h>

namespace sema {

SolarSensor::SolarSensor(const char* id, uint8_t pin, float scale, float offset)
    : id_(id), pin_(pin), scale_(scale), offset_(offset) {}

const char* SolarSensor::id() const { return id_; }
const char* SolarSensor::model() const { return "SOLAR"; }
const char* SolarSensor::interface() const { return "ADC"; }

bool SolarSensor::begin() {
  analogReadResolution(12);
  ok_ = true;
  return ok_;
}

bool SolarSensor::healthy() const { return ok_; }

uint8_t SolarSensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  const int raw = analogRead(pin_);
  const float value = raw * scale_ + offset_;
  const uint32_t ts = nowEpoch();

  out[0].sensorId = id_;
  out[0].channelId = "solar_radiation";
  out[0].measurement = "solar_radiation";
  out[0].value = value;
  out[0].unit = "W/m2";
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
