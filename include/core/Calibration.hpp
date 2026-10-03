#pragma once

#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Calibración y validación de rango por canal
// =============================================================================
// D-0055 / README §43, §84. La calibración es independiente del driver y se
// almacena junto al canal lógico: offset, gain, límites y (más adelante) filtro.

namespace sema {

struct Calibration {
  float offset = 0.0f;
  float gain = 1.0f;
  float min = 0.0f;
  float max = 0.0f;
  bool hasRange = false;  // si false, no se aplica validación de rango
  bool enabled = false;
};

// Aplica calibración (value = value*gain + offset) y, si corresponde, marca la
// medición como OUT_OF_RANGE cuando queda fuera de [min, max].
void applyCalibration(Measurement& m, const Calibration& c);

}  // namespace sema
