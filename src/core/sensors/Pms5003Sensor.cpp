#include "core/sensors/Pms5003Sensor.hpp"

#include <Adafruit_PM25AQI.h>
#include <HardwareSerial.h>

namespace sema {

static Adafruit_PM25AQI aqi;
static PM25_AQI_Data data;

Pms5003Sensor::Pms5003Sensor(const char* id, uint8_t rxPin, uint8_t txPin)
    : id_(id), rx_(rxPin), tx_(txPin) {}

const char* Pms5003Sensor::id() const { return id_; }
const char* Pms5003Sensor::model() const { return "PMS5003"; }
const char* Pms5003Sensor::interface() const { return "UART"; }

bool Pms5003Sensor::begin() {
  // PMS5003: UART 9600 8N1 (Serial2).
  Serial2.begin(9600, SERIAL_8N1, rx_, tx_);
  ok_ = aqi.begin_UART(&Serial2);
  return ok_;
}

bool Pms5003Sensor::healthy() const { return ok_; }

uint8_t Pms5003Sensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 3) {
    return 0;
  }
  if (!aqi.read(&data)) {
    return 0;  // sin trama válida este ciclo
  }

  const uint32_t ts = millis() / 1000;
  uint8_t n = 0;

  out[n].sensorId = id_;
  out[n].channelId = "pm1";
  out[n].measurement = "pm1";
  out[n].value = data.pm10_env;
  out[n].unit = "ug/m3";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "pm25";
  out[n].measurement = "pm25";
  out[n].value = data.pm25_env;
  out[n].unit = "ug/m3";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  out[n].sensorId = id_;
  out[n].channelId = "pm10";
  out[n].measurement = "pm10";
  out[n].value = data.pm100_env;
  out[n].unit = "ug/m3";
  out[n].quality = Quality::Valid;
  out[n].sequence = ++sequence_;
  out[n].timestamp = ts;
  ++n;

  return n;
}

}  // namespace sema
