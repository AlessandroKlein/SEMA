#include "core/web/HttpServer.hpp"

#include <ArduinoJson.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <mbedtls/md.h>
#include <LittleFS.h>

#include "core/SemaCore.hpp"
#include "core/Time.hpp"

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
  server_.on("/config/network", HTTP_GET, [this]() { onNetworkPage(); });
  server_.on("/config/security", HTTP_GET, [this]() { onSecurityPage(); });
  server_.on("/config/system", HTTP_GET, [this]() { onSystemPage(); });
  server_.on("/config/wind", HTTP_GET, [this]() { onWindPage(); });
  server_.on("/config/sensors", HTTP_GET, [this]() { onSensorsPage(); });
  server_.on("/api/v1/status", HTTP_GET, [this]() { onStatus(); });
  server_.on("/api/v1/health", HTTP_GET, [this]() { onHealth(); });
  server_.on("/api/v1/system", HTTP_GET, [this]() { onSystem(); });
  server_.on("/api/v1/config", HTTP_GET, [this]() { onConfig(); });
  server_.on("/api/v1/config", HTTP_PUT, [this]() { onConfigPut(); });
  server_.on("/api/v1/config/network", HTTP_POST, [this]() { onConfigNetwork(); });
  server_.on("/api/v1/config/system", HTTP_POST, [this]() { onConfigSystem(); });
  server_.on("/api/v1/wifi/scan", HTTP_GET, [this]() { onWifiScan(); });
  server_.on("/api/v1/security/keys", HTTP_POST, [this]() { onApiKeys(); });
  server_.on("/api/v1/update/check", HTTP_GET, [this]() { onUpdateCheck(); });
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
  server_.on("/api/v1/dashboard/layout", HTTP_GET, [this]() { onDashboardLayoutGet(); });
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
:root{--bg:#0d1117;--fg:#e6edf3;--card:#161b22;--bd:#30363d;--muted:#8b949e;--acc:#1f6feb}
body{font-family:system-ui,sans-serif;margin:1rem;background:var(--bg);color:var(--fg)}
h1{margin:0 0 .25rem}h2{margin:1.25rem 0 .5rem}
.muted{color:var(--muted)}
a{color:var(--muted);text-decoration:none}
.card{height:100%;box-sizing:border-box;background:var(--card);border:1px solid var(--bd);border-radius:8px;padding:.5rem;position:relative}
.card .t{font-size:.72rem;color:var(--muted);text-transform:uppercase}
.card .v{font-size:1.35rem;font-weight:600;margin:.1rem 0}
.card .v span{font-size:.75rem;color:var(--muted);font-weight:400}
.card .s{font-size:.68rem;color:#58a6ff}
.card .del{position:absolute;top:.2rem;right:.2rem;background:#30363d;color:#f85149;border:0;border-radius:4px;width:22px;height:22px;line-height:1;cursor:pointer;font-size:.8rem;padding:0}
canvas.chart{width:100%;height:100%;display:block}
.grid-stack{background:var(--bg)}
.grid-stack>.grid-stack-item>.grid-stack-item-content{overflow:hidden}
input,button{box-sizing:border-box;padding:.45rem;margin:.3rem 0;border:1px solid var(--bd);border-radius:6px;background:var(--bg);color:var(--fg);font-size:.9rem}
input{display:block;width:100%}
button{width:100%;background:var(--acc);color:#fff;border:0;cursor:pointer;border-radius:6px}
button.sec{background:#21262d}
.bar{display:flex;gap:.5rem;flex-wrap:wrap;margin:.75rem 0}
.bar button{flex:1;min-width:130px}
.nav{display:flex;align-items:center;justify-content:center;gap:.5rem;padding:.55rem 1rem;margin:0 0 1rem;border:1px solid var(--bd);border-radius:12px;background:var(--card)}
.nav a{color:var(--muted);text-decoration:none;padding:.3rem .6rem;border-radius:7px;font-size:.9rem}
.nav a:hover{color:var(--fg);background:#21262d}
.nav .out{color:#f85149}
.nav-right{margin-left:auto;display:flex;gap:.4rem;align-items:center}
.grid-wind{display:grid;grid-template-columns:repeat(4,1fr);gap:.4rem}
label{font-size:.75rem;color:var(--muted);display:block}
section{border:1px solid var(--bd);border-radius:8px;padding:1rem;margin:1rem 0;background:var(--card)}
.modal{position:fixed;inset:0;background:rgba(0,0,0,.6);display:none;z-index:50;overflow:auto}
.modal.open{display:block}
.modal-box{background:var(--bg);border:1px solid var(--bd);border-radius:8px;max-width:520px;margin:2rem auto;padding:1rem}
.catalog{max-height:60vh;overflow:auto;border:1px solid var(--bd);border-radius:6px;padding:.5rem}
.cat-item{display:flex;justify-content:space-between;align-items:center;padding:.5rem;border-bottom:1px solid #21262d;cursor:pointer}
.cat-item:hover{background:var(--card)}
.cat-item .add{background:#238636;border:0;border-radius:4px;color:#fff;padding:.2rem .6rem;cursor:pointer;width:auto}
.cat-group{font-size:.72rem;color:var(--muted);text-transform:uppercase;margin:.6rem 0 .2rem}
.range{display:flex;gap:2px;margin:.2rem 0}
.rbtn{width:auto;padding:.1rem .5rem;font-size:.68rem;background:#21262d;border:1px solid var(--bd);border-radius:3px;cursor:pointer;margin:0;color:var(--muted)}
.rbtn.on{background:var(--acc);color:#fff;border-color:var(--acc)}
.legend{display:flex;flex-wrap:wrap;gap:.5rem;font-size:.68rem;color:var(--muted);margin-top:.2rem}
body.light{--bg:#f6f8fa;--fg:#24292f;--card:#fff;--bd:#d0d7de;--muted:#57606a}
body.light input{background:#fff;color:#24292f}
</style>
<script src="/gridstack-all.min.js"></script>
</head>
<body>
<h1>SEMA</h1>
<div id="status" class="muted">Cargando…</div>

<nav class="nav">
  <a href="/">📊 Dashboard</a>
  <a href="/config/network">🌐 Red</a>
  <a href="/config/sensors">🔌 Sensores</a>
  <a href="/config/security">🔐 Seguridad</a>
  <a href="/config/wind">🧭 Veleta</a>
  <a href="/config/system">⚙️ Sistema</a>
  <div class="nav-right">
    <button class="sec" onclick="toggleTheme()">🌓</button>
    <a href="/logout" class="out">Salir</a>
  </div>
</nav>

<div class="bar" id="grid">
  <button onclick="openCatalog()">➕ Añadir tarjeta</button>
  <button class="sec" onclick="toggleEdit()">✏️ Editar layout</button>
  <button class="sec" onclick="saveLayout()">💾 Guardar layout</button>
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

<script>
const DIRS=['N','NE','E','SE','S','SO','O','NO'];
const COLORS=['#58a6ff','#f0883e','#3fb950','#d29922','#bc8cff','#ff7b72','#56d4dd','#79c0ff'];
let grid=null, editMode=false, cfg={}, layout=[], lastMeasurements=[];

function cid(type,k){ return type+'_'+String(k).replace(/[|]/g,'~'); }
function mkey(m){ return m.sensor_id+'|'+m.measurement; }

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
    if(!nets.length){ document.getElementById('wifiList').innerHTML='<p class="muted">Sin redes encontradas</p>'; return; }
    let h='<div class="catalog">';
    for(const n of nets){
      h+='<div class="cat-item"><span>'+n.ssid+' <span class="muted">('+n.rssi+' dBm'+(n.secure?' 🔒':'')+')</span></span><button class="add" data-ssid="'+n.ssid.replace(/"/g,'&quot;')+'" onclick="pickSsid(this)">Usar</button></div>';
    }
    h+='</div>';
    document.getElementById('wifiList').innerHTML=h;
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
  const isClock=m&&m.measurement==='clock';
  const v=m?(isClock?new Date((+m.value)*1000).toLocaleTimeString():(+m.value).toFixed(2)):'—';
  const u=m&&!isClock?m.unit:'';
  const q=m?m.quality:'';
  const nm=m?m.measurement:it.key;
  return '<div class="card"><div class="t">'+nm+'</div>'+
         '<div class="v">'+v+' <span>'+u+'</span></div>'+
         '<div class="s">'+(m?m.sensor_id:'')+' · '+q+'</div>'+
         '<button class="del" onclick="deleteCard(\''+cid(it.type,it.key)+'\')">✕</button></div>';
}

function renderCards(){
  if(!grid){
    grid=GridStack.init({column:12, cellHeight:72, margin:6, float:false});
  }
  grid.removeAll();
  for(const it of layout){
    grid.addWidget({
      id:cid(it.type,it.key), x:it.x, y:it.y, w:it.w, h:it.h,
      content:cardContent(it)
    });
  }
  grid.compact();
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
    if(!grid){ renderCards(); }
    else { updateCards(); }
  }catch(e){}
}

function updateCards(){
  for(const it of layout){
    if(it.type!=='value') continue;
    const card=document.getElementById(cid(it.type,it.key));
    if(!card) continue;
    const m=findValue(it.key);
    const vEl=card.querySelector('.v');
    const sEl=card.querySelector('.s');
    if(vEl) vEl.innerHTML=(m?(+m.value).toFixed(2):'—')+' <span>'+(m?m.unit:'')+'</span>';
    if(sEl) sEl.innerHTML=(m?m.sensor_id:'')+' · '+(m?m.quality:'');
  }
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
  // y=99: se coloca al fondo; grid.compact() lo acomoda en el hueco libre.
  layout.push({type:type,key:key,x:0,y:99,w:type==='chart'?6:3,h:type==='chart'?3:1});
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
  }catch(e){}
  try{
    const lr=await(await fetch('/api/v1/dashboard/layout')).json();
    const l=JSON.parse(lr.layout||'[]');
    if(Array.isArray(l)&&l.length) layout=l;
  }catch(e){}
}

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
  setInterval(drawCharts,60000);
}
boot();
</script>
</body>
</html>
)html";
  server_.send(200, "text/html", kIndexHtml);
}

namespace {
const char kBaseCss[] PROGMEM = R"html(
<style>
:root{--bg:#0d1117;--fg:#e6edf3;--card:#161b22;--bd:#30363d;--muted:#8b949e;--acc:#1f6feb}
*{box-sizing:border-box}body{margin:0;padding:1rem;background:var(--bg);color:var(--fg);font-family:system-ui,sans-serif}
.wrap{max-width:820px;margin:0 auto}
h2{font-size:1.05rem;margin:0 0 .6rem;font-weight:600}
section{background:var(--card);border:1px solid var(--bd);border-radius:10px;padding:1.1rem;margin:0 auto 1rem;max-width:760px;box-shadow:0 1px 3px rgba(0,0,0,.12)}
input,select{width:100%;padding:.55rem .6rem;margin:.3rem 0;background:#0d1117;color:var(--fg);border:1px solid var(--bd);border-radius:7px;font-size:.9rem}
input:focus,select:focus{outline:none;border-color:var(--acc)}
button{background:var(--acc);color:#fff;border:0;padding:.55rem 1rem;border-radius:7px;cursor:pointer;margin:.3rem .3rem 0 0;font-size:.88rem}
button:hover{opacity:.9}
button.sec{background:#21262d}
button.theme{background:transparent;border:1px solid var(--bd);font-size:1rem}
.muted{color:var(--muted);font-size:.8rem}
a{color:var(--acc);text-decoration:none}
.nav{display:flex;align-items:center;justify-content:center;gap:.5rem;padding:.55rem 1rem;margin-bottom:1.2rem;border:1px solid var(--bd);border-radius:12px;background:var(--card);box-shadow:0 1px 3px rgba(0,0,0,.12)}
.nav-links{display:flex;gap:.25rem;flex-wrap:wrap;justify-content:center}
.nav a{color:var(--muted);text-decoration:none;padding:.3rem .6rem;border-radius:7px;font-size:.9rem}
.nav a:hover{color:var(--fg);background:#21262d}
.nav a.on{color:#fff;background:var(--acc)}
.nav .out{color:#f85149}
.nav-right{margin-left:auto;display:flex;gap:.4rem;align-items:center}
.row{display:grid;grid-template-columns:1fr 1fr;gap:.6rem}
.cat-item{display:flex;justify-content:space-between;align-items:center;gap:.5rem;padding:.5rem .6rem;border-bottom:1px solid var(--bd);font-size:.9rem}
.cat-item:last-child{border-bottom:0}
code{background:#21262d;padding:.1rem .4rem;border-radius:4px;font-size:.85em}
body.light{--bg:#f6f8fa;--fg:#24292f;--card:#fff;--bd:#d0d7de;--muted:#57606a}
body.light input{background:#fff;color:#24292f}
body.light code{background:#f0f3f6}
</style>
)html";

const char kNav[] PROGMEM = R"html(
<nav class="nav">
<div class="nav-links">
<a href="/">📊 Dashboard</a>
<a href="/config/network">🌐 Red</a>
<a href="/config/sensors">🔌 Sensores</a>
<a href="/config/security">🔐 Seguridad</a>
<a href="/config/wind">🧭 Veleta</a>
<a href="/config/system">⚙️ Sistema</a>
</div>
<div class="nav-right">
<button class="theme" onclick="toggleTheme()" title="Cambiar tema">🌓</button>
<a href="/logout" class="out">Salir</a>
</div>
</nav>
)html";

const char kThemeJs[] PROGMEM = R"html(
<script>
function toggleTheme(){document.body.classList.toggle('light');try{localStorage.setItem('sema_theme',document.body.classList.contains('light')?'light':'dark')}catch(e){}}
try{if(localStorage.getItem('sema_theme')==='light')document.body.classList.add('light')}catch(e){}
</script>
)html";

void serveAuthedPage(WebServer& srv, bool authed, const String& body) {
  if (!authed) {
    srv.send(200, "text/html", kLoginHtml);
    return;
  }
  String html = String("<!DOCTYPE html><html lang=\"es\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>SEMA</title>") +
                String(kBaseCss) + "</head><body>" + String(kNav) +
                "<div class=\"wrap\">" + body + "</div></body></html>";
  srv.send(200, "text/html", html);
}
}  // namespace

void HttpServer::onNetworkPage() {
  const bool authed = webAuthed();
  const String body = R"html(
<section><h2>Red (WiFi)</h2>
<button class="sec" onclick="scanWifi()">🔍 Buscar redes</button><div id="wifiList"></div>
<form onsubmit="saveNetwork();return false;">
<select id="cfg_mode"><option value="STA">Estación (conectarse a un router)</option><option value="AP">Punto de acceso (AP propio)</option></select>
<input id="cfg_ssid" placeholder="WiFi SSID">
<input id="cfg_pass" type="password" placeholder="WiFi contraseña">
<input id="cfg_host" placeholder="Hostname (mDNS)">
<button type="button" class="sec" onclick="openMdns()">🔗 Abrir http://&lt;hostname&gt;.local</button>
<div class="muted">IP estática (dejar vacío = DHCP):</div>
<div class="row">
<input id="cfg_ip" placeholder="IP (ej. 192.168.1.50)">
<input id="cfg_gateway" placeholder="Gateway (ej. 192.168.1.1)">
<input id="cfg_subnet" placeholder="Máscara (ej. 255.255.255.0)">
<input id="cfg_dns" placeholder="DNS (ej. 8.8.8.8)">
</div>
<button type="submit">Guardar red (reinicia)</button>
</form></section>
<script>
async function loadNet(){try{const r=await(await fetch('/api/v1/config')).json();document.getElementById('cfg_mode').value=r.network?r.network.mode:'STA';document.getElementById('cfg_ssid').value=r.network?r.network.ssid:'';document.getElementById('cfg_pass').value=r.network?r.network.password:'';document.getElementById('cfg_host').value=r.network?r.network.hostname:'';document.getElementById('cfg_ip').value=r.network?r.network.ip:'';document.getElementById('cfg_gateway').value=r.network?r.network.gateway:'';document.getElementById('cfg_subnet').value=r.network?r.network.subnet:'';document.getElementById('cfg_dns').value=r.network?r.network.dns:''}catch(e){}}
function openMdns(){const h=document.getElementById('cfg_host').value.trim();if(h)window.open('http://'+h+'.local','_blank');else alert('Poné un hostname primero')}
async function saveNetwork(){const b={mode:document.getElementById('cfg_mode').value,ssid:document.getElementById('cfg_ssid').value,password:document.getElementById('cfg_pass').value,hostname:document.getElementById('cfg_host').value,ip:document.getElementById('cfg_ip').value,gateway:document.getElementById('cfg_gateway').value,subnet:document.getElementById('cfg_subnet').value,dns:document.getElementById('cfg_dns').value};try{const r=await fetch('/api/v1/config/network',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(b)});alert(r.ok?'Guardado (reiniciando…)':'Error')}catch(e){alert('Error de red')}}
async function scanWifi(){document.getElementById('wifiList').innerHTML='<p class="muted">Escaneando…</p>';try{const r=await(await fetch('/api/v1/wifi/scan')).json();const n=(r.networks||[]).sort((a,b)=>b.rssi-a.rssi);if(!n.length){document.getElementById('wifiList').innerHTML='<p class="muted">Sin redes</p>';return}let h='';for(const x of n)h+='<div class="cat-item"><span>'+x.ssid+' <span class="muted">('+x.rssi+' dBm)</span></span><button data-ssid="'+x.ssid+'" onclick="pickSsid(this)">Usar</button></div>';document.getElementById('wifiList').innerHTML=h}catch(e){document.getElementById('wifiList').innerHTML='<p class="muted">Error al escanear</p>'}}
function pickSsid(b){document.getElementById('cfg_ssid').value=b.getAttribute('data-ssid');document.getElementById('cfg_pass').focus()}
loadNet();
</script>
)html";
  serveAuthedPage(server_, authed, body);
}

void HttpServer::onSecurityPage() {
  const bool authed = webAuthed();
  const String body = R"html(
<section><h2>Estación</h2>
<form onsubmit="saveStation();return false;">
<input id="cfg_name" placeholder="Nombre de la estación">
<button type="submit">Guardar</button>
</form></section>
<section><h2>Login (usuario/contraseña)</h2>
<form onsubmit="saveLogin();return false;">
<input id="cfg_user" placeholder="Usuario (vacío = admin)">
<input id="cfg_loginpass" type="password" placeholder="Contraseña (vacío = sin login)">
<button type="submit">Guardar login</button>
</form></section>
<section><h2>Claves API</h2>
<div class="muted">Claves adicionales con nombre (revocables).</div>
<div class="row"><input id="keyname" placeholder="Nombre (ej. Cliente 1)"><button class="sec" onclick="genKey()">➕ Generar</button></div>
<div id="keyList"></div>
<input id="cfg_apikey" type="password" placeholder="API key principal">
<input id="cfg_serverkey" type="password" placeholder="Server key (Central)">
<button onclick="saveKeys()">Guardar claves</button>
</section>
<script>
var cfg={};
async function loadSec(){try{const r=await(await fetch('/api/v1/config')).json();cfg=r;document.getElementById('cfg_name').value=r.station?r.station.name:'';document.getElementById('cfg_user').value=r.security?r.security.username:'';document.getElementById('cfg_loginpass').value=r.security?r.security.password:'';document.getElementById('cfg_apikey').value=r.security?r.security.api_key:'';document.getElementById('cfg_serverkey').value=r.security?r.security.server_key:'';renderKeys()}catch(e){}}
function renderKeys(){let h='';try{const k=JSON.parse(cfg.security.extra_keys||'{}');for(const n in k)h+='<div class="cat-item"><span>'+n+' <span class="muted">'+k[n]+'</span></span><button class="sec" onclick="revokeKey(this)" data-n="'+n+'">🗑️</button></div>'}catch(e){}document.getElementById('keyList').innerHTML=h||'<p class="muted">Sin claves adicionales</p>'}
async function genKey(){const n=document.getElementById('keyname').value.trim();if(!n)return alert('Poné un nombre');try{const r=await(await fetch('/api/v1/security/keys',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'generate',name:n})})).json();if(r.ok){alert('Clave generada: '+r.key);loadSec()}else alert('Error')}catch(e){alert('Error de red')}}
async function revokeKey(b){const n=b.getAttribute('data-n');try{const r=await(await fetch('/api/v1/security/keys',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'revoke',name:n})})).json();if(r.ok){loadSec()}else alert('Error')}catch(e){alert('Error de red')}}
async function saveStation(){cfg.station=cfg.station||{};cfg.station.name=document.getElementById('cfg_name').value;await save()}
async function saveLogin(){cfg.security=cfg.security||{};cfg.security.username=document.getElementById('cfg_user').value;cfg.security.password=document.getElementById('cfg_loginpass').value;await save()}
async function saveKeys(){cfg.security=cfg.security||{};cfg.security.api_key=document.getElementById('cfg_apikey').value;cfg.security.server_key=document.getElementById('cfg_serverkey').value;await save()}
async function save(){try{const r=await fetch('/api/v1/config',{method:'PUT',headers:{'Content-Type':'application/json'},body:JSON.stringify(cfg)});alert(r.ok?'Guardado':'Error')}catch(e){alert('Error de red')}}
loadSec();
</script>
)html";
  serveAuthedPage(server_, authed, body);
}

void HttpServer::onSystemPage() {
  const bool authed = webAuthed();
  const String body = R"html(
<section><h2>Sistema</h2>
<div id="status" class="muted">Cargando…</div>
<pre id="sysinfo" class="muted"></pre></section>
<section><h2>NTP y zona horaria</h2>
<form onsubmit="saveSystem();return false;">
<label class="muted">Zona horaria</label>
<select id="cfg_timezone">
<option value="UTC">UTC (0)</option>
<option value="America/Argentina/Buenos_Aires">Buenos Aires (-3)</option>
<option value="America/Sao_Paulo">Sao Paulo (-3)</option>
<option value="America/Santiago">Santiago (-4/-3)</option>
<option value="America/Bogota">Bogotá (-5)</option>
<option value="America/Mexico_City">Ciudad de México (-6)</option>
<option value="America/New_York">Nueva York (-5/-4)</option>
<option value="America/Los_Angeles">Los Ángeles (-8/-7)</option>
<option value="Europe/Madrid">Madrid (+1/+2)</option>
<option value="Europe/London">Londres (0/+1)</option>
<option value="Europe/Berlin">Berlín (+1/+2)</option>
<option value="Asia/Tokyo">Tokio (+9)</option>
<option value="Australia/Sydney">Sídney (+10/+11)</option>
</select>
<label class="muted">Servidor NTP</label>
<select id="cfg_ntp">
<option value="pool.ntp.org">pool.ntp.org (mundial)</option>
<option value="time.google.com">time.google.com</option>
<option value="time.nist.gov">time.nist.gov</option>
<option value="time.windows.com">time.windows.com</option>
<option value="0.south-america.pool.ntp.org">Sudamérica</option>
<option value="__custom__">Personalizado…</option>
</select>
<input id="cfg_ntp_custom" placeholder="Servidor NTP propio">
<button type="submit">Guardar</button>
</form></section>
<section><h2>Actualización (OTA)</h2>
<div style="text-align:center;margin:.4rem 0 1rem"><button onclick="checkUpdate()">🔎 Comprobar actualización</button></div>
<div id="upd" class="muted" style="text-align:center;margin-bottom:1rem"></div>
<input type="file" id="fwfile" accept=".bin">
<div id="otaBar" style="display:none;background:#21262d;border-radius:6px;height:16px;margin:.5rem 0;overflow:hidden"><div id="otaFill" style="width:0%;height:100%;background:#1f6feb"></div></div>
<div id="otaMsg" class="muted"></div>
<button onclick="doOta()">⬆️ Subir firmware</button></section>
<section><h2>Acciones</h2>
<button onclick="location.href='/api/v1/history?limit=3000&format=csv'">⬇️ CSV histórico</button>
<button class="sec" onclick="doRestart()">🔄 Reiniciar</button></section>
<script>
function setNtp(v){const s=document.getElementById('cfg_ntp');const opts=[...s.options].map(o=>o.value);if(opts.includes(v)){s.value=v;document.getElementById('cfg_ntp_custom').value=''}else{s.value='__custom__';document.getElementById('cfg_ntp_custom').value=v}}
function getNtp(){const s=document.getElementById('cfg_ntp');return s.value==='__custom__'?document.getElementById('cfg_ntp_custom').value.trim():s.value}
async function load(){try{const s=await(await fetch('/api/v1/status')).json();document.getElementById('status').textContent=s.name+' — v'+s.firmware;const y=await(await fetch('/api/v1/system')).json();document.getElementById('sysinfo').textContent='Board: '+y.board+'\nFlash: '+y.flash_mb+' MB\nFirmware: '+y.firmware_file;const c=await(await fetch('/api/v1/config')).json();const tz=c.system?c.system.timezone:'';const tzs=[...document.getElementById('cfg_timezone').options].map(o=>o.value);if(tzs.includes(tz))document.getElementById('cfg_timezone').value=tz;setNtp(c.system?c.system.ntp_server:'')}catch(e){}}
async function saveSystem(){try{const r=await fetch('/api/v1/config/system',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({timezone:document.getElementById('cfg_timezone').value,ntp_server:getNtp()})});alert(r.ok?'Guardado':'Error')}catch(e){alert('Error de red')}}
async function checkUpdate(){document.getElementById('upd').textContent='Comprobando…';try{const r=await(await fetch('/api/v1/update/check')).json();if(r.update){document.getElementById('upd').innerHTML='Hay una nueva versión: <b>'+r.latest+'</b> (actual '+r.current+'). <a href="'+(r.url||'https://github.com/AlessandroKlein/SEMA/releases')+'" target="_blank">Ver release</a>'}else if(r.latest){document.getElementById('upd').textContent='Estás al día (v'+r.current+')'}else{document.getElementById('upd').textContent='No se pudo consultar GitHub'}}catch(e){document.getElementById('upd').textContent='Error al comprobar'}}
function doOta(){const f=document.getElementById('fwfile').files[0];if(!f)return alert('Elegí un archivo .bin');if(!confirm('¿Actualizar con '+f.name+'?'))return;const bar=document.getElementById('otaBar'),fill=document.getElementById('otaFill'),msg=document.getElementById('otaMsg');bar.style.display='block';msg.textContent='Subiendo…';const fd=new FormData();fd.append('firmware',f);const xhr=new XMLHttpRequest();xhr.open('POST','/api/v1/ota');xhr.upload.onprogress=e=>{if(e.lengthComputable){const p=Math.round(e.loaded/e.total*100);fill.style.width=p+'%';msg.textContent='Subiendo '+p+'%'}};xhr.onload=()=>{fill.style.width='100%';msg.textContent='Flasheado. Reiniciando…';setTimeout(()=>location.href='/',12000)};xhr.onerror=()=>{msg.textContent='Error al subir'};xhr.send(fd)}
async function doRestart(){if(!confirm('¿Reiniciar?'))return;try{await fetch('/api/v1/restart',{method:'POST'});alert('Reiniciando…')}catch(e){}}
load();
</script>
)html";
  serveAuthedPage(server_, authed, body);
}

void HttpServer::onWindPage() {
  const bool authed = webAuthed();
  const String body = R"html(
<section><h2>Calibración de la veleta (WH-SP-WD)</h2>
<p class="muted">Ingresá los valores de las 8 resistencias en el orden del datasheet (empezando por N) y el pull-up. Las 16 posiciones (8 directas + 8 en paralelo) se calculan automáticamente.</p>
<div class="row" id="windInputs"></div>
<div style="max-width:280px"><label class="muted">Resistencia pull-up (Ω)</label><input id="wrp" type="number" step="1" value="10000"></div>
<button onclick="saveWind()">Guardar resistencias</button>
<button class="sec" onclick="calibrateNorth()">🧭 Calibrar norte</button>
</section>
<script>
const DIRS=['N','NE','E','SE','S','SO','O','NO'];
function renderWind(){const c=document.getElementById('windInputs');DIRS.forEach((d,i)=>{const b=document.createElement('div');b.innerHTML='<label class="muted">R'+(i+1)+' — '+d+' (Ω)</label><input id="wr'+i+'" type="number" step="1" value="0">';c.appendChild(b)})}
async function loadWind(){try{const r=await(await fetch('/api/v1/config')).json();document.getElementById('wrp').value=r.system&&r.system.wind_rpull?r.system.wind_rpull:10000;const wr=(r.system&&r.system.wind_resistors)||[];DIRS.forEach((d,i)=>{const e=document.getElementById('wr'+i);if(e)e.value=wr[i]!==undefined?wr[i]:0})}catch(e){}}
async function saveWind(){const resistors=DIRS.map((d,i)=>parseFloat(document.getElementById('wr'+i).value)||0);const rpull=parseFloat(document.getElementById('wrp').value)||10000;try{const resp=await fetch('/api/v1/wind/resistors',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({rpull:rpull,resistors:resistors})});alert(resp.ok?'Resistencias guardadas':'Error')}catch(e){alert('Error de red')}}
async function calibrateNorth(){try{const resp=await fetch('/api/v1/wind/north',{method:'POST'});alert(resp.ok?'Norte calibrado':'Error')}catch(e){alert('Error de red')}}
renderWind();loadWind();
</script>
)html";
  serveAuthedPage(server_, authed, body);
}

void HttpServer::onSensorsPage() {
  const bool authed = webAuthed();
  const String body = R"html(
<section><h2>Sensores</h2>
<div id="sensorList" class="muted">Cargando…</div></section>
<section><h2>Pines</h2>
<div id="pinInfo" class="muted">Cargando…</div></section>
<script>
async function load(){try{
  const s=await(await fetch('/api/v1/sensors')).json();
  let h='<div class="catalog">';
  (s.catalog||[]).forEach(c=>{h+='<div class="cat-item"><span>'+c.id+' — '+c.model+' <span class="muted">('+c.interface+')</span></span><span class="muted">'+(c.healthy?'✅':'⚠️')+'</span></div>'});
  h+='</div>';
  document.getElementById('sensorList').innerHTML=h||'<p class="muted">Sin sensores</p>';
  const y=await(await fetch('/api/v1/system')).json();
  const cfg=await(await fetch('/api/v1/config')).json();
  let p='<p>Board: '+y.board+' · Flash: '+y.flash_mb+' MB'+(y.demo?' · <b>DEMO</b>':'')+'</p>';
  p+='<p>Origen de pines: <b>'+(y.pins_from_file?'PCB (fijos, no configurables)':'Web (configurables)')+'</b></p>';
  const n=cfg.network||{}, m=cfg.modbus||{}, ca=cfg.can||{}, l=cfg.lora||{}, z=cfg.zigbee||{}, e=cfg.ethernet||{};
  const ss=cfg.sensors||[];
  p+='<div class="catalog">';
  p+='<div class="cat-item"><span>I²C SDA / SCL</span><span>'+((ss[0]&&ss[0].sda)||'—')+' / '+((ss[0]&&ss[0].scl)||'—')+'</span></div>';
  p+='<div class="cat-item"><span>Modbus RX / TX (DE/RE)</span><span>'+m.rx+' / '+m.tx+' ('+m.de_re+')</span></div>';
  p+='<div class="cat-item"><span>CAN TX / RX</span><span>'+ca.tx+' / '+ca.rx+'</span></div>';
  p+='<div class="cat-item"><span>LoRa CS / RST / DIO1 / BUSY</span><span>'+l.cs+' / '+l.rst+' / '+l.dio1+' / '+l.busy+'</span></div>';
  p+='<div class="cat-item"><span>Zigbee RX / TX</span><span>'+z.rx+' / '+z.tx+'</span></div>';
  p+='<div class="cat-item"><span>Ethernet MDC / MDIO / PHY</span><span>'+e.mdc+' / '+e.mdio+' / '+e.phy_addr+'</span></div>';
  p+='</div>';
  document.getElementById('pinInfo').innerHTML=p;
}catch(e){document.getElementById('sensorList').innerHTML='<p class="muted">Error al cargar</p>'}}
load();
</script>
)html";
  serveAuthedPage(server_, authed, body);
}

bool HttpServer::webAuthed() {
  const SecurityConfig& sec = core_->config().get().security;
  if (sec.password.length() == 0) {
    return true;  // sin login configurado → abierto (primera configuración)
  }
  return sessionAuthorized() || authorized();
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
  if (!webAuthed()) {
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
  DynamicJsonDocument doc(8192);
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
  doc["pins_from_file"] = (SEMA_PINS_FROM_FILE != 0);
  doc["demo"] = (SEMA_DEMO != 0);
  doc["firmware_file"] =
      String("sema_") + SEMA_FW_VERSION + "_" + SEMA_BOARD_ID + ".bin";
  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onConfig() {
  // La config expone claves (api_key/server_key): requiere autenticación.
  if (!webAuthed()) {
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
  // Credenciales: web local (api_key), Servidor Central (server_key) y las
  // claves adicionales con nombre (extra_keys). Sin claves → permitir.
  const SecurityConfig& sec = core_->config().get().security;
  if (sec.apiKey.length() == 0 && sec.serverKey.length() == 0 &&
      sec.extraKeys.length() == 0) {
    return true;  // sin claves configuradas → permitir (primera configuración)
  }
  if (!server_.hasHeader("X-API-Key")) {
    return false;
  }
  const String key = server_.header("X-API-Key");
  if (sec.apiKey.length() > 0 && key == sec.apiKey) {
    return true;
  }
  if (sec.serverKey.length() > 0 && key == sec.serverKey) {
    return true;
  }
  if (sec.extraKeys.length() > 0) {
    DynamicJsonDocument doc(1024);
    if (!deserializeJson(doc, sec.extraKeys)) {
      for (JsonPair p : doc.as<JsonObject>()) {
        if (String(p.value().as<const char*>()) == key) {
          return true;
        }
      }
    }
  }
  return false;
}

void HttpServer::onConfigNetwork() {
  if (!webAuthed()) {
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
  if (doc.containsKey("ip")) next.network.ip = doc["ip"] | "";
  if (doc.containsKey("gateway")) next.network.gateway = doc["gateway"] | "";
  if (doc.containsKey("subnet")) next.network.subnet = doc["subnet"] | "";
  if (doc.containsKey("dns")) next.network.dns = doc["dns"] | "";
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
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  int n = WiFi.scanNetworks();
  if (n < 0) {
    delay(200);
    n = WiFi.scanNetworks();
  }
  if (n < 0) {
    n = 0;
  }
  DynamicJsonDocument doc(8192);
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

void HttpServer::onApiKeys() {
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"body required\"}");
    return;
  }
  DynamicJsonDocument doc(1024);
  if (deserializeJson(doc, server_.arg("plain"))) {
    server_.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }
  const String action = doc["action"] | "";
  const String name = doc["name"] | "";
  if (action.length() == 0 || name.length() == 0) {
    server_.send(400, "application/json", "{\"error\":\"action/name required\"}");
    return;
  }

  Config next = core_->config().get();
  DynamicJsonDocument keys(1024);
  const String existing = next.security.extraKeys.length() ? next.security.extraKeys : "{}";
  if (deserializeJson(keys, existing)) {
    server_.send(500, "application/json", "{\"error\":\"extra_keys parse\"}");
    return;
  }

  String generated;
  if (action == "generate") {
    char buf[33];
    for (int i = 0; i < 16; ++i) {
      snprintf(buf + i * 2, 3, "%02x", static_cast<unsigned>(esp_random() & 0xFF));
    }
    buf[32] = 0;
    generated = String(buf);
    keys[name] = generated;
  } else if (action == "revoke") {
    keys.remove(name);
  } else {
    server_.send(400, "application/json", "{\"error\":\"unknown action\"}");
    return;
  }

  String outKeys;
  serializeJson(keys, outKeys);
  next.security.extraKeys = outKeys;
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }

  DynamicJsonDocument resp(256);
  resp["ok"] = true;
  resp["name"] = name;
  if (generated.length()) {
    resp["key"] = generated;
  }
  String out;
  serializeJson(resp, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onConfigSystem() {
  if (!webAuthed()) {
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
  if (doc.containsKey("timezone")) next.system.timezone = doc["timezone"] | "America/Argentina/Buenos_Aires";
  if (doc.containsKey("ntp_server")) next.system.ntpServer = doc["ntp_server"] | "pool.ntp.org";
  if (!core_->config().apply(next)) {
    server_.send(500, "application/json", "{\"error\":\"config apply failed\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onUpdateCheck() {
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  DynamicJsonDocument resp(512);
  resp["current"] = SEMA_FW_VERSION;
  resp["latest"] = "";
  resp["update"] = false;
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // solo para leer la versión; el OTA real usa X-SHA256
  if (http.begin(client, "https://raw.githubusercontent.com/AlessandroKlein/SEMA/refs/heads/main/firmware_manifest.json")) {
    http.setTimeout(8000);
    const int code = http.GET();
    if (code == 200) {
      DynamicJsonDocument doc(8192);
      if (!deserializeJson(doc, http.getString())) {
        const String ver = doc["version"] | "";
        resp["latest"] = ver;
        resp["update"] = (ver.length() > 0 && ver != String(SEMA_FW_VERSION));
        resp["url"] = String("https://github.com/AlessandroKlein/SEMA/releases/tag/v") + ver;
      }
    }
    http.end();
  }
  String out;
  serializeJson(resp, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onRestart() {
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
  delay(100);
  ESP.restart();
}

void HttpServer::onConfigPut() {
  if (!webAuthed()) {
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

#if SEMA_DEMO
  // Modo demo: valores ficticios dentro del rango estándar de cada magnitud.
  {
    DynamicJsonDocument ddoc(8192);
    JsonArray dcat = ddoc.createNestedArray("catalog");
    const char* models[] = {"BME280", "SHT40", "SCD30", "PMS5003", "VEML6075", "WH-SP-WD", "RG-9"};
    const char* ids[] = {"ext", "int", "co2", "pm", "uv", "wind", "rain"};
    for (int i = 0; i < 7; ++i) {
      JsonObject c = dcat.createNestedObject();
      c["id"] = ids[i];
      c["model"] = models[i];
      c["interface"] = "demo";
      c["healthy"] = true;
    }
    const float t = millis() / 1000.0f;
    JsonArray darr = ddoc.createNestedArray("measurements");
    auto wave = [&](float seed, float lo, float hi, float period) {
      return lo + (hi - lo) * (0.5f + 0.5f * sinf(t * 6.283185f / period + seed));
    };
    auto add = [&](const char* id, const char* meas, float v, const char* u) {
      JsonObject o = darr.createNestedObject();
      o["sensor_id"] = id;
      o["channel_id"] = "0";
      o["measurement"] = meas;
      o["value"] = v;
      o["unit"] = u;
      o["quality"] = "VALID";
      o["sequence"] = 1;
    };
    add("ext", "temperature", wave(0.0f, 18.0f, 28.0f, 3600), imperial ? "°F" : "°C");
    add("ext", "humidity", wave(1.0f, 45.0f, 75.0f, 5400), "%");
    add("ext", "pressure", wave(2.0f, 1008.0f, 1018.0f, 7200), "hPa");
    add("int", "temperature", wave(3.0f, 20.0f, 26.0f, 4200), imperial ? "°F" : "°C");
    add("int", "humidity", wave(4.0f, 40.0f, 65.0f, 4800), "%");
    add("uv", "uvi", wave(5.0f, 0.5f, 7.0f, 3000), "");
    add("uv", "light", wave(6.0f, 200.0f, 9000.0f, 2800), "lux");
    add("co2", "co2", wave(7.0f, 420.0f, 750.0f, 6000), "ppm");
    add("pm", "pm25", wave(8.0f, 8.0f, 25.0f, 5000), "µg/m³");
    add("wind", "wind_speed", wave(9.0f, 2.0f, 12.0f, 2400), imperial ? "mph" : "m/s");
    add("wind", "wind_direction", fmodf(t * 12.0f, 360.0f), "°");
    add("rain", "rain", wave(10.0f, 0.0f, 3.0f, 9000), imperial ? "in" : "mm");
    add("clock", "clock", static_cast<float>(nowEpoch()), "epoch");
    ddoc["units"] = units;
    String dout;
    serializeJson(ddoc, dout);
    server_.send(200, "application/json", dout);
  }
  return;
#endif

  DynamicJsonDocument doc(8192);

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

  // Reloj (hora NTP/UTC) para la tarjeta de reloj del dashboard.
  {
    JsonObject o = arr.createNestedObject();
    o["sensor_id"] = "clock";
    o["channel_id"] = "0";
    o["measurement"] = "clock";
    o["value"] = static_cast<float>(nowEpoch());
    o["unit"] = "epoch";
    o["quality"] = "VALID";
    o["sequence"] = 0;
  }

  doc["units"] = units;

  String out;
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
}

void HttpServer::onWindNorth() {
  if (!webAuthed()) {
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
  if (!webAuthed()) {
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
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  if (!server_.hasArg("plain")) {
    server_.send(400, "application/json", "{\"error\":\"missing body\"}");
    return;
  }
  if (!core_->config().saveDashboardLayout(server_.arg("plain"))) {
    server_.send(500, "application/json", "{\"error\":\"layout save failed\"}");
    return;
  }
  server_.send(200, "application/json", "{\"ok\":true}");
}

void HttpServer::onDashboardLayoutGet() {
  if (!webAuthed()) {
    server_.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }
  String layout;
  core_->config().loadDashboardLayout(layout);
  String out;
  DynamicJsonDocument doc(2048);
  doc["layout"] = layout.length() ? layout : "[]";
  serializeJson(doc, out);
  server_.send(200, "application/json", out);
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

#if SEMA_DEMO
  // Histórico ficticio para las gráficas en modo demo.
  {
    const uint32_t now = nowEpoch();
    const char* meas[] = {"temperature", "humidity", "pressure", "light", "co2", "pm25", "uvi", "wind_speed"};
    const char* sens[] = {"ext", "ext", "ext", "uv", "co2", "pm", "uv", "wind"};
    const float base[] = {23.0f, 60.0f, 1013.0f, 5000.0f, 600.0f, 15.0f, 4.0f, 6.0f};
    const float amp[] = {5.0f, 15.0f, 5.0f, 4000.0f, 150.0f, 8.0f, 3.0f, 4.0f};
    DynamicJsonDocument doc(8192);
    JsonArray arr = doc.createNestedArray("history");
    const int samples = 40;
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < samples; ++j) {
        const uint32_t ts = now - static_cast<uint32_t>(samples - j) * 90;
        JsonObject o = arr.createNestedObject();
        o["ts"] = ts;
        o["sensor"] = sens[i];
        o["channel"] = "0";
        o["measurement"] = meas[i];
        o["value"] = base[i] + amp[i] * sinf(static_cast<float>(j) / samples * 6.28318f + i);
        o["unit"] = "";
        o["quality"] = "VALID";
        o["seq"] = j;
      }
    }
    String out;
    serializeJson(doc, out);
    server_.send(200, "application/json", out);
    return;
  }
#endif

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
  DynamicJsonDocument doc(8192);
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
  DynamicJsonDocument doc(8192);
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
  if (!webAuthed()) {
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
  if (!webAuthed()) {
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
  if (!webAuthed()) {
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
  if (!webAuthed()) {
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
  if (!webAuthed()) {
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
