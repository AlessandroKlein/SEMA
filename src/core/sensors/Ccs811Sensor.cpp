#include "core/sensors/Ccs811Sensor.hpp"

#include <Adafruit_CCS811.h>
#include <Wire.h>

namespace sema {

static const uint8_t kCcs811DefaultAddr = 0x5A;

static Adafruit_CCS811 ccs811;

Ccs811Sensor::Ccs811Sensor(const char* id, uint8_t sda, uint8_t scl,
                           uint8_t address)
    : id_(id), sda_(sda), scl_(scl), address_(address) {}

const char* Ccs811Sensor::id() const { return id_; }
const char* Ccs811Sensor::model() const { return "CCS811"; }
const char* Ccs811Sensor::interface() const { return "I2C"; }

bool Ccs811Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = ccs811.begin(address_ ? address_ : kCcs811DefaultAddr);
  return ok_;
}

bool Ccs811Sensor::healthy() const { return ok_; }

uint8_t Ccs811Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }
  if (!ccs811.available() || !ccs811.readData()) {
    return 0;
  }

  const uint32_t ts = nowEpoch();
  uint8_t n = 0;

  // eCO₂ (CO₂ equivalente): devuelve 0 mientras el sensor calienta (~20 min).
  out[n].sensorId = id_;
  out[n].channelId = "eco2";
  out[n].measurement = "eco2";
  out[n].value = ccs811.geteCO2();
  out[n].unit = "ppm";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "tvoc";
  out[n].measurement = "tvoc";
  out[n].value = ccs811.getTVOC();
  out[n].unit = "ppb";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
