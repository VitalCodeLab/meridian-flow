#pragma once
#include <Arduino.h>
#include "control.hpp"
#include "meridian.hpp"
#include "enhanced_led_controller.hpp"

// 声明外部变量
extern Mode mode;
extern DebouncedButton button;
extern EnhancedLEDController controller;
extern uint16_t stepIndex;
extern bool gLedAudioMirror;
extern uint8_t currentAudioMode;
extern const char* audioModeNames[];
extern const uint8_t AUDIO_MODE_COUNT;
extern OnboardMirror mirror;

/**
 * 按钮操作处理
 * 
 * 该函数处理所有按钮相关的交互，包括短按和长按操作。
 * 
 * 长按功能：
 * 1. 在FLOW和STEP两种模式之间切换
 * 2. 当音频效果启用时，切换不同的音频效果模式
 * 
 * 短按功能：
 * - 在FLOW模式下：启动/停止流动效果
 * - 在STEP模式下：切换到下一个点位置
 */
void handleButtonActions();
