#pragma once

#include <cstddef>
#include <cstdint>

// =============================================================================
// SEMA — Health Monitor (D-0020)
// =============================================================================
// Estado de salud agregado: liveness (heartbeat) + estado de sensores.

namespace sema {

class HealthMonitor {
public:
  void begin();
  void tick();                                   // heartbeat (llamar periódicamente)
  void setSensorStats(size_t online, size_t total);

  const char* status() const;                    // "HEALTHY" | "DEGRADED" | "ERROR"
  uint32_t uptimeSeconds() const;

private:
  bool heartbeatHealthy() const;
  uint32_t startMs_ = 0;
  uint32_t lastTickMs_ = 0;
  size_t online_ = 0;
  size_t total_ = 0;
};

}  // namespace sema
