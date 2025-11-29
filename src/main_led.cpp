#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "meridian.hpp"
#include "webui.hpp"
#include "control.hpp"
#include "enhanced_led_controller.hpp"

// 拆分后的模块
#include "audio_handler.h"
#include "button_handler.h"
#include "hardware_check.h"

//---------- 配置参数 ----------//

const uint8_t LED_PIN = 0;       // WS2812B LED 连接到 GPIO0
const uint16_t LED_COUNT = 160;   // 灯珠数量
const uint8_t BUTTON_PIN = 9;
const bool BUTTON_ACTIVE_LOW = true;
const uint8_t AUDIO_PIN = 3;     // MAX9814 OUT to ADC

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

uint16_t stepIndex = 0;          // 当前步进索引

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
bool gPitchMapEnable = false;   // 是否启用音高映射
float gPitchMapScale = 1.0f;    // 敏感度 0..2
float gPitchMapMinHz = 110.0f;  // 最低音高 A2
float gPitchMapMaxHz = 880.0f;  // 最高音高 A5

// 为 webui.hpp 提供的 TCM 模式标志（在 LED-only 固件中始终为 false）
bool gTcmMode = false;

//---------- 对象实例化 ----------//

// EnhancedLEDCanvas 的全局参数访问器
uint8_t &EnhancedLEDCanvas::globalBrightness() { return gBrightness; }
uint16_t &EnhancedLEDCanvas::powerLimit_mA() { return gPowerLimit_mA; }
uint8_t &EnhancedLEDCanvas::ledFull_mA() { return gLedFull_mA; }
uint32_t &EnhancedLEDCanvas::lastCurrentEst_mA() { return gLastCurrentEst_mA; }

// 运行状态和控制器
Mode mode = FLOW; // 初始模式为 FLOW

// 音频分析器
OptimizedAudioAnalyzer analyzer(AUDIO_PIN);

// LED 控制器
EnhancedLEDController controller(LED_COUNT, LED_PIN, analyzer);
static EnhancedFlowEffect &flow = controller.flow();
static EnhancedPointEffect &point = controller.point();
static EnhancedAudioEffect &audioEff = controller.audioEffect();

// 按钮控制
DebouncedButton button(BUTTON_PIN, BUTTON_ACTIVE_LOW, DEBOUNCE_MS, LONG_PRESS_MS);

// 音频相关变量
unsigned long lastAudioLogAt = 0; // 上次音频日志输出时间

// 音频效果模式控制
uint8_t currentAudioMode = 0; // 当前音频模式
const char *audioModeNames[] = {"LEVEL_BAR", "SPECTRUM", "BEAT_PULSE", "PITCH_COLOR"};
const uint8_t AUDIO_MODE_COUNT = 4;

//---------- 初始化函数 ----------//

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nESP32-C3 LED+Audio (LED-only firmware) Starting...");

  // 硬件检查
  checkHardwareConnections();

  // 初始化按钮与音频分析器
  button.begin();
  analyzer.begin();

  // 设置全局亮度
  gBrightness = 60;

  // 初始化 LED 控制器
  controller.begin();

  // 配置流动效果
  flow.setColor(0x00FF00);
  flow.setTail(30);
  flow.setInterval(30);
  flow.start();

  // 默认关闭音频效果
  audioEff.setEnabled(false);
  audioEff.setSensitivity(1.2f);

  // 启动 WiFi AP 并注册 Web 界面
  const char *apSsid = "meridian-c3";
  startAp(apSsid);

  registerWeb(server, apSsid, mode, gBrightness, gPowerLimit_mA, gLedFull_mA, gLastCurrentEst_mA,
              controller, analyzer, stepIndex, gPitchArmed, gPitchTargetHz, gPitchConfThresh,
              gPitchTolCents, gPitchMapEnable, gPitchMapScale, gPitchMapMinHz, gPitchMapMaxHz,
              FLOW_INTERVAL_MS, FLOW_TAIL);

  Serial.println("LED-only setup complete, entering main loop");
}

//---------- 主循环 ----------//

void loop()
{
  // 1. 按钮输入
  button.poll();

  // 2. 音频处理（仅在确实需要音频功能时才采样和处理）
  bool audioNeeded = controller.audioEnabled() || gPitchArmed || gPitchMapEnable;
  if (audioNeeded)
  {
    analyzer.tick();
    updateAudioLog();
    handleAudioEffects();
    handlePitchDetection();
  }

  // 3. 按钮交互（FLOW/STEP 模式切换等）
  handleButtonActions();

  // 4. Web 请求处理
  server.handleClient();

  // 5. 渲染 LED（不涉及 TCM，始终由主控制器驱动）
  controller.tick();

  delay(2);
}
