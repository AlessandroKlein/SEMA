#include "core/ShiftRegisterManager.hpp"

#include <Arduino.h>

#include "hw/HwProfile.hpp"

namespace sema {

void ShiftRegisterManager::apply(const ShiftRegisterConfig& cfg) {
  cfg_ = cfg;
  if (cfg_.latchPin == 0) {
    return;
  }
  // DAT (SER/QH) y CLK (SRCLK/CLK) son los pines SPI del bus (fijos).
  pinMode(SEMA_SPI_MOSI, cfg_.type == "74HC165" ? INPUT : OUTPUT);
  pinMode(SEMA_SPI_SCK, OUTPUT);
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
  shiftOut(SEMA_SPI_MOSI, SEMA_SPI_SCK, MSBFIRST, value);
  digitalWrite(cfg_.latchPin, HIGH);
}

uint8_t ShiftRegisterManager::readByte() {
  if (!configured() || isOutput()) {
    return 0;
  }
  digitalWrite(cfg_.latchPin, LOW);  // SH/LD: carga en paralelo
  delayMicroseconds(5);
  digitalWrite(cfg_.latchPin, HIGH);
  return shiftIn(SEMA_SPI_MOSI, SEMA_SPI_SCK, MSBFIRST);
}

}  // namespace sema
