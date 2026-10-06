#include "core/GpioManager.hpp"

#include <Arduino.h>

namespace sema {

void GpioManager::apply(const std::vector<GpioSpec>& specs) {
  specs_ = specs;
  for (const GpioSpec& s : specs_) {
    uint8_t mode = INPUT;
    if (s.mode == "output") {
      mode = OUTPUT;
    } else if (s.mode == "input_pullup") {
      mode = INPUT_PULLUP;
    } else if (s.mode == "input_pulldown") {
      mode = INPUT_PULLDOWN;
    }
    pinMode(s.pin, mode);
    if (s.mode == "output") {
      digitalWrite(s.pin, s.initial ? HIGH : LOW);
    }
  }
}

int GpioManager::read(uint8_t pin) const {
  return digitalRead(pin);
}

void GpioManager::write(uint8_t pin, int value) {
  digitalWrite(pin, value ? HIGH : LOW);
}

}  // namespace sema
