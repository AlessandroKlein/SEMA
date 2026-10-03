#include "core/sensors/I2cScanner.hpp"

#include <Wire.h>

namespace sema {

size_t I2cScanner::scan(std::vector<DetectedDevice>& out) {
  out.clear();
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      DetectedDevice d;
      d.address = address;
      d.model = modelForAddress(address);
      out.push_back(d);
    }
  }
  return out.size();
}

String I2cScanner::modelForAddress(uint8_t address) {
  switch (address) {
    case 0x23: return "BH1750";
    case 0x38:
    case 0x39: return "AHT20";
    case 0x40: return "SHT31/HTU21D";
    case 0x44:
    case 0x45: return "SHT40/SHT3x";
    case 0x5C: return "AM2320";
    case 0x61: return "SCD30";
    case 0x62: return "SCD40/SCD41";
    case 0x68: return "MPU6050/DS3231";
    case 0x76:
    case 0x77: return "BME280/BMP280";
    default: return "";
  }
}

}  // namespace sema
