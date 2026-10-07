#include "core/HealthMonitor.hpp"

#include <Arduino.h>
#include <string.h>

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

void HealthMonitor::registerTask(const char* name, uint32_t timeoutMs) {
  if (taskCount_ >= kMaxTasks) {
    return;
  }
  TaskWatch& t = tasks_[taskCount_++];
  t.name = name;
  t.timeoutMs = timeoutMs;
  t.lastMs = millis();
}

void HealthMonitor::taskHeartbeat(const char* name) {
  TaskWatch* t = findTask(name);
  if (t != nullptr) {
    t->lastMs = millis();
  }
}

bool HealthMonitor::taskHealthy(const char* name) const {
  for (size_t i = 0; i < taskCount_; ++i) {
    if (tasks_[i].name != nullptr && strcmp(tasks_[i].name, name) == 0) {
      return (millis() - tasks_[i].lastMs) < tasks_[i].timeoutMs;
    }
  }
  return true;  // tarea no registrada → no se evalúa
}

bool HealthMonitor::allTasksHealthy() const {
  const uint32_t now = millis();
  for (size_t i = 0; i < taskCount_; ++i) {
    if (tasks_[i].name != nullptr && (now - tasks_[i].lastMs) >= tasks_[i].timeoutMs) {
      return false;
    }
  }
  return true;
}

HealthMonitor::TaskWatch* HealthMonitor::findTask(const char* name) {
  for (size_t i = 0; i < taskCount_; ++i) {
    if (tasks_[i].name != nullptr && strcmp(tasks_[i].name, name) == 0) {
      return &tasks_[i];
    }
  }
  return nullptr;
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
  if (!allTasksHealthy()) {
    return "DEGRADED";   // watchdog jerárquico: alguna tarea estancada
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
