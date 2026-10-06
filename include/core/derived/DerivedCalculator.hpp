#pragma once

#include <vector>

#include "core/ConfigManager.hpp"
#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Calculador de magnitudes derivadas + conversión de unidades
// =============================================================================
// Calcula magnitudes derivadas (punto de rocío, índice de calor, sensación
// térmica, QNH, VPD, AQI, altitud barométrica, tasa de lluvia, dirección de
// viento) a partir de las mediciones crudas. Sólo calcula lo que tiene los
// sensores de origen encendidos.

namespace sema {

class DerivedCalculator {
public:
  void configure(const SystemConfig& sys) { sys_ = sys; }

  // Añade a `out` las magnitudes derivadas calculables, convertidas a `units`.
  void compute(const std::vector<Measurement>& raw, std::vector<Measurement>& out,
               const String& units);

  // Conversión de unidades (canónica métrica → imperial si procede).
  static float convertUnit(float value, const String& measurement,
                           const String& unit, bool imperial, String& outUnit);

  // Ángulo bruto (0–360°, 0 = norte) de la veleta, sin offset.
  static float windVaneRawAngle(uint16_t adc, const SystemConfig& sys);
  // Dirección de viento (0–360°) con el offset de norte aplicado.
  static float windDirection(uint16_t adc, const SystemConfig& sys);

private:
  SystemConfig sys_;
  float rainTotal_ = 0.0f;   // precipitación acumulada (mm)
  uint32_t lastRainTs_ = 0;  // timestamp de la última muestra de lluvia
};

}  // namespace sema
