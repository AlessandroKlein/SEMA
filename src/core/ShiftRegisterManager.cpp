#include "core/ShiftRegisterManager.hpp"

#include <Arduino.h>

namespace sema {

void ShiftRegisterManager::apply(const ShiftRegisterConfig& cfg) {
  cfg_ = cfg;
  if (cfg_.dataPin == 0) {
    return;
  }
  pinMode(cfg_.dataPin, cfg_.type == "74HC165" ? INPUT : OUTPUT);
  pinMode(cfg_.clockPin, OUTPUT);
  pinMode(cfg_.latchPin, OUTPUT);
  if (isOutput()) {
    digitalWrite(cfg_.latchPin, LOW);
  }
}

void ShiftRegisterManager::writeByte(uint8_t value) {
  if (!configured() || !isOutput()) {
    return;
  }
  digitalWrite(cfg_.latchPin, LOW);
  shiftOut(cfg_.dataPin, cfg_.clockPin, MSBFIRST, value);
  digitalWrite(cfg_.latchPin, HIGH);
}

uint8_t ShiftRegisterManager::readByte() {
  if (!configured() || isOutput()) {
    return 0;
  }
  digitalWrite(cfg_.latchPin, LOW);  // SH/LD: carga en paralelo
  delayMicroseconds(5);
  digitalWrite(cfg_.latchPin, HIGH);
  return shiftIn(cfg_.dataPin, cfg_.clockPin, MSBFIRST);
}

}  // namespace sema
