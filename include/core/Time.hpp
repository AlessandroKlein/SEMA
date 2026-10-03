#pragma once

#include <Arduino.h>
#include <time.h>

// =============================================================================
// SEMA — Reloj (NTP / epoch)
// =============================================================================
// D-0044. `nowEpoch()` devuelve la época Unix (UTC, segundos). Hasta que el
// reloj se sincronice por NTP (configTime en SemaCore), cae a `millis()/1000`
// (uptime) para no devolver 0.

namespace sema {

inline uint32_t nowEpoch() {
  const time_t t = time(nullptr);
  // time() vale ~0 antes de la sincronización; un valor > 2001-09-09 indica
  // reloj sincronizado (época real).
  return (t > 1000000000) ? static_cast<uint32_t>(t) : millis() / 1000;
}

}  // namespace sema
