#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include "optimized_audio.hpp"
#include "audio_visualizer.hpp"

// 定义最大LED数量
#define MAX_LEDS 300

/**
 * FastLED版本的灯带画布类
 */
class EnhancedLEDCanvas {
public:
  explicit EnhancedLEDCanvas(uint16_t count, uint8_t pin) : numLeds_(count), pin_(pin) {
    // 初始化FastLED
    switch(pin) {
      case 0: FastLED.addLeds<WS2812B, 0>(leds_, numLeds_); break;  // 添加GPIO0支持
      case 2: FastLED.addLeds<WS2812B, 2>(leds_, numLeds_); break;
      case 3: FastLED.addLeds<WS2812B, 3>(leds_, numLeds_); break;
      case 4: FastLED.addLeds<WS2812B, 4>(leds_, numLeds_); break;
      case 5: FastLED.addLeds<WS2812B, 5>(leds_, numLeds_); break;
      case 6: FastLED.addLeds<WS2812B, 6>(leds_, numLeds_); break;
      case 7: FastLED.addLeds<WS2812B, 7>(leds_, numLeds_); break;
      case 8: FastLED.addLeds<WS2812B, 8>(leds_, numLeds_); break;
      case 9: FastLED.addLeds<WS2812B, 9>(leds_, numLeds_); break;
      case 10: FastLED.addLeds<WS2812B, 10>(leds_, numLeds_); break;
      default: FastLED.addLeds<WS2812B, 0>(leds_, numLeds_); break; // 默认使用GPIO0
    }
    clear();
  }

  void begin() { 
    FastLED.clear();
    FastLED.show();
  }

  uint16_t length() const { return numLeds_; }

  void clear() { 
    FastLED.clear();
  }

  void blendPixel(uint16_t idx, uint32_t c) {
    if (idx >= numLeds_) return;
    
    // 从32位颜色值中提取RGB
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    
    // 使用FastLED的颜色混合
    leds_[idx] |= CRGB(r, g, b);
  }

  void setPixel(uint16_t idx, uint32_t c) {
    if (idx >= numLeds_) return;
    
    // 从32位颜色值中提取RGB
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    
    // 设置像素颜色
    leds_[idx] = CRGB(r, g, b);
  }

  // 全局参数访问器
  static uint8_t& globalBrightness();
  static uint16_t& powerLimit_mA();
  static uint8_t& ledFull_mA();
  static uint32_t& lastCurrentEst_mA();

  void show() {
    uint32_t sum = 0;
    for (uint16_t i = 0; i < numLeds_; ++i) {
      sum += leds_[i].r + leds_[i].g + leds_[i].b;
    }
    
    float currentEst = (float)ledFull_mA() * (float)sum / (255.0f * 3.0f);
    if (currentEst < 0) currentEst = 0;
    lastCurrentEst_mA() = (uint32_t)(currentEst + 0.5f);
    
    // 增强的功率管理和自动亮度调节
    float scale = 1.0f;
    if (powerLimit_mA() > 0) {
      // 当估计电流超过限制的90%时开始逐渐降低亮度
      if (currentEst > (float)powerLimit_mA() * 0.9f) {
        // 渐进缩放，避免突然变暗
        scale = (float)powerLimit_mA() / currentEst;
        // 添加平滑过渡
        static float lastScale = 1.0f;
        scale = lastScale * 0.7f + scale * 0.3f;
        lastScale = scale;
      }
    }
    
    uint16_t effBrightness = (uint16_t)((float)globalBrightness() * scale);
    if (effBrightness > 255) effBrightness = 255;
    
    // 设置FastLED亮度并显示
    FastLED.setBrightness((uint8_t)effBrightness);
    FastLED.show();
  }

  // 获取LED数组的直接访问
  CRGB* leds() { return leds_; }

private:
  uint16_t numLeds_;
  uint8_t pin_;
  CRGB leds_[MAX_LEDS]; // 使用静态数组避免动态内存分配
};

/**
 * 路径类 - 定义LED灯带上的节点路径
 */
class EnhancedLEDPath {
public:
  explicit EnhancedLEDPath(EnhancedLEDCanvas& canvas) : canvas_(canvas) {}
  
  void setNodes(const std::vector<uint16_t>& nodes) { nodes_ = nodes; }
  
  uint16_t size() const { return (uint16_t)nodes_.size(); }
  
  uint16_t node(uint16_t i) const { return nodes_[i % nodes_.size()]; }
  
  EnhancedLEDCanvas& canvas() { return canvas_; }

private:
  EnhancedLEDCanvas& canvas_;
  std::vector<uint16_t> nodes_;
};

/**
 * 效果基类
 */
class EnhancedLEDEffect {
public:
  virtual ~EnhancedLEDEffect() {}
  virtual void render(unsigned long now) = 0;
};

/**
 * 流动效果类
 */
class EnhancedFlowEffect : public EnhancedLEDEffect {
public:
  EnhancedFlowEffect(EnhancedLEDPath& path, uint32_t color, uint8_t tail, uint16_t interval_ms)
  : path_(path), color_(color), tail_(tail), interval_(interval_ms) {}
  
  void start() { running_ = true; }
  void stop() { running_ = false; }
  bool running() const { return running_; }
  
  void setColor(uint32_t c) { color_ = c; }
  void setTail(uint8_t t) { tail_ = t; }
  void setInterval(uint16_t ms) { interval_ = ms; }
  
  void render(unsigned long now) override {
    if (!running_ || path_.size() == 0) return;
    if (now - lastStepAt_ < interval_) return;
    
    lastStepAt_ = now;
    head_ = (head_ + 1) % path_.size();
    
    for (uint8_t k = 0; k <= tail_; ++k) {
      uint16_t idxInPath = (head_ + path_.size() - k) % path_.size();
      uint16_t ledIdx = path_.node(idxInPath);
      uint8_t s = (uint8_t)(255 - (255 * k) / (tail_ + 1));
      
      // 缩放颜色
      uint8_t r = ((color_ >> 16) & 0xFF) * s / 255;
      uint8_t g = ((color_ >> 8) & 0xFF) * s / 255;
      uint8_t b = (color_ & 0xFF) * s / 255;
      
      path_.canvas().blendPixel(ledIdx, (r << 16) | (g << 8) | b);
    }
  }

private:
  EnhancedLEDPath& path_;
  uint32_t color_;
  uint8_t tail_ = 8;
  uint16_t interval_ = 40;
  bool running_ = false;
  uint16_t head_ = 0;
  unsigned long lastStepAt_ = 0;
};

/**
 * 点效果类
 */
class EnhancedPointEffect : public EnhancedLEDEffect {
public:
  EnhancedPointEffect(EnhancedLEDPath& path) : path_(path) {}
  
  void setPoint(uint16_t idxInPath, uint32_t color) { 
    point_ = idxInPath; 
    color_ = color; 
    hasPoint_ = true; 
  }
  
  void clearPoint() { hasPoint_ = false; }
  
  void render(unsigned long) override {
    if (!hasPoint_ || path_.size() == 0) return;
    uint16_t ledIdx = path_.node(point_ % path_.size());
    path_.canvas().blendPixel(ledIdx, color_);
  }

private:
  EnhancedLEDPath& path_;
  uint16_t point_ = 0;
  uint32_t color_ = 0;
  bool hasPoint_ = false;
};

/**
 * 音频可视化效果包装类
 */
class EnhancedAudioEffect : public EnhancedLEDEffect {
public:
  EnhancedAudioEffect(EnhancedLEDPath& path, OptimizedAudioAnalyzer& analyzer)
  : path_(path), analyzer_(analyzer) {
    // 初始化可视化器
    visualizer_.setSensitivity(1.5f);
    visualizer_.setBeatThreshold(0.3f);
    visualizer_.setBeatCooldown(200);
    visualizer_.setPitchRange(110.0f, 880.0f);
  }

  void setEnabled(bool en) { 
    enabled_ = en; 
    visualizer_.setEnabled(en);
  }
  
  bool enabled() const { return enabled_; }
  
  void setSensitivity(float s) { 
    visualizer_.setSensitivity(s); 
  }
  
  void setMode(AudioVisualizer::EffectType mode) {
    visualizer_.setEffect(mode);
  }
  
  AudioVisualizer::EffectType getMode() const {
    return visualizer_.getCurrentEffect();
  }
  
  void setExternalLenEnabled(bool en) { useExternalLen_ = en; }
  void setExternalLen(uint16_t v) { externalLen_ = v; }

  void render(unsigned long) override {
    if (!enabled_ || path_.size() == 0) return;
    
    // 使用音频可视化器渲染效果
    visualizer_.render(path_.canvas().leds(), path_.size(), analyzer_);
  }

private:
  EnhancedLEDPath& path_;
  OptimizedAudioAnalyzer& analyzer_;
  AudioVisualizer visualizer_;
  bool enabled_ = false;
  bool useExternalLen_ = false;
  uint16_t externalLen_ = 0;
};

/**
 * 效果管理器类
 */
class EnhancedEffectManager {
public:
  void addCanvas(EnhancedLEDCanvas* c) { canvases_.push_back(c); }
  void addEffect(EnhancedLEDEffect* e) { effects_.push_back(e); }
  
  void tick() {
    unsigned long now = millis();
    for (auto* c : canvases_) c->clear();
    for (auto* e : effects_) e->render(now);
    for (auto* c : canvases_) c->show();
  }

private:
  std::vector<EnhancedLEDCanvas*> canvases_;
  std::vector<EnhancedLEDEffect*> effects_;
};

/**
 * 增强型LED控制器类
 */
class EnhancedLEDController {
public:
  EnhancedLEDController(uint16_t ledCount, uint8_t ledPin, OptimizedAudioAnalyzer& analyzer)
  : canvas_(ledCount, ledPin), path_(canvas_),
    flow_(path_, /*color*/((uint32_t)255<<16), /*tail*/8, /*interval*/40),
    point_(path_), audioEff_(path_, analyzer) {}

  void begin() {
    canvas_.begin();
    // 默认路径: 0..N-1
    std::vector<uint16_t> nodes; nodes.reserve(canvas_.length());
    for (uint16_t i=0;i<canvas_.length();++i) nodes.push_back(i);
    path_.setNodes(nodes);
    mgr_.addCanvas(&canvas_);
    mgr_.addEffect(&flow_);
    mgr_.addEffect(&point_);
    mgr_.addEffect(&audioEff_);
  }

  void tick() { mgr_.tick(); }

  // Flow controls
  void startFlow() { flow_.start(); }
  void stopFlow() { flow_.stop(); }
  void setFlow(uint32_t color, uint8_t tail, uint16_t intervalMs) {
    flow_.setColor(color); flow_.setTail(tail); flow_.setInterval(intervalMs);
  }

  // Point controls
  void setPoint(uint16_t idxInPath, uint32_t color) { point_.setPoint(idxInPath, color); }
  void clearPoint() { point_.clearPoint(); }

  // Audio effect controls
  void enableAudio(bool en) { audioEff_.setEnabled(en); }
  void setAudioSensitivity(float s) { audioEff_.setSensitivity(s); }
  void setExternalLenEnabled(bool en) { audioEff_.setExternalLenEnabled(en); }
  void setExternalLen(uint16_t len) { audioEff_.setExternalLen(len); }
  bool audioEnabled() const { return audioEff_.enabled(); }
  
  // 音频效果模式控制
  void setAudioMode(AudioVisualizer::EffectType mode) { audioEff_.setMode(mode); }
  AudioVisualizer::EffectType getAudioMode() const { return audioEff_.getMode(); }

  // Accessors
  EnhancedFlowEffect& flow() { return flow_; }
  EnhancedPointEffect& point() { return point_; }
  EnhancedAudioEffect& audioEffect() { return audioEff_; }
  EnhancedLEDPath& path() { return path_; }
  EnhancedLEDCanvas& canvas() { return canvas_; }

private:
  EnhancedLEDCanvas canvas_;
  EnhancedLEDPath path_;
  EnhancedFlowEffect flow_;
  EnhancedPointEffect point_;
  EnhancedAudioEffect audioEff_;
  EnhancedEffectManager mgr_;
};
