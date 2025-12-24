#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "matrix_display.hpp"
#include "optimized_audio.hpp"
#include "ble_control.hpp"
#include "matrix_hardware_check.hpp"

// BLE控制实例
BLEControl bleControl;

// 8x8 LED矩阵显示实例
MatrixDisplay matrix;

// 音频处理实例 (使用真实麦克风)
OptimizedAudioAnalyzer audioAnalyzer(3); // 使用GPIO3连接MAX9814麦克风

// 系统状态
bool systemReady = false;
bool demoMode = true; // 恢复演示模式，测试文字显示
unsigned long lastDemoSwitch = 0;
int currentDemo = 0;

// 演示模式列表
enum DemoMode {
    DEMO_AUDIO_SPECTRUM,
    DEMO_TEXT_SCROLL,
    DEMO_ANIMATIONS,
    DEMO_PATTERNS,
    DEMO_COUNT
};

void setupBLE() {
    Serial.println("启动BLE控制...");
    
    bleControl.begin("ESP32_Matrix");
    bleControl.setMatrix(&matrix);
    
    Serial.println("BLE控制已启动");
    Serial.println("设备名称: ESP32_Matrix");
    Serial.println("请使用BLE终端应用连接控制");
}

void runDemoMode() {
    if (!demoMode) return;
    
    if (millis() - lastDemoSwitch > 5000) { // 每5秒切换一次
        currentDemo = (currentDemo + 1) % DEMO_COUNT;
        lastDemoSwitch = millis();
        
        switch (currentDemo) {
            case DEMO_AUDIO_SPECTRUM:
                matrix.setMode(MatrixDisplay::MODE_AUDIO_SPECTRUM);
                matrix.setVisualizationType(MatrixDisplay::VIZ_BARS);
                Serial.println("演示: 音频频谱 - 频谱条");
                break;
                
            case DEMO_TEXT_SCROLL:
                matrix.setText("ESP32 Matrix Demo!");
                matrix.setMode(MatrixDisplay::MODE_TEXT_SCROLL);
                Serial.println("演示: 文字滚动");
                break;
                
            case DEMO_ANIMATIONS:
                matrix.startAnimation();
                Serial.println("演示: 动画效果");
                break;
                
            case DEMO_PATTERNS:
                // 随机显示预设图案
                int pattern = random(6);
                switch (pattern) {
                    case 0: matrix.showHeart(); break;
                    case 1: matrix.showSmiley(); break;
                    case 2: matrix.showArrow(random(4)); break;
                    case 3: matrix.showNumber(random(10)); break;
                    case 4: matrix.showEqualizer(); break;
                    case 5: 
                        matrix.setMode(MatrixDisplay::MODE_AUDIO_SPECTRUM);
                        matrix.setVisualizationType(MatrixDisplay::VIZ_CIRCLE);
                        break;
                }
                Serial.println("演示: 预设图案");
                break;
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("=== ESP32 8x8 LED矩阵显示系统 ===");
    
    // 硬件检查
    if (!checkHardware()) {
        Serial.println("硬件检查失败，系统暂停");
        while (true) delay(1000);
    }
    
    // 初始化LED矩阵
    matrix.begin(0); // 使用GPIO0连接LED矩阵
    Serial.println("LED矩阵初始化完成");
    
    // LED测试 - 显示红色确认LED工作
    Serial.println("LED测试...");
    matrix.showHeart(); // 显示爱心图案作为测试
    delay(2000); // 显示2秒
    matrix.clearMatrix();
    matrix.update();
    Serial.println("LED测试完成");
    
    // 初始化音频处理 (真实麦克风)
    audioAnalyzer.begin();
    audioAnalyzer.setSensitivity(0.15); // 进一步降低灵敏度
    Serial.println("MAX9814麦克风初始化完成");
    
    // 设置BLE控制
    setupBLE();
    
    // 启动音频频谱模式 (而不是演示模式)
    matrix.setMode(MatrixDisplay::MODE_AUDIO_SPECTRUM);
    matrix.setVisualizationType(MatrixDisplay::VIZ_BARS);
    Serial.println("启动音频频谱模式 - 等待BLE控制");
    
    systemReady = true;
    Serial.println("系统初始化完成！");
    Serial.println("请使用BLE终端应用连接 'ESP32_Matrix' 进行控制");
}

void loop() {
    if (!systemReady) {
        delay(100);
        return;
    }
    
    // 处理BLE命令
    bleControl.update();
    
    // 更新音频数据 (真实麦克风) - 只有在音频模式下才更新
    if (matrix.getMode() == MatrixDisplay::MODE_AUDIO_SPECTRUM || 
        matrix.getMode() == MatrixDisplay::MODE_AUDIO_WAVEFORM) {
        audioAnalyzer.tick(); // 更新音频分析
        
        // 获取音频频谱数据并映射到矩阵
        float audioSamples[64];
        for (int i = 0; i < 64; i++) {
            // 将频段数据映射到音频样本格式，进一步降低放大倍数
            if (i < 8) {
                // 低频段
                audioSamples[i] = audioAnalyzer.low() * 2.5f; // 从5.0降低到2.5
            } else if (i < 32) {
                // 中频段  
                audioSamples[i] = audioAnalyzer.mid() * 2.0f; // 从4.0降低到2.0
            } else {
                // 高频段
                audioSamples[i] = audioAnalyzer.high() * 1.5f; // 从3.0降低到1.5
            }
        }
        matrix.updateAudioData(audioSamples, 64);
    }
    
    // 运行演示模式 - 只有在没有BLE连接时才运行
    if (!bleControl.isConnected()) {
        runDemoMode();
    }
    
    // 更新LED显示
    matrix.update();
    
    delay(10); // 小延迟以保持系统稳定
}
