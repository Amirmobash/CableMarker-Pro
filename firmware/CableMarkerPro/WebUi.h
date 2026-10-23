#pragma once

#include <pgmspace.h>

static const char kWebUi[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,viewport-fit=cover">
  <meta name="robots" content="noindex,nofollow">
  <meta name="theme-color" content="#f97316">
  <title>CableMarker Pro</title>
  <style>
    :root{--orange:#f97316;--orange-dark:#dc5f0c;--bg:#f7f7f5;--card:#fff;--text:#202124;--muted:#6b7280;--line:#ece7e1;--green:#16865a;--red:#b42318;--amber:#a15c00}
    *{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
    html,body{margin:0;min-height:100%;background:var(--bg);color:var(--text);font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif}
    body{padding:14px}
    .app{max-width:560px;margin:0 auto}
    .top{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:12px}
    h1{font-size:22px;margin:0}.sub{font-size:12px;color:var(--muted);margin-top:2px}
    .lang{display:flex;gap:6px}.lang button{width:auto;padding:0 11px;height:34px}
    .card{background:var(--card);border:1px solid var(--line);border-radius:16px;padding:16px;margin-bottom:10px;box-shadow:0 3px 16px rgba(0,0,0,.04)}
    .distance{font-size:46px;font-weight:850;letter-spacing:-1px;color:var(--orange);line-height:1;text-align:center;margin:6px 0}
    .raw{text-align:center;color:var(--muted);font-size:13px}
    .statusRow{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:14px}
    .status{border-radius:11px;padding:10px;text-align:center;font-size:12px;font-weight:750;background:#f4f4f4}
    .ok{background:#eaf8f1;color:var(--green)}.bad{background:#fff0ee;color:var(--red)}.warn{background:#fff5e7;color:var(--amber)}
    .actions{display:grid;grid-template-columns:1.35fr 1fr;gap:8px}
    button{height:48px;border:0;border-radius:12px;font-size:14px;font-weight:800;cursor:pointer;touch-action:manipulation}
    .primary{background:var(--orange);color:#fff}.primary:active{background:var(--orange-dark)}
    .secondary{background:#f2f2f2;color:#363636;border:1px solid #e5e5e5}
    .meta{display:grid;grid-template-columns:1fr 1fr;gap:7px;font-size:12px;color:var(--muted)}
    .meta div{background:#faf8f5;border-radius:10px;padding:9px}.meta b{display:block;color:var(--text);font-size:13px;margin-top:2px}
    .notice{font-size:11px;color:var(--muted);line-height:1.45;margin-top:10px}
    #toast{min-height:18px;text-align:center;font-size:12px;color:var(--muted);margin-top:8px}
  </style>
</head>
<body>
<div class="app">
  <div class="top">
    <div><h1>CableMarker Pro</h1><div class="sub" id="subtitle">Cable & pipe measuring and marking</div></div>
    <div class="lang"><button class="secondary" onclick="setLang('en')">EN</button><button class="secondary" onclick="setLang('de')">DE</button></div>
  </div>

  <section class="card">
    <div class="distance" id="distance">---.---</div>
    <div class="raw" id="raw">Waiting for measurement…</div>
    <div class="statusRow">
      <div class="status warn" id="laserStatus">Laser: starting</div>
      <div class="status ok" id="markerStatus">Marker: ready</div>
    </div>
  </section>

  <section class="card">
    <div class="actions">
      <button class="primary" id="markButton" onclick="markNow()">MARK NOW</button>
      <button class="secondary" id="restartButton" onclick="restartLaser()">RESTART LASER</button>
    </div>
    <div id="toast"></div>
  </section>

  <section class="card">
    <div class="meta">
      <div id="calLabel">Calibration offset<b id="calibration">-33 mm</b></div>
      <div id="countLabel">Completed marks<b id="count">0</b></div>
      <div id="versionLabel">Firmware<b id="version">2.0.0</b></div>
      <div id="uptimeLabel">Uptime<b id="uptime">0 s</b></div>
    </div>
    <div class="notice" id="rangeNote">Long-range operation depends on the installed laser rangefinder, target surface, alignment and environmental conditions. Use a sensor rated for the distance you need.</div>
  </section>
</div>
<script>
const T={
 en:{subtitle:'Cable & pipe measuring and marking',waiting:'Waiting for measurement…',laser:'Laser',connected:'connected',disconnected:'disconnected',restarting:'restarting',marker:'Marker',ready:'ready',active:'active',mark:'MARK NOW',restart:'RESTART LASER',cal:'Calibration offset',count:'Completed marks',version:'Firmware',uptime:'Uptime',marked:'Mark cycle started.',busy:'Marker is busy or in cooldown.',restartMsg:'Laser restart requested.',error:'Request failed.',range:'Long-range operation depends on the installed laser rangefinder, target surface, alignment and environmental conditions. Use a sensor rated for the distance you need.'},
 de:{subtitle:'Kabel- & Rohrmessung mit Markierfunktion',waiting:'Warte auf Messwert…',laser:'Laser',connected:'verbunden',disconnected:'getrennt',restarting:'Neustart läuft',marker:'Markierer',ready:'bereit',active:'aktiv',mark:'JETZT MARKIEREN',restart:'LASER NEUSTART',cal:'Kalibrier-Offset',count:'Ausgeführte Markierungen',version:'Firmware',uptime:'Laufzeit',marked:'Markierzyklus gestartet.',busy:'Markierer ist aktiv oder noch in der Sperrzeit.',restartMsg:'Laser-Neustart angefordert.',error:'Anfrage fehlgeschlagen.',range:'Der Langstreckenbetrieb hängt vom eingesetzten Laser-Entfernungssensor, der Zieloberfläche, Ausrichtung und Umgebung ab. Verwenden Sie einen Sensor, der für die benötigte Distanz ausgelegt ist.'}
};
let lang=localStorage.getItem('cm_lang')||'en',lastState=null;
function tr(k){return T[lang][k]}
function setLang(v){lang=v;localStorage.setItem('cm_lang',v);document.documentElement.lang=v;renderText();if(lastState)render(lastState)}
function renderText(){document.getElementById('subtitle').textContent=tr('subtitle');document.getElementById('markButton').textContent=tr('mark');document.getElementById('restartButton').textContent=tr('restart');document.getElementById('calLabel').childNodes[0].nodeValue=tr('cal');document.getElementById('countLabel').childNodes[0].nodeValue=tr('count');document.getElementById('versionLabel').childNodes[0].nodeValue=tr('version');document.getElementById('uptimeLabel').childNodes[0].nodeValue=tr('uptime');document.getElementById('rangeNote').textContent=tr('range')}
function render(s){lastState=s;let d=document.getElementById('distance'),r=document.getElementById('raw');if(s.distance_mm>=0){d.textContent=(s.distance_mm/1000).toFixed(3)+' m';r.textContent=s.distance_mm+' mm'}else{d.textContent='---.---';r.textContent=tr('waiting')}
 let ls=document.getElementById('laserStatus');if(s.laser_restarting){ls.className='status warn';ls.textContent=tr('laser')+': '+tr('restarting')}else if(s.laser_healthy){ls.className='status ok';ls.textContent=tr('laser')+': '+tr('connected')}else{ls.className='status bad';ls.textContent=tr('laser')+': '+tr('disconnected')}
 let ms=document.getElementById('markerStatus');ms.className=s.marker_busy?'status warn':'status ok';ms.textContent=tr('marker')+': '+(s.marker_busy?tr('active'):tr('ready'));document.getElementById('calibration').textContent=s.calibration_offset_mm+' mm';document.getElementById('count').textContent=s.completed_marks;document.getElementById('version').textContent=s.version;document.getElementById('uptime').textContent=Math.floor(s.uptime_ms/1000)+' s'}
async function api(path,method='GET'){let res=await fetch(path,{method,cache:'no-store'});let data=await res.json();if(!res.ok)throw new Error(data.message||tr('error'));return data}
async function refresh(){try{render(await api('/api/status'))}catch(e){document.getElementById('toast').textContent=e.message}}
async function markNow(){try{let s=await api('/api/mark','POST');render(s);document.getElementById('toast').textContent=tr('marked')}catch(e){document.getElementById('toast').textContent=e.message||tr('busy')}}
async function restartLaser(){try{let s=await api('/api/laser/restart','POST');render(s);document.getElementById('toast').textContent=tr('restartMsg')}catch(e){document.getElementById('toast').textContent=e.message||tr('error')}}
renderText();setInterval(refresh,750);refresh();
</script>
</body>
</html>
)HTML";
