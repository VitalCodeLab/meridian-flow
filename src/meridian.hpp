/**
 * meridian.hpp - 核心类型定义
 * 
 * 本文件包含项目中使用的核心枚举和类型定义
 * 已精简，只保留必要的类型
 */

#pragma once
#include <Arduino.h>

/**
 * 运行模式枚举
 */
enum Mode { FLOW, STEP };

/**
 * 音频效果模式枚举
 * 已被AudioVisualizer::EffectType替代，但为了兼容性保留
 */
enum AudioEffectMode { 
  LEVEL_BAR,   // 音量条
  SPECTRUM,    // 频谱显示
  BEAT_PULSE,  // 节拍脉冲
  PITCH_COLOR  // 音高对应颜色
};

/**
 * 颜色混合工具函数
 * 在两个颜色之间按比例混合
 * 
 * @param ca 第一个颜色 (RGB格式)
 * @param cb 第二个颜色 (RGB格式)
 * @param t 混合比例 (0.0-1.0)
 * @return 混合后的颜色
 */
static inline uint32_t mixColors(uint32_t ca, uint32_t cb, float t) {
  if (t < 0.f) t = 0.f; if (t > 1.f) t = 1.f;
  uint8_t ar = (uint8_t)(ca >> 16), ag = (uint8_t)((ca >> 8) & 0xFF), ab = (uint8_t)(ca & 0xFF);
  uint8_t br = (uint8_t)(cb >> 16), bg = (uint8_t)((cb >> 8) & 0xFF), bb = (uint8_t)(cb & 0xFF);
  uint8_t rr = (uint8_t)(ar + (float)(br - ar) * t);
  uint8_t rg = (uint8_t)(ag + (float)(bg - ag) * t);
  uint8_t rb = (uint8_t)(ab + (float)(bb - ab) * t);
  return ((uint32_t)rr << 16) | ((uint32_t)rg << 8) | (uint32_t)rb;
}

/**
 * 颜色缩放工具函数
 * 按比例缩放颜色亮度
 * 
 * @param c 原始颜色 (RGB格式)
 * @param s 缩放因子 (0-255)
 * @return 缩放后的颜色
 */
static inline uint32_t scaleColor(uint32_t c, uint8_t s) {
  uint8_t r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
  uint16_t r2 = (uint16_t)r * s / 255, g2 = (uint16_t)g * s / 255, b2 = (uint16_t)b * s / 255;
  return ((uint32_t)r2 << 16) | ((uint32_t)g2 << 8) | (uint32_t)b2;
}
