#include "core/sensors/PcntSensor.hpp"

#include <Arduino.h>
#include <driver/pcnt.h>

namespace sema {

PcntSensor::PcntSensor(const char* id, uint8_t pin, const char* channel,
                       const char* unit, float scale)
    : id_(id), pin_(pin), channel_(channel), unit_(unit), scale_(scale) {}

const char* PcntSensor::id() const { return id_; }
const char* PcntSensor::model() const { return "PCNT"; }
const char* PcntSensor::interface() const { return "GPIO"; }

bool PcntSensor::begin() {
  // Contador por flanco ascendente (legacy API de ESP-IDF 4.x).
  pcnt_config_t cfg = {};
  cfg.pulse_gpio_num = pin_;
  cfg.ctrl_gpio_num = PCNT_PIN_NOT_USED;
  cfg.lctrl_mode = PCNT_MODE_KEEP;
  cfg.hctrl_mode = PCNT_MODE_KEEP;
  cfg.pos_mode = PCNT_COUNT_INC;
  cfg.neg_mode = PCNT_COUNT_DIS;
  cfg.counter_h_lim = 32767;
  cfg.counter_l_lim = 0;
  cfg.unit = PCNT_UNIT_0;
  cfg.channel = PCNT_CHANNEL_0;

  ok_ = (pcnt_unit_config(&cfg) == ESP_OK);
  if (ok_) {
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    pcnt_counter_resume(PCNT_UNIT_0);
  }
  return ok_;
}

bool PcntSensor::healthy() const { return ok_; }

uint8_t PcntSensor::measure(Measurement out[], uint8_t max) {
  if (!ok_ || max < 1) {
    return 0;
  }

  int16_t count = 0;
  pcnt_get_counter_value(PCNT_UNIT_0, &count);
  pcnt_counter_clear(PCNT_UNIT_0);

  const float value = count * scale_;
  const uint32_t ts = millis() / 1000;

  out[0].sensorId = id_;
  out[0].channelId = channel_;
  out[0].measurement = channel_;
  out[0].value = value;
  out[0].unit = unit_;
  out[0].quality = Quality::Valid;
  out[0].sequence = ++sequence_;
  out[0].timestamp = ts;
  return 1;
}

}  // namespace sema
