#pragma once

#include <cstdint>
#include <vector>

#include "core/ConfigManager.hpp"

// =============================================================================
// SEMA — Maestro Modbus RTU sobre RS485
// =============================================================================
// Lee registros holding de un esclavo (transceiver RS485, control DE/RE opcional).

namespace sema {

class ModbusManager {
public:
  void apply(const ModbusConfig& cfg);
  uint8_t read();  // lee los registros holding; 0x00 = éxito (ku8MBSuccess)

  bool ready() const { return ready_; }
  const ModbusConfig& config() const { return cfg_; }
  const std::vector<uint16_t>& values() const { return values_; }

private:
  ModbusConfig cfg_;
  bool ready_ = false;
  std::vector<uint16_t> values_;
};

}  // namespace sema
