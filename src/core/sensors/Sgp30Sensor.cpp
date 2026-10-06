#include "core/sensors/Sgp30Sensor.hpp"

#include <Adafruit_SGP30.h>
#include <Wire.h>

namespace sema {

static Adafruit_SGP30 sgp30;

Sgp30Sensor::Sgp30Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Sgp30Sensor::id() const { return id_; }
const char* Sgp30Sensor::model() const { return "SGP30"; }
const char* Sgp30Sensor::interface() const { return "I2C"; }

bool Sgp30Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = sgp30.begin();
  return ok_;
}

bool Sgp30Sensor::healthy() const { return ok_; }

uint8_t Sgp30Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }
  if (!sgp30.IAQmeasure()) {
    return 0;
  }

  const uint32_t ts = nowEpoch();
  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "eco2";
  out[n].measurement = "eco2";
  out[n].value = sgp30.eCO2;
  out[n].unit = "ppm";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "tvoc";
  out[n].measurement = "tvoc";
  out[n].value = sgp30.TVOC;
  out[n].unit = "ppb";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
