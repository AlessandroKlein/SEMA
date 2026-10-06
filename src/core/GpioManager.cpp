#include "core/GpioManager.hpp"

#include <Arduino.h>
#include <Wire.h>

namespace sema {

const GpioSpec* GpioManager::find(uint8_t pin) const {
  for (const GpioSpec& s : specs_) {
    if (s.pin == pin) {
      return &s;
    }
  }
  return nullptr;
}

void GpioManager::apply(const std::vector<GpioSpec>& specs) {
  specs_ = specs;
  mcpReady_ = false;
  mcpAddr_ = 0;

  // Detecta e inicializa un MCP23017 si algún pin lo usa (expanderAddr != 0).
  for (const GpioSpec& s : specs_) {
    if (s.expanderAddr != 0) {
      mcpAddr_ = s.expanderAddr;
      mcpReady_ = mcp_.begin_I2C(mcpAddr_);
      break;
    }
  }

  for (const GpioSpec& s : specs_) {
    uint8_t mode = INPUT;
    if (s.mode == "output") {
      mode = OUTPUT;
    } else if (s.mode == "input_pullup") {
      mode = INPUT_PULLUP;
    } else if (s.mode == "input_pulldown") {
      mode = INPUT_PULLDOWN;
    }

    if (s.expanderAddr != 0 && mcpReady_) {
      mcp_.pinMode(s.pin, mode);
      if (s.mode == "output") {
        mcp_.digitalWrite(s.pin, s.initial ? HIGH : LOW);
      }
    } else if (s.expanderAddr == 0) {
      pinMode(s.pin, mode);
      if (s.mode == "output") {
        digitalWrite(s.pin, s.initial ? HIGH : LOW);
      }
    }
  }
}

int GpioManager::read(uint8_t pin) const {
  const GpioSpec* s = find(pin);
  if (s != nullptr && s->expanderAddr != 0 && mcpReady_) {
    return mcp_.digitalRead(pin);
  }
  return digitalRead(pin);
}

void GpioManager::write(uint8_t pin, int value) {
  const GpioSpec* s = find(pin);
  if (s != nullptr && s->expanderAddr != 0 && mcpReady_) {
    mcp_.digitalWrite(pin, value ? HIGH : LOW);
  } else {
    digitalWrite(pin, value ? HIGH : LOW);
  }
}

}  // namespace sema
