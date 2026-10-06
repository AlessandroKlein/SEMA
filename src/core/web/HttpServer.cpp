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

static const uint32_t kSessionTimeoutMs = 3600000UL;  // 1 h de sesión (deslizante)

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
  server_.on("/api/v1/gpio", HTTP_GET, [this]() { onGpio(); });
  server_.on("/api/v1/gpio", HTTP_POST, [this]() { onGpioWrite(); });
  server_.on("/api/v1/shift", HTTP_GET, [this]() { onShift(); });
  server_.on("/api/v1/shift", HTTP_POST, [this]() { onShiftWrite(); });
#if SEMA_USE_MODBUS
  server_.on("/api/v1/modbus", HTTP_GET, [this]() { onModbus(); });
#endif
#if SEMA_USE_CAN
  server_.on("/api/v1/can", HTTP_GET, [this]() { onCan(); });
  server_.on("/api/v1/can", HTTP_POST, [this]() { onCanWrite(); });
#endif
#if SEMA_USE_LORA
  server_.on("/api/v1/lora", HTTP_GET, [this]() { onLora(); });
  server_.on("/api/v1/lora", HTTP_POST, [this]() { onLoraWrite(); });
#endif
#if SEMA_USE_ZIGBEE
  server_.on("/api/v1/zigbee", HTTP_GET, [this]() { onZigbee(); });
  server_.on("/api/v1/zigbee", HTTP_POST, [this]() { onZigbeeWrite(); });
#endif
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
input{display:block;width:100%;box-sizing:border-box;padding:.4rem;margin:.3rem 0;border:1px solid #30363d;border-radius:4px;background:#0d1117;color:#e6edf3}
button{width:100%;padding:.5rem;border:0;border-radius:4px;background:#1f6feb;color:#fff;cursor:pointer;margin-top:.4rem}
</style>
</head>
<body>
<h1>SEMA</h1>
<div id="status" class="muted">Cargando…</div>
<h2>Sensores</h2>
<table><thead><tr><th>Sensor</th><th>Canal</th><th>Valor</th><th>Unidad</th><th>Calidad</th></tr></thead>
<tbody id="rows"><tr><td colspan="5" class="muted">Cargando…</td></tr></tbody></table>
<h2>Histórico</h2>
<canvas id="chart" width="600" height="160" style="max-width:100%;border:1px solid #30363d;border-radius:4px;margin-bottom:.5rem"></canvas>
<table><thead><tr><th>Fecha</th><th>Sensor</th><th>Canal</th><th>Valor</th></tr></thead>
<tbody id="hist"><tr><td colspan="4" class="muted">Cargando…</td></tr></tbody></table>
<h2>Configuración</h2>
<form onsubmit="saveConfig();return false;">
<input id="cfg_name" placeholder="Nombre de la estación">
<input id="cfg_ssid" placeholder="WiFi SSID">
<input id="cfg_pass" type="password" placeholder="WiFi contraseña">
<input id="cfg_host" placeholder="Hostname (mDNS)">
<input id="cfg_apikey" type="password" placeholder="API key (web)">
<input id="cfg_serverkey" type="password" placeholder="Server key (Central)">
<button type="submit">Guardar</button>
</form>
<a href="/logout" style="display:inline-block;margin-top:1rem;color:#8b949e">Cerrar sesión</a>
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
function drawChart(vals){
  const cv=document.getElementById('chart');
  if(!cv)return;
  const ctx=cv.getContext('2d');
  ctx.clearRect(0,0,cv.width,cv.height);
  if(vals.length<2)return;
  const w=cv.width,h=cv.height,pad=16;
  const min=Math.min.apply(null,vals),max=Math.max.apply(null,vals);
  const range=(max-min)||1;
  ctx.strokeStyle='#30363d';
  ctx.beginPath();
  for(let i=0;i<=3;i++){const y=pad+(h-2*pad)*i/3;ctx.moveTo(pad,y);ctx.lineTo(w-pad,y);}
  ctx.stroke();
  ctx.strokeStyle='#58a6ff';ctx.lineWidth=2;
  ctx.beginPath();
  for(let i=0;i<vals.length;i++){
    const x=pad+(w-2*pad)*i/(vals.length-1);
    const y=pad+(h-2*pad)*(1-(vals[i]-min)/range);
    i?ctx.lineTo(x,y):ctx.moveTo(x,y);
  }
  ctx.stroke();
}
async function loadHistory(){
  try{
    const r=await(await fetch('/api/v1/history?limit=100')).json();
    const items=r.history||[];
    let h='';
    for(const m of items.slice(-20)){
      const d=new Date(m.ts*1000);
      const t=(m.ts>1000000000)?d.toLocaleString():('uptime '+m.ts+' s');
      h+='<tr><td>'+t+'</td><td>'+m.sensor+'</td><td>'+m.channel+'</td><td>'+m.value+' '+m.unit+'</td></tr>';
    }
    document.getElementById('hist').innerHTML=h||'<tr><td colspan="4" class="muted">Sin datos</td></tr>';
    const temps=items.filter(m=>m.channel==='temperature').map(m=>m.value);
    drawChart(temps);
  }catch(e){}
}
loadHistory();
setInterval(loadHistory,10000);
let cfg={};
async function loadConfig(){
  try{
    const r=await(await fetch('/api/v1/config')).json();
    cfg=r;
    document.getElementById('cfg_name').value=r.station?r.station.name:'';
    document.getElementById('cfg_ssid').value=r.network?r.network.ssid:'';
    document.getElementById('cfg_pass').value=r.network?r.network.password:'';
    document.getElementById('cfg_host').value=r.network?r.network.hostname:'';
    document.getElementById('cfg_apikey').value=r.security?r.security.api_key:'';
    document.getElementById('cfg_serverkey').value=r.security?r.security.server_key:'';
  }catch(e){}
}
async function saveConfig(){
  cfg.station=cfg.station||{};cfg.station.name=document.getElementById('cfg_name').value;
  cfg.network=cfg.network||{};cfg.network.ssid=document.getElementById('cfg_ssid').value;
  cfg.network.password=document.getElementById('cfg_pass').value;
  cfg.network.hostname=document.getElementById('cfg_host').value;
  cfg.security=cfg.security||{};cfg.security.api_key=document.getElementById('cfg_apikey').value;
  cfg.security.server_key=document.getElementById('cfg_serverkey').value;
  try{
    const resp=await fetch('/api/v1/config',{method:'PUT',headers:{'Content-Type':'application/json'},body:JSON.stringify(cfg)});
    alert(resp.ok?'Guardado':'Error al guardar');
  }catch(e){alert('Error de red');}
}
loadConfig();
refresh();
setInterval(refresh,5000);
</script>
</body>
</html>
)html";
  server_.send(200, "text/html", kIndexHtml);
}

bool HttpServer::sessionAuthorized() {
  if (sessionStartMs_ == 0 || millis() - sessionStartMs_ > kSessionTimeoutMs) {
    return false;
  }
  if (!server_.hasHeader("Cookie")) {
    return false;
  }
  const String cookie = server_.header("Cookie");
  if (cookie.indexOf("sema_auth=" + sessionToken_) < 0) {
    return false;
  }
  sessionStartMs_ = millis();  // sesión deslizante: refresca al validar
  return true;
}

void HttpServer::onLoginPost() {
  const uint32_t now = millis();
  // Rate limiting (D-0048): bloquea tras 5 intentos fallidos durante 60 s.
  if (now < lockoutUntilMs_) {
    server_.send(429, "text/plain", "Demasiados intentos. Reintentá más tarde.");
    return;
  }

  const String password = server_.arg("password");
  const SecurityConfig& sec = core_->config().get().security;
  const bool ok = (sec.apiKey.length() > 0 && password == sec.apiKey) ||
                  (sec.serverKey.length() > 0 && password == sec.serverKey);
  if (ok) {
    failedLogins_ = 0;
    lockoutUntilMs_ = 0;
    sessionStartMs_ = millis();
    server_.sendHeader("Set-Cookie", "sema_auth=" + sessionToken_ + "; Path=/; HttpOnly; SameSite=Strict");
    server_.sendHeader("Location", "/");
    server_.send(302, "text/plain", "");
  } else {
    ++failedLogins_;
    if (failedLogins_ >= 5) {
      failedLogins_ = 0;
      lockoutUntilMs_ = now + 60000;
    }
    server_.send(401, "text/html", kLoginHtml);
  }
}

void HttpServer::onLogout() {
  sessionStartMs_ = 0;
  server_.sendHeader("Set-Cookie", "sema_auth=; Path=/; Max-Age=0; SameSite=Strict");
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
  DynamicJsonDocument doc(384);
  doc["id"] = core_->config().get().station.id;
  doc["name"] = core_->config().get().station.name;
  doc["firmware"] = SEMA_FW_VERSION;
  doc["hw"] = SEMA_HW_VERSION;
  doc["config_schema"] = SEMA_CONFIG_SCHEMA_VERSION;
  doc["protocol"] = SEMA_PROTOCOL_VERSION;
  doc["board"] = SEMA_BOARD_ID;
  doc["flash_mb"] = SEMA_FLASH_MB;
  doc["firmware_file"] =
      String("sema_") + SEMA_FW_VERSION + "_" + SEMA_BOARD_ID + ".bin";
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
    // Re-aplica la config sin reiniciar (sensores primero, luego el resto).
    core_->applySensors();
    core_->applyRules();
    core_->applyCalibrations();
    core_->applyGpio();
    core_->applyPublishers();
    core_->applyShift();
#if SEMA_USE_MODBUS
    core_->applyModbus();
#endif
#if SEMA_USE_CAN
    core_->applyCan();
#endif
#if SEMA_USE_LORA
    core_->applyLora();
#endif
#if SEMA_USE_ZIGBEE
    core_->applyZigbee();
#endif
#if SEMA_USE_ETHERNET
    core_->applyEthernet();
#endif
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
  DynamicJsonDocument doc(384);
  doc["mode"] = core_->wifi().isAp() ? "AP" : "STA";
  doc["connected"] = core_->wifi().connected();
  doc["ip"] = core_->wifi().localIP();
  doc["rssi"] = core_->wifi().rssi();
  doc["ethernet"]["enabled"] = core_->ethernet().enabled();
  doc["ethernet"]["connected"] = core_->ethernet().connected();
  doc["ethernet"]["ip"] = core_->ethernet().localIP();
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

void HttpServer::onGpio() {
  DynamicJsonDocument doc(1024);
  JsonArray arr = doc.createNestedArray("gpio");
  const GpioManager& gpio = core_->gpio();
  for (const GpioSpec& s : gpio.specs()) {
    JsonObject o = arr.createNestedObject();
    o["id"] = s.id;
    o["pin"] = s.pin;
    o["mode"] = s.mode;
    o["value"] = gpio.read(s.pin);
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onGpioWrite() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  const uint8_t pin = doc["pin"] | 0;
  const int value = doc["value"] | 0;
  core_->gpio().write(pin, value);
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onShift() {
  DynamicJsonDocument doc(256);
  doc["type"] = core_->shift().config().type;
  doc["value"] = core_->shift().readByte();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onShiftWrite() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  const uint8_t value = doc["value"] | 0;
  core_->shift().writeByte(value);
  server_.send(200, "application/json", "{\"ok\":true}");
}

#if SEMA_USE_MODBUS
void HttpServer::onModbus() {
  const uint8_t result = core_->modbus().read();
  DynamicJsonDocument doc(1024);
  doc["ready"] = core_->modbus().ready();
  doc["result"] = result;
  JsonArray arr = doc.createNestedArray("values");
  for (uint16_t v : core_->modbus().values()) {
    arr.add(v);
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}
#endif

#if SEMA_USE_CAN
void HttpServer::onCan() {
  uint32_t id = 0;
  uint8_t dlc = 0;
  bool extd = false;
  uint8_t data[8] = {0};
  const bool got = core_->can().receive(id, data, dlc, extd);

  DynamicJsonDocument doc(256);
  doc["ready"] = core_->can().ready();
  doc["received"] = got;
  if (got) {
    doc["id"] = id;
    doc["extd"] = extd;
    JsonArray arr = doc.createNestedArray("data");
    for (uint8_t i = 0; i < dlc; ++i) {
      arr.add(data[i]);
    }
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onCanWrite() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  const uint32_t id = doc["id"] | 0;
  const bool extd = doc["extd"] | false;
  uint8_t data[8] = {0};
  uint8_t dlc = 0;
  JsonArray arr = doc["data"].as<JsonArray>();
  for (JsonVariant v : arr) {
    if (dlc >= 8) break;
    data[dlc++] = v.as<uint8_t>();
  }
  const bool ok = core_->can().send(id, data, dlc, extd);
  server_.send(ok ? 200 : 500, "application/json",
               ok ? "{\"ok\":true}" : "{\"error\":\"send failed\"}");
}
#endif

#if SEMA_USE_LORA
void HttpServer::onLora() {
  uint8_t buf[64] = {0};
  const uint8_t n = core_->lora().receive(buf, sizeof(buf));

  DynamicJsonDocument doc(512);
  doc["ready"] = core_->lora().ready();
  doc["received"] = n > 0;
  if (n > 0) {
    JsonArray arr = doc.createNestedArray("data");
    for (uint8_t i = 0; i < n; ++i) {
      arr.add(buf[i]);
    }
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onLoraWrite() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  uint8_t buf[64] = {0};
  uint8_t len = 0;
  JsonArray arr = doc["data"].as<JsonArray>();
  for (JsonVariant v : arr) {
    if (len >= sizeof(buf)) break;
    buf[len++] = v.as<uint8_t>();
  }
  const bool ok = core_->lora().send(buf, len);
  server_.send(ok ? 200 : 500, "application/json",
               ok ? "{\"ok\":true}" : "{\"error\":\"send failed\"}");
}
#endif

#if SEMA_USE_ZIGBEE
void HttpServer::onZigbee() {
  uint8_t buf[128] = {0};
  uint8_t len = 0;
  if (core_->zigbee().available()) {
    core_->zigbee().takeMessage(buf, sizeof(buf), len);
  }

  DynamicJsonDocument doc(512);
  doc["ready"] = core_->zigbee().ready();
  doc["received"] = len > 0;
  doc["src"] = core_->zigbee().lastSrc();
  if (len > 0) {
    JsonArray arr = doc.createNestedArray("data");
    for (uint8_t i = 0; i < len; ++i) {
      arr.add(buf[i]);
    }
  }
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onZigbeeWrite() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  const uint16_t destination = doc["destination"] | 0;
  uint8_t buf[110] = {0};
  uint8_t len = 0;
  JsonArray arr = doc["data"].as<JsonArray>();
  for (JsonVariant v : arr) {
    if (len >= sizeof(buf)) break;
    buf[len++] = v.as<uint8_t>();
  }
  const bool ok = core_->zigbee().send(destination, buf, len);
  server_.send(ok ? 200 : 500, "application/json",
               ok ? "{\"ok\":true}" : "{\"error\":\"send failed\"}");
}
#endif

void HttpServer::onNotFound() {
  server_.send(404, "application/json", "{\"error\":\"not found\"}");
}

}  // namespace sema
