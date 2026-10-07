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
             const String& hostname, const String& ip, const String& gateway,
             const String& subnet, const String& dns);
  void loop();

  bool connected() const;
  String localIP() const;
  int32_t rssi() const;
  bool isAp() const { return !staMode_; }
  bool mdnsStarted() const { return mdnsStarted_; }

private:
  void startAp(const String& hostname);

  bool staMode_ = false;
  bool mdnsStarted_ = false;
  uint32_t lastReconnectAttempt_ = 0;
  uint32_t reconnectAttempts_ = 0;
  uint32_t reconnectInterval_ = 2000;  // backoff exponencial: 2 s → 60 s
};

}  // namespace sema
