#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include "optimized_audio.hpp"

/**
 * 音频可视化效果基类
 */
class AudioEffect {
public:
  virtual ~AudioEffect() {}
  virtual void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) = 0;
};

/**
 * 音量条效果
 * 显示一个随音量变化的彩色条
 */
class VUMeterEffect : public AudioEffect {
public:
  VUMeterEffect() {}
  
  void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) override {
    // 获取音量并应用敏感度
    float level = analyzer.level() * sensitivity_;
    if (level > 1.0f) level = 1.0f;
    
    // 计算要点亮的LED数量
    int activeLength = (int)(level * numLeds);
    
    // 渐变色填充
    for (int i = 0; i < numLeds; i++) {
      if (i < activeLength) {
        // 计算此LED的颜色 (从蓝到红的渐变)
        float hue = 160.0f - (i * 160.0f / numLeds);
        leds[i] = CHSV((int)hue, 255, 255);
      } else {
        // 不活跃的LED设为黑色
        leds[i] = CRGB::Black;
      }
    }
  }
  
  void setSensitivity(float sensitivity) {
    sensitivity_ = constrain(sensitivity, 0.1f, 5.0f);
  }
  
private:
  float sensitivity_ = 1.5f;
};

/**
 * 频谱显示效果
 * 将低、中、高频能量映射到不同区域
 */
class SpectrumEffect : public AudioEffect {
public:
  SpectrumEffect() {}
  
  void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) override {
    // 将灯带分成三个区域
    int lowSection = numLeds / 3;
    int midSection = numLeds / 3;
    int highSection = numLeds - lowSection - midSection;
    
    // 获取频段能量
    float lowLevel = analyzer.low() * sensitivity_;
    float midLevel = analyzer.mid() * sensitivity_;
    float highLevel = analyzer.high() * sensitivity_;
    
    // 限制范围
    lowLevel = constrain(lowLevel, 0.0f, 1.0f);
    midLevel = constrain(midLevel, 0.0f, 1.0f);
    highLevel = constrain(highLevel, 0.0f, 1.0f);
    
    // 计算活跃LED数量
    int lowActive = (int)(lowLevel * lowSection);
    int midActive = (int)(midLevel * midSection);
    int highActive = (int)(highLevel * highSection);
    
    // 渲染低频区域 (蓝色)
    for (int i = 0; i < lowSection; i++) {
      leds[i] = (i < lowActive) ? CRGB::Blue : CRGB::Black;
    }
    
    // 渲染中频区域 (绿色)
    for (int i = 0; i < midSection; i++) {
      leds[i + lowSection] = (i < midActive) ? CRGB::Green : CRGB::Black;
    }
    
    // 渲染高频区域 (红色)
    for (int i = 0; i < highSection; i++) {
      leds[i + lowSection + midSection] = (i < highActive) ? CRGB::Red : CRGB::Black;
    }
  }
  
  void setSensitivity(float sensitivity) {
    sensitivity_ = constrain(sensitivity, 0.1f, 5.0f);
  }
  
private:
  float sensitivity_ = 1.5f;
};

/**
 * 节拍脉冲效果
 * 检测音频中的节拍并产生脉冲效果
 */
class BeatPulseEffect : public AudioEffect {
public:
  BeatPulseEffect() {}
  
  void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) override {
    // 获取当前音量
    float currentLevel = analyzer.level();
    
    // 检测节拍
    unsigned long now = millis();
    bool isBeat = false;
    
    if (now - lastBeatTime_ > beatCooldownMs_) {
      if (currentLevel > lastLevel_ * (1.0f + beatThreshold_)) {
        isBeat = true;
        lastBeatTime_ = now;
        beatIntensity_ = 1.0f;
      }
    }
    
    // 脉冲衰减
    if (!isBeat && beatIntensity_ > 0) {
      beatIntensity_ -= 0.05f;
      if (beatIntensity_ < 0) beatIntensity_ = 0;
    }
    
    // 更新上一帧的音量级别
    lastLevel_ = currentLevel * 0.8f + lastLevel_ * 0.2f;
    
    // 根据脉冲强度渲染所有LED
    CRGB color;
    if (beatIntensity_ > 0.7f) {
      color = CRGB::Red;
    } else if (beatIntensity_ > 0.3f) {
      color = CRGB::Green;
    } else {
      color = CRGB::Blue;
    }
    
    // 调整颜色亮度
    uint8_t brightness = (uint8_t)(beatIntensity_ * 255.0f);
    color.nscale8(brightness);
    
    // 应用到所有LED
    for (int i = 0; i < numLeds; i++) {
      leds[i] = color;
    }
  }
  
  void setBeatThreshold(float threshold) {
    beatThreshold_ = constrain(threshold, 0.1f, 1.0f);
  }
  
  void setBeatCooldown(unsigned long cooldownMs) {
    beatCooldownMs_ = cooldownMs;
  }
  
private:
  float lastLevel_ = 0.0f;
  float beatIntensity_ = 0.0f;
  float beatThreshold_ = 0.3f;
  unsigned long lastBeatTime_ = 0;
  unsigned long beatCooldownMs_ = 200;
};

/**
 * 音高颜色效果
 * 根据检测到的音高改变颜色
 */
class PitchColorEffect : public AudioEffect {
public:
  PitchColorEffect() {}
  
  void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) override {
    // 获取音高和置信度
    float pitch = analyzer.pitchHz();
    float confidence = analyzer.pitchConf();
    
    // 如果音高检测置信度太低，使用音量条模式
    if (pitch <= 0 || confidence < 0.3f) {
      renderFallback(leds, numLeds, analyzer);
      return;
    }
    
    // 将音高映射到色相角度 (0-360度)
    float hue = 0;
    if (pitch >= minHz_ && pitch <= maxHz_) {
      // 对数映射更符合人耳感知
      hue = 360.0f * log(pitch / minHz_) / log(maxHz_ / minHz_);
    }
    
    // 转换为HSV颜色
    CHSV hsv((uint8_t)(hue * 255.0f / 360.0f), 255, 255);
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    
    // 根据音量决定显示长度
    float level = analyzer.level() * sensitivity_;
    if (level > 1.0f) level = 1.0f;
    int activeLength = (int)(level * numLeds);
    
    // 应用颜色到LED
    for (int i = 0; i < numLeds; i++) {
      if (i < activeLength) {
        leds[i] = rgb;
      } else {
        leds[i] = CRGB::Black;
      }
    }
  }
  
  void setSensitivity(float sensitivity) {
    sensitivity_ = constrain(sensitivity, 0.1f, 5.0f);
  }
  
  void setPitchRange(float minHz, float maxHz) {
    minHz_ = minHz;
    maxHz_ = maxHz;
  }
  
private:
  void renderFallback(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) {
    // 简单的音量条作为后备效果
    float level = analyzer.level() * sensitivity_;
    if (level > 1.0f) level = 1.0f;
    int activeLength = (int)(level * numLeds);
    
    for (int i = 0; i < numLeds; i++) {
      leds[i] = (i < activeLength) ? CRGB::Blue : CRGB::Black;
    }
  }
  
  float sensitivity_ = 1.5f;
  float minHz_ = 110.0f; // A2
  float maxHz_ = 880.0f; // A5
};

/**
 * 音频可视化管理器
 * 管理不同的音频效果并在它们之间切换
 */
class AudioVisualizer {
public:
  enum EffectType {
    EFFECT_VU_METER = 0,
    EFFECT_SPECTRUM,
    EFFECT_BEAT_PULSE,
    EFFECT_PITCH_COLOR,
    EFFECT_COUNT
  };
  
  AudioVisualizer() {
    // 创建效果
    effects_[EFFECT_VU_METER] = new VUMeterEffect();
    effects_[EFFECT_SPECTRUM] = new SpectrumEffect();
    effects_[EFFECT_BEAT_PULSE] = new BeatPulseEffect();
    effects_[EFFECT_PITCH_COLOR] = new PitchColorEffect();
    
    // 默认效果
    currentEffect_ = EFFECT_VU_METER;
  }
  
  ~AudioVisualizer() {
    for (int i = 0; i < EFFECT_COUNT; i++) {
      delete effects_[i];
    }
  }
  
  void render(CRGB* leds, int numLeds, OptimizedAudioAnalyzer& analyzer) {
    if (enabled_ && effects_[currentEffect_]) {
      effects_[currentEffect_]->render(leds, numLeds, analyzer);
    }
  }
  
  void setEffect(EffectType effect) {
    if (effect >= 0 && effect < EFFECT_COUNT) {
      currentEffect_ = effect;
    }
  }
  
  EffectType getCurrentEffect() const {
    return currentEffect_;
  }
  
  void setEnabled(bool enabled) {
    enabled_ = enabled;
  }
  
  bool isEnabled() const {
    return enabled_;
  }
  
  void setSensitivity(float sensitivity) {
    // 设置所有效果的敏感度
    ((VUMeterEffect*)effects_[EFFECT_VU_METER])->setSensitivity(sensitivity);
    ((SpectrumEffect*)effects_[EFFECT_SPECTRUM])->setSensitivity(sensitivity);
    ((PitchColorEffect*)effects_[EFFECT_PITCH_COLOR])->setSensitivity(sensitivity);
  }
  
  void setBeatThreshold(float threshold) {
    ((BeatPulseEffect*)effects_[EFFECT_BEAT_PULSE])->setBeatThreshold(threshold);
  }
  
  void setBeatCooldown(unsigned long cooldownMs) {
    ((BeatPulseEffect*)effects_[EFFECT_BEAT_PULSE])->setBeatCooldown(cooldownMs);
  }
  
  void setPitchRange(float minHz, float maxHz) {
    ((PitchColorEffect*)effects_[EFFECT_PITCH_COLOR])->setPitchRange(minHz, maxHz);
  }
  
private:
  AudioEffect* effects_[EFFECT_COUNT];
  EffectType currentEffect_ = EFFECT_VU_METER;
  bool enabled_ = false;
};
