#include "led_handler.h"
#include "control.hpp"
#include "enhanced_led_controller.hpp"

void handleOnboardLEDs() {
  if (gLedOverride) {
    // 如果启用了LED覆盖模式
    if (!gPwmAttached) {
      // 首次启用时附加PWM到引脚
      ledcAttachPin(ONBOARD_L1, LEDC_CH_L1);
      ledcAttachPin(ONBOARD_L2, LEDC_CH_L2);
      gPwmAttached = true;
    }
    
    if (gLedAudioMirror && analyzer.level() > 0) {
      // 音频映射模式 - 将音频特征映射到LED亮度
      uint8_t l1 = analyzer.levelByte();     // L1显示总音量
      uint8_t l2 = analyzer.bandByteHigh();  // L2显示高频能量
      ledcWrite(LEDC_CH_L1, l1);
      ledcWrite(LEDC_CH_L2, l2);
    } else {
      // 直接控制模式 - 使用设定的亮度值
      
      // 处理脉冲衰减（如音高检测触发的脉冲）
      unsigned long now = millis();
      if (gLedOverridePulseUntil && now > gLedOverridePulseUntil) { 
        gLedOverrideL1 = 0;           // 脉冲结束，关闭LED
        gLedOverridePulseUntil = 0;   // 清除脉冲计时器
      }
      
      // 应用设定的亮度值
      ledcWrite(LEDC_CH_L1, gLedOverrideL1);
      ledcWrite(LEDC_CH_L2, gLedOverrideL2);
    }
  } else {
    // 如果禁用了LED覆盖模式，使用标准模式
    if (gPwmAttached) {
      // 如果之前附加了PWM，现在删除它
      ledcDetachPin(ONBOARD_L1);
      ledcDetachPin(ONBOARD_L2);
      pinMode(ONBOARD_L1, OUTPUT);  // 重新配置为标准输出引脚
      pinMode(ONBOARD_L2, OUTPUT);
      gPwmAttached = false;
    }
    
    // 只有在板载模式启用时才驱动板载指示灯
    if (gRenderOnboard) mirror.tick();
  }
}
