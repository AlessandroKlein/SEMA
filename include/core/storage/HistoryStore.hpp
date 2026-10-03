#pragma once

#include <Arduino.h>
#include <cstdint>

#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Almacén de histórico (backend LittleFS)
// =============================================================================
// D-0046 / D-0057. Registro JSONL de mediciones sobre LittleFS. La retención por
// niveles (alta resolución + agregados) se añadirá en una iteración posterior.

namespace sema {

class HistoryStore {
public:
  bool begin(const char* path = "/history.jsonl");
  bool append(const Measurement& m);

  size_t count() const { return count_; }
  uint32_t maxEntries() const { return maxEntries_; }
  void setMaxEntries(uint32_t max) { maxEntries_ = max; }

private:
  String path_;
  size_t count_ = 0;
  uint32_t maxEntries_ = 10000;
};

}  // namespace sema
