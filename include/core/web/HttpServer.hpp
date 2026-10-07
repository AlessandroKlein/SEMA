#pragma once

#include "hw/HwProfile.hpp"

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
  void onNetworkPage();
  void onSecurityPage();
  void onSystemPage();
  void onWindPage();
  void onSensorsPage();
  void onStatus();
  void onHealth();
  void onSystem();
  void onConfig();
  void onConfigPut();
  void onConfigNetwork();
  void onConfigSystem();
  void onConfigSensors();
  void onConfigIo();
  void onWifiScan();
  void onApiKeys();
  void onUpdateCheck();
  void onBackup();
  void onLoginPost();
  void onLoginPage();
  void onLogout();
  void onRestart();
  void onOta();
  void onOtaUpload();
  void onCapabilities();
  void onNetwork();
  void onEnergy();
  void onDiagnostics();
  void onSensors();
  void onWindNorth();
  void onWindResistors();
  void onDashboardLayout();
  void onDashboardLayoutGet();
  void onStaticFile(const char* path, const char* type);
  void onHistory();
  void onEvents();
  void onAlarms();
  void onGpio();
  void onGpioWrite();
  void onShift();
  void onShiftWrite();
#if SEMA_USE_MODBUS
  void onModbus();
#endif
#if SEMA_USE_CAN
  void onCan();
  void onCanWrite();
#endif
#if SEMA_USE_LORA
  void onLora();
  void onLoraWrite();
#endif
#if SEMA_USE_ZIGBEE
  void onZigbee();
  void onZigbeeWrite();
#endif
  void onNotFound();

  bool authorized();
  bool sessionAuthorized();
  bool webAuthed();  // acceso web: sin login configurado, o sesión, o API key

  ::WebServer server_;
  ::WebSocketsServer ws_{81};
  SemaCore* core_ = nullptr;
  bool otaAuthorized_ = false;
  String otaExpectedSha_;  // SHA-256 esperado del firmware (cabecera X-SHA256)
  bool otaShaOk_ = true;   // false = checksum no coincide → no reiniciar
  String sessionToken_;
  uint32_t failedLogins_ = 0;
  uint32_t lockoutUntilMs_ = 0;
  uint32_t sessionStartMs_ = 0;  // 0 = sin sesión activa
};

}  // namespace sema
