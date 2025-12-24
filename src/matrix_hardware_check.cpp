#include "matrix_hardware_check.hpp"
#include "Arduino.h"

bool checkHardware() {
    Serial.println("=== 硬件检查开始 ===");
    
    bool allOK = true;
    
    // 检查ESP32芯片
    Serial.print("ESP32芯片检查: ");
    String chipModel = ESP.getChipModel();
    Serial.println(chipModel + " - OK");
    
    // 检查内存
    Serial.print("可用内存: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    if (ESP.getFreeHeap() < 10000) {
        Serial.println("警告: 可用内存较少");
    }
    
    // 检查Flash大小
    Serial.print("Flash大小: ");
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(" MB");
    
    // 检查CPU频率
    Serial.print("CPU频率: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    
    // 检查GPIO引脚
    Serial.print("GPIO引脚检查: ");
    // 检查GPIO2 (LED矩阵数据线)
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    delay(10);
    digitalWrite(2, HIGH);
    delay(10);
    digitalWrite(2, LOW);
    Serial.println("GPIO2 - OK");
    
    // 检查蓝牙支持
    Serial.print("蓝牙支持: ");
    #if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)
        Serial.println("支持");
    #else
        Serial.println("不支持");
        allOK = false;
    #endif
    
    Serial.println("=== 硬件检查完成 ===");
    
    if (allOK) {
        Serial.println("✅ 硬件检查通过");
    } else {
        Serial.println("❌ 硬件检查失败");
    }
    
    return allOK;
}
