#pragma once
#include <Arduino.h>
#include "meridian.hpp"
#include "optimized_audio.hpp"
#include "enhanced_led_controller.hpp"

// 前向声明
class OnboardMirror;

// 声明外部变量
extern bool gLedOverride;
extern uint8_t gLedOverrideL1;
extern uint8_t gLedOverrideL2;
extern bool gPwmAttached;
extern bool gLedAudioMirror;
extern unsigned long gLedOverridePulseUntil;
extern bool gRenderOnboard;
extern OnboardMirror mirror;
extern OptimizedAudioAnalyzer analyzer;
extern const uint8_t ONBOARD_L1;
extern const uint8_t ONBOARD_L2;
extern const int LEDC_CH_L1;
extern const int LEDC_CH_L2;

/**
 * 板载LED控制
 * 
 * 该函数管理ESP32-C3板载的LED指示灯，支持三种工作模式：
 * 1. 标准模式：通过OnboardMirror类提供模式和状态指示
 * 2. 覆盖模式：直接控制LED亮度
 * 3. 音频映射模式：将音频特征映射到LED亮度
 * 
 * 该函数还处理PWM通道的附加和分离，以及脉冲效果的衰减。
 */
void handleOnboardLEDs();
