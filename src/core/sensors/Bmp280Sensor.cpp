#include "core/sensors/Bmp280Sensor.hpp"

#include <Adafruit_BMP280.h>
#include <Wire.h>

namespace sema {

static Adafruit_BMP280 bmp;

Bmp280Sensor::Bmp280Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Bmp280Sensor::id() const { return id_; }
const char* Bmp280Sensor::model() const { return "BMP280"; }
const char* Bmp280Sensor::interface() const { return "I2C"; }

bool Bmp280Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = bmp.begin(0x76);
  return ok_;
}

bool Bmp280Sensor::healthy() const { return ok_; }

uint8_t Bmp280Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }

  const float t = bmp.readTemperature();
  const float p = bmp.readPressure() / 100.0f;  // Pa → hPa
  const uint32_t ts = nowEpoch();

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
  out[n].channelId = "pressure";
  out[n].measurement = "pressure";
  out[n].value = p;
  out[n].unit = "hPa";
  out[n].quality = isnan(p) ? Quality::CommunicationError : Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
