#pragma once

#include <WebServer.h>

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

private:
  void onStatus();
  void onHealth();
  void onSystem();
  void onConfig();
  void onDiagnostics();
  void onSensors();
  void onHistory();
  void onAlarms();
  void onNotFound();

  ::WebServer server_;
  SemaCore* core_ = nullptr;
};

}  // namespace sema
