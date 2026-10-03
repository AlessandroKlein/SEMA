#include "core/alarms/RuleEngine.hpp"

#include <Arduino.h>

namespace sema {

RuleEngine::RuleEngine(EventBus& bus) : bus_(bus) {}

void RuleEngine::addRule(const Rule& rule) {
  rules_.push_back(rule);
}

bool RuleEngine::matches(const Rule& r, const Measurement& m) const {
  if (r.sensorId != nullptr && r.sensorId[0] != '\0' && m.sensorId != r.sensorId) {
    return false;
  }
  if (m.channelId != r.channelId) {
    return false;
  }
  switch (r.op) {
    case RuleOp::Gt: return m.value > r.threshold;
    case RuleOp::Lt: return m.value < r.threshold;
    case RuleOp::Ge: return m.value >= r.threshold;
    case RuleOp::Le: return m.value <= r.threshold;
    default: return false;
  }
}

void RuleEngine::evaluate(const std::vector<Measurement>& measurements) {
  for (const Rule& r : rules_) {
    for (const Measurement& m : measurements) {
      if (matches(r, m)) {
        Event e;
        e.id = 0;
        e.timestampMs = millis();
        e.source = m.sensorId.c_str();
        e.type = EventType::Alarm;
        e.severity = Severity::Warning;
        e.value = static_cast<int32_t>(m.value * 100.0f);
        e.correlationId = r.id;
        e.target = nullptr;
        bus_.publish(e);
      }
    }
  }
}

}  // namespace sema
