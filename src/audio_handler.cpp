#include "audio_handler.h"

void updateAudioLog() {
  unsigned long now = millis();
  if (now - lastAudioLogAt >= 500) {
    lastAudioLogAt = now;
    Serial.printf("audio level=%.3f low=%.3f mid=%.3f high=%.3f en=%d mode=%s\n",
                  analyzer.level(), analyzer.low(), analyzer.mid(), analyzer.high(),
                  controller.audioEnabled()?1:0, audioModeNames[currentAudioMode]);
  }
}

void handleAudioEffects() {
  // 设置当前音频效果模式
  if (controller.audioEnabled()) {
    controller.setAudioMode(static_cast<AudioVisualizer::EffectType>(currentAudioMode));
  }

  // 音高到长度映射
  if (controller.audioEnabled() && gPitchMapEnable) {
    controller.setExternalLenEnabled(true);
    controller.setExternalLen(analyzer.mapPitchToLen(gPitchMapMinHz, gPitchMapMaxHz, gPitchMapScale, LED_COUNT));
  } else {
    controller.setExternalLenEnabled(false);
  }
}

void handlePitchDetection() {
  if (!gPitchArmed) return;
  
  float hz = analyzer.pitchHz();        // 获取当前音高（Hz）
  float conf = analyzer.pitchConf();    // 获取音高置信度（0-1）
  unsigned long now = millis();
  
  // 检查是否满足音高检测条件
  if (hz > 0 && conf >= gPitchConfThresh && (now - gPitchLastHit) > gPitchCooldownMs) {
    // 计算音分差异（1200音分 = 1个八度）
    float cents = 1200.0f * log(hz / gPitchTargetHz) / log(2.0f);
    if (cents < 0) cents = -cents;  // 取绝对值，只关心差异大小
    
    // 如果在容差范围内
    if (cents <= gPitchTolCents) {
      gPitchLastHit = now;  // 更新上次呼应时间
      
      // 视觉反馈：在当前索引位置设置绿色点
      // 在当前索引位置设置绿色点
      controller.point().setPoint(stepIndex, (0u<<16)|(255u<<8)|0u);  // RGB格式：绿色
    }
  }
}
