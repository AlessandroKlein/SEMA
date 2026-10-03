#include "core/network/WiFiManager.hpp"

#include <ESPmDNS.h>
#include <WiFi.h>

namespace sema {

void WiFiManager::begin(const String& mode, const String& ssid,
                        const String& password, const String& hostname) {
  if (mode == "STA" && ssid.length() > 0) {
    staMode_ = true;
    WiFi.mode(WIFI_STA);
    if (password.length() > 0) {
      WiFi.begin(ssid.c_str(), password.c_str());
    } else {
      WiFi.begin(ssid.c_str());
    }
  } else {
    startAp(hostname);
  }

  // mDNS (README §61): resuelve <hostname>.local.
  if (hostname.length() > 0) {
    mdnsStarted_ = MDNS.begin(hostname.c_str());
    if (mdnsStarted_) {
      MDNS.addService("http", "tcp", 80);
    }
  }
}

void WiFiManager::startAp(const String& hostname) {
  staMode_ = false;
  const String ap = hostname.length() > 0 ? hostname : "SEMA";
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap.c_str());
}

bool WiFiManager::connected() const {
  return staMode_ ? (WiFi.status() == WL_CONNECTED) : true;
}

String WiFiManager::localIP() const {
  return staMode_ ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
}

int32_t WiFiManager::rssi() const {
  return staMode_ ? WiFi.RSSI() : 0;
}

void WiFiManager::loop() {
  // Reintento simple de STA cuando se pierde la conexión. Se mejora con backoff
  // en una iteración posterior (README §183).
  if (staMode_ && WiFi.status() != WL_CONNECTED) {
    // reservado para política de reconexión
  }
  // mDNS se procesa internamente por la tarea de WiFi (ESPmDNS).
}

}  // namespace sema
