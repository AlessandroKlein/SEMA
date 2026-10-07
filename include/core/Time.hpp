#pragma once

#include <Arduino.h>
#include <time.h>

// =============================================================================
// SEMA — Reloj (NTP / epoch)
// =============================================================================
// D-0044. `nowEpoch()` devuelve la época local (zona horaria configurada) en
// segundos. Hasta que el reloj se sincronice por NTP (configTzTime en SemaCore),
// cae a `millis()/1000` (uptime) para no devolver 0.

namespace sema {

// Offset en segundos entre la hora local (zona configurada) y UTC.
inline int32_t timezoneOffsetSeconds() {
  const time_t t = time(nullptr);
  if (t <= 1000000000) {
    return 0;  // aún sin NTP
  }
  struct tm gt;
  gmtime_r(&t, &gt);
  // mktime interpreta la hora UTC como local → t + (-offset).
  return static_cast<int32_t>(t - mktime(&gt));
}

inline uint32_t nowEpoch() {
  const time_t t = time(nullptr);
  // time() vale ~0 antes de la sincronización; un valor > 2001-09-09 indica
  // reloj sincronizado (época real).
  if (t <= 1000000000) {
    return millis() / 1000;
  }
  return static_cast<uint32_t>(static_cast<int64_t>(t) + timezoneOffsetSeconds());
}

}  // namespace sema
