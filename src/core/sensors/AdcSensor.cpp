#include "core/sensors/AdcSensor.hpp"

#include <Arduino.h>

namespace sema {

AdcSensor::AdcSensor(const char* id, uint8_t pin, const char* channel,
                     const char* unit, float scale, float offset)
    : id_(id), pin_(pin), channel_(channel), unit_(unit), scale_(scale), offset_(offset) {}

const char* AdcSensor::id() const { return id_; }
const char* AdcSensor::model() const { return "ESP32-ADC"; }
const char* AdcSensor::interface() const { return "ADC"; }

bool AdcSensor::begin() {
  analogReadResolution(12);
  ok_ = true;
  return ok_;
}

bool AdcSensor::healthy() const { return ok_; }

uint8_t AdcSensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  const int raw = analogRead(pin_);
  const float value = raw * scale_ + offset_;
  const uint32_t ts = nowEpoch();

  out[0].sensorId = id_;
  out[0].channelId = channel_;
  out[0].measurement = channel_;
  out[0].value = value;
  out[0].unit = unit_;
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
