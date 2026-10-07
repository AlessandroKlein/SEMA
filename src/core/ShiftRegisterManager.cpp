#include "core/ShiftRegisterManager.hpp"

#include <Arduino.h>

#include "hw/HwProfile.hpp"

namespace sema {

void ShiftRegisterManager::apply(const std::vector<ShiftRegisterConfig>& cfg) {
  cfg_ = cfg;
  for (const ShiftRegisterConfig& c : cfg_) {
    if (c.latchPin == 0) {
      continue;
    }
    pinMode(SEMA_SPI_MOSI, c.type == "74HC165" ? INPUT : OUTPUT);
    pinMode(SEMA_SPI_SCK, OUTPUT);
    pinMode(c.latchPin, OUTPUT);
  }
}

bool ShiftRegisterManager::hasOutput() const {
  for (const ShiftRegisterConfig& c : cfg_) {
    if (c.type != "74HC165" && c.latchPin != 0) return true;
  }
  return false;
}

bool ShiftRegisterManager::hasInput() const {
  for (const ShiftRegisterConfig& c : cfg_) {
    if (c.type == "74HC165" && c.latchPin != 0) return true;
  }
  return false;
}

void ShiftRegisterManager::writeByte(uint8_t value) {
  for (const ShiftRegisterConfig& c : cfg_) {
    if (c.type == "74HC165" || c.latchPin == 0) {
      continue;
    }
    digitalWrite(c.latchPin, LOW);
    shiftOut(SEMA_SPI_MOSI, SEMA_SPI_SCK, MSBFIRST, value);
    digitalWrite(c.latchPin, HIGH);
  }
}

uint8_t ShiftRegisterManager::readByte() {
  for (const ShiftRegisterConfig& c : cfg_) {
    if (c.type != "74HC165" || c.latchPin == 0) {
      continue;
    }
    digitalWrite(c.latchPin, LOW);  // SH/LD: carga en paralelo
    delayMicroseconds(5);
    digitalWrite(c.latchPin, HIGH);
    return shiftIn(SEMA_SPI_MOSI, SEMA_SPI_SCK, MSBFIRST);
  }
  return 0;
}

}  // namespace sema
