#include "core/sensors/Bh1750Sensor.hpp"

#include <Wire.h>

namespace sema {

static const uint8_t kBh1750Addr = 0x23;
static const uint8_t kContHighRes = 0x10;  // modo continuo, alta resolución (1 lux)

Bh1750Sensor::Bh1750Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Bh1750Sensor::id() const { return id_; }
const char* Bh1750Sensor::model() const { return "BH1750"; }
const char* Bh1750Sensor::interface() const { return "I2C"; }

bool Bh1750Sensor::begin() {
  Wire.begin(sda_, scl_);
  Wire.beginTransmission(kBh1750Addr);
  Wire.write(kContHighRes);
  ok_ = (Wire.endTransmission() == 0);
  return ok_;
}

bool Bh1750Sensor::healthy() const { return ok_; }

uint8_t Bh1750Sensor::measure(Measurement out[], uint8_t max) {
  if (max < 1) {
    return 0;
  }

  out[0].sensorId = id_;
  out[0].channelId = "light";
  out[0].measurement = "light";
  out[0].unit = "lux";
  out[0].sequence = ++sequence_;
  out[0].timestamp = millis() / 1000;

  if (!ok_) {
    out[0].value = 0.0f;
    out[0].quality = Quality::SensorDisconnected;
    return 1;
  }

  Wire.requestFrom(kBh1750Addr, 2);
  if (Wire.available() < 2) {
    out[0].value = 0.0f;
    out[0].quality = Quality::CommunicationError;
    return 1;
  }

  const uint16_t raw = static_cast<uint16_t>((Wire.read() << 8) | Wire.read());
  out[0].value = raw / 1.2f;  // lux
  out[0].quality = Quality::Valid;
  return 1;
}

}  // namespace sema
