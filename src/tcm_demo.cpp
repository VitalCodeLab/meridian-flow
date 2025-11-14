#include <Arduino.h>
#include <FastLED.h>
#include <WebServer.h>
#include <WiFi.h>
#include <time.h>
#include "meridian_tcm.hpp"
#include "enhanced_led_controller.hpp"

// 定义硬件参数
// 目前使用一条 60 颗 WS2812B 灯带，连接在 GPIO0 上
#define LED_PIN 0        // WS2812B 数据线连接到 GPIO0
#define LED_COUNT 60     // 实际灯珠数量
#define BRIGHTNESS 64    // 亮度设置 (0-255)

// 创建中医经络系统实例
static TCMMeridianSystem *meridianSystem = nullptr;
extern EnhancedLEDController controller;
extern bool gTcmMode;

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
const char* password = "12345678";         // WiFi密码

// 当前选中的经络
MeridianType currentMeridian = LUNG;

// 子午流注相关变量（供 HTTP API 使用）
bool autoModeEnabled = false;        // 自动模式开关
bool ziwuliuzhuEnabled = false;      // 子午流注开关
unsigned long lastAutoUpdateTime = 0; // 上次自动更新时间
unsigned long autoSwitchIntervalMs = 30000; // 自动切换间隔（毫秒），默认30秒

// 供主程序调用的初始化函数：只初始化经络系统和时间，同一 AP/Server 由 main.cpp 管理
void initTcmSystem() {
  if (!meridianSystem) {
    CRGB *sharedLeds = controller.canvas().leds();
    uint16_t count = controller.canvas().length();
    meridianSystem = new TCMMeridianSystem(sharedLeds, count);
  }

  meridianSystem->begin();
  meridianSystem->setBrightness(BRIGHTNESS);

  if (!meridianSystem->initFromConfig()) {
    meridianSystem->initMeridians();
    meridianSystem->initAcupoints();
  }

  // 配置时区和 NTP 服务器，但不阻塞等待时间同步，避免卡死 setup()
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // 打印初始化完成日志，便于在串口监视器中确认初始化流程已结束
  Serial.println("TCM meridian system init done (config from SPIFFS if available)");
}

// 将经络相关 HTTP 接口注册到主程序提供的 WebServer 上
void registerTcmRoutes(WebServer &server) {
  // 选择经络
  server.on("/api/select", HTTP_GET, [&server]() {
    if (server.hasArg("meridian")) {
      int meridian = server.arg("meridian").toInt();
      if (meridian >= 0 && meridian < 12) {
        currentMeridian = static_cast<MeridianType>(meridian);
        gTcmMode = true;
        if (meridianSystem) {
          meridianSystem->showMeridian(currentMeridian);
        }
        server.send(200, "text/plain", "已选择" + getMeridianChineseName(currentMeridian));
      } else {
        server.send(400, "text/plain", "无效的经络索引");
      }
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });

  // 显示当前经络
  server.on("/api/show", HTTP_GET, [&server]() {
    gTcmMode = true;
    meridianSystem->showMeridian(currentMeridian);
    server.send(200, "text/plain", "显示" + getMeridianChineseName(currentMeridian));
  });

  // 显示所有经络
  server.on("/api/showall", HTTP_GET, [&server]() {
    gTcmMode = true;
    meridianSystem->showAllMeridians();
    server.send(200, "text/plain", "显示所有经络");
  });

  // 模拟当前经络循行
  server.on("/api/flow", HTTP_GET, [&server]() {
    int speed = 30;
    if (server.hasArg("speed")) {
      speed = server.arg("speed").toInt();
      if (speed < 10) speed = 10;
      if (speed > 100) speed = 100;
    }

    gTcmMode = true;
    server.send(200, "text/plain", "正在模拟" + getMeridianChineseName(currentMeridian) + "循行");

    meridianSystem->flowMeridian(currentMeridian, 5, 100 - speed);
  });

  // 模拟全身经络循行
  server.on("/api/flowall", HTTP_GET, [&server]() {
    int speed = 30;
    if (server.hasArg("speed")) {
      speed = server.arg("speed").toInt();
      if (speed < 10) speed = 10;
      if (speed > 100) speed = 100;
    }

    gTcmMode = true;
    server.send(200, "text/plain", "正在模拟全身经络循行");

    meridianSystem->flowAllMeridians(100 - speed);
  });

  // 按当前子午流注经络循行一次
  server.on("/api/flow/current", HTTP_GET, [&server]() {
    int speed = 30;
    if (server.hasArg("speed")) {
      speed = server.arg("speed").toInt();
      if (speed < 10) speed = 10;
      if (speed > 100) speed = 100;
    }

    gTcmMode = true;
    server.send(200, "text/plain", "正在按当前子午流注经络循行");

    // 使用子午流注计算当前当令经络并执行一次循行
    meridianSystem->flowCurrentTimeMeridian(5, 100 - speed);
  });

  // 设置亮度（TCM 专用）
  server.on("/api/tcm/brightness", HTTP_GET, [&server]() {
    if (server.hasArg("value")) {
      int brightness = server.arg("value").toInt();
      if (brightness < 0) brightness = 0;
      if (brightness > 255) brightness = 255;
      meridianSystem->setBrightness(brightness);
      server.send(200, "text/plain", "亮度已设置为 " + String(brightness));
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });

  // 显示特定穴位
  server.on("/api/acupoint", HTTP_GET, [&server]() {
    if (server.hasArg("name")) {
      String name = server.arg("name");

      gTcmMode = true;
      meridianSystem->blinkAcupoint(name.c_str(), 5, 200, CRGB::White);

      for (const auto &acupoint : meridianSystem->getAcupoints()) {
        if (strcmp(acupoint.name, name.c_str()) == 0) {
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
  server.on("/api/acupoints", HTTP_GET, [&server]() {
    String json = "[";
    bool first = true;

    for (const auto &acupoint : meridianSystem->getAcupoints()) {
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
  server.on("/api/ziwuliuzhu", HTTP_GET, [&server]() {
    if (server.hasArg("enable")) {
      ziwuliuzhuEnabled = (server.arg("enable").toInt() != 0);
      meridianSystem->enableZiwuliuzhu(ziwuliuzhuEnabled);
      if (ziwuliuzhuEnabled) {
        gTcmMode = true;
      }
      server.send(200, "text/plain", ziwuliuzhuEnabled ? "子午流注已启用" : "子午流注已禁用");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });

  // 自动模式控制API
  server.on("/api/auto", HTTP_GET, [&server]() {
    if (server.hasArg("enable")) {
      autoModeEnabled = (server.arg("enable").toInt() != 0);
      server.send(200, "text/plain", autoModeEnabled ? "自动切换已启用" : "自动切换已禁用");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });

  // 自动切换间隔设置API
  server.on("/api/auto/interval", HTTP_GET, [&server]() {
    if (server.hasArg("value")) {
      int interval = server.arg("value").toInt();
      if (interval < 5) interval = 5;
      if (interval > 60) interval = 60;

      autoSwitchIntervalMs = (unsigned long)interval * 1000UL;

      server.send(200, "text/plain", "自动切换间隔已设置为" + String(interval) + "秒");
    } else {
      server.send(400, "text/plain", "缺少参数");
    }
  });

  // 获取当前活跃经络信息API
  server.on("/api/current-meridian", HTTP_GET, [&server]() {
    if (!meridianSystem->isZiwuliuzhuEnabled()) {
      server.send(200, "application/json", "{\"name\":\"未启用\",\"description\":\"未启用子午流注\"}");
      return;
    }

    MeridianType activeMeridian = meridianSystem->getCurrentActiveMeridian();
    String description = meridianSystem->getCurrentTimeSlotDescription();
    String name = getMeridianChineseName(activeMeridian);

    String json = "{\"name\":\"" + name + "\",\"description\":\"" + description + "\"}";
    server.send(200, "application/json", json);
  });
}
