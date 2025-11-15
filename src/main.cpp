/**
 * ESP32-C3 WS2812B LED Controller
 *
 * 功能描述:
 * - 控制WS2812B LED灯带，实现指定灯珠按指定亮度颜色亮起
 * - 支持灯光流转效果
 * - 根据麦克风采集的音频信息控制灯带
 * - 支持多种音频响应模式: 音量条、频谱显示、节拍脉冲、音高颜色
 * - 通过Web界面远程控制
 *
 * 硬件连接:
 * - LED灯带: GPIO4
 * - 音频输入: GPIO3 (MAX9814麦克风放大器)
 * - 按钮输入: GPIO9
 * - 板载LED: GPIO12, GPIO13
 *
 * 作者: 用户
 * 日期: 2025年11月
 */

#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "meridian.hpp"
#include "webui.hpp"
#include "control.hpp"
#include "enhanced_led_controller.hpp"

// 引入拆分后的模块
#include "audio_handler.h"
#include "button_handler.h"
#include "hardware_check.h"

//---------- 配置参数 ----------//

// 实际硬件连接配置
const uint8_t LED_PIN = 0; // WS2812B LED 连接到 GPIO0
const uint16_t LED_COUNT = 100;
const uint8_t BUTTON_PIN = 9;
const bool BUTTON_ACTIVE_LOW = true;
const uint8_t AUDIO_PIN = 3; // MAX9814 OUT to ADC pin (moved from 4)

static const uint16_t FLOW_INTERVAL_MS = 40;
static const uint8_t FLOW_TAIL = 8;

static const unsigned long DEBOUNCE_MS = 40;
static const unsigned long LONG_PRESS_MS = 600;

//---------- 全局变量 ----------//

// Web服务器
static WebServer server(80);

// 亮度和功率控制
uint8_t gBrightness = 64;        // 全局亮度 0..255
uint16_t gPowerLimit_mA = 1500;  // 功率限制，0=禁用
uint8_t gLedFull_mA = 60;        // 单个LED全白时的毫安数
uint32_t gLastCurrentEst_mA = 0; // 当前帧的电流估计值

uint16_t stepIndex = 0; // 当前步进索引

// 音高检测控制
bool gPitchArmed = false;              // 是否启用音高检测
float gPitchTargetHz = 440.0f;         // 目标音高
float gPitchConfThresh = 0.3f;         // 置信度阈值
float gPitchTolCents = 50.0f;          // 差异容差度
unsigned long gPitchCooldownMs = 1200; // 冷却时间
unsigned long gPitchLastHit = 0;       // 上次呼应时间

// Pitch 命中点显示状态
bool gPitchPointActive = false;
unsigned long gPitchPointLastOn = 0;

// 音高映射控制
bool gPitchMapEnable = false;  // 是否启用音高映射
float gPitchMapScale = 1.0f;   // 敏感度 0..2
float gPitchMapMinHz = 110.0f; // 最低音高 A2
float gPitchMapMaxHz = 880.0f; // 最高音高 A5

//---------- 对象实例化 ----------//

// EnhancedLEDCanvas的全局参数访问器
uint8_t &EnhancedLEDCanvas::globalBrightness() { return gBrightness; }
uint16_t &EnhancedLEDCanvas::powerLimit_mA() { return gPowerLimit_mA; }
uint8_t &EnhancedLEDCanvas::ledFull_mA() { return gLedFull_mA; }
uint32_t &EnhancedLEDCanvas::lastCurrentEst_mA() { return gLastCurrentEst_mA; }

// 运行状态和控制器
Mode mode = FLOW; // 初始模式为FLOW

// 音频分析器 - 使用优化的音频分析器
OptimizedAudioAnalyzer analyzer(AUDIO_PIN);

// LED灯带控制器 - 使用增强的LED控制器
EnhancedLEDController controller(LED_COUNT, LED_PIN, analyzer);
static EnhancedFlowEffect &flow = controller.flow();             // 流动效果引用
static EnhancedPointEffect &point = controller.point();          // 点效果引用
static EnhancedAudioEffect &audioEff = controller.audioEffect(); // 音频效果引用

// 按钮控制
DebouncedButton button(BUTTON_PIN, BUTTON_ACTIVE_LOW, DEBOUNCE_MS, LONG_PRESS_MS);

// 音频相关变量
unsigned long lastAudioLogAt = 0; // 上次音频日志输出时间

// 音频效果模式控制
uint8_t currentAudioMode = 0; // 当前音频模式
const char *audioModeNames[] = {"LEVEL_BAR", "SPECTRUM", "BEAT_PULSE", "PITCH_COLOR"};
const uint8_t AUDIO_MODE_COUNT = 4;

// TCM 模式开关：启用时，不再运行音频效果和跑马灯渲染，避免与经络系统争用LED
bool gTcmMode = false;

// TCM 经络系统初始化与路由注册（在 tcm_demo.cpp 中实现）
extern void initTcmSystem();
extern void registerTcmRoutes(WebServer &server);
extern void tcmTick();
extern void stopTcmFlow();

// 硬件检查函数已移至hardware_check.cpp

//---------- 初始化函数 ----------//

void setup()
{
  // 初始化串口通信
  Serial.begin(115200);
  delay(500); // 等待串口稳定
  Serial.println("\n\nESP32-C3 WS2812B LED Controller Starting...");

  // 检查硬件连接
  checkHardwareConnections();

  // 初始化按钮、音频分析器和灯带控制器
  button.begin();
  analyzer.begin(); // 初始化音频分析器

  // 设置全局亮度
  gBrightness = 60; // 设置亮度为60

  // 初始化LED控制器
  controller.begin();

  // 初始化中医经络系统（仅做数据初始化，不再单独开启AP）
  initTcmSystem();

  // 配置流动效果
  flow.setColor(0x00FF00); // 初始颜色为绿色
  flow.setTail(30);        // 增加拖尾长度使效果更平滑
  flow.setInterval(30);    // 减小间隔使流动更流畅
  flow.start();            // 启动流动效果

  // 禁用音频效果
  audioEff.setEnabled(false);
  audioEff.setSensitivity(1.2f); // 敏感度

  // 启动WiFi接入点并注册Web界面
  const char *apSsid = "meridian-c3";
  startAp(apSsid);

  // 注册Web界面和控制API
  registerWeb(server, apSsid, mode, gBrightness, gPowerLimit_mA, gLedFull_mA, gLastCurrentEst_mA,
              controller, analyzer, stepIndex, gPitchArmed, gPitchTargetHz, gPitchConfThresh,
              gPitchTolCents, gPitchMapEnable, gPitchMapScale, gPitchMapMinHz, gPitchMapMaxHz,
              FLOW_INTERVAL_MS, FLOW_TAIL);

  // 注册 TCM 模式控制 API：/api/tcm?enable=0/1
  server.on("/api/tcm", HTTP_GET, []() {
    if (!server.hasArg("enable")) {
      server.send(400, "application/json", "{\"ok\":false,\"error\":\"enable required\"}");
      return;
    }
    int en = server.arg("enable").toInt();
    bool newMode = (en != 0);

    if (!newMode && gTcmMode) {
      // 关闭 TCM 模式时，停止任何正在进行的经络动画
      stopTcmFlow();
    }

    gTcmMode = newMode;
    server.send(200, "application/json", String("{\"ok\":true,\"tcm\":") + (gTcmMode?"true":"false") + "}");
  });

  // 注册经络相关HTTP接口，使 /tcm 页面可以通过同一 WebServer 控制经络系统
  registerTcmRoutes(server);

  Serial.println("Setup complete, entering main loop");
}

//---------- 功能模块函数 ----------//

// 这些函数已经移动到各自的模块文件中
// updateAudioLog - 移动到 audio_handler.cpp
// handleAudioEffects - 移动到 audio_handler.cpp
// handlePitchDetection - 移动到 audio_handler.cpp
// handleButtonActions - 移动到 button_handler.cpp

//---------- 主循环 ----------//

/**
 * 主循环函数
 *
 * 该函数是程序的主要执行入口，按照以下顺序处理各个模块：
 * 1. 输入处理：检测按钮状态和采集音频数据
 * 2. 功能模块处理：处理音频、音高检测和按钮交互
 * 3. Web服务器处理：响应Web请求
 * 4. 渲染处理：更新LED灯带状态
 *
 * 整个循环设计为非阻塞式，确保各个模块能够平滑运行。
 */
void loop()
{
  // 1. 输入处理
  button.poll();   // 检测按钮状态变化

  // 如果未启用 TCM 模式，且确实需要音频相关功能时，才采集音频并运行音频逻辑
  if (!gTcmMode)
  {
    // 需要音频处理的条件：
    // 1) 已启用音频可视化效果（/api/audio?enable=1），或
    // 2) 已武装音高检测，或
    // 3) 启用了音高映射到长度功能
    bool audioNeeded = controller.audioEnabled() || gPitchArmed || gPitchMapEnable;

    if (audioNeeded)
    {
      analyzer.tick();      // 采集和分析音频数据
      updateAudioLog();     // 音频状态日志（内部已再次判断是否启用音频）
      handleAudioEffects(); // 音频效果处理
      handlePitchDetection(); // 音高检测
    }
  }

  // 按钮交互始终可用（用于切换 FLOW/STEP 等）
  handleButtonActions();

  // 3. Web服务器处理
  server.handleClient(); // 响应Web请求

  // 4. 渲染处理：只有在非 TCM 模式下才使用增强控制器驱动灯带
  if (!gTcmMode)
  {
    controller.tick(); // 更新灯带状态
  }
  else
  {
    // TCM 模式下通过 tcmTick() 推进非阻塞经络动画
    tcmTick();
  }

  // 小延时以减轻 CPU 负载并稳定帧率
  delay(2);
}
