#include "core/EventBus.hpp"

#include <utility>

namespace sema {

void EventBus::publish(const Event& event) {
  for (const Subscription& sub : subs_) {
    if (sub.type == event.type) {
      sub.handler(event);
    }
  }
}

void EventBus::subscribe(EventType type, Handler handler) {
  subs_.push_back({type, std::move(handler)});
}

}  // namespace sema
