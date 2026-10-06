#include "core/sensors/As3935Sensor.hpp"

#include <SparkFun_AS3935.h>
#include <Wire.h>

namespace sema {

static SparkFun_AS3935 lightning(0x03);  // dirección I²C por defecto

As3935Sensor::As3935Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* As3935Sensor::id() const { return id_; }
const char* As3935Sensor::model() const { return "AS3935"; }
const char* As3935Sensor::interface() const { return "I2C"; }

bool As3935Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = lightning.begin();
  if (ok_) {
    lightning.setIndoorOutdoor(OUTDOOR);  // exterior por defecto
  }
  return ok_;
}

bool As3935Sensor::healthy() const { return ok_; }

uint8_t As3935Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }
  const uint8_t irq = lightning.readInterruptReg();
  if (!(irq & 0x08)) {  // bit LIGHTNING del registro de interrupción
    return 0;
  }

  const uint32_t ts = nowEpoch();
  out[0].sensorId = id_;
  out[0].channelId = "distance";
  out[0].measurement = "lightning_distance";
  out[0].value = lightning.distanceToStorm();
  out[0].unit = "km";
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
