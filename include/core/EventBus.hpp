#pragma once

#include <cstdint>
#include <functional>
#include <vector>

// =============================================================================
// SEMA — Event Bus tipado
// =============================================================================
// D-0045 / D-0008. Desacopla módulos: varios suscriptores reaccionan al mismo
// evento sin conocerse entre sí. Funciona entre tasks sin bloquear la adquisición.

namespace sema {

// Eventos comunes (README.md §205).
enum class EventType : uint8_t {
  Sensor,
  Rain,
  Lightning,
  Battery,
  Network,
  Alarm,
  System,
  Wake,
  Sleep
};

// Severidad (README.md §195, §219).
enum class Severity : uint8_t {
  Debug,
  Info,
  Notice,
  Warning,
  Error,
  Critical
};

// Evento tipado (D-0045): event_id, timestamp, source, type, severity, payload,
// correlation_id y destino opcional.
struct Event {
  uint32_t id = 0;                   // event_id (secuencia)
  uint32_t timestampMs = 0;          // millis() monotónico
  const char* source = nullptr;      // id del sensor/módulo
  EventType type = EventType::System;
  Severity severity = Severity::Info;
  int32_t value = 0;                 // payload numérico simple
  const char* correlationId = nullptr;  // correlación (opcional)
  const char* target = nullptr;      // destino (opcional)
};

class EventBus {
public:
  using Handler = std::function<void(const Event&)>;

  void publish(const Event& event);
  void subscribe(EventType type, Handler handler);

private:
  struct Subscription {
    EventType type;
    Handler handler;
  };

  std::vector<Subscription> subs_;
};

}  // namespace sema
