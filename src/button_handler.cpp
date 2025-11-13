#include "button_handler.h"
#include "enhanced_led_controller.hpp"

void handleButtonActions() {
  // 处理长按
  if (button.consumeLongPress()) {
    // 切换模式（FLOW/STEP）
    mode = (mode == FLOW) ? STEP : FLOW;
    
    // 根据新模式设置相应状态
    if (mode == FLOW) { 
      controller.startFlow();       // 启动流动效果
      controller.point().clearPoint();      // 清除点效果
    } else { 
      controller.stopFlow();        // 停止流动效果
      controller.point().setPoint(stepIndex, (255u<<16)); // 设置红色点
    }
    
    // 如果音频效果启用，则切换音频效果模式
    if (controller.audioEnabled()) {
      // 循环切换到下一个音频效果模式
      currentAudioMode = (currentAudioMode + 1) % AUDIO_MODE_COUNT;
      Serial.printf("Audio mode changed to: %s\n", audioModeNames[currentAudioMode]);
      
      // 通过双闪烁提供视觉反馈
      mirror.flashStep(80);
      delay(80);
      mirror.flashStep(80);
    }
  }
  
  // 处理短按
  if (button.consumeShortPress()) {
    if (mode == FLOW) {
      // FLOW模式下切换流动状态（启动/停止）
      if (controller.flow().running()) controller.stopFlow(); else controller.startFlow();
    } else {
      // STEP模式下切换到下一个点
      stepIndex = (stepIndex + 1) % controller.path().size();
      controller.point().setPoint(stepIndex, (255u<<16));  // 设置红色点
      mirror.flashStep(120);  // 闪烁提供反馈
    }
  }
}
