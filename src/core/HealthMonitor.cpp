#include "core/HealthMonitor.hpp"

#include <Arduino.h>

namespace sema {

void HealthMonitor::begin() {
  startMs_ = millis();
  lastTickMs_ = startMs_;
}

void HealthMonitor::tick() {
  lastTickMs_ = millis();
}

void HealthMonitor::setSensorStats(size_t online, size_t total) {
  online_ = online;
  total_ = total;
}

const char* HealthMonitor::status() const {
  if (total_ > 0 && online_ == 0) {
    return "ERROR";      // hay sensores configurados y ninguno responde
  }
  if (total_ > 0 && online_ < total_) {
    return "DEGRADED";   // alguno no responde
  }
  if (!heartbeatHealthy()) {
    return "DEGRADED";   // el bucle no late (posible bloqueo)
  }
  return "HEALTHY";
}

uint32_t HealthMonitor::uptimeSeconds() const {
  return (millis() - startMs_) / 1000;
}

bool HealthMonitor::heartbeatHealthy() const {
  // El heartbeat corre cada 5 s; se considera sano si latió en los últimos 30 s.
  return (millis() - lastTickMs_) < 30000;
}

}  // namespace sema
