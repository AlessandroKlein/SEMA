#include "core/sensors/Sht40Sensor.hpp"

#include <Adafruit_SHT4x.h>
#include <Wire.h>

namespace sema {

static Adafruit_SHT4x sht4;

Sht40Sensor::Sht40Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Sht40Sensor::id() const { return id_; }
const char* Sht40Sensor::model() const { return "SHT40"; }
const char* Sht40Sensor::interface() const { return "I2C"; }

bool Sht40Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = sht4.begin();
  return ok_;
}

bool Sht40Sensor::healthy() const { return ok_; }

uint8_t Sht40Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }

  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);

  const float t = temp.temperature;
  const float h = humidity.relative_humidity;
  // TODO(D-0044): sustituir por epoch UTC real vía NTP/RTC (README §127-129).
  const uint32_t ts = nowEpoch();

  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "temperature";
  out[n].measurement = "temperature";
  out[n].value = t;
  out[n].unit = "degC";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "humidity";
  out[n].measurement = "humidity";
  out[n].value = h;
  out[n].unit = "percent";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
