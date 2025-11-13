#include <Arduino.h>
#include <FastLED.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>
#include "meridian_tcm.hpp"

// 定义硬件参数
#define LED_PIN 4        // WS2812B数据线连接到GPIO4
#define LED_COUNT 432    // 3米 × 144颗/米 = 432颗灯珠
#define BRIGHTNESS 64    // 亮度设置 (0-255)

// 创建中医经络系统实例
TCMMeridianSystem meridianSystem(LED_COUNT, LED_PIN);

// 创建Web服务器实例
WebServer server(80);

// WiFi配置
const char* ssid = "TCM_Meridian_System";  // WiFi名称

// 获取经络中文名称
String getMeridianChineseName(MeridianType type) {
  switch (type) {
    case LUNG: return "手太阴肺经";
    case LARGE_INTESTINE: return "手阳明大肠经";
    case STOMACH: return "足阳明胃经";
    case SPLEEN: return "足太阴脏经";
    case HEART: return "手少阴心经";
    case SMALL_INTESTINE: return "手太阳小肠经";
    case BLADDER: return "足太阳膜胱经";
    case KIDNEY: return "足少阴肾经";
    case PERICARDIUM: return "手厚阴心包经";
    case TRIPLE_ENERGIZER: return "手少阳三焦经";
    case GALLBLADDER: return "足少阳胆经";
    case LIVER: return "足厚阴肝经";
    default: return "未知经络";
  }
}
const char* password = "12345678";         // WiFi密码

// 当前选中的经络
MeridianType currentMeridian = LUNG;

// 子午流注相关变量
bool autoModeEnabled = false;        // 自动模式开关
bool ziwuliuzhuEnabled = false;      // 子午流注开关
unsigned long lastAutoUpdateTime = 0; // 上次自动更新时间

// 启动动画
void startupAnimation() {
  // 创建临时的LED数组
  CRGB leds[LED_COUNT];
  
  // 从中间向两端扩散的白光
  int middle = LED_COUNT / 2;
  for (int i = 0; i <= middle; i++) {
    // 清除所有LED
    for (int j = 0; j < LED_COUNT; j++) {
      leds[j] = CRGB::Black;
    }
    
    // 设置当前活跃的LED
    if (middle - i >= 0) {
      leds[middle - i] = CRGB::White;
    }
    if (middle + i < LED_COUNT) {
      leds[middle + i] = CRGB::White;
    }
    
    // 显示
    FastLED.showColor(CRGB::Black); // 先清除所有
    for (int j = 0; j < LED_COUNT; j++) {
      if (leds[j] != CRGB::Black) {
        meridianSystem.showPixel(j, leds[j]);
      }
    }
    delay(5);
  }
  
  // 淡出
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(10);
  }
  
  // 重置亮度
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}

// 网页HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>中医经络循行模拟系统</title>
  <style>
    body {
      font-family: 'Microsoft YaHei', Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #f5f5f5;
      color: #333;
    }
    h1 {
      color: #8b0000;
      text-align: center;
      border-bottom: 2px solid #8b0000;
      padding-bottom: 10px;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
      background-color: white;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0,0,0,0.1);
    }
    .section {
      margin-bottom: 20px;
      padding: 15px;
      border-left: 4px solid #8b0000;
      background-color: #fff8f8;
    }
    h2 {
      margin-top: 0;
      color: #8b0000;
    }
    .btn-group {
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-bottom: 15px;
    }
    button {
      padding: 10px 15px;
      border: none;
      border-radius: 5px;
      background-color: #8b0000;
      color: white;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    button:hover {
      background-color: #5c0000;
    }
    .meridian-btn {
      flex: 1 0 30%;
      margin-bottom: 10px;
      text-align: left;
    }
    .control-btn {
      flex: 1 0 45%;
    }
    .acupoint-btn {
      flex: 1 0 45%;
      background-color: #4b6584;
    }
    .acupoint-btn:hover {
      background-color: #2c3e50;
    }
    .slider-container {
      margin: 15px 0;
    }
    .slider {
      width: 100%;
    }
    .status {
      padding: 10px;
      margin-top: 15px;
      background-color: #f0f0f0;
      border-radius: 5px;
      font-family: monospace;
    }
    .info-box {
      background-color: #e8f4f8;
      border-left: 4px solid #3498db;
      padding: 10px 15px;
      margin-bottom: 15px;
      border-radius: 0 4px 4px 0;
      font-weight: 500;
    }
    .active-btn {
      background-color: #27ae60;
    }
    .active-btn:hover {
      background-color: #219653;
    }
    .acupoint-info {
      background-color: #f8f9fa;
      border: 1px solid #e9ecef;
      border-radius: 5px;
      padding: 15px;
      margin-top: 15px;
    }
    .info-row {
      margin-bottom: 8px;
      line-height: 1.5;
    }
    .info-label {
      font-weight: bold;
      color: #8b0000;
      margin-right: 5px;
    }
    .color-indicator {
      display: inline-block;
      width: 15px;
      height: 15px;
      border-radius: 50%;
      margin-right: 5px;
      vertical-align: middle;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>中医经络循行模拟系统</h1>
    
    <div class="section">
      <h2>经络选择</h2>
      <div class="btn-group">
        <button class="meridian-btn" onclick="selectMeridian(0)"><span class="color-indicator" style="background-color: red;"></span> 手太阴肺经</button>
        <button class="meridian-btn" onclick="selectMeridian(1)"><span class="color-indicator" style="background-color: orange;"></span> 手阳明大肠经</button>
        <button class="meridian-btn" onclick="selectMeridian(2)"><span class="color-indicator" style="background-color: yellow;"></span> 足阳明胃经</button>
        <button class="meridian-btn" onclick="selectMeridian(3)"><span class="color-indicator" style="background-color: #80ff00;"></span> 足太阴脾经</button>
        <button class="meridian-btn" onclick="selectMeridian(4)"><span class="color-indicator" style="background-color: green;"></span> 手少阴心经</button>
        <button class="meridian-btn" onclick="selectMeridian(5)"><span class="color-indicator" style="background-color: #00ff80;"></span> 手太阳小肠经</button>
        <button class="meridian-btn" onclick="selectMeridian(6)"><span class="color-indicator" style="background-color: blue;"></span> 足太阳膀胱经</button>
        <button class="meridian-btn" onclick="selectMeridian(7)"><span class="color-indicator" style="background-color: #4b0082;"></span> 足少阴肾经</button>
        <button class="meridian-btn" onclick="selectMeridian(8)"><span class="color-indicator" style="background-color: purple;"></span> 手厥阴心包经</button>
        <button class="meridian-btn" onclick="selectMeridian(9)"><span class="color-indicator" style="background-color: magenta;"></span> 手少阳三焦经</button>
        <button class="meridian-btn" onclick="selectMeridian(10)"><span class="color-indicator" style="background-color: pink;"></span> 足少阳胆经</button>
        <button class="meridian-btn" onclick="selectMeridian(11)"><span class="color-indicator" style="background-color: #8b0000;"></span> 足厥阴肝经</button>
      </div>
    </div>
    
    <div class="section">
      <h2>经络控制</h2>
      <div class="btn-group">
        <button class="control-btn" onclick="showMeridian()">显示当前经络</button>
        <button class="control-btn" onclick="showAllMeridians()">显示所有经络</button>
        <button class="control-btn" onclick="flowMeridian()">模拟当前经络循行</button>
        <button class="control-btn" onclick="flowAllMeridians()">模拟全身经络循行</button>
      </div>
      
      <div class="slider-container">
        <label for="brightness">亮度调节:</label>
        <input type="range" min="10" max="255" value="64" class="slider" id="brightness" onchange="setBrightness(this.value)">
        <span id="brightness-value">64</span>
      </div>
      
      <div class="slider-container">
        <label for="speed">循行速度:</label>
        <input type="range" min="10" max="100" value="30" class="slider" id="speed" onchange="setSpeed(this.value)">
        <span id="speed-value">30</span>
      </div>
    </div>
    
    <div class="section">
      <h2>子午流注控制</h2>
      <div class="info-box" id="ziwu-info">当前时间：<span id="current-time">--:--</span> | 当令经络：<span id="current-meridian">无</span></div>
      <div class="btn-group">
        <button class="control-btn" id="ziwu-toggle" onclick="toggleZiwuliuzhu()">启用子午流注</button>
        <button class="control-btn" id="auto-toggle" onclick="toggleAutoMode()">启用自动切换</button>
      </div>
      <div class="slider-container">
        <label for="auto-interval">自动切换间隔(秒):</label>
        <input type="range" min="5" max="60" value="30" class="slider" id="auto-interval" onchange="setAutoInterval(this.value)">
        <span id="auto-interval-value">30</span>
      </div>
    </div>
    
    <div class="section">
      <h2>常用穴位</h2>
      <div class="btn-group">
        <button class="acupoint-btn" onclick="showAcupoint('Hegu')">合谷穴</button>
        <button class="acupoint-btn" onclick="showAcupoint('Zusanli')">足三里穴</button>
        <button class="acupoint-btn" onclick="showAcupoint('Sanyinjiao')">三阴交穴</button>
        <button class="acupoint-btn" onclick="showAcupoint('Shenmen')">神门穴</button>
        <button class="acupoint-btn" onclick="showAcupoint('Neiguan')">内关穴</button>
        <button class="acupoint-btn" onclick="showAcupoint('Taichong')">太冲穴</button>
      </div>
      
      <div class="acupoint-info" id="acupoint-info" style="display:none;">
        <h3 id="acupoint-title">穴位信息</h3>
        <div class="info-row"><span class="info-label">拼音：</span><span id="acupoint-pinyin"></span></div>
        <div class="info-row"><span class="info-label">位置：</span><span id="acupoint-location"></span></div>
        <div class="info-row"><span class="info-label">功效：</span><span id="acupoint-functions"></span></div>
        <div class="info-row"><span class="info-label">适应症：</span><span id="acupoint-indications"></span></div>
      </div>
    </div>
    
    <div class="status" id="status">状态: 系统就绪</div>
  </div>
  
  <script>
    var currentMeridian = 0;
    var flowSpeed = 30;
    var autoInterval = 30;
    var ziwuliuzhuEnabled = false;
    var autoModeEnabled = false;
    
    // 更新时钟显示
    function updateClock() {
      const now = new Date();
      const hours = String(now.getHours()).padStart(2, '0');
      const minutes = String(now.getMinutes()).padStart(2, '0');
      const seconds = String(now.getSeconds()).padStart(2, '0');
      document.getElementById('current-time').textContent = `${hours}:${minutes}:${seconds}`;
    }
    
    // 每秒更新时钟
    setInterval(updateClock, 1000);
    
    // 选择经络
    function selectMeridian(meridian) {
      currentMeridian = meridian;
      fetch('/api/select?meridian=' + meridian)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function showMeridian() {
      fetch('/api/show')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function showAllMeridians() {
      fetch('/api/showall')
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function flowMeridian() {
      fetch('/api/flow?speed=' + flowSpeed)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function flowAllMeridians() {
      fetch('/api/flowall?speed=' + flowSpeed)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function setBrightness(value) {
      document.getElementById('brightness-value').innerText = value;
      fetch('/api/brightness?value=' + value)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function setSpeed(value) {
      flowSpeed = value;
      document.getElementById('speed-value').innerText = value;
      document.getElementById('status').innerText = '状态: 循行速度已设置为 ' + value;
    }
    
    function showAcupoint(name) {
      fetch('/api/acupoint?name=' + name)
        .then(response => response.json())
        .then(data => {
          // 更新状态
          document.getElementById('status').innerText = '状态: 正在显示' + data.chineseName + '穴';
          
          // 显示穴位信息区域
          document.getElementById('acupoint-info').style.display = 'block';
          
          // 填充穴位信息
          document.getElementById('acupoint-title').innerText = data.chineseName + ' (' + data.name + ')';
          document.getElementById('acupoint-pinyin').innerText = data.pinyin || '无数据';
          document.getElementById('acupoint-location').innerText = data.location || '无数据';
          document.getElementById('acupoint-functions').innerText = data.functions || '无数据';
          document.getElementById('acupoint-indications').innerText = data.indications || '无数据';
        })
        .catch(error => {
          document.getElementById('status').innerText = '状态: 获取穴位信息失败';
          console.error('Error:', error);
        });
    }
    
    // 子午流注相关函数
    function toggleZiwuliuzhu() {
      ziwuliuzhuEnabled = !ziwuliuzhuEnabled;
      const btn = document.getElementById('ziwu-toggle');
      
      if (ziwuliuzhuEnabled) {
        btn.classList.add('active-btn');
        btn.innerText = '禁用子午流注';
      } else {
        btn.classList.remove('active-btn');
        btn.innerText = '启用子午流注';
      }
      
      fetch('/api/ziwuliuzhu?enable=' + (ziwuliuzhuEnabled ? 1 : 0))
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
          updateCurrentMeridian();
        });
    }
    
    function toggleAutoMode() {
      autoModeEnabled = !autoModeEnabled;
      const btn = document.getElementById('auto-toggle');
      
      if (autoModeEnabled) {
        btn.classList.add('active-btn');
        btn.innerText = '禁用自动切换';
      } else {
        btn.classList.remove('active-btn');
        btn.innerText = '启用自动切换';
      }
      
      fetch('/api/auto?enable=' + (autoModeEnabled ? 1 : 0))
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function setAutoInterval(value) {
      autoInterval = value;
      document.getElementById('auto-interval-value').innerText = value;
      fetch('/api/auto/interval?value=' + value)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function updateCurrentMeridian() {
      if (ziwuliuzhuEnabled) {
        fetch('/api/current-meridian')
          .then(response => response.json())
          .then(data => {
            document.getElementById('current-meridian').textContent = data.name;
          });
      } else {
        document.getElementById('current-meridian').textContent = '未启用';
      }
    }
    
    // 每10秒更新当前经络显示
    setInterval(updateCurrentMeridian, 10000);
  </script>
</body>
</html>
)rawliteral";

// 设置Web服务器路由
void setupWebServer() {
  // 根路径返回主页
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });
  
  // 选择经络
  server.on("/api/select", HTTP_GET, []() {
    if (server.hasArg("meridian")) {
      int meridian = server.arg("meridian").toInt();
      if (meridian >= 0 && meridian < 12) {
        currentMeridian = static_cast<MeridianType>(meridian);
        server.send(200, "text/plain", "已选择" + getMeridianChineseName(currentMeridian));
      } else {
        server.send(400, "text/plain", "无效的经络索引");
      }
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 显示当前经络
  server.on("/api/show", HTTP_GET, []() {
    meridianSystem.showMeridian(currentMeridian);
    server.send(200, "text/plain", "显示" + getMeridianChineseName(currentMeridian));
  });
  
  // 显示所有经络
  server.on("/api/showall", HTTP_GET, []() {
    meridianSystem.showAllMeridians();
    server.send(200, "text/plain", "显示所有经络");
  });
  
  // 模拟当前经络循行
  server.on("/api/flow", HTTP_GET, []() {
    int speed = 30;
    if (server.hasArg("speed")) {
      speed = server.arg("speed").toInt();
      if (speed < 10) speed = 10;
      if (speed > 100) speed = 100;
    }
    
    // 发送响应
    server.send(200, "text/plain", "正在模拟" + getMeridianChineseName(currentMeridian) + "循行");
    
    // 执行循行效果
    meridianSystem.flowMeridian(currentMeridian, 5, 100 - speed);
  });
  
  // 模拟全身经络循行
  server.on("/api/flowall", HTTP_GET, []() {
    int speed = 30;
    if (server.hasArg("speed")) {
      speed = server.arg("speed").toInt();
      if (speed < 10) speed = 10;
      if (speed > 100) speed = 100;
    }
    
    // 发送响应
    server.send(200, "text/plain", "正在模拟全身经络循行");
    
    // 执行循行效果
    meridianSystem.flowAllMeridians(100 - speed);
  });
  
  // 设置亮度
  server.on("/api/brightness", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int brightness = server.arg("value").toInt();
      if (brightness < 0) brightness = 0;
      if (brightness > 255) brightness = 255;
      meridianSystem.setBrightness(brightness);
      server.send(200, "text/plain", "亮度已设置为 " + String(brightness));
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 显示特定穴位
  server.on("/api/acupoint", HTTP_GET, []() {
    if (server.hasArg("name")) {
      String name = server.arg("name");
      
      // 闪烁穴位
      meridianSystem.blinkAcupoint(name.c_str(), 5, 200, CRGB::White);
      
      // 获取穴位详细信息
      for (const auto& acupoint : meridianSystem.getAcupoints()) {
        if (strcmp(acupoint.name, name.c_str()) == 0) {
          // 创建JSON响应
          String json = "{";
          json += "\"name\":\"" + String(acupoint.name) + "\",";
          json += "\"chineseName\":\"" + String(acupoint.chineseName) + "\",";
          json += "\"pinyin\":\"" + String(acupoint.pinyin ? acupoint.pinyin : "") + "\",";
          json += "\"location\":\"" + String(acupoint.location ? acupoint.location : "") + "\",";
          json += "\"functions\":\"" + String(acupoint.functions ? acupoint.functions : "") + "\",";
          json += "\"indications\":\"" + String(acupoint.indications ? acupoint.indications : "") + "\",";
          json += "\"importance\":" + String(acupoint.importance);
          json += "}";
          
          server.send(200, "application/json", json);
          return;
        }
      }
      
      server.send(200, "text/plain", "正在显示" + name + "穴");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 获取所有穴位列表
  server.on("/api/acupoints", HTTP_GET, []() {
    String json = "[";
    bool first = true;
    
    for (const auto& acupoint : meridianSystem.getAcupoints()) {
      if (!first) json += ",";
      first = false;
      
      json += "{";
      json += "\"name\":\"" + String(acupoint.name) + "\",";
      json += "\"chineseName\":\"" + String(acupoint.chineseName) + "\",";
      json += "\"meridian\":" + String(acupoint.meridian) + ",";
      json += "\"importance\":" + String(acupoint.importance);
      json += "}";
    }
    
    json += "]";
    server.send(200, "application/json", json);
  });
  
  // 子午流注控制API
  server.on("/api/ziwuliuzhu", HTTP_GET, []() {
    if (server.hasArg("enable")) {
      ziwuliuzhuEnabled = (server.arg("enable").toInt() != 0);
      meridianSystem.enableZiwuliuzhu(ziwuliuzhuEnabled);
      server.send(200, "text/plain", ziwuliuzhuEnabled ? "子午流注已启用" : "子午流注已禁用");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 自动模式控制API
  server.on("/api/auto", HTTP_GET, []() {
    if (server.hasArg("enable")) {
      autoModeEnabled = (server.arg("enable").toInt() != 0);
      server.send(200, "text/plain", autoModeEnabled ? "自动切换已启用" : "自动切换已禁用");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 自动切换间隔设置API
  server.on("/api/auto/interval", HTTP_GET, []() {
    if (server.hasArg("value")) {
      int interval = server.arg("value").toInt();
      if (interval < 5) interval = 5;
      if (interval > 60) interval = 60;
      server.send(200, "text/plain", "自动切换间隔已设置为" + String(interval) + "秒");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });
  
  // 获取当前活跃经络信息API
  server.on("/api/current-meridian", HTTP_GET, []() {
    if (!meridianSystem.isZiwuliuzhuEnabled()) {
      server.send(200, "application/json", "{\"name\":\"未启用\",\"description\":\"未启用子午流注\"}");
      return;
    }
    
    MeridianType activeMeridian = meridianSystem.getCurrentActiveMeridian();
    String description = meridianSystem.getCurrentTimeSlotDescription();
    String name = getMeridianChineseName(activeMeridian);
    
    String json = "{\"name\":\"" + name + "\",\"description\":\"" + description + "\"}"; 
    server.send(200, "application/json", json);
  });
  
  // 启动服务器
  server.begin();
}

// 获取经络中文名称
String getMeridianChineseName(MeridianType type) {
  switch (type) {
    case LUNG: return "手太阴肺经";
    case LARGE_INTESTINE: return "手阳明大肠经";
    case STOMACH: return "足阳明胃经";
    case SPLEEN: return "足太阴脾经";
    case HEART: return "手少阴心经";
    case SMALL_INTESTINE: return "手太阳小肠经";
    case BLADDER: return "足太阳膀胱经";
    case KIDNEY: return "足少阴肾经";
    case PERICARDIUM: return "手厥阴心包经";
    case TRIPLE_ENERGIZER: return "手少阳三焦经";
    case GALLBLADDER: return "足少阳胆经";
    case LIVER: return "足厥阴肝经";
    default: return "未知经络";
  }
}

void setupBak() {
  // 初始化串口
  Serial.begin(115200);
  Serial.println("\n中医经络循行模拟系统启动中...");
  
  // 初始化经络系统
  meridianSystem.begin();
  meridianSystem.setBrightness(BRIGHTNESS);
  
  // 从配置文件初始化经络系统
  if (!meridianSystem.initFromConfig()) {
    Serial.println("从配置文件初始化经络系统失败，将使用默认配置");
    // 如果配置文件加载失败，使用默认初始化方法
    meridianSystem.initMeridians();
    meridianSystem.initAcupoints();
  }
  
  // 设置WiFi接入点
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("接入点IP地址: ");
  Serial.println(IP);
  
  // 设置时间
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // 东八区
  Serial.println("正在同步时间...");
  
  // 等待时间同步
  time_t now = 0;
  while (now < 24 * 3600) {
    delay(500);
    time(&now);
  }
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  Serial.println("当前时间: " + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min));
  
  // 设置Web服务器
  setupWebServer();
  Serial.println("HTTP服务器已启动");
  
  // 启动动画
  startupAnimation();
  
  Serial.println("系统就绪");
}

void loopBak() {
  // 处理Web服务器请求
  server.handleClient();
  
  // 子午流注自动切换处理
  unsigned long currentTime = millis();
  
  // 如果启用了自动模式和子午流注
  if (autoModeEnabled && ziwuliuzhuEnabled) {
    // 每30秒自动切换一次
    if (currentTime - lastAutoUpdateTime > 30000) { // 30秒
      lastAutoUpdateTime = currentTime;
      
      // 获取当前活跃经络
      MeridianType activeMeridian = meridianSystem.getCurrentActiveMeridian();
      
      // 流动当前经络
      meridianSystem.flowMeridian(activeMeridian, 5, 30);
      
      // 输出日志
      Serial.print("子午流注自动切换：");
      Serial.println(getMeridianChineseName(activeMeridian));
    }
  }
  
  delay(10);
}

// 启动动画
void startupAnimation() {
  // 创建临时的LED数组
  CRGB leds[LED_COUNT];
  
  // 从中间向两端扩散的白光
  int middle = LED_COUNT / 2;
  for (int i = 0; i <= middle; i++) {
    // 清除所有LED
    for (int j = 0; j < LED_COUNT; j++) {
      leds[j] = CRGB::Black;
    }
    
    // 设置当前活跃的LED
    if (middle - i >= 0) {
      leds[middle - i] = CRGB::White;
    }
    if (middle + i < LED_COUNT) {
      leds[middle + i] = CRGB::White;
    }
    
    // 显示
    FastLED.showColor(CRGB::Black); // 先清除所有
    for (int j = 0; j < LED_COUNT; j++) {
      if (leds[j] != CRGB::Black) {
        meridianSystem.showPixel(j, leds[j]);
      }
    }
    delay(5);
  }
  
  // 淡出
  for (int brightness = 255; brightness >= 0; brightness -= 5) {
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(10);
  }
  
  // 重置亮度
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
}
