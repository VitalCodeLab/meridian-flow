#pragma once
#include <Arduino.h>

// 声明外部变量
extern const uint8_t LED_PIN;
extern const uint8_t AUDIO_PIN;
extern const uint8_t BUTTON_PIN;
extern const bool BUTTON_ACTIVE_LOW;
extern const uint16_t LED_COUNT;
extern uint16_t gPowerLimit_mA;

/**
 * 确认硬件连接是否正常
 * 
 * 该函数在启动时检查并输出硬件连接信息，包括：
 * - LED灯带连接的GPIO引脚
 * - 音频输入连接的GPIO引脚
 * - 按钮连接的GPIO引脚及其激活电平
 * - LED数量和功率限制设置
 */
void checkHardwareConnections();
