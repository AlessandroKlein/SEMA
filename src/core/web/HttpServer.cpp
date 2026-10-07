#include "core/web/HttpServer.hpp"

#include <ArduinoJson.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <mbedtls/md.h>
#include <LittleFS.h>

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
<input type="text" name="username" placeholder="Usuario" value="admin">
<input type="password" name="password" placeholder="Contraseña" autofocus>
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
  server_.on("/api/v1/config/network", HTTP_POST, [this]() { onConfigNetwork(); });
  server_.on("/api/v1/wifi/scan", HTTP_GET, [this]() { onWifiScan(); });
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
  server_.on("/api/v1/wind/north", HTTP_POST, [this]() { onWindNorth(); });
  server_.on("/api/v1/wind/resistors", HTTP_POST, [this]() { onWindResistors(); });
  server_.on("/api/v1/dashboard/layout", HTTP_POST, [this]() { onDashboardLayout(); });
  server_.on("/gridstack.min.css", HTTP_GET, [this]() { onStaticFile("/gridstack.min.css", "text/css"); });
  server_.on("/gridstack-all.min.js", HTTP_GET, [this]() { onStaticFile("/gridstack-all.min.js", "application/javascript"); });
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
  const bool noPass = sec.password.length() == 0;
  if (!noPass && !sessionAuthorized()) {
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
<link rel="stylesheet" href="/gridstack.min.css">
<style>
body{font-family:system-ui,sans-serif;margin:1rem;background:#0d1117;color:#e6edf3}
h1{margin:0 0 .25rem}h2{margin:1.25rem 0 .5rem}
.muted{color:#8b949e}
a{color:#8b949e}
.card{height:100%;box-sizing:border-box;background:#161b22;border:1px solid #30363d;border-radius:6px;padding:.5rem;position:relative}
.card .t{font-size:.72rem;color:#8b949e;text-transform:uppercase}
.card .v{font-size:1.35rem;font-weight:600;margin:.1rem 0}
.card .v span{font-size:.75rem;color:#8b949e;font-weight:400}
.card .s{font-size:.68rem;color:#58a6ff}
.card .del{position:absolute;top:.2rem;right:.2rem;background:#30363d;color:#f85149;border:0;border-radius:4px;width:22px;height:22px;line-height:1;cursor:pointer;font-size:.8rem;padding:0}
canvas.chart{width:100%;height:100%;display:block}
.grid-stack{background:#0d1117}
.grid-stack>.grid-stack-item>.grid-stack-item-content{overflow:hidden}
input,button{box-sizing:border-box;padding:.45rem;margin:.3rem 0;border:1px solid #30363d;border-radius:4px;background:#0d1117;color:#e6edf3;font-size:.9rem}
input{display:block;width:100%}
button{width:100%;background:#1f6feb;color:#fff;border:0;cursor:pointer}
button.sec{background:#30363d}
.bar{display:flex;gap:.5rem;flex-wrap:wrap;margin:.75rem 0}
.bar button{flex:1;min-width:130px}
.grid-wind{display:grid;grid-template-columns:repeat(4,1fr);gap:.4rem}
label{font-size:.75rem;color:#8b949e;display:block}
section{border:1px solid #30363d;border-radius:8px;padding:1rem;margin:1rem 0}
.modal{position:fixed;inset:0;background:rgba(0,0,0,.6);display:none;z-index:50;overflow:auto}
.modal.open{display:block}
.modal-box{background:#0d1117;border:1px solid #30363d;border-radius:8px;max-width:520px;margin:2rem auto;padding:1rem}
.catalog{max-height:60vh;overflow:auto;border:1px solid #30363d;border-radius:6px;padding:.5rem}
.cat-item{display:flex;justify-content:space-between;align-items:center;padding:.5rem;border-bottom:1px solid #21262d;cursor:pointer}
.cat-item:hover{background:#161b22}
.cat-item .add{background:#238636;border:0;border-radius:4px;color:#fff;padding:.2rem .6rem;cursor:pointer;width:auto}
.cat-group{font-size:.72rem;color:#8b949e;text-transform:uppercase;margin:.6rem 0 .2rem}
.range{display:flex;gap:2px;margin:.2rem 0}
.rbtn{width:auto;padding:.1rem .5rem;font-size:.68rem;background:#21262d;border:1px solid #30363d;border-radius:3px;cursor:pointer;margin:0;color:#8b949e}
.rbtn.on{background:#1f6feb;color:#fff;border-color:#1f6feb}
.legend{display:flex;flex-wrap:wrap;gap:.5rem;font-size:.68rem;color:#8b949e;margin-top:.2rem}
body.light{background:#f6f8fa;color:#24292f}
body.light .card{background:#ffffff;border-color:#d0d7de}
body.light input{background:#ffffff;color:#24292f;border-color:#d0d7de}
body.light .muted,body.light a{color:#57606a}
body.light section{border-color:#d0d7de}
body.light .cat-item:hover{background:#f6f8fa}
</style>
<script src="/gridstack-all.min.js"></script>
</head>
<body>
<h1>SEMA</h1>
<div id="status" class="muted">Cargando…</div>

<nav style="display:flex;gap:.6rem;flex-wrap:wrap;margin:.5rem 0;padding-bottom:.5rem;border-bottom:1px solid #30363d">
  <a href="#grid" style="color:#8b949e;text-decoration:none">📊 Dashboard</a>
  <a href="#wind" style="color:#8b949e;text-decoration:none">🧭 Veleta</a>
  <a href="#net" style="color:#8b949e;text-decoration:none">🌐 Red</a>
  <a href="#sec" style="color:#8b949e;text-decoration:none">🔐 Seguridad</a>
  <a href="/logout" style="color:#f85149;text-decoration:none">Salir</a>
</nav>

<div class="bar" id="grid">
  <button onclick="openCatalog()">➕ Añadir tarjeta</button>
  <button class="sec" onclick="toggleEdit()">✏️ Editar layout</button>
  <button class="sec" onclick="saveLayout()">💾 Guardar layout</button>
  <button class="sec" onclick="calibrateNorth()">🧭 Norte de la veleta</button>
  <button class="sec" onclick="location.href='/api/v1/history?limit=3000&format=csv'">⬇️ CSV</button>
  <button class="sec" onclick="toggleTheme()">🌓 Tema</button>
</div>

<div class="grid-stack" id="grid"></div>

<div class="modal" id="modal">
  <div class="modal-box">
    <h2>Añadir tarjeta</h2>
    <p class="muted">Elegí qué mostrar. Los gráficos muestran la última hora.</p>
    <div class="catalog" id="catalog"></div>
    <button class="sec" onclick="closeCatalog()">Cerrar</button>
  </div>
</div>

<section id="wind">
<h2>Calibración de la veleta (WH-SP-WD)</h2>
<p class="muted">Ingresá los valores de las 8 resistencias en el orden del datasheet (empezando por N), y el pull-up. Las 16 posiciones (8 directas + 8 en paralelo) se calculan automáticamente.</p>
<div class="grid-wind" id="windInputs"></div>
<div style="max-width:260px"><label>Resistencia pull-up (Ω)</label><input id="wrp" type="number" step="1" value="10000"></div>
<button onclick="saveWind()">Guardar resistencias</button>
</section>

<section id="net">
<h2>Red (WiFi)</h2>
<button type="button" class="sec" onclick="scanWifi()">🔍 Buscar redes</button>
<div id="wifiList"></div>
<form onsubmit="saveNetwork();return false;">
<select id="cfg_mode">
  <option value="STA">Estación (conectarse a un router)</option>
  <option value="AP">Punto de acceso (AP propio)</option>
</select>
<input id="cfg_ssid" placeholder="WiFi SSID">
<input id="cfg_pass" type="password" placeholder="WiFi contraseña">
<input id="cfg_host" placeholder="Hostname (mDNS)">
<button type="submit">Guardar red (reinicia)</button>
</form>
</section>

<section id="sec">
<h2>Estación y seguridad</h2>
<form onsubmit="saveConfig();return false;">
<input id="cfg_name" placeholder="Nombre de la estación">
<input id="cfg_user" placeholder="Usuario del login (vacío = admin)">
<input id="cfg_loginpass" type="password" placeholder="Contraseña del login (vacío = sin login)">
<input id="cfg_apikey" type="password" placeholder="API key (web)">
<input id="cfg_serverkey" type="password" placeholder="Server key (Central)">
<button type="submit">Guardar</button>
</form>
<a href="/logout" style="display:inline-block;margin-top:.5rem">Cerrar sesión</a>
</section>

<script>
const DIRS=['N','NE','E','SE','S','SO','O','NO'];
const COLORS=['#58a6ff','#f0883e','#3fb950','#d29922','#bc8cff','#ff7b72','#56d4dd','#79c0ff'];
let grid=null, editMode=false, cfg={}, layout=[], lastMeasurements=[];

function cid(type,k){ return type+'_'+String(k).replace(/[|]/g,'~'); }
function mkey(m){ return m.sensor_id+'|'+m.channel_id; }

function setEdit(on){
  editMode=on;
  if(!grid)return;
  grid.setStatic(!on);
  grid.enableMove(on);
  grid.enableResize(on);
}
function toggleEdit(){ setEdit(!editMode); }
function toggleTheme(){
  document.body.classList.toggle('light');
  try{localStorage.setItem('sema_theme', document.body.classList.contains('light')?'light':'dark');}catch(e){}
}
try{if(localStorage.getItem('sema_theme')==='light')document.body.classList.add('light');}catch(e){}
async function scanWifi(){
  document.getElementById('wifiList').innerHTML='<p class="muted">Escaneando…</p>';
  try{
    const r=await(await fetch('/api/v1/wifi/scan')).json();
    const nets=(r.networks||[]).sort((a,b)=>b.rssi-a.rssi);
    let h='<div class="catalog">';
    for(const n of nets){
      h+='<div class="cat-item"><span>'+n.ssid+' <span class="muted">('+n.rssi+' dBm'+(n.secure?' 🔒':'')+')</span></span><button class="add" data-ssid="'+n.ssid.replace(/"/g,'&quot;')+'" onclick="pickSsid(this)">Usar</button></div>';
    }
    h+='</div>';
    document.getElementById('wifiList').innerHTML=h||'<p class="muted">Sin redes</p>';
  }catch(e){document.getElementById('wifiList').innerHTML='<p class="muted">Error al escanear</p>';}
}
function pickSsid(btn){
  document.getElementById('cfg_ssid').value=btn.getAttribute('data-ssid');
  document.getElementById('cfg_pass').focus();
}

function defaultLayout(ms){
  const lay=[];
  const chans=[...new Set(ms.map(m=>m.measurement))];
  if(chans.includes('temperature')) lay.push({type:'chart',key:'temperature',x:0,y:0,w:6,h:3});
  let x=6,y=0;
  for(const m of ms){
    if(x+3>12){x=0;y++;}
    lay.push({type:'value',key:mkey(m),x:x,y:y,w:3,h:1});
    x+=3;
  }
  return lay;
}

function findValue(key){
  return lastMeasurements.find(m=>mkey(m)===key);
}

function cardContent(it){
  if(it.type==='chart'){
    return '<div class="card"><div class="t">📈 '+it.key+'</div>'+
           '<div class="range">'+
             '<button class="rbtn on" data-r="1" onclick="setRange(this,\''+it.key+'\')">1h</button>'+
             '<button class="rbtn" data-r="24" onclick="setRange(this,\''+it.key+'\')">24h</button>'+
             '<button class="rbtn" data-r="168" onclick="setRange(this,\''+it.key+'\')">7d</button>'+
           '</div>'+
           '<canvas class="chart" data-key="'+it.key+'" data-range="1"></canvas>'+
           '<div class="legend" data-key="'+it.key+'"></div>'+
           '<button class="del" onclick="deleteCard(\''+cid(it.type,it.key)+'\')">✕</button></div>';
  }
  const m=findValue(it.key);
  const v=m?(+m.value).toFixed(2):'—';
  const u=m?m.unit:'';
  const q=m?m.quality:'';
  const nm=m?m.measurement:it.key;
  return '<div class="card"><div class="t">'+nm+'</div>'+
         '<div class="v">'+v+' <span>'+u+'</span></div>'+
         '<div class="s">'+(m?m.sensor_id:'')+' · '+q+'</div>'+
         '<button class="del" onclick="deleteCard(\''+cid(it.type,it.key)+'\')">✕</button></div>';
}

function renderCards(){
  if(!grid){
    grid=GridStack.init({column:12, cellHeight:72, margin:6});
  }
  grid.removeAll(false);
  for(const it of layout){
    grid.addWidget({
      id:cid(it.type,it.key), x:it.x, y:it.y, w:it.w, h:it.h,
      content:cardContent(it)
    });
  }
  setEdit(editMode);
  drawCharts();
}

function fmtTime(ts){
  if(ts>1000000000){const d=new Date(ts*1000);return d.toLocaleTimeString([],{hour:'2-digit',minute:'2-digit'});}
  return '+'+(ts/60).toFixed(0)+'m';
}

function drawChartCard(cv, key, rangeH){
  const ctx=cv.getContext('2d');
  const dpr=window.devicePixelRatio||1;
  const w=cv.clientWidth||cv.width||200, h=cv.clientHeight||cv.height||130;
  cv.width=w*dpr; cv.height=h*dpr; ctx.scale(dpr,dpr);
  ctx.clearRect(0,0,w,h);
  ctx.fillStyle='#8b949e'; ctx.font='10px system-ui';
  ctx.fillText('Cargando…',8,14);

  fetch('/api/v1/history?limit=3000').then(r=>r.json()).then(r=>{
    const now=Date.now()/1000;
    const since=now-rangeH*3600;
    const items=(r.history||[]).filter(m=>m.measurement===key && m.ts>=since);
    const sensors=[...new Set(items.map(m=>m.sensor))];
    const unit=(items[0]||{}).unit||'';
    ctx.clearRect(0,0,w,h);

    if(items.length<2){ ctx.fillStyle='#8b949e'; ctx.font='10px system-ui'; ctx.fillText('Sin datos suficientes',8,14); return; }

    const padL=36,padR=8,padT=8,padB=18;
    const pw=w-padL-padR, ph=h-padT-padB;
    const vals=items.map(m=>m.value);
    let min=Math.min.apply(null,vals), max=Math.max.apply(null,vals);
    if(max===min){max=min+1;}
    const range=max-min;

    // Rejilla + eje Y (valores)
    ctx.strokeStyle='#21262d'; ctx.fillStyle='#8b949e'; ctx.font='9px system-ui';
    for(let i=0;i<=3;i++){
      const y=padT+ph*i/3;
      ctx.beginPath(); ctx.moveTo(padL,y); ctx.lineTo(w-padR,y); ctx.stroke();
      const v=max-range*i/3;
      ctx.fillText((Math.abs(v)>=100?v.toFixed(0):v.toFixed(1)),2,y+3);
    }
    // Eje X (tiempo)
    for(let i=0;i<=4;i++){
      const ts=since+(now-since)*i/4;
      const x=padL+pw*i/4;
      ctx.fillText(fmtTime(ts), x-12, h-5);
    }
    // Unidad (esquina superior izquierda)
    ctx.fillStyle='#8b949e'; ctx.font='9px system-ui';
    ctx.fillText(unit, 2, padT-1);

    // Series (una por sensor)
    sensors.forEach((s,si)=>{
      const series=items.filter(m=>m.sensor===s).sort((a,b)=>a.ts-b.ts);
      if(series.length<2) return;
      ctx.strokeStyle=COLORS[si%COLORS.length]; ctx.lineWidth=1.6; ctx.beginPath();
      series.forEach((m,i)=>{
        const x=padL+pw*(m.ts-since)/(now-since);
        const y=padT+ph*(1-(m.value-min)/range);
        i?ctx.lineTo(x,y):ctx.moveTo(x,y);
      });
      ctx.stroke();
    });

    // Leyenda
    const lg=document.querySelector('.legend[data-key="'+key+'"]');
    if(lg) lg.innerHTML=sensors.map((s,i)=>'<span style="color:'+COLORS[i%COLORS.length]+'">● '+s+'</span>').join(' ');
  }).catch(()=>{});
}

function setRange(btn, key){
  const card=btn.closest('.card');
  card.querySelectorAll('.rbtn').forEach(b=>b.classList.remove('on'));
  btn.classList.add('on');
  const cv=card.querySelector('canvas.chart');
  const r=parseInt(btn.getAttribute('data-r'),10);
  cv.setAttribute('data-range', r);
  drawChartCard(cv, key, r);
}

async function drawCharts(){
  document.querySelectorAll('canvas.chart').forEach(cv=>{
    const k=cv.getAttribute('data-key');
    const r=parseInt(cv.getAttribute('data-range')||'1',10);
    drawChartCard(cv, k, r);
  });
}

async function refresh(){
  try{
    const s=await(await fetch('/api/v1/status')).json();
    document.getElementById('status').textContent=s.name+' — v'+s.firmware+' — '+s.board;
  }catch(e){document.getElementById('status').textContent='Sin conexión';}
  try{
    const r=await(await fetch('/api/v1/sensors')).json();
    lastMeasurements=r.measurements||[];
    renderCards();
  }catch(e){}
}

function buildCatalog(){
  const c=document.getElementById('catalog');
  let h='<div class="cat-group">Gráficos (última hora)</div>';
  const chans=[...new Set(lastMeasurements.map(m=>m.measurement))];
  chans.forEach(ch=>{
    h+='<div class="cat-item"><span>📈 '+ch+'</span><button class="add" onclick="addCard(\'chart\',\''+ch+'\')">Añadir</button></div>';
  });
  h+='<div class="cat-group">Valores actuales</div>';
  lastMeasurements.forEach(m=>{
    h+='<div class="cat-item"><span>'+m.measurement+' <span class="muted">('+m.sensor_id+')</span></span><button class="add" onclick="addCard(\'value\',\''+mkey(m)+'\')">Añadir</button></div>';
  });
  c.innerHTML=h||'<div class="muted">Sin datos disponibles</div>';
}

function openCatalog(){ buildCatalog(); document.getElementById('modal').classList.add('open'); }
function closeCatalog(){ document.getElementById('modal').classList.remove('open'); }

function addCard(type,key){
  if(layout.some(it=>it.type===type && it.key===key)) return;
  let x=0,y=0;
  if(layout.length){ const l=layout[layout.length-1]; x=l.x+l.w; y=l.y; if(x+l.w>12){x=0;y=l.y+1;} }
  layout.push({type:type,key:key,x:x,y:y,w:type==='chart'?6:3,h:type==='chart'?3:1});
  closeCatalog();
  renderCards();
  saveLayout();
}

function deleteCard(id){
  layout=layout.filter(it=>cid(it.type,it.key)!==id);
  renderCards();
  saveLayout();
}

async function saveLayout(){
  if(!grid)return;
  const lay=grid.save(false).map(it=>{
    const l=layout.find(l=>cid(l.type,l.key)===it.id);
    return {type:l?l.type:'value', key:l?l.key:it.id, x:it.x, y:it.y, w:it.w, h:it.h};
  });
  layout=lay;
  try{
    await fetch('/api/v1/dashboard/layout',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(lay)});
  }catch(e){}
}

async function loadConfig(){
  try{
    const r=await(await fetch('/api/v1/config')).json();
    cfg=r;
    document.getElementById('cfg_name').value=r.station?r.station.name:'';
    document.getElementById('cfg_mode').value=r.network?r.network.mode:'STA';
    document.getElementById('cfg_ssid').value=r.network?r.network.ssid:'';
    document.getElementById('cfg_pass').value=r.network?r.network.password:'';
    document.getElementById('cfg_host').value=r.network?r.network.hostname:'';
    document.getElementById('cfg_apikey').value=r.security?r.security.api_key:'';
    document.getElementById('cfg_serverkey').value=r.security?r.security.server_key:'';
    document.getElementById('cfg_user').value=r.security?r.security.username:'';
    document.getElementById('cfg_loginpass').value=r.security?r.security.password:'';
    document.getElementById('wrp').value=r.system&&r.system.wind_rpull?r.system.wind_rpull:10000;
    const wr=(r.system&&r.system.wind_resistors)||[];
    DIRS.forEach((d,i)=>{
      const inp=document.getElementById('wr'+i);
      if(inp) inp.value=wr[i]!==undefined?wr[i]:(i===0?33000:i===1?8200:i===2?1000:i===3?2200:i===4?3900:i===5?16000:i===6?120000:64900);
    });
    if(r.system&&r.system.dashboard_layout){
      try{ const l=JSON.parse(r.system.dashboard_layout); if(Array.isArray(l)&&l.length) layout=l; }catch(e){}
    }
  }catch(e){}
}

async function saveNetwork(){
  const body={mode:document.getElementById('cfg_mode').value,ssid:document.getElementById('cfg_ssid').value,password:document.getElementById('cfg_pass').value,hostname:document.getElementById('cfg_host').value};
  try{
    const resp=await fetch('/api/v1/config/network',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
    alert(resp.ok?'Red guardada (reiniciá para aplicar)':'Error al guardar red');
  }catch(e){alert('Error de red');}
}

async function saveConfig(){
  cfg.station=cfg.station||{};cfg.station.name=document.getElementById('cfg_name').value;
  cfg.security=cfg.security||{};cfg.security.api_key=document.getElementById('cfg_apikey').value;
  cfg.security.server_key=document.getElementById('cfg_serverkey').value;
  cfg.security.username=document.getElementById('cfg_user').value;
  cfg.security.password=document.getElementById('cfg_loginpass').value;
  try{
    const resp=await fetch('/api/v1/config',{method:'PUT',headers:{'Content-Type':'application/json'},body:JSON.stringify(cfg)});
    alert(resp.ok?'Guardado':'Error al guardar');
  }catch(e){alert('Error de red');}
}

async function saveWind(){
  const resistors=DIRS.map((d,i)=>parseFloat(document.getElementById('wr'+i).value)||0);
  const rpull=parseFloat(document.getElementById('wrp').value)||10000;
  try{
    const resp=await fetch('/api/v1/wind/resistors',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({rpull:rpull,resistors:resistors})});
    alert(resp.ok?'Resistencias guardadas':'Error al guardar');
  }catch(e){alert('Error de red');}
}

async function calibrateNorth(){
  try{
    const resp=await fetch('/api/v1/wind/north',{method:'POST'});
    alert(resp.ok?'Norte calibrado (apuntá la veleta al norte y guardá)':'Error al calibrar norte');
  }catch(e){alert('Error de red');}
}

(function(){
  const c=document.getElementById('windInputs');
  DIRS.forEach((d,i)=>{
    const box=document.createElement('div');
    box.innerHTML='<label>R'+(i+1)+' — '+d+' (Ω)</label><input id="wr'+i+'" type="number" step="1" value="0">';
    c.appendChild(box);
  });
})();

async function boot(){
  await loadConfig();
  // Si no hay layout guardado, usar el por defecto al recibir la primera lectura.
  await refresh();
  if(!layout.length && lastMeasurements.length){
    layout=defaultLayout(lastMeasurements);
    renderCards();
    saveLayout();
  }
  setInterval(refresh,5000);
  setInterval(drawCharts,30000);
}
boot();
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

  const String user = server_.arg("username");
  const String password = server_.arg("password");
  const SecurityConfig& sec = core_->config().get().security;
  const String uname = sec.username.length() ? sec.username : "admin";
  const bool ok = (sec.password.length() > 0 && user == uname && password == sec.password);
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

void HttpServer::onConfigNetwork() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"body required\"}");
    return;
  }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  Config next = core_->config().get();
  if (doc.containsKey("mode")) next.network.mode = doc["mode"] | "STA";
  if (doc.containsKey("ssid")) next.network.ssid = doc["ssid"] | "";
  if (doc.containsKey("password")) next.network.password = doc["password"] | "";
  if (doc.containsKey("hostname")) next.network.hostname = doc["hostname"] | "";
  if (doc.containsKey("mdns")) next.network.mdns = doc["mdns"] | true;
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
  // Auto-reinicio para aplicar el cambio de red (AP → STA o viceversa).
  delay(300);
  ESP.restart();
}

void HttpServer::onWifiScan() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  const int n = WiFi.scanNetworks();
  DynamicJsonDocument doc(4096);
  JsonArray arr = doc.createNestedArray("networks");
  for (int i = 0; i < n && i < 40; ++i) {
    JsonObject o = arr.createNestedObject();
    o["ssid"] = WiFi.SSID(i);
    o["rssi"] = WiFi.RSSI(i);
    o["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
  }
  WiFi.scanDelete();
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
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

namespace {
// Calcula el SHA-256 (32 bytes) de la partición OTA recién escrita.
bool otaPartitionSha256(uint8_t out[32]) {
  const esp_partition_t* part = esp_ota_get_next_update_partition(nullptr);
  if (part == nullptr) {
    return false;
  }
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  if (mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0) != 0) {
    return false;
  }
  mbedtls_md_starts(&ctx);
  uint8_t buf[1024];
  for (size_t off = 0; off < part->size; off += sizeof(buf)) {
    const size_t n = (part->size - off < sizeof(buf)) ? (part->size - off) : sizeof(buf);
    if (esp_partition_read(part, off, buf, n) != ESP_OK) {
      mbedtls_md_free(&ctx);
      return false;
    }
    mbedtls_md_update(&ctx, buf, n);
  }
  mbedtls_md_finish(&ctx, out);
  mbedtls_md_free(&ctx);
  return true;
}

String toHex(const uint8_t* data, size_t len) {
  static const char* hex = "0123456789abcdef";
  String s;
  s.reserve(len * 2);
  for (size_t i = 0; i < len; ++i) {
    s += hex[(data[i] >> 4) & 0xF];
    s += hex[data[i] & 0xF];
  }
  return s;
}
}  // namespace

void HttpServer::onOtaUpload() {
  HTTPUpload& upload = server_.upload();
  if (upload.status == UPLOAD_FILE_START) {
    otaAuthorized_ = authorized();
    otaShaOk_ = true;
    otaExpectedSha_ = server_.hasHeader("X-SHA256") ? server_.header("X-SHA256") : "";
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
      // Verificación de integridad (opcional, cabecera X-SHA256).
      if (otaExpectedSha_.length() == 64) {
        uint8_t sha[32] = {0};
        if (otaPartitionSha256(sha)) {
          otaShaOk_ = (toHex(sha, 32) == otaExpectedSha_);
        } else {
          otaShaOk_ = false;
        }
      }
    }
  }
}

void HttpServer::onOta() {
  if (!otaAuthorized_) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    otaAuthorized_ = false;
    return;
  }
  if (!otaShaOk_) {
    server_.send(400, "application/json", "{\"error\":\"sha256 mismatch\"}");
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
  const String units = core_->config().get().system.units;
  const bool imperial = (units == "imperial");

  DynamicJsonDocument doc(4096);

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

  // Magnitudes derivadas (punto de rocío, índice de calor, QNH, VPD, AQI, …).
  std::vector<Measurement> derived;
  core_->derived().compute(core_->sensors().measurements(), derived, units);

  JsonArray arr = doc.createNestedArray("measurements");
  for (const Measurement& m : core_->sensors().measurements()) {
    String u;
    const float v = DerivedCalculator::convertUnit(m.value, m.measurement,
                                                   m.unit, imperial, u);
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = v;
    o["unit"] = u;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }
  for (const Measurement& m : derived) {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = m.sensorId;
    o["channel_id"] = m.channelId;
    o["measurement"] = m.measurement;
    o["value"] = m.value;
    o["unit"] = m.unit;
    o["quality"] = qualityName(m.quality);
    o["sequence"] = m.sequence;
  }

  doc["units"] = units;

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onWindNorth() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  const SystemConfig& sys = core_->config().get().system;
  if (sys.windDirectionPin == 0) {
    server_.send(400, "application/json",
                 "{\"error\":\"wind direction not configured\"}");
    return;
  }
  // Guarda el ángulo bruto actual como referencia de NORTE (auto-calibración).
  const uint16_t adc = analogRead(sys.windDirectionPin);
  const float rawAngle = DerivedCalculator::windVaneRawAngle(adc, sys);
  Config next = core_->config().get();
  next.system.windNorthOffset = rawAngle;
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }
  core_->derived().configure(core_->config().get().system);
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onWindResistors() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  Config next = core_->config().get();
  next.system.windRpull = doc["rpull"] | 10000.0f;
  JsonArray arr = doc["resistors"].as<JsonArray>();
  for (uint8_t i = 0; i < 8 && i < arr.size(); ++i) {
    next.system.windResistors[i] = arr[i].as<float>();
  }
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }
  core_->derived().configure(core_->config().get().system);
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onDashboardLayout() {
  if (!authorized() && !sessionAuthorized()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  Config next = core_->config().get();
  next.system.dashboardLayout = server_.arg("plain");
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onStaticFile(const char* path, const char* type) {
  if (!LittleFS.begin(false) || !LittleFS.exists(path)) {
    server_.send(404, "text/plain", "Not found");
    return;
  }
  File f = LittleFS.open(path, "r");
  if (!f) {
    server_.send(404, "text/plain", "Not found");
    return;
  }
  server_.streamFile(f, type);
  f.close();
}

void HttpServer::onHistory() {
  size_t limit = 50;
  if (server_.hasArg("limit")) {
    const long l = server_.arg("limit").toInt();
    if (l > 0 && l <= 3000) {
      limit = static_cast<size_t>(l);
    }
  }
  const String format = server_.hasArg("format") ? server_.arg("format") : "json";

  std::deque<Measurement> items;
  core_->history().readRecent(items, limit);

  if (format == "csv") {
    // Export CSV (descarga del histórico).
    String csv;
    csv.reserve(items.size() * 64 + 64);
    csv += "ts,sensor,channel,measurement,value,unit,quality,seq\n";
    for (const Measurement& m : items) {
      csv += String(m.timestamp) + "," + m.sensorId + "," + m.channelId + "," +
             m.measurement + "," + String(m.value, 4) + "," + m.unit + "," +
             qualityName(m.quality) + "," + String(m.sequence) + "\n";
    }
    server_.sendHeader("Content-Disposition",
                       "attachment; filename=sema_history.csv");
    server_.send(200, "text/csv", csv);
    return;
  }

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
