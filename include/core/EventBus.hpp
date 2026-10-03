#pragma once

#include <Arduino.h>
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
  uint32_t id = 0;              // event_id (secuencia)
  uint32_t timestampMs = 0;     // millis() monotónico
  String source;                // id del sensor/módulo
  EventType type = EventType::System;
  Severity severity = Severity::Info;
  int32_t value = 0;            // payload numérico simple
  String correlationId;         // correlación (opcional)
  String target;                // destino (opcional)
};

// Nombre de la severidad, para serialización/API.
inline const char* severityName(Severity s) {
  switch (s) {
    case Severity::Debug: return "DEBUG";
    case Severity::Info: return "INFO";
    case Severity::Notice: return "NOTICE";
    case Severity::Warning: return "WARNING";
    case Severity::Error: return "ERROR";
    case Severity::Critical: return "CRITICAL";
    default: return "UNKNOWN";
  }
}

// Convierte un nombre de severidad al enum, para deserialización.
inline Severity parseSeverity(const char* name) {
  if (name == nullptr) return Severity::Info;
  if (strcmp(name, "DEBUG") == 0) return Severity::Debug;
  if (strcmp(name, "INFO") == 0) return Severity::Info;
  if (strcmp(name, "NOTICE") == 0) return Severity::Notice;
  if (strcmp(name, "WARNING") == 0) return Severity::Warning;
  if (strcmp(name, "ERROR") == 0) return Severity::Error;
  if (strcmp(name, "CRITICAL") == 0) return Severity::Critical;
  return Severity::Info;
}

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
