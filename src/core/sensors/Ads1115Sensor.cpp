#include "core/sensors/Ads1115Sensor.hpp"

#include <Adafruit_ADS1X15.h>
#include <Wire.h>

namespace sema {

static Adafruit_ADS1115 ads1115;

Ads1115Sensor::Ads1115Sensor(const char* id, uint8_t sda, uint8_t scl,
                             uint8_t channel, const char* name, const char* unit,
                             float scale, float offset)
    : id_(id), sda_(sda), scl_(scl), channel_(channel), name_(name), unit_(unit),
      scale_(scale), offset_(offset) {}

const char* Ads1115Sensor::id() const { return id_; }
const char* Ads1115Sensor::model() const { return "ADS1115"; }
const char* Ads1115Sensor::interface() const { return "I2C"; }

bool Ads1115Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = ads1115.begin();
  return ok_;
}

bool Ads1115Sensor::healthy() const { return ok_; }

uint8_t Ads1115Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  const int16_t raw = ads1115.readADC_SingleEnded(channel_);
  const float value = raw * scale_ + offset_;
  const uint32_t ts = nowEpoch();

  out[0].sensorId = id_;
  out[0].channelId = name_;
  out[0].measurement = name_;
  out[0].value = value;
  out[0].unit = unit_;
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
