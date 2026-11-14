#include "core/sensors/Hdc1080Sensor.hpp"

#include <Wire.h>

namespace sema {

static const uint8_t kHdc1080Addr = 0x40;
static const uint8_t kHdc1080RegTemp = 0x00;
static const uint8_t kHdc1080RegHum = 0x01;
static const uint8_t kHdc1080ConvMs = 15;  // conversión máx. (14 bits) ≈ 6.5 ms

Hdc1080Sensor::Hdc1080Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Hdc1080Sensor::id() const { return id_; }
const char* Hdc1080Sensor::model() const { return "HDC1080"; }
const char* Hdc1080Sensor::interface() const { return "I2C"; }

bool Hdc1080Sensor::begin() {
  Wire.begin(sda_, scl_);
  // Sondeo de escritura para detectar el dispositivo (apunta a temperatura).
  Wire.beginTransmission(kHdc1080Addr);
  ok_ = (Wire.endTransmission() == 0);
  return ok_;
}

bool Hdc1080Sensor::healthy() const { return ok_; }

// Lee un registro de 16 bits big-endian. La escritura del puntero dispara la
// conversión; se espera el tiempo de conversión antes de leer el resultado.
static bool read16(uint8_t reg, uint16_t* out) {
  Wire.beginTransmission(kHdc1080Addr);
  Wire.write(reg);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  delay(kHdc1080ConvMs);
  if (Wire.requestFrom(kHdc1080Addr, static_cast<uint8_t>(2)) < 2) {
    return false;
  }
  const uint16_t hi = static_cast<uint16_t>(Wire.read());
  const uint16_t lo = static_cast<uint16_t>(Wire.read());
  *out = static_cast<uint16_t>((hi << 8) | lo);
  return true;
}

uint8_t Hdc1080Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 2) {
    return 0;
  }

  uint16_t tRaw = 0;
  uint16_t hRaw = 0;
  if (!read16(kHdc1080RegTemp, &tRaw) || !read16(kHdc1080RegHum, &hRaw)) {
    return 0;
  }

  const float t = (static_cast<float>(tRaw) / 65536.0f) * 165.0f - 40.0f;
  const float h = (static_cast<float>(hRaw) / 65536.0f) * 100.0f;
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
