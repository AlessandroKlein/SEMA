#include "core/sensors/Aht20Sensor.hpp"

#include <Adafruit_AHTX0.h>
#include <Wire.h>

namespace sema {

static Adafruit_AHTX0 aht;

Aht20Sensor::Aht20Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Aht20Sensor::id() const { return id_; }
const char* Aht20Sensor::model() const { return "AHT20"; }
const char* Aht20Sensor::interface() const { return "I2C"; }

bool Aht20Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = aht.begin();
  return ok_;
}

bool Aht20Sensor::healthy() const { return ok_; }

uint8_t Aht20Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  const float t = temp.temperature;
  const float h = humidity.relative_humidity;
  const uint32_t ts = millis() / 1000;

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
