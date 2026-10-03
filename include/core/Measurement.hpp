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

// Nombre del Quality Flag (D-0056), para serialización/API.
inline const char* qualityName(Quality q) {
  switch (q) {
    case Quality::Valid: return "VALID";
    case Quality::Invalid: return "INVALID";
    case Quality::Stale: return "STALE";
    case Quality::Timeout: return "TIMEOUT";
    case Quality::OutOfRange: return "OUT_OF_RANGE";
    case Quality::CalibrationError: return "CALIBRATION_ERROR";
    case Quality::CommunicationError: return "COMMUNICATION_ERROR";
    case Quality::SensorDisconnected: return "SENSOR_DISCONNECTED";
    default: return "UNKNOWN";
  }
}

// Convierte un nombre de Quality Flag (D-0056) al enum, para deserialización.
inline Quality parseQuality(const char* name) {
  if (name == nullptr) return Quality::Valid;
  if (strcmp(name, "INVALID") == 0) return Quality::Invalid;
  if (strcmp(name, "STALE") == 0) return Quality::Stale;
  if (strcmp(name, "TIMEOUT") == 0) return Quality::Timeout;
  if (strcmp(name, "OUT_OF_RANGE") == 0) return Quality::OutOfRange;
  if (strcmp(name, "CALIBRATION_ERROR") == 0) return Quality::CalibrationError;
  if (strcmp(name, "COMMUNICATION_ERROR") == 0) return Quality::CommunicationError;
  if (strcmp(name, "SENSOR_DISCONNECTED") == 0) return Quality::SensorDisconnected;
  return Quality::Valid;
}

}  // namespace sema
