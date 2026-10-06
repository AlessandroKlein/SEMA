#pragma once

#include <WebServer.h>
#include <WebSocketsServer.h>
#include <vector>

#include "core/Measurement.hpp"

// =============================================================================
// SEMA — Servidor HTTP local (REST /api/v1)
// =============================================================================
// D-0041 / D-0006. Expone la API local sin depender de ningún servidor.
// WebSocket (/ws) se añadirá en una iteración posterior.

namespace sema {

class SemaCore;

class HttpServer {
public:
  void begin(SemaCore& core);
  void loop();
  void broadcastMeasurements(const std::vector<Measurement>& measurements);

private:
  void onRoot();
  void onStatus();
  void onHealth();
  void onSystem();
  void onConfig();
  void onConfigPut();
  void onBackup();
  void onLoginPost();
  void onLogout();
  void onRestart();
  void onOta();
  void onOtaUpload();
  void onCapabilities();
  void onNetwork();
  void onEnergy();
  void onDiagnostics();
  void onSensors();
  void onHistory();
  void onEvents();
  void onAlarms();
  void onGpio();
  void onGpioWrite();
  void onNotFound();

  bool authorized();
  bool sessionAuthorized();

  ::WebServer server_;
  ::WebSocketsServer ws_{81};
  SemaCore* core_ = nullptr;
  bool otaAuthorized_ = false;
  String sessionToken_;
  uint32_t failedLogins_ = 0;
  uint32_t lockoutUntilMs_ = 0;
  uint32_t sessionStartMs_ = 0;  // 0 = sin sesión activa
};

}  // namespace sema
