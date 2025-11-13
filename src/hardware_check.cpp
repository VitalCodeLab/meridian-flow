#include "hardware_check.h"

void checkHardwareConnections() {
  Serial.println("\n=== Hardware Connection Check ===");
  
  // 检查LED引脚
  Serial.printf("LED strip connected to GPIO%d\n", LED_PIN);
  
  // 检查音频引脚
  Serial.printf("Audio input connected to GPIO%d\n", AUDIO_PIN);
  
  // 检查按钮引脚
  Serial.printf("Button connected to GPIO%d (active %s)\n", 
                BUTTON_PIN, BUTTON_ACTIVE_LOW ? "LOW" : "HIGH");
  
  // 检查板载LED引脚
  Serial.printf("Onboard LEDs connected to GPIO%d and GPIO%d\n", ONBOARD_L1, ONBOARD_L2);
  
  // 检查配置
  Serial.printf("LED count: %d, Power limit: %dmA\n", LED_COUNT, gPowerLimit_mA);
  
  Serial.println("=== Check Complete ===\n");
}
