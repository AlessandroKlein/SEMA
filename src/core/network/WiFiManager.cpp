#include "core/network/WiFiManager.hpp"

#include <ESPmDNS.h>
#include <WiFi.h>

namespace sema {

void WiFiManager::begin(const String& mode, const String& ssid,
                        const String& password, const String& hostname,
                        const String& ip, const String& gateway,
                        const String& subnet, const String& dns, bool mdns) {
  if (mode == "STA" && ssid.length() > 0) {
    staMode_ = true;
    WiFi.mode(WIFI_STA);
    if (ip.length() > 0 && gateway.length() > 0 && subnet.length() > 0) {
      // IP estática (D-0090): IP, gateway, máscara y DNS.
      IPAddress aip, agw, asub, adns;
      if (aip.fromString(ip) && agw.fromString(gateway) &&
          asub.fromString(subnet) && adns.fromString(dns)) {
        WiFi.config(aip, agw, asub, adns);
      }
    }
    if (password.length() > 0) {
      WiFi.begin(ssid.c_str(), password.c_str());
    } else {
      WiFi.begin(ssid.c_str());
    }
  } else {
    startAp(hostname);
  }

  // mDNS (README §61): resuelve <hostname>.local. Se sanea el nombre (solo
  // letras, dígitos y guiones; sin espacios ni caracteres especiales).
  if (mdns && hostname.length() > 0) {
    String safe;
    safe.reserve(hostname.length());
    for (size_t i = 0; i < hostname.length(); ++i) {
      const char c = hostname[i];
      safe += (isalnum(static_cast<unsigned char>(c)) || c == '-') ? c : '-';
    }
    if (safe.length() == 0) {
      safe = "sema";
    }
    mdnsStarted_ = MDNS.begin(safe.c_str());
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
  const uint32_t now = millis();
  if (staMode_ && WiFi.status() != WL_CONNECTED) {
    // Reconexión con backoff exponencial (README §183): 2 s → 4 s → … → 60 s.
    if (now - lastReconnectAttempt_ >= reconnectInterval_) {
      lastReconnectAttempt_ = now;
      ++reconnectAttempts_;
      WiFi.reconnect();
      reconnectInterval_ = reconnectInterval_ * 2 > 60000 ? 60000 : reconnectInterval_ * 2;
    }
  } else if (staMode_ && reconnectAttempts_ > 0) {
    // Reconectado: se restablece el backoff.
    reconnectAttempts_ = 0;
    reconnectInterval_ = 2000;
  }
  // mDNS se procesa internamente por la tarea de WiFi (ESPmDNS).
}

}  // namespace sema
