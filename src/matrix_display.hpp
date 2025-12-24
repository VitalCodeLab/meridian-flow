#ifndef MATRIX_DISPLAY_HPP
#define MATRIX_DISPLAY_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "arduinoFFT.h"

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
#define MATRIX_SIZE (MATRIX_WIDTH * MATRIX_HEIGHT)

class MatrixDisplay
{
public:
    enum DisplayMode
    {
        MODE_OFF,
        MODE_AUDIO_SPECTRUM,
        MODE_AUDIO_WAVEFORM,
        MODE_TEXT_SCROLL,
        MODE_TEXT_STATIC,
        MODE_ANIMATION,
        MODE_CUSTOM_PATTERN
    };

    enum VisualizationType
    {
        VIZ_BARS,
        VIZ_CIRCLE,
        VIZ_WAVE,
        VIZ_SPECTRUM,
        VIZ_PARTICLES
    };

private:
    CRGB leds[MATRIX_SIZE];
    DisplayMode currentMode;
    VisualizationType currentVizType;

    // 音频相关
    float audioData[MATRIX_SIZE];
    arduinoFFT *fft;
    uint8_t spectrumData[MATRIX_SIZE];

    // 文字显示相关
    char textBuffer[64];
    int textLength;
    int scrollPosition;
    unsigned long lastScrollTime;
    int scrollSpeed;
    CRGB textColor;

    // 动画相关
    unsigned long lastUpdateTime;
    int animationFrame;
    bool animationRunning;

    // 自定义图案
    uint8_t customPattern[MATRIX_SIZE];

    // 设置参数
    uint8_t brightness;
    bool autoMode;
    unsigned long autoModeInterval;
    unsigned long lastAutoSwitch;

    // 私有方法
    void updateAudioSpectrum();
    void updateAudioWaveform();
    void scrollText();
    void drawChar(char c, int x, int y, CRGB color);
    void runAnimation();
    void drawCustomPattern();
    void mapSpectrumToLEDs();
    void drawCircularViz();
    void drawParticleViz();

public:
    MatrixDisplay();
    ~MatrixDisplay();

    // 初始化
    void begin(int ledPin = 2);
    void update();

    // 模式控制
    void setMode(DisplayMode mode);
    DisplayMode getMode() const { return currentMode; }
    void setVisualizationType(VisualizationType type);
    VisualizationType getVisualizationType() const { return currentVizType; }

    // 音频可视化
    void updateAudioData(float *audioSamples, int sampleCount);
    void processFFT(float *audioSamples, int sampleCount);

    // 文字显示
    void setText(const char *text);
    void setTextColor(CRGB color);
    void setScrollSpeed(int speed);
    void showStaticText(const char *text);

    // 自定义图案
    void setCustomPattern(uint8_t *pattern);
    void clearCustomPattern();

    // 动画控制
    void startAnimation();
    void stopAnimation();
    void setAnimationFrame(int frame);

    // 设置
    void setBrightness(uint8_t brightness);
    void setAutoMode(bool enabled, unsigned long intervalMs = 5000);

    // 预设图案
    void showHeart();
    void showSmiley();
    void showArrow(int direction);
    void showNumber(int num);
    void showEqualizer();

    // 音频响应效果
    void showBeatDetection();
    void showFrequencyBands();
    void showVolumeMeter();

    // 获取状态
    bool isAnimationRunning() const { return animationRunning; }
    const char *getCurrentText() const { return textBuffer; }
    uint8_t *getSpectrumData() { return spectrumData; }
    
    // LED控制方法
    void clearMatrix();
    void xy(int x, int y, CRGB color);
};

// 字体数据 (5x7像素)
extern const uint8_t font5x7[96][7];

#endif
