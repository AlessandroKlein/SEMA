#pragma once

#include <cstdint>
#include <functional>
#include <vector>

// =============================================================================
// SEMA — Bus interno de eventos
// =============================================================================
// README.md §204-205. Desacopla módulos: varios suscriptores reaccionan al mismo
// evento sin conocerse entre sí (p. ej. RAIN_START → Storage, Alarm, MQTT, …).

namespace sema {

enum class EventType : uint8_t {
  SENSOR,
  RAIN,
  LIGHTNING,
  BATTERY,
  NETWORK,
  ALARM,
  SYSTEM,
  WAKE,
  SLEEP
};

struct Event {
  EventType type;
  uint32_t timestampMs;
  int32_t value;
  const char* source;
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
