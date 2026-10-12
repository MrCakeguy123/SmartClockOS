#pragma once

#include "Config.h"
#include "DataManager.h"
#include "Backlight.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <time.h>

namespace WebConfig {

static WebServer server(80);
static bool running = false;

// Live framebuffer source for /api/screenshot. Installed by the GUI task at
// boot rather than #including Display.h here, which would drag the whole
// graphics stack into every file that just wants logPrintf().
static uint16_t *(*fbProvider)() = nullptr;
static uint16_t fbW = 0, fbH = 0;

void setFramebufferSource(uint16_t *(*fn)(), uint16_t w, uint16_t h) {
    fbProvider = fn;
    fbW = w;
    fbH = h;
}

// Circular log buffer for serial output
static constexpr size_t LOG_BUFFER_SIZE = 8192;
static char logBuffer[LOG_BUFFER_SIZE];
static size_t logWritePos = 0;
static size_t logReadPos = 0;
static portMUX_TYPE logLock = portMUX_INITIALIZER_UNLOCKED;

// Log capture functions
static void logPrint(const char* text) {
    size_t len = strlen(text);
    portENTER_CRITICAL(&logLock);
    for (size_t i = 0; i < len; i++) {
        logBuffer[logWritePos] = text[i];
        logWritePos = (logWritePos + 1) % LOG_BUFFER_SIZE;
        if (logWritePos == logReadPos) {
            logReadPos = (logReadPos + 1) % LOG_BUFFER_SIZE;
        }
    }
    portEXIT_CRITICAL(&logLock);
    Serial.print(text);
}

static void logPrintf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    logPrint(buf);
}

static String getLogs(size_t maxBytes = 4096) {
    String result;
    result.reserve(maxBytes);

    portENTER_CRITICAL(&logLock);
    size_t bytesToRead = 0;
    size_t pos = logReadPos;

    if (logWritePos >= logReadPos) {
        bytesToRead = logWritePos - logReadPos;
    } else {
        bytesToRead = LOG_BUFFER_SIZE - logReadPos + logWritePos;
    }

    if (bytesToRead > maxBytes) {
        logReadPos = (logWritePos - maxBytes + LOG_BUFFER_SIZE) % LOG_BUFFER_SIZE;
        pos = logReadPos;
        bytesToRead = maxBytes;
    }

    for (size_t i = 0; i < bytesToRead; i++) {
        result += logBuffer[pos];
        pos = (pos + 1) % LOG_BUFFER_SIZE;
    }
    portEXIT_CRITICAL(&logLock);

    return result;
}

static const char* mainPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SmartClockOS</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;
background:linear-gradient(135deg,#0f0c29 0%,#302b63 50%,#24243e 100%);
min-height:100vh;color:#e0e0e0}
.container{max-width:900px;margin:0 auto;padding:20px}
header{display:flex;justify-content:space-between;align-items:center;
margin-bottom:20px;flex-wrap:wrap;gap:10px}
h1{font-size:1.8em;color:#fff;text-shadow:0 2px 10px rgba(0,0,0,.3)}
.version{color:#a0a0a0;font-size:.9em}
nav{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:20px}
nav button{background:rgba(255,255,255,.1);border:1px solid rgba(255,255,255,.2);
color:#fff;padding:12px 24px;border-radius:8px;cursor:pointer;font-size:.9em;
transition:all .2s}
nav button:hover{background:rgba(255,255,255,.2)}
nav button.active{background:#e94560;border-color:#e94560}
.card{background:rgba(255,255,255,.05);backdrop-filter:blur(10px);
border-radius:16px;padding:24px;margin-bottom:20px;
border:1px solid rgba(255,255,255,.1)}
.card h2{color:#e94560;margin-bottom:16px;font-size:1.2em}
.tab{display:none}
.tab.active{display:block}
.row{display:flex;gap:16px;flex-wrap:wrap;margin-bottom:16px;align-items:center}
.row label{flex:1;min-width:180px;color:#ccc}
.row input[type=text],.row input[type=password],.row input[type=number],.row select{
flex:2;min-width:200px;padding:10px 14px;border-radius:8px;
border:1px solid rgba(255,255,255,.2);background:rgba(0,0,0,.3);color:#fff;font-size:1em}
.row input[type=range]{flex:2;min-width:200px}
.row input:focus,.row select:focus{outline:none;border-color:#e94560}
.row input[type=checkbox]{width:20px;height:20px;cursor:pointer}
button.primary{background:linear-gradient(135deg,#e94560,#c73450);color:#fff;
border:none;padding:14px 28px;border-radius:8px;cursor:pointer;
font-size:1em;font-weight:600;transition:transform .2s}
button.primary:hover{transform:scale(1.02)}
.status-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:16px}
.status-item{background:rgba(0,0,0,.2);padding:16px;border-radius:12px;text-align:center}
.status-item .value{font-size:1.8em;font-weight:700;color:#e94560}
.status-item .label{color:#999;font-size:.85em;margin-top:4px}
.status-item.ok .value{color:#4ade80}
.status-item.warn .value{color:#fbbf24}
.status-item.err .value{color:#f87171}
.log-box{background:#0a0a0a;border-radius:8px;padding:16px;
font-family:'Fira Code','Consolas',monospace;font-size:.85em;
height:400px;overflow-y:auto;white-space:pre-wrap;word-break:break-all;
color:#a0a0a0}
.log-controls{display:flex;gap:10px;margin-bottom:10px;align-items:center;flex-wrap:wrap}
.log-controls button{background:rgba(255,255,255,.1);border:none;color:#fff;
padding:8px 16px;border-radius:6px;cursor:pointer}
.log-controls button:hover{background:rgba(255,255,255,.2)}
.auto-scroll{color:#ccc;font-size:.85em}
#otaProgress{display:none;margin-top:16px}
.progress-bar{height:24px;background:rgba(0,0,0,.3);border-radius:12px;overflow:hidden}
.progress-fill{height:100%;background:linear-gradient(90deg,#e94560,#ff6b6b);
transition:width .3s;width:0%}
.progress-text{text-align:center;color:#fff;font-weight:600;line-height:24px}
.file-input{padding:12px;background:rgba(0,0,0,.3);border-radius:8px;
border:1px dashed rgba(255,255,255,.3);color:#fff;cursor:pointer;display:inline-block}
.file-input input{color:#fff}
.refresh-btn{background:rgba(255,255,255,.1);border:none;color:#fff;
padding:8px 16px;border-radius:6px;cursor:pointer;margin-left:8px}
.refresh-btn:hover{background:rgba(255,255,255,.2)}
hr{border:none;border-top:1px solid rgba(255,255,255,.1);margin:20px 0}
h3{color:#e94560;margin-bottom:12px}
@media(max-width:600px){
.row{flex-direction:column}
.row label,.row input,.row select{min-width:100%}
nav button{padding:10px 16px;font-size:.85em}
}
</style>
</head>
<body>
<div class="container">
<header>
<div><h1>SmartClockOS</h1><span class="version">v)rawliteral" CLOCK_VERSION R"rawliteral(</span></div>
<span id="ip">Loading...</span>
</header>
<nav>
<button class="active" data-tab="status" onclick="showTab('status', this)">Status</button>
<button data-tab="settings" onclick="showTab('settings', this)">Settings</button>
<button data-tab="diagnostics" onclick="showTab('diagnostics', this)">Diagnostics</button>
<button data-tab="logs" onclick="showTab('logs', this)">Logs</button>
<button data-tab="update" onclick="showTab('update', this)">Update</button>
</nav>
<div id="status" class="tab active card">
<h2>Current Status</h2>
<div class="status-grid">
<div class="status-item"><div class="value" id="curTemp">--</div><div class="label">Temperature</div></div>
<div class="status-item"><div class="value" id="curHumidity">--</div><div class="label">Humidity</div></div>
<div class="status-item"><div class="value" id="curCondition">--</div><div class="label">Condition</div></div>
<div class="status-item"><div class="value" id="curWind">--</div><div class="label">Wind</div></div>
<div class="status-item"><div class="value" id="curPrecip">--</div><div class="label">Precipitation</div></div>
<div class="status-item"><div class="value" id="curTime">--</div><div class="label">Time</div></div>
</div>
<button class="refresh-btn" onclick="refreshStatus()">Refresh</button>
</div>
<div id="settings" class="tab card">
<h2>Settings</h2>
<div class="row"><label>Display Brightness</label>
<input type="range" id="brightness" min="10" max="255" value="255" oninput="updateBrightness(this.value)">
<span id="brightnessVal">100%</span></div>
<div class="row"><label>Temperature Unit</label>
<select id="tempUnit" onchange="saveSettings()">
<option value="0">Fahrenheit</option>
<option value="1">Celsius</option>
</select></div>
<div class="row"><label>Wind Speed Unit</label>
<select id="windUnit" onchange="saveSettings()">
<option value="0">mph</option>
<option value="1">km/h</option>
</select></div>
<div class="row"><label>Precipitation Unit</label>
<select id="precipUnit" onchange="saveSettings()">
<option value="0">inches</option>
<option value="1">mm</option>
</select></div>
<div class="row"><label>WiFi SSID</label>
<input type="text" id="wifiSsid" placeholder="Leave blank to keep current"></div>
<div class="row"><label>WiFi Password</label>
<input type="password" id="wifiPass" placeholder="Leave blank to keep current"></div>
<hr>
<h3>Night Mode</h3>
<div class="row"><label>Enable Night Mode</label>
<input type="checkbox" id="nightMode" onchange="saveSettings()"></div>
<div class="row"><label>Night Brightness</label>
<input type="range" id="nightBrightness" min="5" max="100" value="30" oninput="document.getElementById('nightBrightVal').textContent=this.value+'%'">
<span id="nightBrightVal">30%</span></div>
<div class="row"><label>Night Start Hour (0-23)</label>
<input type="number" id="nightStart" min="0" max="23" value="22"></div>
<div class="row"><label>Night End Hour (0-23)</label>
<input type="number" id="nightEnd" min="0" max="23" value="6"></div>
<button class="primary" onclick="saveSettings()" style="margin-top:16px">Save All Settings</button>
<span id="settingsMsg" style="margin-left:16px;color:#4ade80"></span>
</div>
<div id="diagnostics" class="tab card">
<h2>System Diagnostics</h2>
<div class="status-grid">
<div class="status-item"><div class="value" id="diagFps">--</div><div class="label">FPS</div></div>
<div class="status-item"><div class="value" id="diagHeap">--</div><div class="label">Heap Free</div></div>
<div class="status-item"><div class="value" id="diagPsram">--</div><div class="label">PSRAM Free</div></div>
<div class="status-item" id="wifiItem"><div class="value" id="diagWifi">--</div><div class="label">WiFi</div></div>
<div class="status-item" id="weatherItem"><div class="value" id="diagWeather">--</div><div class="label">Weather API</div></div>
<div class="status-item"><div class="value" id="diagFan">--</div><div class="label">CPU / Fan</div></div>
<div class="status-item"><div class="value" id="diagUptime">--</div><div class="label">Uptime</div></div>
</div>
<button class="refresh-btn" onclick="refreshDiagnostics()">Refresh</button>
</div>
<div id="logs" class="tab card">
<h2>Serial Logs</h2>
<div class="log-controls">
<button onclick="refreshLogs()">Refresh</button>
<button onclick="clearLogs()">Clear</button>
<label class="auto-scroll"><input type="checkbox" id="autoScroll" checked> Auto-scroll</label>
</div>
<div class="log-box" id="logContent">Loading...</div>
</div>
<div id="update" class="tab card">
<h2>Firmware Update</h2>
<p style="color:#999;margin-bottom:16px">Upload a .bin file to update the firmware. The device will reboot automatically.</p>
<label class="file-input">
<input type="file" id="firmwareFile" accept=".bin" onchange="document.getElementById('fileLabel').textContent=this.files[0]?this.files[0].name:'Choose file'">
<span id="fileLabel">Choose file</span>
</label>
<button class="primary" onclick="startUpdate()" style="margin-top:16px;margin-left:12px">Upload and Flash</button>
<div id="otaProgress">
<div class="progress-bar"><div class="progress-fill" id="otaFill"></div></div>
<div class="progress-text" id="otaText">0%</div>
</div>
<div id="otaMsg" style="margin-top:12px;color:#4ade80"></div>
</div>
</div>
<script>
let autoRefresh;
function showTab(id,btn){
document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
document.querySelectorAll('nav button').forEach(b=>b.classList.remove('active'));
document.getElementById(id).classList.add('active');
const activeBtn=btn||document.querySelector('nav button[data-tab="'+id+'"]');
if(activeBtn)activeBtn.classList.add('active');
if(id==='logs')startLogRefresh();else stopLogRefresh();
}
function refreshStatus(){
fetch('/api/status').then(r=>r.json()).then(d=>{
document.getElementById('curTemp').textContent=d.temp||'--';
document.getElementById('curHumidity').textContent=d.humidity||'--';
document.getElementById('curCondition').textContent=d.condition||'--';
document.getElementById('curWind').textContent=d.wind||'--';
document.getElementById('curPrecip').textContent=d.precip||'--';
document.getElementById('curTime').textContent=d.time||'--';
document.getElementById('ip').textContent=d.ip||'';
}).catch(e=>console.error(e));
}
function loadSettings(){
fetch('/api/settings').then(r=>r.json()).then(d=>{
document.getElementById('brightness').value=d.brightness;
document.getElementById('brightnessVal').textContent=Math.round(d.brightness*100/255)+'%';
document.getElementById('tempUnit').value=d.tempUnit;
document.getElementById('windUnit').value=d.windUnit;
document.getElementById('precipUnit').value=d.precipUnit;
document.getElementById('wifiSsid').value=d.wifiSsid||'';
document.getElementById('nightMode').checked=d.nightMode;
document.getElementById('nightBrightness').value=d.nightBrightness;
document.getElementById('nightBrightVal').textContent=d.nightBrightness+'%';
document.getElementById('nightStart').value=d.nightStart;
document.getElementById('nightEnd').value=d.nightEnd;
}).catch(e=>console.error(e));
}
function updateBrightness(v){
document.getElementById('brightnessVal').textContent=Math.round(v*100/255)+'%';
fetch('/api/brightness?v='+v);
}
function saveSettings(){
const data={
brightness:parseInt(document.getElementById('brightness').value),
tempUnit:parseInt(document.getElementById('tempUnit').value),
windUnit:parseInt(document.getElementById('windUnit').value),
precipUnit:parseInt(document.getElementById('precipUnit').value),
nightMode:document.getElementById('nightMode').checked,
nightBrightness:parseInt(document.getElementById('nightBrightness').value),
nightStart:Math.min(23,Math.max(0,parseInt(document.getElementById('nightStart').value)||0)),
nightEnd:Math.min(23,Math.max(0,parseInt(document.getElementById('nightEnd').value)||0))
};
const wifiSsid=document.getElementById('wifiSsid').value.trim();
const wifiPass=document.getElementById('wifiPass').value;
if(wifiSsid.length>0)data.wifiSsid=wifiSsid;
if(wifiPass.length>0)data.wifiPass=wifiPass;
fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(data)})
.then(r=>r.json()).then(d=>{
document.getElementById('settingsMsg').textContent=d.success?'Saved!':'Error';
setTimeout(()=>document.getElementById('settingsMsg').textContent='',2000);
}).catch(e=>{document.getElementById('settingsMsg').textContent='Error';});
}
function refreshDiagnostics(){
fetch('/api/diagnostics').then(r=>r.json()).then(d=>{
document.getElementById('diagFps').textContent=d.fps.toFixed(1);
document.getElementById('diagHeap').textContent=(d.heapFree/1024).toFixed(0)+'KB';
document.getElementById('diagPsram').textContent=d.psramFree>0?(d.psramFree/1024).toFixed(0)+'KB':'--';
document.getElementById('diagWifi').textContent=d.wifiConnected?d.wifiRssi+'dBm':'Offline';
document.getElementById('wifiItem').className='status-item '+(d.wifiConnected?'ok':'err');
document.getElementById('diagWeather').textContent=d.weatherOk?d.weatherLatency+'ms':'ERR';
document.getElementById('weatherItem').className='status-item '+(d.weatherOk?'ok':'err');
document.getElementById('diagFan').textContent=(d.dieTemp?d.dieTemp.toFixed(0)+'°C / '+d.fanDuty+'%':'--');
document.getElementById('diagUptime').textContent=formatUptime(d.uptime);
}).catch(e=>console.error(e));
}
function formatUptime(ms){
if(!ms)return'--';
const s=Math.floor(ms/1000);
const h=Math.floor(s/3600);
const m=Math.floor((s%3600)/60);
if(h>0)return h+'h '+m+'m';
return m+'m';
}
function refreshLogs(){
fetch('/api/logs').then(r=>r.text()).then(t=>{
const el=document.getElementById('logContent');
el.textContent=t;
if(document.getElementById('autoScroll').checked)el.scrollTop=el.scrollHeight;
}).catch(e=>console.error(e));
}
function clearLogs(){
fetch('/api/logs/clear',{method:'POST'});
document.getElementById('logContent').textContent='';
}
function startLogRefresh(){
if(autoRefresh)return;
autoRefresh=setInterval(refreshLogs,1000);
refreshLogs();
}
function stopLogRefresh(){
if(autoRefresh){clearInterval(autoRefresh);autoRefresh=null;}
}
function startUpdate(){
const file=document.getElementById('firmwareFile').files[0];
if(!file){alert('Please select a file');return;}
document.getElementById('otaProgress').style.display='block';
document.getElementById('otaMsg').textContent='';
const xhr=new XMLHttpRequest();
xhr.upload.onprogress=e=>{
if(e.lengthComputable){
const p=Math.round(e.loaded*100/e.total);
document.getElementById('otaFill').style.width=p+'%';
document.getElementById('otaText').textContent=p+'%';
}
};
xhr.onload=()=>{
if(xhr.status===200){
document.getElementById('otaMsg').textContent='Success! Rebooting...';
}else{
document.getElementById('otaMsg').textContent='Error: '+xhr.responseText;
document.getElementById('otaMsg').style.color='#f87171';
}
};
xhr.open('POST','/update');
const fd=new FormData();
fd.append('update',file);
xhr.send(fd);
}
refreshStatus();
loadSettings();
refreshDiagnostics();
</script>
</body>
</html>
)rawliteral";

static void handleRoot() {
    server.send(200, "text/html", mainPage);
}

static void handleApiStatus() {
    CurrentWeather cur = Data.getCurrent();
    NetworkState net = Data.getNetwork();

    JsonDocument doc;
    doc["temp"] = cur.tempStr;
    doc["humidity"] = cur.humidityStr;
    doc["condition"] = cur.conditionStr;
    doc["wind"] = cur.windStr;
    doc["precip"] = cur.precipStr;
    doc["time"] = net.timeStr;
    doc["ip"] = WiFi.localIP().toString();

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

static void handleApiSettings() {
    if (server.method() == HTTP_GET) {
        Settings s = Data.getSettings();
        JsonDocument doc;
        doc["brightness"] = s.brightness;
        doc["tempUnit"] = (int)s.tempUnit;
        doc["windUnit"] = (int)s.windUnit;
        doc["precipUnit"] = (int)s.precipUnit;
        doc["wifiSsid"] = s.wifiSsid;
        doc["nightMode"] = s.nightModeEnabled;
        doc["nightBrightness"] = (int)((s.nightBrightness * 100) / BACKLIGHT_MAX);
        doc["nightStart"] = s.nightStartHour;
        doc["nightEnd"] = s.nightEndHour;

        String out;
        serializeJson(doc, out);
        server.send(200, "application/json", out);
    } else if (server.method() == HTTP_POST) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, server.arg("plain"));

        if (err) {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        Settings s = Data.getSettings();
        bool unitsChanged = false;
        bool wifiChanged = false;

        if (doc["brightness"].is<int>())
            s.brightness = doc["brightness"].as<int>();
        if (doc["tempUnit"].is<int>()) {
            TempUnit tempUnit = (TempUnit)doc["tempUnit"].as<int>();
            if (s.tempUnit != tempUnit) {
                s.tempUnit = tempUnit;
                unitsChanged = true;
            }
        }
        if (doc["windUnit"].is<int>()) {
            WindUnit windUnit = (WindUnit)doc["windUnit"].as<int>();
            if (s.windUnit != windUnit) {
                s.windUnit = windUnit;
                unitsChanged = true;
            }
        }
        if (doc["precipUnit"].is<int>()) {
            PrecipUnit precipUnit = (PrecipUnit)doc["precipUnit"].as<int>();
            if (s.precipUnit != precipUnit) {
                s.precipUnit = precipUnit;
                unitsChanged = true;
            }
        }
        if (doc["wifiSsid"].is<const char*>()) {
            const char* wifiSsid = doc["wifiSsid"].as<const char*>();
            if (wifiSsid && wifiSsid[0] != '\0' && strcmp(s.wifiSsid, wifiSsid) != 0) {
                strncpy(s.wifiSsid, wifiSsid, sizeof(s.wifiSsid) - 1);
                s.wifiSsid[sizeof(s.wifiSsid) - 1] = '\0';
                wifiChanged = true;
            }
        }
        if (doc["wifiPass"].is<const char*>()) {
            const char* wifiPass = doc["wifiPass"].as<const char*>();
            if (wifiPass && wifiPass[0] != '\0' && strcmp(s.wifiPass, wifiPass) != 0) {
                strncpy(s.wifiPass, wifiPass, sizeof(s.wifiPass) - 1);
                s.wifiPass[sizeof(s.wifiPass) - 1] = '\0';
                wifiChanged = true;
            }
        }
        if (doc["nightMode"].is<bool>())
            s.nightModeEnabled = doc["nightMode"].as<bool>();
        if (doc["nightBrightness"].is<int>()) {
            int nightBrightnessPct = constrain(doc["nightBrightness"].as<int>(), 5, 100);
            s.nightBrightness = (nightBrightnessPct * BACKLIGHT_MAX) / 100;
        }
        if (doc["nightStart"].is<int>())
            s.nightStartHour = constrain(doc["nightStart"].as<int>(), 0, 23);
        if (doc["nightEnd"].is<int>())
            s.nightEndHour = constrain(doc["nightEnd"].as<int>(), 0, 23);

        Data.setSettings(s);
        SettingsStore.save(s);
        Backlight::set(s.brightness);
        Backlight::setNightMode(isNightModeActiveNow(s), s.nightBrightness);

        if (unitsChanged) {
            Data.requestWeatherRefresh();
        }

        if (wifiChanged && strlen(s.wifiSsid) > 0 && strlen(s.wifiPass) > 0) {
            Data.requestWifiReconnect();
        }

        server.send(200, "application/json", "{\"success\":true}");
    }
}

static void handleApiBrightness() {
    if (server.hasArg("v")) {
        int val = server.arg("v").toInt();
        val = constrain(val, 10, BACKLIGHT_MAX);
        Backlight::set(val);
    }
    server.send(200, "text/plain", "OK");
}

static void handleApiDiagnostics() {
    DiagnosticsData diag = Data.getDiagnostics();

    JsonDocument doc;
    doc["fps"] = diag.fps;
    doc["heapFree"] = diag.heapFree;
    doc["heapTotal"] = diag.heapTotal;
    doc["psramFree"] = diag.psramFree;
    doc["psramTotal"] = diag.psramTotal;
    doc["wifiConnected"] = diag.wifiConnected;
    doc["wifiRssi"] = diag.wifiRssi;
    doc["wifiQuality"] = diag.wifiQuality;
    doc["weatherLatency"] = diag.weatherLatencyMs;
    doc["weatherOk"] = diag.weatherOk;
    doc["sensorHealthy"] = diag.sensorHealthy;
    doc["sensorLastMs"] = diag.sensorLastMs;
    doc["dieTemp"] = diag.dieTempC;
    doc["fanDuty"] = diag.fanDutyPct;
#if ENABLE_BME280
    IndoorData indoor = Data.getIndoor();
    doc["indoorValid"] = indoor.valid;
    doc["indoorTemp"] = indoor.tempStr;
    doc["indoorHumidity"] = indoor.humidityStr;
#endif
    doc["uptime"] = millis();

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

static void handleApiLogs() {
    server.send(200, "text/plain", getLogs());
}

static void handleApiLogsClear() {
    portENTER_CRITICAL(&logLock);
    logWritePos = 0;
    logReadPos = 0;
    portEXIT_CRITICAL(&logLock);
    server.send(200, "text/plain", "OK");
}

static void handleUpdateUpload() {
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        logPrintf("[OTA] Receiving: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            logPrintf("[OTA] Success: %u bytes\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

// GET /api/screenshot[?scale=2] — the live panel contents as a BMP.
// Streams straight out of the RGB framebuffer in PSRAM, converting RGB565
// to 24-bit BGR a few rows at a time, so it never needs a full-size buffer.
static void handleScreenshot() {
    if (!fbProvider) {
        server.send(503, "text/plain", "display not ready");
        return;
    }
    uint16_t *fb = fbProvider();
    if (!fb || fbW == 0 || fbH == 0) {
        server.send(503, "text/plain", "no framebuffer");
        return;
    }

    uint32_t scale = 1;
    if (server.hasArg("scale")) {
        uint32_t s = server.arg("scale").toInt();
        if (s == 2 || s == 4) scale = s;
    }

    const uint32_t w = fbW / scale;
    const uint32_t h = fbH / scale;
    const uint32_t rowBytes = ((w * 3) + 3) & ~3U; // BMP rows are 4-byte aligned
    const uint32_t pixBytes = rowBytes * h;
    const uint32_t fileSize = 54 + pixBytes;

    uint8_t hdr[54];
    memset(hdr, 0, sizeof(hdr));
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = fileSize & 0xFF;         hdr[3] = (fileSize >> 8) & 0xFF;
    hdr[4] = (fileSize >> 16) & 0xFF; hdr[5] = (fileSize >> 24) & 0xFF;
    hdr[10] = 54;   // pixel data offset
    hdr[14] = 40;   // DIB header size
    hdr[18] = w & 0xFF;         hdr[19] = (w >> 8) & 0xFF;
    hdr[20] = (w >> 16) & 0xFF; hdr[21] = (w >> 24) & 0xFF;
    int32_t negH = -(int32_t)h; // negative height = rows stored top-down
    hdr[22] = negH & 0xFF;         hdr[23] = (negH >> 8) & 0xFF;
    hdr[24] = (negH >> 16) & 0xFF; hdr[25] = (negH >> 24) & 0xFF;
    hdr[26] = 1;    // planes
    hdr[28] = 24;   // bits per pixel
    hdr[34] = pixBytes & 0xFF;         hdr[35] = (pixBytes >> 8) & 0xFF;
    hdr[36] = (pixBytes >> 16) & 0xFF; hdr[37] = (pixBytes >> 24) & 0xFF;

    const uint32_t ROWS = 24;
    uint8_t *chunk = (uint8_t *)heap_caps_malloc(rowBytes * ROWS,
                                                 MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!chunk) {
        server.send(500, "text/plain", "screenshot alloc failed");
        return;
    }

    server.setContentLength(fileSize);
    server.send(200, "image/bmp", "");
    server.sendContent((const char *)hdr, sizeof(hdr));

    for (uint32_t y0 = 0; y0 < h; y0 += ROWS) {
        // Bail out if the browser hung up, rather than writing into the void.
        if (!server.client().connected())
            break;

        uint32_t n = (h - y0 < ROWS) ? (h - y0) : ROWS;
        memset(chunk, 0, rowBytes * n);
        for (uint32_t r = 0; r < n; r++) {
            const uint16_t *src = fb + (size_t)(y0 + r) * scale * fbW;
            uint8_t *p = chunk + (size_t)r * rowBytes;
            for (uint32_t x = 0; x < w; x++) {
                uint16_t c = src[x * scale];
                uint8_t rr = (c >> 11) & 0x1F;
                uint8_t gg = (c >> 5) & 0x3F;
                uint8_t bb = c & 0x1F;
                *p++ = (uint8_t)((bb << 3) | (bb >> 2)); // BMP stores BGR
                *p++ = (uint8_t)((gg << 2) | (gg >> 4));
                *p++ = (uint8_t)((rr << 3) | (rr >> 2));
            }
        }
        server.sendContent((const char *)chunk, rowBytes * n);

        // A full-res frame is ~1.1 MB. Without yielding, this loop starves
        // the core-0 idle task and the task watchdog panics the device
        // mid-transfer (observed: connection reset, then a reboot). One tick
        // per chunk costs ~20 ms total and keeps the system healthy.
        delay(1);
    }

    heap_caps_free(chunk);
}

static void handleUpdateEnd() {
    if (Update.hasError()) {
        server.send(500, "text/plain", "Update failed");
    } else {
        server.send(200, "text/plain", "OK");
        delay(500);
        ESP.restart();
    }
}

void begin() {
    if (running) return;

    logBuffer[0] = '\0';

    if (!MDNS.begin(CLOCK_NAME)) {
        logPrintf("[WebConfig] mDNS failed\n");
    } else {
        MDNS.addService("http", "tcp", 80);
        logPrintf("[WebConfig] mDNS: http://%s.local/\n", CLOCK_NAME);
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/status", HTTP_GET, handleApiStatus);
    server.on("/api/settings", HTTP_GET, handleApiSettings);
    server.on("/api/settings", HTTP_POST, handleApiSettings);
    server.on("/api/brightness", HTTP_GET, handleApiBrightness);
    server.on("/api/diagnostics", HTTP_GET, handleApiDiagnostics);
    server.on("/api/logs", HTTP_GET, handleApiLogs);
    server.on("/api/screenshot", HTTP_GET, handleScreenshot);
    server.on("/api/logs/clear", HTTP_POST, handleApiLogsClear);
    server.on("/update", HTTP_POST, handleUpdateEnd, handleUpdateUpload);

    server.enableCORS(true);
    server.begin();
    running = true;

    logPrintf("[WebConfig] Ready at http://%s/\n", WiFi.localIP().toString().c_str());
}

void handle() {
    if (running)
        server.handleClient();
}

String getUrl() {
    if (WiFi.status() != WL_CONNECTED)
        return String("Not connected");
    return String("http://") + WiFi.localIP().toString() + "/";
}

}
