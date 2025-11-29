#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "meridian.hpp"
#include "enhanced_led_controller.hpp"
#include "optimized_audio.hpp"
#include "hardware_check.h"
#include "tcm_page.h"

// TCM 辅助函数（在 tcm_demo.cpp 中实现）
extern void initTcmSystem();
extern void registerTcmRoutes(WebServer &server);
extern void tcmTick();
extern void stopTcmFlow();

//---------- 硬件与全局参数 ----------//

// 供 hardware_check 使用的硬件常量
const uint8_t LED_PIN = 0;       // WS2812B 连接到 GPIO0
const uint16_t LED_COUNT = 160;   // 灯珠数量
const uint8_t BUTTON_PIN = 9;    // 仍然暴露按钮引脚，方便后续扩展
const bool BUTTON_ACTIVE_LOW = true;
const uint8_t AUDIO_PIN = 3;     // 不主动使用音频，但保持一致的引脚定义

uint16_t gPowerLimit_mA = 1500;  // 功率限制，供电流估算打印使用

// EnhancedLEDCanvas 的功率与亮度相关全局变量
uint8_t gBrightness = 64;        // 虽然 TCM 主要用自己的亮度设置，这里保持与主工程一致
uint8_t gLedFull_mA = 60;
uint32_t gLastCurrentEst_mA = 0;

// 为 meridian_tcm / tcm_demo 提供的 TCM 模式标志
bool gTcmMode = true;            // TCM-only 固件中始终处于 TCM 模式

// Web 服务器
static WebServer server(80);

//----------- EnhancedLEDCanvas 全局访问器实现 -----------//

uint8_t &EnhancedLEDCanvas::globalBrightness() { return gBrightness; }
uint16_t &EnhancedLEDCanvas::powerLimit_mA() { return gPowerLimit_mA; }
uint8_t &EnhancedLEDCanvas::ledFull_mA() { return gLedFull_mA; }
uint32_t &EnhancedLEDCanvas::lastCurrentEst_mA() { return gLastCurrentEst_mA; }

//----------- LED 控制器与音频分析器（仅用于共享 LED 缓冲区） -----------//

// 虽然 TCM-only 固件不做音频处理，但 EnhancedLEDController 需要一个音频分析器引用
OptimizedAudioAnalyzer analyzer(AUDIO_PIN);
EnhancedLEDController controller(LED_COUNT, LED_PIN, analyzer);

//----------- WiFi AP 启动工具函数 -----------//

static inline void startAp(const char *ssid)
{
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(ssid);
  Serial.printf("AP %s %s, IP: %s\n", ssid, ok ? "started" : "failed",
                WiFi.softAPIP().toString().c_str());
}

//----------- 初始化 -----------//

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nESP32-C3 TCM-only firmware starting...");

  // 硬件连接检查
  checkHardwareConnections();

  // 初始化 LED 控制器（用于提供共享 CRGB 缓冲区给 TCMMeridianSystem）
  controller.begin();

  // 初始化中医经络系统（会从 controller.canvas().leds() 共享灯带数据）
  initTcmSystem();

  // 启动 WiFi AP
  const char *apSsid = "meridian-tcm";
  startAp(apSsid);

  // 注册经络相关 HTTP 接口
  registerTcmRoutes(server);

  // 注册 TCM 控制页面作为根页面和 /tcm 页面
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", TCM_PAGE_HTML);
  });
  server.on("/tcm", HTTP_GET, []() {
    server.send(200, "text/html", TCM_PAGE_HTML);
  });

  // 一些常见噪音路径的简单处理
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });
  server.on("/robots.txt", HTTP_GET, []() { server.send(200, "text/plain", "User-agent: *\nDisallow: /\n"); });

  server.begin();
  Serial.println("HTTP server started for TCM-only firmware");
}

//----------- 主循环 -----------//

void loop()
{
  // 处理 HTTP 请求
  server.handleClient();

  // 推进 TCM 非阻塞动画
  tcmTick();

  // 轻微延时，降低 CPU 占用并稳定帧率
  delay(2);
}
