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
#include "led_handler.h"
#include "hardware_check.h"

//---------- 配置参数 ----------//

// 实际硬件连接配置
const uint8_t LED_PIN = 0; // WS2812B LED 连接到 GPIO0
const uint16_t LED_COUNT = 60;
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

// 板载LED配置 - 实际硬件连接
const uint8_t ONBOARD_L1 = 12; // 板载LED1 连接到 GPIO12
const uint8_t ONBOARD_L2 = 13; // 板载LED2 连接到 GPIO13
uint16_t stepIndex = 0;        // 当前步进索引

// LEDC通道配置
const int LEDC_FREQ = 5000;  // PWM频率
const int LEDC_RES_BITS = 8; // PWM分辨率
const int LEDC_CH_L1 = 0;    // L1使用的通道
const int LEDC_CH_L2 = 1;    // L2使用的通道

// LED覆盖控制
bool gLedOverride = false;                // 是否启用LED覆盖
uint8_t gLedOverrideL1 = 0;               // L1的覆盖亮度
uint8_t gLedOverrideL2 = 0;               // L2的覆盖亮度
bool gPwmAttached = false;                // PWM是否已附加
bool gLedAudioMirror = false;             // 音频映射到板载LED
unsigned long gLedOverridePulseUntil = 0; // 覆盖脉冲结束时间

// 音高检测控制
bool gPitchArmed = false;              // 是否启用音高检测
float gPitchTargetHz = 440.0f;         // 目标音高
float gPitchConfThresh = 0.3f;         // 置信度阈值
float gPitchTolCents = 50.0f;          // 差异容差度
unsigned long gPitchCooldownMs = 1200; // 冷却时间
unsigned long gPitchLastHit = 0;       // 上次呼应时间

// 渲染目标控制
bool gRenderOnboard = true; // true=仅板载, false=灯带

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

// 按钮和板载LED控制
DebouncedButton button(BUTTON_PIN, BUTTON_ACTIVE_LOW, DEBOUNCE_MS, LONG_PRESS_MS);
OnboardMirror mirror(ONBOARD_L1, ONBOARD_L2, mode, flow);

// 音频相关变量
unsigned long lastAudioLogAt = 0; // 上次音频日志输出时间

// 音频效果模式控制
uint8_t currentAudioMode = 0; // 当前音频模式
const char *audioModeNames[] = {"LEVEL_BAR", "SPECTRUM", "BEAT_PULSE", "PITCH_COLOR"};
const uint8_t AUDIO_MODE_COUNT = 4;

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

  // 初始化按钮和灯带控制器
  button.begin();
  controller.begin();
  flow.start();
  mirror.begin();

  // 设置板载LED的PWM通道
  ledcSetup(LEDC_CH_L1, LEDC_FREQ, LEDC_RES_BITS);
  ledcSetup(LEDC_CH_L2, LEDC_FREQ, LEDC_RES_BITS);

  // 初始化音频分析器
  analyzer.begin();

  // 设置音频效果默认参数
  audioEff.setEnabled(false); // 默认禁用
  // 最大长度已在构造函数中设置
  audioEff.setSensitivity(1.2f); // 敏感度

  // 启动WiFi接入点并注册Web界面
  const char *apSsid = "meridian-c3";
  startAp(apSsid);

  // 注册Web界面和控制API
  registerWeb(server, apSsid,
              // 模式和亮度控制
              mode, gBrightness, gPowerLimit_mA, gLedFull_mA, gLastCurrentEst_mA,
              // 控制器和分析器
              controller, analyzer, stepIndex,
              // 板载LED控制
              ONBOARD_L1, ONBOARD_L2, gLedOverride, gLedOverrideL1, gLedOverrideL2, gLedAudioMirror,
              // 渲染目标
              gRenderOnboard,
              // 音高检测参数
              gPitchArmed, gPitchTargetHz, gPitchConfThresh, gPitchTolCents,
              // 音高映射参数
              gPitchMapEnable, gPitchMapScale, gPitchMapMinHz, gPitchMapMaxHz,
              // 流动效果参数
              FLOW_INTERVAL_MS, FLOW_TAIL);

  Serial.println("Setup complete, entering main loop");
}

//---------- 功能模块函数 ----------//

// 这些函数已经移动到各自的模块文件中
// updateAudioLog - 移动到 audio_handler.cpp
// handleAudioEffects - 移动到 audio_handler.cpp
// handlePitchDetection - 移动到 audio_handler.cpp
// handleButtonActions - 移动到 button_handler.cpp
// handleOnboardLEDs - 移动到 led_handler.cpp

//---------- 主循环 ----------//

/**
 * 主循环函数
 *
 * 该函数是程序的主要执行入口，按照以下顺序处理各个模块：
 * 1. 输入处理：检测按钮状态和采集音频数据
 * 2. 功能模块处理：处理音频、音高检测和按钮交互
 * 3. Web服务器处理：响应Web请求
 * 4. 渲染处理：更新LED灯带状态
 * 5. 板载LED控制：管理板载LED指示灯
 *
 * 整个循环设计为非阻塞式，确保各个模块能够平滑运行。
 */
void loop()
{
  // 1. 输入处理
  button.poll();   // 检测按钮状态变化
  analyzer.tick(); // 采集和分析音频数据

  // 2. 功能模块处理 - 调用各模块中的函数
  updateAudioLog();       // 音频状态日志
  handleAudioEffects();   // 音频效果处理
  handlePitchDetection(); // 音高检测
  handleButtonActions();  // 按钮交互

  // 3. Web服务器处理
  server.handleClient(); // 响应Web请求

  // 4. 渲染处理
  if (!gRenderOnboard)
  {
    controller.tick(); // 如果渲染到灯带，更新灯带状态
  }

  // 5. 板载LED控制
  handleOnboardLEDs(); // 管理板载LED指示灯

  // 小延时以减轻 CPU 负载并稳定帧率
  delay(2);
}
