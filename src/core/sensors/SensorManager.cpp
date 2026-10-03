#include "core/sensors/SensorManager.hpp"

#include "core/derived/DerivedEngine.hpp"

namespace sema {

void SensorManager::registerSensor(Sensor* sensor) {
  sensors_.push_back(sensor);
}

void SensorManager::beginAll() {
  for (Sensor* sensor : sensors_) {
    sensor->begin();
  }
}

void SensorManager::setCalibration(const String& key, const Calibration& c) {
  calibrations_[key] = c;
}

void SensorManager::readAll() {
  measurements_.clear();

  Measurement buffer[4];
  for (Sensor* sensor : sensors_) {
    if (!sensor->healthy()) {
      continue;
    }
    const uint8_t n = sensor->measure(buffer, 4);
    for (uint8_t i = 0; i < n; ++i) {
      const String key = buffer[i].sensorId + ":" + buffer[i].channelId;
      auto it = calibrations_.find(key);
      if (it != calibrations_.end()) {
        applyCalibration(buffer[i], it->second);
      }
      measurements_.push_back(buffer[i]);
    }
  }

  // Procesamiento derivado (D-0044 §83): punto de rocío, índice de calor, …
  DerivedEngine::compute(measurements_);
}

size_t SensorManager::onlineCount() const {
  size_t n = 0;
  for (const Sensor* sensor : sensors_) {
    if (sensor->healthy()) {
      ++n;
    }
  }
  return n;
}

}  // namespace sema
