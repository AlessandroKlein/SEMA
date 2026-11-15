#include "core/sensors/S12sdSensor.hpp"

#include <Arduino.h>

namespace sema {

S12sdSensor::S12sdSensor(const char* id, uint8_t pin, float scale, float offset)
    : id_(id), pin_(pin), scale_(scale), offset_(offset) {}

const char* S12sdSensor::id() const { return id_; }
const char* S12sdSensor::model() const { return "S12SD"; }
const char* S12sdSensor::interface() const { return "ADC"; }

bool S12sdSensor::begin() {
  analogReadResolution(12);
  ok_ = true;
  return ok_;
}

bool S12sdSensor::healthy() const { return ok_; }

uint8_t S12sdSensor::measure(base::Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  const int raw = analogRead(pin_);
  const float value = raw * scale_ + offset_;
  const uint32_t ts = base::nowEpoch();

  out[0].sensorId = id_;
  out[0].channelId = "uvi";
  out[0].measurement = "uvi";
  out[0].value = value;
  out[0].unit = "index";
  out[0].quality = base::Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
