#include "core/sensors/Bme280Sensor.hpp"

#include <Adafruit_BME280.h>
#include <Wire.h>

namespace sema {

static Adafruit_BME280 bme280;

Bme280Sensor::Bme280Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Bme280Sensor::id() const { return id_; }
const char* Bme280Sensor::model() const { return "BME280"; }
const char* Bme280Sensor::interface() const { return "I2C"; }

bool Bme280Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = bme280.begin(0x76) || bme280.begin(0x77);
  return ok_;
}

bool Bme280Sensor::healthy() const { return ok_; }

uint8_t Bme280Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 3) {
    return 0;
  }

  const float t = bme280.readTemperature();
  const float h = bme280.readHumidity();
  const float p = bme280.readPressure() / 100.0f;  // Pa → hPa
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

  out[n].sensorId = id_;
  out[n].channelId = "pressure";
  out[n].measurement = "pressure";
  out[n].value = p;
  out[n].unit = "hPa";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
