#include "core/sensors/Scd30Sensor.hpp"

#include <Adafruit_SCD30.h>
#include <Wire.h>

namespace sema {

static Adafruit_SCD30 scd30;

Scd30Sensor::Scd30Sensor(const char* id, uint8_t sda, uint8_t scl)
    : id_(id), sda_(sda), scl_(scl) {}

const char* Scd30Sensor::id() const { return id_; }
const char* Scd30Sensor::model() const { return "SCD30"; }
const char* Scd30Sensor::interface() const { return "I2C"; }

bool Scd30Sensor::begin() {
  Wire.begin(sda_, scl_);
  ok_ = scd30.begin();
  return ok_;
}

bool Scd30Sensor::healthy() const { return ok_; }

uint8_t Scd30Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 3) {
    return 0;
  }

  // Actualiza las lecturas cuando hay datos nuevos (intervalo del SCD30: 2 s).
  if (scd30.dataReady()) {
    scd30.read();
  }

  const float co2 = scd30.CO2;
  const float t = scd30.temperature;
  const float h = scd30.relative_humidity;
  const uint32_t ts = millis() / 1000;

  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "co2";
  out[n].measurement = "co2";
  out[n].value = co2;
  out[n].unit = "ppm";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

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
