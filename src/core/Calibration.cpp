#include "core/Calibration.hpp"

namespace sema {

void applyCalibration(Measurement& m, const Calibration& c) {
  if (!c.enabled) {
    return;
  }
  m.value = (m.value * c.gain) + c.offset;

  if (c.hasRange && (m.value < c.min || m.value > c.max)) {
    m.quality = Quality::OutOfRange;
  }
}

}  // namespace sema
