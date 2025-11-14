#pragma once

#include <Arduino.h>

// 经络控制页面 HTML（从 tcm_demo.cpp 的 index_html 提取）
static const char TCM_PAGE_HTML[] PROGMEM = R"HTML(
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
        <button class="control-btn" onclick="flowCurrentZiwuliuzhu()">按当前子午流注循行一次</button>
        <button class="control-btn" onclick="exitTcm()">退出TCM，恢复主灯效</button>
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

    function flowCurrentZiwuliuzhu() {
      fetch('/api/flow/current?speed=' + flowSpeed)
        .then(response => response.text())
        .then(data => {
          document.getElementById('status').innerText = '状态: ' + data;
        });
    }
    
    function setBrightness(value) {
      document.getElementById('brightness-value').innerText = value;
      fetch('/api/tcm/brightness?value=' + value)
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

    function exitTcm() {
      fetch('/api/tcm?enable=0')
        .then(response => response.json())
        .then(data => {
          document.getElementById('status').innerText = '状态: 已退出TCM模式，恢复主灯效';
        })
        .catch(error => {
          document.getElementById('status').innerText = '状态: 退出TCM模式失败';
          console.error('Error exiting TCM mode:', error);
        });
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

    // 自动模式下，按设定间隔触发一次当前子午流注经络循行
    function autoFlowTick() {
      if (ziwuliuzhuEnabled && autoModeEnabled) {
        flowCurrentZiwuliuzhu();
      }
      // 根据自动切换间隔定时触发
      setTimeout(autoFlowTick, autoInterval * 1000);
    }
    
    // 每10秒更新当前经络显示
    setInterval(updateCurrentMeridian, 10000);

    // 启动子午流注自动循行轮询
    setTimeout(autoFlowTick, autoInterval * 1000);
  </script>
</body>
</html>
)HTML";

