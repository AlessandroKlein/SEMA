#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Abstracción de tarea (semilla del Runtime Manager)
// =============================================================================
// D-0012 / D-0039 / D-0052 / D-0053. Oculta FreeRTOS detrás de una interfaz
// propia para no acoplar SEMA a su API. La afinidad queda en AUTO por defecto
// (xTaskCreate, tskNO_AFFINITY); el pinning solo se usará con justificación.

namespace sema {

using TaskFunction = void (*)(void*);

class Task {
public:
  Task(TaskFunction fn, const char* name, uint32_t stackBytes, uint32_t priority, void* arg = nullptr);
  ~Task();

  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;

  bool start();
  void stop();
  bool running() const { return handle_ != nullptr; }

private:
  TaskFunction fn_;
  const char* name_;
  uint32_t stack_;
  uint32_t priority_;
  void* arg_;
  void* handle_ = nullptr;
};

}  // namespace sema
