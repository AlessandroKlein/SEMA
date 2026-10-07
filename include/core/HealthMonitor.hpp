#pragma once

#include <cstddef>
#include <cstdint>

// =============================================================================
// SEMA — Health Monitor (D-0020)
// =============================================================================
// Estado de salud agregado: liveness (heartbeat) + estado de sensores +
// watchdog jerárquico por tarea (cada tarea late con su propio timeout).

namespace sema {

class HealthMonitor {
public:
  static constexpr size_t kMaxTasks = 10;

  void begin();
  void tick();                                   // heartbeat (llamar periódicamente)
  void setSensorStats(size_t online, size_t total);

  // Watchdog jerárquico por tarea.
  void registerTask(const char* name, uint32_t timeoutMs);
  void taskHeartbeat(const char* name);
  bool taskHealthy(const char* name) const;
  bool allTasksHealthy() const;
  size_t taskCount() const { return taskCount_; }
  const char* taskName(size_t i) const { return (i < taskCount_) ? tasks_[i].name : nullptr; }

  const char* status() const;                    // "HEALTHY" | "DEGRADED" | "ERROR"
  uint32_t uptimeSeconds() const;

private:
  struct TaskWatch {
    const char* name = nullptr;
    uint32_t timeoutMs = 0;
    uint32_t lastMs = 0;
  };

  bool heartbeatHealthy() const;
  TaskWatch* findTask(const char* name);

  uint32_t startMs_ = 0;
  uint32_t lastTickMs_ = 0;
  size_t online_ = 0;
  size_t total_ = 0;
  TaskWatch tasks_[kMaxTasks];
  size_t taskCount_ = 0;
};

}  // namespace sema
