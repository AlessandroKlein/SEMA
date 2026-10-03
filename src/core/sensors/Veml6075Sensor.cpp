#include "core/sensors/Veml6075Sensor.hpp"

#include <Adafruit_VEML6075.h>
#include <Wire.h>

namespace sema {

static Adafruit_VEML6075 uv;

Veml6075Sensor::Veml6075Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Veml6075Sensor::id() const { return id_; }
const char* Veml6075Sensor::model() const { return "VEML6075"; }
const char* Veml6075Sensor::interface() const { return "I2C"; }

bool Veml6075Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = uv.begin();
  return ok_;
}

bool Veml6075Sensor::healthy() const { return ok_; }

uint8_t Veml6075Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 3) {
    return 0;
  }

  const float uva = uv.readUVA();
  const float uvb = uv.readUVB();
  const float uvi = uv.readUVI();
  const uint32_t ts = millis() / 1000;

  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "uva";
  out[n].measurement = "uva";
  out[n].value = uva;
  out[n].unit = "W/m2";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "uvb";
  out[n].measurement = "uvb";
  out[n].value = uvb;
  out[n].unit = "W/m2";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "uvi";
  out[n].measurement = "uvi";
  out[n].value = uvi;
  out[n].unit = "index";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
