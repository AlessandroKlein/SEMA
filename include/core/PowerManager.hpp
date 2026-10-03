#pragma once

#include <cstdint>

// =============================================================================
// SEMA — Gestión de energía
// =============================================================================
// D-0021 / D-0054. Perfiles energéticos y deep sleep con wake por timer RTC.

namespace sema {

enum class EnergyProfile : uint8_t {
  Performance,
  Normal,
  LowPower,
  UltraLowPower
};

inline const char* energyProfileName(EnergyProfile p) {
  switch (p) {
    case EnergyProfile::Performance: return "performance";
    case EnergyProfile::Normal: return "normal";
    case EnergyProfile::LowPower: return "low_power";
    case EnergyProfile::UltraLowPower: return "ultra_low_power";
    default: return "unknown";
  }
}

class PowerManager {
public:
  static PowerManager& instance();

  EnergyProfile profile() const { return profile_; }
  void setProfile(EnergyProfile p) { profile_ = p; }

  void sleep(uint64_t seconds);   // deep sleep con wake por timer RTC
  uint32_t wakeReason() const;    // esp_sleep_get_wakeup_cause()

private:
  PowerManager() = default;

  EnergyProfile profile_ = EnergyProfile::Normal;
};

}  // namespace sema
