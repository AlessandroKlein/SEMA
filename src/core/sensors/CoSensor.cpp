#include "core/sensors/CoSensor.hpp"

#include <Arduino.h>

namespace sema {

CoSensor::CoSensor(const char* id, uint8_t pin, float scale, float offset)
    : id_(id), pin_(pin), scale_(scale), offset_(offset) {}

const char* CoSensor::id() const { return id_; }
const char* CoSensor::model() const { return "CO"; }
const char* CoSensor::interface() const { return "ADC"; }

bool CoSensor::begin() {
  analogReadResolution(12);
  ok_ = true;
  return ok_;
}

bool CoSensor::healthy() const { return ok_; }

uint8_t CoSensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  const int raw = analogRead(pin_);
  const float value = raw * scale_ + offset_;
  const uint32_t ts = nowEpoch();

  out[0].sensorId = id_;
  out[0].channelId = "co";
  out[0].measurement = "co";
  out[0].value = value;
  out[0].unit = "ppm";
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
