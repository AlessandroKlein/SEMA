#include "core/CapabilityManager.hpp"

namespace sema {

CapabilityManager& CapabilityManager::instance() {
  static CapabilityManager mgr;
  return mgr;
}

void CapabilityManager::set(Capability c, bool available) {
  const uint32_t bit = 1u << static_cast<uint8_t>(c);
  if (available) {
    mask_ |= bit;
  } else {
    mask_ &= ~bit;
  }
}

bool CapabilityManager::has(Capability c) const {
  const uint32_t bit = 1u << static_cast<uint8_t>(c);
  return (mask_ & bit) != 0;
}

}  // namespace sema
