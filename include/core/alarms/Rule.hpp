#pragma once

#include <Arduino.h>
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
  String id;          // id de la regla (para correlación)
  String sensorId;    // "" = cualquier sensor
  String channelId;   // magnitud a vigilar ("temperature", …)
  RuleOp op;
  float threshold;
};

// Convierte un nombre de operador al enum (configuración → regla).
inline RuleOp parseRuleOp(const char* name) {
  if (name == nullptr) return RuleOp::Gt;
  if (strcmp(name, "lt") == 0) return RuleOp::Lt;
  if (strcmp(name, "ge") == 0) return RuleOp::Ge;
  if (strcmp(name, "le") == 0) return RuleOp::Le;
  return RuleOp::Gt;  // "gt" o desconocido
}

}  // namespace sema
