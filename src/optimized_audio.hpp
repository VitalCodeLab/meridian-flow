#pragma once
#include <Arduino.h>
#include <arduinoFFT.h>

/**
 * 优化的音频分析器类
 * 专为MAX9814麦克风模块设计
 * 使用ArduinoFFT进行频谱分析
 * 使用自相关算法进行音高检测
 */
class OptimizedAudioAnalyzer {
public:
  explicit OptimizedAudioAnalyzer(uint8_t adcPin) : pin_(adcPin) {
    // 初始化FFT
    fft = new arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);
  }

  ~OptimizedAudioAnalyzer() {
    delete fft;
  }

  void begin() {
    analogReadResolution(12); // ESP32-C3 ADC up to 12-bit
    analogSetPinAttenuation(pin_, ADC_11db); // 扩展输入范围至3.3V
    lastTick_ = micros();
  }

  // 频繁调用以更新音频分析
  void tick() {
    unsigned long now = micros();
    if (now - lastTick_ < 2500) return; // ~400 Hz更新率
    lastTick_ = now;

    // 采样音频数据
    for (int i = 0; i < SAMPLES; i++) {
      vReal[i] = analogRead(pin_) - 2048; // 转换为有符号值
      vImag[i] = 0;
    }
    
    // 计算RMS音量
    float sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
      sum += vReal[i] * vReal[i];
    }
    float rms = sqrt(sum / SAMPLES) / 2048.0;
    
    // 平滑RMS值
    levelSmoothed_ = levelSmoothed_ * 0.8f + rms * 0.2f;
    
    // 执行FFT
    fft->Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft->Compute(FFT_FORWARD);
    fft->ComplexToMagnitude();
    
    // 计算频段能量
    calculateBands();
    
    // 检测音高
    detectPitch();
  }

  // 获取音频特征
  float level() const { return levelSmoothed_; }
  float low() const { return low_; }
  float mid() const { return mid_; }
  float high() const { return high_; }
  float pitchHz() const { return pitchHz_; }
  float pitchConf() const { return pitchConf_; }

  // 便捷函数
  uint8_t levelByte() const { 
    float x = level(); 
    if (x < 0) x = 0; 
    if (x > 1) x = 1; 
    return (uint8_t)(x * 255.0f + 0.5f); 
  }
  
  uint8_t bandByteLow() const { 
    float x = low(); 
    if (x < 0) x = 0; 
    if (x > 1) x = 1; 
    return (uint8_t)(x * 255.0f + 0.5f); 
  }
  
  uint8_t bandByteMid() const { 
    float x = mid(); 
    if (x < 0) x = 0; 
    if (x > 1) x = 1; 
    return (uint8_t)(x * 255.0f + 0.5f); 
  }
  
  uint8_t bandByteHigh() const { 
    float x = high(); 
    if (x < 0) x = 0; 
    if (x > 1) x = 1; 
    return (uint8_t)(x * 255.0f + 0.5f); 
  }
  
  // 音高到长度映射
  uint16_t mapPitchToLen(float minHz, float maxHz, float scale, uint16_t maxLen) const {
    if (pitchHz_ <= 0 || maxHz <= minHz) return 0;
    float num = logf(pitchHz_ / minHz) / logf(2.0f);
    float den = logf(maxHz / minHz) / logf(2.0f);
    float norm = (den > 0) ? (num / den) : 0.0f;
    if (norm < 0) norm = 0; 
    if (norm > 1) norm = 1;
    float s = scale; 
    if (s < 0.1f) s = 0.1f; 
    if (s > 2.0f) s = 2.0f;
    float v = norm * s; 
    if (v > 1.0f) v = 1.0f;
    return (uint16_t)(v * (float)maxLen + 0.5f);
  }

  // 设置敏感度
  void setSensitivity(float value) {
    sensitivity_ = value;
    if (sensitivity_ < 0.1f) sensitivity_ = 0.1f;
    if (sensitivity_ > 5.0f) sensitivity_ = 5.0f;
  }

private:
  // 计算频段能量
  void calculateBands() {
    // 低频段 (0-150Hz)
    float lowSum = 0;
    int lowCount = 0;
    for (int i = 2; i < 8; i++) { // 跳过DC和非常低的频率
      lowSum += vReal[i];
      lowCount++;
    }
    
    // 中频段 (150-1kHz)
    float midSum = 0;
    int midCount = 0;
    for (int i = 8; i < 25; i++) {
      midSum += vReal[i];
      midCount++;
    }
    
    // 高频段 (1kHz+)
    float highSum = 0;
    int highCount = 0;
    for (int i = 25; i < SAMPLES/2; i++) {
      highSum += vReal[i];
      highCount++;
    }
    
    // 归一化并平滑
    float newLow = lowCount > 0 ? lowSum / (lowCount * 2048.0f) : 0;
    float newMid = midCount > 0 ? midSum / (midCount * 2048.0f) : 0;
    float newHigh = highCount > 0 ? highSum / (highCount * 2048.0f) : 0;
    
    // 应用敏感度
    newLow *= sensitivity_;
    newMid *= sensitivity_;
    newHigh *= sensitivity_;
    
    // 限制范围
    if (newLow > 1.0f) newLow = 1.0f;
    if (newMid > 1.0f) newMid = 1.0f;
    if (newHigh > 1.0f) newHigh = 1.0f;
    
    // 平滑过渡
    low_ = low_ * 0.85f + newLow * 0.15f;
    mid_ = mid_ * 0.85f + newMid * 0.15f;
    high_ = high_ * 0.85f + newHigh * 0.15f;
  }
  
  // 使用自相关算法检测音高
  void detectPitch() {
    // 准备数据 - 只使用一部分样本以提高效率
    const int useSamples = SAMPLES / 2;
    
    // 寻找最佳周期
    float maxCorrelation = 0;
    int bestPeriod = 0;
    
    // 搜索合理范围的周期 (对应约80Hz-1000Hz)
    const int minPeriod = SAMPLING_FREQ / 1000; // ~1000Hz
    const int maxPeriod = SAMPLING_FREQ / 80;   // ~80Hz
    
    for (int period = minPeriod; period <= maxPeriod; period++) {
      float correlation = 0;
      int validSamples = 0;
      
      for (int i = 0; i < useSamples - period; i++) {
        correlation += vReal[i] * vReal[i + period];
        validSamples++;
      }
      
      if (validSamples > 0) {
        correlation /= validSamples;
        
        if (correlation > maxCorrelation) {
          maxCorrelation = correlation;
          bestPeriod = period;
        }
      }
    }
    
    // 计算频率和置信度
    float newPitchHz = 0;
    float newPitchConf = 0;
    
    if (bestPeriod > 0) {
      newPitchHz = (float)SAMPLING_FREQ / bestPeriod;
      
      // 计算置信度 (0-1)
      // 归一化相关值作为置信度指标
      float normCorrelation = maxCorrelation / (level() * 2048.0f);
      newPitchConf = constrain(normCorrelation, 0.0f, 1.0f);
      
      // 如果音量太低，降低置信度
      if (level() < 0.05f) {
        newPitchConf *= level() * 20.0f; // 在低音量时逐渐降低置信度
      }
    }
    
    // 仅在置信度足够高时更新音高
    if (newPitchConf > 0.3f) {
      // 平滑过渡
      pitchHz_ = pitchHz_ * 0.7f + newPitchHz * 0.3f;
      pitchConf_ = pitchConf_ * 0.7f + newPitchConf * 0.3f;
    } else {
      // 如果置信度低，逐渐降低旧值的权重
      pitchHz_ = pitchHz_ * 0.95f;
      pitchConf_ = pitchConf_ * 0.95f;
    }
  }

  // 常量
  static const int SAMPLES = 128;
  static const int SAMPLING_FREQ = 8000;

  // 成员变量
  uint8_t pin_;
  unsigned long lastTick_ = 0;
  float levelSmoothed_ = 0.0f;
  float low_ = 0.0f, mid_ = 0.0f, high_ = 0.0f;
  float pitchHz_ = 0.0f;
  float pitchConf_ = 0.0f;
  float sensitivity_ = 1.0f;
  
  // FFT相关
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  arduinoFFT* fft;
};
