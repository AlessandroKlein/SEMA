#include "core/sensors/Sht31Sensor.hpp"

#include <Adafruit_SHT31.h>
#include <Wire.h>

namespace sema {

static Adafruit_SHT31 sht31;

Sht31Sensor::Sht31Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Sht31Sensor::id() const { return id_; }
const char* Sht31Sensor::model() const { return "SHT31"; }
const char* Sht31Sensor::interface() const { return "I2C"; }

bool Sht31Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = sht31.begin(0x44);
  return ok_;
}

bool Sht31Sensor::healthy() const { return ok_; }

uint8_t Sht31Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }

  const float t = sht31.readTemperature();
  const float h = sht31.readHumidity();
  const uint32_t ts = millis() / 1000;

  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "temperature";
  out[n].measurement = "temperature";
  out[n].value = t;
  out[n].unit = "degC";
  out[n].quality = isnan(t) ? Quality::CommunicationError : Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "humidity";
  out[n].measurement = "humidity";
  out[n].value = h;
  out[n].unit = "percent";
  out[n].quality = isnan(h) ? Quality::CommunicationError : Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
