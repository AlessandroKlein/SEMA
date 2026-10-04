#pragma once

#include <vector>

#include "core/EventBus.hpp"
#include "core/Measurement.hpp"
#include "core/alarms/Rule.hpp"

// =============================================================================
// SEMA — Motor de reglas y alarmas
// =============================================================================
// D-0059. Evalúa reglas sobre las mediciones y publica eventos `Alarm` en el
// Event Bus (D-0045). La combinación lógica y acciones (webhook, registro, …)
// se añaden en iteraciones posteriores.

namespace sema {

class RuleEngine {
public:
  explicit RuleEngine(EventBus& bus);

  void addRule(const Rule& rule);
  void clear() { rules_.clear(); }
  void evaluate(const std::vector<Measurement>& measurements);

private:
  bool matches(const Rule& r, const Measurement& m) const;

  EventBus& bus_;
  std::vector<Rule> rules_;
};

}  // namespace sema
