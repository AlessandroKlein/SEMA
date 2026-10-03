#pragma once

#include <vector>

#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Cálculos derivados
// =============================================================================
// D-0044 / README §83. Magnitudes que no requieren un sensor propio, calculadas a
// partir de las mediciones disponibles (separadas de los drivers de sensores).

namespace sema {

class DerivedEngine {
public:
  // Añade mediciones derivadas (punto de rocío, índice de calor, …) a `measurements`.
  static void compute(std::vector<Measurement>& measurements);

  // Fórmulas públicas y testables.
  static float dewPoint(float tempC, float relHum);   // °C
  static float heatIndex(float tempC, float relHum);  // °C
};

}  // namespace sema
