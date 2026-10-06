#include "core/ModbusManager.hpp"

#include <Arduino.h>
#include <ModbusMaster.h>

namespace sema {

static ModbusMaster modbusNode;
static uint8_t g_deRePin = 0;

static void preTransmission() {
  if (g_deRePin != 0) {
    digitalWrite(g_deRePin, HIGH);
  }
}

static void postTransmission() {
  if (g_deRePin != 0) {
    digitalWrite(g_deRePin, LOW);
  }
}

void ModbusManager::apply(const ModbusConfig& cfg) {
  cfg_ = cfg;
  ready_ = false;
  values_.clear();
  if (!cfg_.enabled) {
    return;
  }

  Serial2.begin(cfg_.baud, SERIAL_8N1, cfg_.rxPin, cfg_.txPin);
  modbusNode.begin(cfg_.slaveId, Serial2);

  g_deRePin = cfg_.deRePin;
  if (cfg_.deRePin != 0) {
    pinMode(cfg_.deRePin, OUTPUT);
    digitalWrite(cfg_.deRePin, LOW);
    modbusNode.preTransmission(preTransmission);
    modbusNode.postTransmission(postTransmission);
  }
  ready_ = true;
}

uint8_t ModbusManager::read() {
  if (!ready_) {
    return 0xFF;
  }
  const uint8_t result =
      modbusNode.readHoldingRegisters(cfg_.registerAddr, cfg_.registerCount);
  if (result == modbusNode.ku8MBSuccess) {
    values_.clear();
    for (uint16_t i = 0; i < cfg_.registerCount; ++i) {
      values_.push_back(modbusNode.getResponseBuffer(i));
    }
  }
  return result;
}

}  // namespace sema
