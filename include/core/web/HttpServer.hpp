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
  void onStatus();
  void onHealth();
  void onSystem();
  void onConfig();
  void onConfigPut();
  void onRestart();
  void onDiagnostics();
  void onSensors();
  void onHistory();
  void onAlarms();
  void onNotFound();

  bool authorized();

  ::WebServer server_;
  ::WebSocketsServer ws_{81};
  SemaCore* core_ = nullptr;
};

}  // namespace sema
