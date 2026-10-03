#pragma once

#include <map>
#include <vector>

#include "core/Calibration.hpp"
#include "core/Measurement.hpp"
#include "core/sensors/Sensor.hpp"

// =============================================================================
// SEMA — Sensor Manager (semilla del Sensor Engine)
// =============================================================================
// D-0007 / D-0029 / DESIGN-SYSTEM §105. Registra drivers y orquesta la lectura
// periódica produciendo el conjunto de mediciones canónicas.

namespace sema {

class SensorManager {
public:
  void registerSensor(Sensor* sensor);
  void beginAll();
  void readAll();

  // Calibración por canal ("sensorId:channelId"), D-0055.
  void setCalibration(const String& key, const Calibration& c);

  size_t count() const { return sensors_.size(); }
  size_t onlineCount() const;
  const std::vector<Measurement>& measurements() const { return measurements_; }

private:
  std::vector<Sensor*> sensors_;
  std::vector<Measurement> measurements_;
  std::map<String, Calibration> calibrations_;
};

}  // namespace sema
