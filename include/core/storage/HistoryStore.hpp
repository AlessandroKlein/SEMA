#pragma once

#include <Arduino.h>
#include <cstdint>
#include <deque>

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
  bool readRecent(std::deque<Measurement>& out, size_t maxCount);

  size_t count() const { return count_; }
  uint32_t maxEntries() const { return maxEntries_; }
  void setMaxEntries(uint32_t max) { maxEntries_ = max; }

  // Retención por tiempo (segundos); 0 = deshabilitada.
  void setRetentionSeconds(uint32_t s) { retentionSeconds_ = s; }
  bool prune(uint32_t nowEpoch);  // descarta lo más antiguo que el umbral

private:
  bool rotate();

  String path_;
  size_t count_ = 0;
  uint32_t maxEntries_ = 10000;
  uint32_t retentionSeconds_ = 0;
};

}  // namespace sema
