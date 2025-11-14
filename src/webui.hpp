#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "meridian.hpp"
#include "enhanced_led_controller.hpp"
#include "optimized_audio.hpp"
#include "tcm_page.h"

// 使用于音频模式同步的全局变量声明
extern uint8_t currentAudioMode;
extern bool gTcmMode; // TCM 模式全局开关，由 main.cpp 定义

static inline void sendJson(WebServer &server, int code, const String &body) { server.send(code, "application/json", body); }

static inline bool parseColorArg(const String &s, uint32_t &out)
{
  int c1 = s.indexOf(',');
  if (c1 < 0)
    return false;
  int c2 = s.indexOf(',', c1 + 1);
  if (c2 < 0)
    return false;
  int r = s.substring(0, c1).toInt();
  int g = s.substring(c1 + 1, c2).toInt();
  int b = s.substring(c2 + 1).toInt();
  if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
    return false;
  out = ((uint32_t)(uint8_t)r << 16) | ((uint32_t)(uint8_t)g << 8) | ((uint32_t)(uint8_t)b);
  return true;
}

static inline bool parseNoteToHz(const String &s, float &hzOut)
{
  // Accept forms: "440" (Hz), or note names like A4, C#5, Db3
  String t = s;
  t.trim();
  // try numeric Hz
  bool allDigit = true;
  int dots = 0;
  for (size_t i = 0; i < t.length(); ++i)
  {
    char c = t[i];
    if (!(c >= '0' && c <= '9'))
    {
      if (c == '.' && dots == 0)
      {
        dots++;
      }
      else
      {
        allDigit = false;
        break;
      }
    }
  }
  if (allDigit && t.length() > 0)
  {
    hzOut = t.toFloat();
    return hzOut > 0;
  }
  if (t.length() < 2)
    return false;
  char n = t[0];
  int semiBase = -1000; // semitone offset of note letter from A within octave
  switch (n)
  {
  case 'C':
    semiBase = -9;
    break;
  case 'D':
    semiBase = -7;
    break;
  case 'E':
    semiBase = -5;
    break;
  case 'F':
    semiBase = -4;
    break;
  case 'G':
    semiBase = -2;
    break;
  case 'A':
    semiBase = 0;
    break;
  case 'B':
    semiBase = 2;
    break;
  default:
    return false;
  }
  int idx = 1;
  int accidental = 0;
  if (idx < t.length())
  {
    if (t[idx] == '#' || t[idx] == '+')
    {
      accidental = +1;
      idx++;
    }
    else if (t[idx] == 'b' || t[idx] == 'B')
    {
      accidental = -1;
      idx++;
    }
  }
  if (idx >= t.length())
    return false;
  int octave = t.substring(idx).toInt();
  if (octave < -1 || octave > 9)
    return false;
  // MIDI number: A4 = 69 at 440 Hz
  int midi = 69 + (octave - 4) * 12 + (semiBase + accidental);
  hzOut = 440.0f * powf(2.0f, (midi - 69) / 12.0f);
  return hzOut > 0;
}

inline void startAp(const char *ssid)
{
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(ssid);
  Serial.printf("AP %s %s, IP: %s\n", ssid, ok ? "started" : "failed", WiFi.softAPIP().toString().c_str());
}

inline void registerWeb(
    WebServer &server,
    const char *apSsid,
    Mode &mode,
    uint8_t &gBrightness,
    uint16_t &powerLimit_mA,
    uint8_t &ledFull_mA,
    uint32_t &lastCurrentEst_mA,
    EnhancedLEDController &ctrl,
    OptimizedAudioAnalyzer &analyzer,
    uint16_t &stepIndex,
    bool &pitchArmed,
    float &pitchTargetHz,
    float &pitchConfThresh,
    float &pitchTolCents,
    bool &pitchMapEnable,
    float &pitchMapScale,
    float &pitchMapMinHz,
    float &pitchMapMaxHz,
    uint16_t defaultIntervalMs,
    uint8_t defaultTail)
{
  // Root UI
  server.on("/", HTTP_GET, [&server]()
            {
    static const char html[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>Meridian Control</title>
<style>
  body{font-family:system-ui,Arial;margin:16px;line-height:1.4;background:#f0f0f0;color:#333}
  .container{max-width:800px;margin:0 auto;background:#fff;padding:20px;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1)}
  h2{color:#2c3e50;border-bottom:2px solid #3498db;padding-bottom:10px;margin-top:0}
  .row{margin:12px 0;display:flex;align-items:center}
  label{display:inline-block;width:140px;font-weight:500}
  input[type=number]{width:90px;padding:5px;border:1px solid #ddd;border-radius:4px}
  .mono{font-family:ui-monospace,Consolas,monospace;background:#f8f8f8;padding:10px;border-radius:4px;overflow:auto;max-height:300px}
  button{margin-right:8px;background:#3498db;color:white;border:none;padding:8px 12px;border-radius:4px;cursor:pointer;transition:background 0.3s}
  button:hover{background:#2980b9}
  .card{background:#f9f9f9;border-left:4px solid #3498db;padding:10px;margin:10px 0;border-radius:0 4px 4px 0}
  .audio-modes{display:flex;flex-wrap:wrap;gap:10px;margin-top:10px}
  .audio-mode{padding:8px 12px;border:2px solid #ddd;border-radius:4px;cursor:pointer;transition:all 0.3s}
  .audio-mode.active{border-color:#3498db;background:#e1f0fa}
</style>
</head>
<body>
<div class="container">
  <h2>Meridian Control</h2>
  
  <div class="card">
    <div class=row>
      <label>Flow Control</label>
      <button onclick="fetch('/api/flow/start')">Start</button>
      <button onclick="fetch('/api/flow/stop')">Stop</button>
    </div>
  </div>
  
  <div class="card">
    <div class=row>
      <label>Audio Effects</label>
      <button onclick="fetch('/api/audio?enable=1')">Enable</button>
      <button onclick="fetch('/api/audio?enable=0')">Disable</button>
    </div>
    
    <div class=row>
      <label>Audio Mode</label>
      <div class="audio-modes">
        <div class="audio-mode" id="mode0" onclick="setAudioMode(0)">Level Bar</div>
        <div class="audio-mode" id="mode1" onclick="setAudioMode(1)">Spectrum</div>
        <div class="audio-mode" id="mode2" onclick="setAudioMode(2)">Beat Pulse</div>
        <div class="audio-mode" id="mode3" onclick="setAudioMode(3)">Pitch Color</div>
      </div>
    </div>
  </div>
  
  <div class="card">
    <div class=row>
      <label>Pitch Detection</label>
      <button onclick="armPitch()">Arm</button>
      <button onclick="fetch('/api/pitch?arm=0')">Disarm</button>
    </div>
    
    <div class=row>
      <label>Pitch→Length</label>
      <label><input type=checkbox id=pm onchange="setPitchMap()"> Enable</label>
    </div>
  </div>
  
  <div class="card">
    <div class=row>
      <label>Brightness</label>
      <input type="range" min="0" max="255" value="64" id="brightness" oninput="updateBrightness()">
      <span id="brightnessValue">64</span>
    </div>
  </div>
  
  <h3>System Status</h3>
  <pre id=state class=mono>{}</pre>
</div>

<script>
async function armPitch(){
  await fetch(`/api/pitch?arm=1`);
}

async function setPitchMap(){
  const en=document.getElementById('pm').checked?1:0;
  await fetch(`/api/pitchmap?enable=${en}`);
}

async function setAudioMode(mode){
  await fetch(`/api/audio/mode?mode=${mode}`);
  updateAudioModeUI(mode);
}

function updateAudioModeUI(mode) {
  // Remove active class from all modes
  document.querySelectorAll('.audio-mode').forEach(el => {
    el.classList.remove('active');
  });
  // Add active class to selected mode
  document.getElementById(`mode${mode}`).classList.add('active');
}

async function updateBrightness() {
  const value = document.getElementById('brightness').value;
  document.getElementById('brightnessValue').textContent = value;
  await fetch(`/api/brightness?value=${value}`);
}

async function setTcm(enable){
  try{
    await fetch(`/api/tcm?enable=${enable}`);
  }catch(e){
    console.error('Error toggling TCM mode:', e);
  }
}

async function poll(){
  try{
    const r=await fetch('/api/state');
    const j=await r.json();
    document.getElementById('state').textContent=JSON.stringify(j,null,2);
    
    // Update UI based on state
    document.getElementById('brightness').value = j.brightness;
    document.getElementById('brightnessValue').textContent = j.brightness;
    document.getElementById('pm').checked = j.pitchmap && j.pitchmap.enable;
    
    // Update TCM mode status
    if (typeof j.tcm === 'boolean') {
      const el = document.getElementById('tcmStatus');
      if (el) {
        el.textContent = j.tcm ? 'ON' : 'OFF';
      }
    }
    
    // Update audio mode
    if (j.audio && typeof j.audio.mode === 'number') {
      updateAudioModeUI(j.audio.mode);
    }
  } catch(e){
    console.error('Error polling state:', e);
  } finally{
    setTimeout(poll, 800);
  }
}

// Start polling
poll();
</script>
</body></html>
)HTML";
    server.send(200, "text/html", html); });

  // TCM meridian control page
  server.on("/tcm", HTTP_GET, [&server]()
            {
    server.send(200, "text/html", TCM_PAGE_HTML);
  });

  // Pitch map: /api/pitchmap?enable=1&scale=1.0&min=110&max=880
  server.on("/api/pitchmap", HTTP_GET, [&]()
            { if (server.hasArg("enable")) pitchMapEnable = (server.arg("enable").toInt()!=0); sendJson(server,200,String("{\"ok\":true,\"enable\":") + (pitchMapEnable?"true":"false") + "}"); });

  // One-click: enable audio, override and mirror
  // removed one-click endpoint

  // Pitch: /api/pitch?arm=1&target=A4|440&conf=0.3&tol=50  or arm=0 to disarm
  server.on("/api/pitch", HTTP_GET, [&]()
            {
    if (!server.hasArg("arm")) { sendJson(server,400,"{\"ok\":false,\"error\":\"arm required\"}"); return; }
    int a = server.arg("arm").toInt();
    pitchArmed = (a!=0);

    // 当关闭音高检测时，清除由 Pitch 命中产生的点效果，避免 LED 长亮无法通过 Web UI 清掉
    if (!pitchArmed) {
      ctrl.clearPoint();
    }

    sendJson(server,200, String("{\"ok\":true,\"armed\":") + (pitchArmed?"true":"false") + "}"); });

  // Audio control: /api/audio?enable=0/1 or /api/audio?set=1&sens=f&maxLen=n&low=r,g,b&mid=r,g,b&high=r,g,b
  server.on("/api/audio", HTTP_GET, [&]()
            {
    if (!server.hasArg("enable")) { sendJson(server,400,"{\"ok\":false,\"error\":\"enable required\"}"); return; }
    bool en = (server.arg("enable").toInt()!=0);
    ctrl.enableAudio(en);

    // 当关闭音频效果时，同时关闭 Pitch 检测和 Pitch→Length，并清除点效果，避免残留LED长亮
    if (!en) {
      pitchArmed = false;
      pitchMapEnable = false;
      ctrl.clearPoint();
    }

    sendJson(server, 200, "{\"ok\":true}"); });

  // Audio mode control: /api/audio/mode?mode=0|1|2|3
  server.on("/api/audio/mode", HTTP_GET, [&]()
            {
    if (!server.hasArg("mode")) { sendJson(server,400,"{\"ok\":false,\"error\":\"mode required\"}"); return; }
    int mode = server.arg("mode").toInt();
    if (mode < 0) mode = 0;
    if (mode > 3) mode = 3;

    // 同步更新全局音频模式，以避免在主循环中被恢复为默认值
    currentAudioMode = static_cast<uint8_t>(mode);

    ctrl.setAudioMode(static_cast<AudioVisualizer::EffectType>(mode));
    sendJson(server, 200, String("{\"ok\":true,\"mode\":") + String(mode) + "}"); });
  server.on("/index.html", HTTP_GET, [&server]()
            { server.sendHeader("Location","/"); server.send(302); });

  // API
  server.on("/api/state", HTTP_GET, [&]()
            {
    String s = "{";
    s += "\"mode\":\""; s += (mode==FLOW?"FLOW":"STEP"); s += "\",";
    s += "\"brightness\":"; s += String((int)gBrightness); s += ",";
    s += "\"power\":{";
      s += "\"limit_ma\":"; s += String((int)powerLimit_mA); s += ",";
      s += "\"estimated_ma\":"; s += String((int)lastCurrentEst_mA);
    s += "},";
    s += "\"flow\":{";
      s += "\"running\":"; s += ctrl.flow().running()?"true":"false"; s += ",";
      s += "\"interval_ms\":"; s += String((int)defaultIntervalMs); s += ",";
      s += "\"tail\":"; s += String((int)defaultTail);
    s += "},";
    s += "\"audio\":{";
      s += "\"enabled\":"; s += ctrl.audioEnabled()?"true":"false"; s += ",";
      s += "\"mode\":"; s += String((int)ctrl.getAudioMode());
    s += "},";
    s += "\"tcm\":"; s += gTcmMode?"true":"false"; s += ",";
    s += "\"pitchmap\":{";
      s += "\"enable\":"; s += pitchMapEnable?"true":"false"; s += ",";
      s += "\"scale\":"; s += String(pitchMapScale,2); s += ",";
      s += "\"min\":"; s += String(pitchMapMinHz,0); s += ",";
      s += "\"max\":"; s += String(pitchMapMaxHz,0);
    s += "},";
    s += "\"pitch\":{";
      s += "\"armed\":"; s += pitchArmed?"true":"false"; s += ",";
      s += "\"target_hz\":"; s += String(pitchTargetHz,2); s += ",";
      s += "\"conf\":"; s += String(pitchConfThresh,2); s += ",";
      s += "\"tol_cents\":"; s += String(pitchTolCents,0);
    s += "},";
    s += "\"point\":{";
      s += "\"index\":"; s += String((int)stepIndex);
    s += "}";
    s += "}";
    sendJson(server, 200, s); });

  server.on("/api/flow/start", HTTP_GET, [&]()
            { ctrl.startFlow(); sendJson(server, 200, "{\"ok\":true}"); });
  server.on("/api/flow/stop", HTTP_GET, [&]()
            { ctrl.stopFlow();  sendJson(server, 200, "{\"ok\":true}"); });

  // removed flow config and point endpoints

  server.on("/api/brightness", HTTP_GET, [&]()
            {
    if (!server.hasArg("value")) { sendJson(server, 400, "{\"ok\":false,\"error\":\"value required\"}"); return; }
    int v = server.arg("value").toInt();
    if (v < 0) v = 0; if (v > 255) v = 255;
    gBrightness = (uint8_t)v;
    sendJson(server, 200, String("{\"ok\":true,\"brightness\":") + String((int)gBrightness) + "}"); });

  server.on("/api/power", HTTP_GET, [&]()
            {
    bool changed = false;
    if (server.hasArg("limit_ma")) { int v = server.arg("limit_ma").toInt(); if (v<0) v=0; if (v>100000) v=100000; powerLimit_mA = (uint16_t)v; changed = true; }
    if (server.hasArg("led_full_ma")) { int v = server.arg("led_full_ma").toInt(); if (v<=0) v=60; if (v>120) v=120; ledFull_mA = (uint8_t)v; changed = true; }
    String s = "{";
    s += "\"ok\":true,\"changed\":"; s += changed?"true":"false"; s += ",";
    s += "\"limit_ma\":"; s += String((int)powerLimit_mA); s += ",";
    s += "\"led_full_ma\":"; s += String((int)ledFull_mA); s += ",";
    s += "\"estimated_ma\":"; s += String((int)lastCurrentEst_mA);
    s += "}";
    sendJson(server, 200, s); });

  // Noise handlers
  server.on("/favicon.ico", HTTP_GET, [&server]()
            { server.send(204); });
  server.on("/robots.txt", HTTP_GET, [&server]()
            { server.send(200, "text/plain", "User-agent: *\nDisallow: /\n"); });
  server.on("/generate_204", HTTP_GET, [&server]()
            { server.send(204); });
  server.on("/hotspot-detect.html", HTTP_GET, [&server]()
            { server.sendHeader("Location","/"); server.send(302); });
  server.onNotFound([&server]()
                    {
    String s = String("{\"ok\":false,\"error\":\"not found\",\"path\":\"") + server.uri() + "\"}";
    server.send(404, "application/json", s); });

  server.begin();
  Serial.printf("HTTP server started on %s\n", WiFi.softAPIP().toString().c_str());
}
