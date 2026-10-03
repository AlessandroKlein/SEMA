#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Regla de alarma
// =============================================================================
// D-0059 / README §63, §87. Operadores básicos de comparación sobre un canal.

namespace sema {

enum class RuleOp : uint8_t {
  Gt,  // >
  Lt,  // <
  Ge,  // >=
  Le   // <=
};

struct Rule {
  const char* id;         // id de la regla (para correlación)
  const char* sensorId;   // "" o nullptr = cualquier sensor
  const char* channelId;  // magnitud a vigilar ("temperature", …)
  RuleOp op;
  float threshold;
};

}  // namespace sema
