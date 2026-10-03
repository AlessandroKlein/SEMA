#pragma once

#include <Arduino.h>

// =============================================================================
// SEMA — Gestión de red Wi-Fi
// =============================================================================
// D-0041 / README §59-60. Conecta en modo STA (configuración) o levanta un
// Access Point de emergencia cuando no hay red configurada.

namespace sema {

class WiFiManager {
public:
  void begin(const String& mode, const String& ssid, const String& password,
             const String& hostname);
  void loop();

  bool connected() const;
  String localIP() const;
  bool isAp() const { return !staMode_; }

private:
  void startAp(const String& hostname);

  bool staMode_ = false;
};

}  // namespace sema
