#pragma once

#include <Arduino.h>
#include <cstdint>

// =============================================================================
// SEMA — Modelo Canónico de Mediciones
// =============================================================================
// D-0044 / D-0007 / D-0026 / D-0028. Una única representación de la medición
// compartida por adquisición, storage, API y publishers (README.md §173).

namespace sema {

// Quality Flags (D-0056).
enum class Quality : uint8_t {
  Valid,
  Invalid,
  Stale,
  Timeout,
  OutOfRange,
  CalibrationError,
  CommunicationError,
  SensorDisconnected
};

// Medición canónica (D-0044): timestamp, sensor_id, channel_id, measurement,
// value, unit, quality, sequence y metadatos opcionales.
struct Measurement {
  String stationId;        // identidad de la estación (D-0028)
  String sensorId;         // id lógico del sensor (p. ej. "TEMP_EXT")
  String channelId;        // canal lógico (un sensor puede aportar varias magnitudes)
  String measurement;      // magnitud ("temperature", "humidity", "pressure", …)
  float value = 0.0f;
  String unit;             // unidad canónica ("degC", "percent", "hPa", …)
  Quality quality = Quality::Valid;
  uint32_t sequence = 0;   // secuencia monotónica por sensor
  uint32_t timestamp = 0;  // epoch seconds UTC
};

}  // namespace sema
