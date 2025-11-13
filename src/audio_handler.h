#pragma once
#include <Arduino.h>
#include "optimized_audio.hpp"
#include "enhanced_led_controller.hpp"

// 声明外部变量
extern OptimizedAudioAnalyzer analyzer;
extern EnhancedLEDController controller;
extern uint8_t currentAudioMode;
extern const char* audioModeNames[];
extern unsigned long lastAudioLogAt;
extern bool gPitchMapEnable;
extern float gPitchMapScale;
extern float gPitchMapMinHz;
extern float gPitchMapMaxHz;
extern const uint16_t LED_COUNT;
extern bool gPitchArmed;
extern float gPitchTargetHz;
extern float gPitchConfThresh;
extern float gPitchTolCents;
extern unsigned long gPitchCooldownMs;
extern unsigned long gPitchLastHit;
extern uint16_t stepIndex;

// 函数声明
/**
 * 音频状态日志输出
 * 
 * 每500毫秒输出一次音频状态信息，包括：
 * - 总音量级别 (level)
 * - 低频能量 (low)
 * - 中频能量 (mid)
 * - 高频能量 (high)
 * - 音频效果是否启用 (en)
 * - 当前音频效果模式 (mode)
 */
void updateAudioLog();

/**
 * 音频效果模式处理
 * 
 * 该函数处理两个主要任务：
 * 1. 设置当前音频效果模式（音量条、频谱显示、节拍脉冲、音高颜色）
 * 2. 处理音高到灯带长度的映射功能
 * 
 * 音高映射功能允许灯带长度根据检测到的音高自动调整，
 * 低音高对应短灯带，高音高对应长灯带。
 */
void handleAudioEffects();

/**
 * 音高检测处理
 * 
 * 该函数实现音高检测功能，当检测到的音高与目标音高在指定容差范围内时，
 * 触发视觉反馈。
 * 
 * 工作流程：
 * 1. 检查音高检测是否启用
 * 2. 获取当前音高和置信度
 * 3. 检查是否满足触发条件（音高有效、置信度足够、超过冷却时间）
 * 4. 计算音高差异（以音分为单位）
 * 5. 如果差异在容差范围内，触发视觉反馈
 * 
 * 音分(cents)是音乐中的小单位，1个半音等于100音分
 */
void handlePitchDetection();
