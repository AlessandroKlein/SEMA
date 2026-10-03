#pragma once

#include <cstdint>
#include <functional>
#include <vector>

// =============================================================================
// SEMA — Scheduler de tareas periódicas
// =============================================================================
// D-0018 / D-0052. Tareas periódicas basadas en capacidades/módulos habilitados.
// Se conduce desde el loop principal; los intervalos son configurables.

namespace sema {

class Scheduler {
public:
  struct Task {
    const char* name;
    uint32_t intervalMs;
    uint32_t lastRun;
    std::function<void()> fn;
  };

  void add(const char* name, uint32_t intervalMs, std::function<void()> fn);
  void run();  // llamar desde el loop principal

  size_t count() const { return tasks_.size(); }

private:
  std::vector<Task> tasks_;
};

}  // namespace sema
