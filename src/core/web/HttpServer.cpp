#include "core/web/HttpServer.hpp"

#include <ArduinoJson.h>
#include <Update.h>
#include <esp_system.h>

#include "core/SemaCore.hpp"

namespace sema {

namespace {
// Página de login (formulario de acceso al dashboard).
const char kLoginHtml[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SEMA — Acceso</title>
<style>
body{font-family:system-ui,sans-serif;background:#0d1117;color:#e6edf3;display:flex;align-items:center;justify-content:center;height:100vh;margin:0}
form{background:#161b22;padding:2rem;border-radius:8px;text-align:center}
input{display:block;width:100%;box-sizing:border-box;padding:.5rem;margin:.6rem 0;border:1px solid #30363d;border-radius:4px;background:#0d1117;color:#e6edf3}
button{width:100%;padding:.5rem;border:0;border-radius:4px;background:#1f6feb;color:#fff;cursor:pointer}
</style>
</head>
<body>
<form method="POST" action="/login">
<h1>SEMA</h1>
<input type="password" name="password" placeholder="Clave de acceso" autofocus>
<button type="submit">Entrar</button>
</form>
</body>
</html>
)html";
}  // namespace

void HttpServer::begin(SemaCore& core) {
  core_ = &core;

  server_.on("/", HTTP_GET, [this]() { onRoot(); });
  server_.on("/api/v1/status", HTTP_GET, [this]() { onStatus(); });
  server_.on("/api/v1/health", HTTP_GET, [this]() { onHealth(); });
  server_.on("/api/v1/system", HTTP_GET, [this]() { onSystem(); });
  server_.on("/api/v1/config", HTTP_GET, [this]() { onConfig(); });
  server_.on("/api/v1/config", HTTP_PUT, [this]() { onConfigPut(); });
  server_.on("/api/v1/backup", HTTP_GET, [this]() { onBackup(); });
  server_.on("/api/v1/backup", HTTP_POST, [this]() { onConfigPut(); });
  server_.on("/login", HTTP_POST, [this]() { onLoginPost(); });
  server_.on("/logout", HTTP_GET, [this]() { onLogout(); });
  server_.on("/api/v1/restart", HTTP_POST, [this]() { onRestart(); });
  server_.on("/api/v1/ota", HTTP_POST, [this]() { onOta(); }, [this]() { onOtaUpload(); });
  server_.on("/api/v1/capabilities", HTTP_GET, [this]() { onCapabilities(); });
  server_.on("/api/v1/network", HTTP_GET, [this]() { onNetwork(); });
  server_.on("/api/v1/energy", HTTP_GET, [this]() { onEnergy(); });
  server_.on("/api/v1/diagnostics", HTTP_GET, [this]() { onDiagnostics(); });
  server_.on("/api/v1/sensors", HTTP_GET, [this]() { onSensors(); });
  server_.on("/api/v1/history", HTTP_GET, [this]() { onHistory(); });
  server_.on("/api/v1/events", HTTP_GET, [this]() { onEvents(); });
  server_.on("/api/v1/alarms", HTTP_GET, [this]() { onAlarms(); });
  server_.onNotFound([this]() { onNotFound(); });

  // Token de sesión aleatorio (login web).
  uint32_t r1 = esp_random();
  uint32_t r2 = esp_random();
  char token[24];
  snprintf(token, sizeof(token), "%08lx%08lx", static_cast<unsigned long>(r1),
           static_cast<unsigned long>(r2));
  sessionToken_ = token;

  static const char* kHeaders[] = {"X-API-Key", "Cookie"};
  server_.collectHeaders(kHeaders, 2);

  server_.begin();
  ws_.begin();
}

void HttpServer::loop() {
  server_.handleClient();
  ws_.loop();
}

void HttpServer::broadcastMeasurements(const std::vector<Measurement>& measurements) {
  if (ws_.connectedClients() == 0) {
    return;
  }

  DynamicJsonDocument doc(8192);
  doc["type"] = "measurements";
  JsonArray arr = doc.createNestedArray("data");
  for (const Measurement& m : measurements) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }
  String out;
  serializeJson(doc, out);
  ws_.broadcastTXT(out.c_str());
}

void HttpServer::onRoot() {
  // Protección del dashboard (login por sesión); sin claves configuradas queda
  // abierto para la primera configuración.
  const SecurityConfig& sec = core_->config().get().security;
  const bool noKeys = sec.apiKey.length() == 0 && sec.serverKey.length() == 0;
  if (!noKeys && !sessionAuthorized()) {
    server_.send(200, "text/html", kLoginHtml);
    return;
  }

  static const char kIndexHtml[] PROGMEM = R"html(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>SEMA</title>
<style>
body{font-family:system-ui,sans-serif;margin:1rem;background:#0d1117;color:#e6edf3}
h1{margin:0 0 .25rem}h2{margin:1.25rem 0 .5rem}
table{border-collapse:collapse;width:100%}
td,th{border:1px solid #30363d;padding:.4rem .6rem;text-align:left}
.muted{color:#8b949e}
</style>
</head>
<body>
<h1>SEMA</h1>
<div id="status" class="muted">Cargando…</div>
<h2>Sensores</h2>
<table><thead><tr><th>Sensor</th><th>Canal</th><th>Valor</th><th>Unidad</th><th>Calidad</th></tr></thead>
<tbody id="rows"><tr><td colspan="5" class="muted">Cargando…</td></tr></tbody></table>
<h2>Histórico</h2>
<table><thead><tr><th>Fecha</th><th>Sensor</th><th>Canal</th><th>Valor</th></tr></thead>
<tbody id="hist"><tr><td colspan="4" class="muted">Cargando…</td></tr></tbody></table>
<script>
async function refresh(){
  try{
    const s=await(await fetch('/api/v1/status')).json();
    document.getElementById('status').textContent=s.name+' ('+s.station+') — v'+s.firmware+' — '+s.uptime_s+' s';
  }catch(e){document.getElementById('status').textContent='Sin conexión';}
  try{
    const r=await(await fetch('/api/v1/sensors')).json();
    let h='';
    for(const m of r.measurements){
      h+='<tr><td>'+m.sensor_id+'</td><td>'+m.channel_id+'</td><td>'+m.value+'</td><td>'+m.unit+'</td><td>'+m.quality+'</td></tr>';
    }
    document.getElementById('rows').innerHTML=h||'<tr><td colspan="5" class="muted">Sin datos</td></tr>';
  }catch(e){}
}
async function loadHistory(){
  try{
    const r=await(await fetch('/api/v1/history?limit=20')).json();
    let h='';
    for(const m of r.history){
      const d=new Date(m.ts*1000);
      const t=(m.ts>1000000000)?d.toLocaleString():('uptime '+m.ts+' s');
      h+='<tr><td>'+t+'</td><td>'+m.sensor+'</td><td>'+m.channel+'</td><td>'+m.value+' '+m.unit+'</td></tr>';
    }
    document.getElementById('hist').innerHTML=h||'<tr><td colspan="4" class="muted">Sin datos</td></tr>';
  }catch(e){}
}
loadHistory();
setInterval(loadHistory,10000);
refresh();
setInterval(refresh,5000);
</script>
</body>
</html>
)html";
  server_.send(200, "text/html", kIndexHtml);
}

bool HttpServer::sessionAuthorized() {
  if (!server_.hasHeader("Cookie")) {
    return false;
  }
  const String cookie = server_.header("Cookie");
  return cookie.indexOf("sema_auth=" + sessionToken_) >= 0;
}

void HttpServer::onLoginPost() {
  const String password = server_.arg("password");
  const SecurityConfig& sec = core_->config().get().security;
  const bool ok = (sec.apiKey.length() > 0 && password == sec.apiKey) ||
                  (sec.serverKey.length() > 0 && password == sec.serverKey);
  if (ok) {
    server_.sendHeader("Set-Cookie", "sema_auth=" + sessionToken_ + "; Path=/; HttpOnly");
    server_.sendHeader("Location", "/");
    server_.send(302, "text/plain", "");
  } else {
    server_.send(401, "text/html", kLoginHtml);
  }
}

void HttpServer::onLogout() {
  server_.sendHeader("Set-Cookie", "sema_auth=; Path=/; Max-Age=0");
  server_.sendHeader("Location", "/");
  server_.send(302, "text/plain", "");
}

void HttpServer::onStatus() {
  DynamicJsonDocument doc(256);
  doc["station"] = core_->config().get().station.id;
  doc["name"] = core_->config().get().station.name;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["uptime_s"] = millis() / 1000;
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onHealth() {
  DynamicJsonDocument doc(384);
  const HealthMonitor& h = core_->health();
  doc["status"] = h.status();
  doc["uptime_s"] = h.uptimeSeconds();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["sensors"]["total"] = core_->sensors().count();
  doc["sensors"]["online"] = core_->sensors().onlineCount();
  doc["sensors"]["error"] = core_->sensors().count() - core_->sensors().onlineCount();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onBackup() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  // Respaldo autodescriptivo: configuración completa + metadatos (D-0044/D-0046).
  // Los campos "backup_*"/"firmware"/"timestamp" se ignoran al restaurar.
  String cfg;
  if (!core_->config().toJson(cfg)) {
    server_.send(500, "application/json", "{\"error\":\"serialization failed\"}");
    return;
  }
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, cfg)) {
    server_.send(500, "application/json", "{\"error\":\"internal\"}");
    return;
  }
  doc["backup_format"] = "sema-backup";
  doc["backup_version"] = 1;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["timestamp"] = millis() / 1000;
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onSystem() {
  DynamicJsonDocument doc(256);
  doc["id"] = core_->config().get().station.id;
  doc["name"] = core_->config().get().station.name;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["hw"] = SEMA_HW_VERSION;
  doc["config_schema"] = SEMA_CONFIG_SCHEMA_VERSION;
  doc["protocol"] = SEMA_PROTOCOL_VERSION;
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onConfig() {
  // La config expone claves (api_key/server_key): requiere autenticación.
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  String out;
  if (core_->config().toJson(out)) {
    server_.send(200, "application/json", out);
  } else {
    server_.send(500, "application/json", "{\"error\":\"serialization failed\"}");
  }
}

bool HttpServer::authorized() {
  // Dos credenciales: la web local (api_key) y el Servidor Central (server_key),
  // que puede enviar configuración de riesgo por API (D-0048).
  const SecurityConfig& sec = core_->config().get().security;
  if (sec.apiKey.length() == 0 && sec.serverKey.length() == 0) {
    return true;  // sin claves configuradas → permitir (primera configuración)
  }
  if (!server_.hasHeader("X-API-Key")) {
    return false;
  }
  const String key = server_.header("X-API-Key");
  return (sec.apiKey.length() > 0 && key == sec.apiKey) ||
         (sec.serverKey.length() > 0 && key == sec.serverKey);
}

void HttpServer::onRestart() {
  if (!authorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
  delay(100);
  ESP.restart();
}

void HttpServer::onConfigPut() {
  if (!authorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"body required\"}");
    return;
  }
  const String body = server_.arg("plain");
  if (core_->config().applyJson(body)) {
    // Re-aplica la config no hardware-dependiente sin reiniciar.
    core_->applyRules();
    core_->applyCalibrations();
    server_.send(200, "application/json", "{\"ok\":true}");
  } else {
    server_.send(400, "application/json", "{\"error\":\"invalid config\"}");
  }
}

void HttpServer::onOtaUpload() {
  HTTPUpload& upload = server_.upload();
  if (upload.status == UPLOAD_FILE_START) {
    otaAuthorized_ = authorized();
    if (!otaAuthorized_) {
      return;  // no escribir nada si no está autorizado
    }
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (otaAuthorized_ &&
        Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (otaAuthorized_) {
      Update.end(true);
    }
  }
}

void HttpServer::onOta() {
  if (!otaAuthorized_) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    otaAuthorized_ = false;
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
  delay(100);
  ESP.restart();
}

void HttpServer::onCapabilities() {
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.createNestedArray("capabilities");
  CapabilityManager& caps = core_->capabilities();
  for (uint8_t i = 0; i < static_cast<uint8_t>(Capability::Count); ++i) {
    const Capability c = static_cast<Capability>(i);
    if (caps.has(c)) {
      arr.add(capabilityName(c));
    }
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onNetwork() {
  DynamicJsonDocument doc(256);
  doc["mode"] = core_->wifi().isAp() ? "AP" : "STA";
  doc["connected"] = core_->wifi().connected();
  doc["ip"] = core_->wifi().localIP();
  doc["rssi"] = core_->wifi().rssi();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onEnergy() {
  DynamicJsonDocument doc(256);
  doc["profile"] = energyProfileName(core_->power().profile());
  doc["wake_reason"] = core_->power().wakeReason();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onDiagnostics() {
  DynamicJsonDocument doc(1024);
  doc["firmware"] = SEMA_FW_VERSION;
  doc["hw"] = SEMA_HW_VERSION;
  doc["uptime_s"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["reset_reason"] = static_cast<int>(esp_reset_reason());
  doc["health"] = core_->health().status();
  doc["history"]["entries"] = core_->history().count();
  doc["history"]["max"] = core_->history().maxEntries();
  doc["tasks"] = core_->scheduler().count();
  doc["modules"] = core_->modules().count();
  doc["events"] = core_->eventLog().events().size();

  JsonArray i2c = doc.createNestedArray("i2c_devices");
  for (const DetectedDevice& d : core_->detectedDevices()) {
    JsonObject o = i2c.createNestedObject();
    char addr[8];
    snprintf(addr, sizeof(addr), "0x%02X", d.address);
    o["address"] = addr;
    o["model"] = d.model;
  }

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onSensors() {
  DynamicJsonDocument doc(2048);

  JsonArray catalog = doc.createNestedArray("catalog");
  std::vector<SensorInfo> info;
  core_->sensors().describe(info);
  for (const SensorInfo& s : info) {
    JsonObject o = catalog.createNestedObject();
    o["id"] = s.id;
    o["model"] = s.model;
    o["interface"] = s.interface;
    o["healthy"] = s.healthy;
  }

  JsonArray arr = doc.createNestedArray("measurements");
  for (const Measurement& m : core_->sensors().measurements()) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onHistory() {
  size_t limit = 50;
  if (server_.hasArg("limit")) {
    const long l = server_.arg("limit").toInt();
    if (l > 0 && l <= 100) {
      limit = static_cast<size_t>(l);
    }
  }

  std::deque<Measurement> items;
  core_->history().readRecent(items, limit);

  DynamicJsonDocument doc(16384);
  JsonArray arr = doc.createNestedArray("history");
  for (const Measurement& m : items) {
    JsonObject o = arr.createNestedObject();
    o["ts"] = m.timestamp;
    o["sensor"] = m.sensorId;
    o["channel"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["seq"] = m.sequence;
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onEvents() {
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("events");
  for (const Event& e : core_->eventLog().events()) {
    JsonObject o = arr.createNestedObject();
    o["ts"] = e.timestampMs;
    o["type"] = eventTypeName(e.type);
    o["source"] = e.source;
    o["rule"] = e.correlationId;
    o["severity"] = severityName(e.severity);
    o["value"] = e.value;
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onAlarms() {
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("alarms");
  for (const Event& e : core_->eventLog().events()) {
    if (e.type != EventType::Alarm) {
      continue;
    }
    JsonObject o = arr.createNestedObject();
    o["ts"] = e.timestampMs;
    o["source"] = e.source;
    o["rule"] = e.correlationId;
    o["severity"] = severityName(e.severity);
    o["value"] = e.value;
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onNotFound() {
  server_.send(404, "application/json", "{\"error\":\"not found\"}");
}

}  // namespace sema
