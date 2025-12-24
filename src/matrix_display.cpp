#include "matrix_display.hpp"
#include <math.h>

// 5x7像素字体数据
const uint8_t font5x7[96][7] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' '
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}, // '!'
    {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // '"'
    {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}, // '#'
    {0x04, 0x1E, 0x05, 0x0E, 0x14, 0x0F, 0x04}, // '$'
    {0x08, 0x09, 0x04, 0x02, 0x04, 0x12, 0x11}, // '%'
    {0x04, 0x0A, 0x0A, 0x05, 0x1E, 0x12, 0x0D}, // '&'
    {0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}, // '''
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, // '('
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}, // ')'
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00}, // '*'
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, // '+'
    {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08}, // ','
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04}, // '.'
    {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}, // '/'
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // '0'
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // '1'
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, // '2'
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // '3'
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // '4'
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // '5'
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // '6'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // '7'
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // '8'
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x11, 0x0E}, // '9'
    {0x00, 0x04, 0x04, 0x00, 0x00, 0x04, 0x04}, // ':'
    {0x00, 0x04, 0x04, 0x00, 0x00, 0x04, 0x08}, // ';'
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}, // '<'
    {0x00, 0x00, 0x1F, 0x00, 0x00, 0x1F, 0x00}, // '='
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, // '>'
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}, // '?'
    {0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0F}, // '@'
    {0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11}, // 'A'
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // 'B'
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // 'C'
    {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}, // 'D'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // 'E'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // 'F'
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // 'G'
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // 'H'
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 'I'
    {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E}, // 'J'
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // 'K'
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // 'L'
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // 'M'
    {0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x11}, // 'N'
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // 'O'
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // 'P'
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // 'Q'
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // 'R'
    {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E}, // 'S'
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // 'T'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // 'U'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // 'V'
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}, // 'W'
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // 'X'
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // 'Y'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // 'Z'
    {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E}, // '['
    {0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01}, // '\\'
    {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E}, // ']'
    {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00}, // '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, // '_'
    {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}, // '`'
    {0x00, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F}, // 'a'
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1E}, // 'b'
    {0x00, 0x00, 0x0E, 0x11, 0x10, 0x11, 0x0E}, // 'c'
    {0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x0F}, // 'd'
    {0x00, 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E}, // 'e'
    {0x06, 0x09, 0x08, 0x1C, 0x08, 0x08, 0x08}, // 'f'
    {0x00, 0x0F, 0x11, 0x11, 0x0F, 0x01, 0x0E}, // 'g'
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11}, // 'h'
    {0x04, 0x00, 0x0C, 0x04, 0x04, 0x04, 0x0E}, // 'i'
    {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0C}, // 'j'
    {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12}, // 'k'
    {0x0C, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 'l'
    {0x00, 0x00, 0x1A, 0x15, 0x15, 0x15, 0x15}, // 'm'
    {0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}, // 'n'
    {0x00, 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E}, // 'o'
    {0x00, 0x00, 0x16, 0x19, 0x19, 0x16, 0x10}, // 'p'
    {0x00, 0x00, 0x0D, 0x13, 0x13, 0x0D, 0x01}, // 'q'
    {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}, // 'r'
    {0x00, 0x00, 0x0E, 0x10, 0x0E, 0x01, 0x1E}, // 's'
    {0x08, 0x08, 0x1C, 0x08, 0x08, 0x09, 0x06}, // 't'
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0D}, // 'u'
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04}, // 'v'
    {0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A}, // 'w'
    {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11}, // 'x'
    {0x00, 0x00, 0x11, 0x11, 0x0F, 0x01, 0x0E}, // 'y'
    {0x00, 0x00, 0x1F, 0x02, 0x04, 0x08, 0x1F}, // 'z'
    {0x02, 0x04, 0x04, 0x08, 0x04, 0x04, 0x02}, // '{'
    {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // '|'
    {0x08, 0x04, 0x04, 0x02, 0x04, 0x04, 0x08}, // '}'
    {0x00, 0x08, 0x15, 0x02, 0x00, 0x00, 0x00}, // '~'
    {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}  // DEL
};

MatrixDisplay::MatrixDisplay() {
  currentMode = MODE_OFF;
  currentVizType = VIZ_BARS;
  brightness = 60;
  autoMode = false;
  autoModeInterval = 5000;
  lastAutoSwitch = 0;
  lastUpdateTime = 0;
  animationFrame = 0;
  animationRunning = false;
  scrollPosition = 0;
  lastScrollTime = 0;
  scrollSpeed = 100;
  textColor = CRGB::White;
  textLength = 0;
  memset(textBuffer, 0, sizeof(textBuffer));
  memset(audioData, 0, sizeof(audioData));
  memset(spectrumData, 0, sizeof(spectrumData));
  memset(customPattern, 0, sizeof(customPattern));

  fft = new arduinoFFT();
}

MatrixDisplay::~MatrixDisplay() { delete fft; }

void MatrixDisplay::begin(int ledPin) {
  // 根据传入的引脚使用对应的FastLED配置
  switch (ledPin) {
  case 0:
    FastLED.addLeds<WS2812B, 0, GRB>(leds, MATRIX_SIZE);
    break;
  case 1:
    FastLED.addLeds<WS2812B, 1, GRB>(leds, MATRIX_SIZE);
    break;
  case 2:
    FastLED.addLeds<WS2812B, 2, GRB>(leds, MATRIX_SIZE);
    break;
  case 3:
    FastLED.addLeds<WS2812B, 3, GRB>(leds, MATRIX_SIZE);
    break;
  case 4:
    FastLED.addLeds<WS2812B, 4, GRB>(leds, MATRIX_SIZE);
    break;
  case 5:
    FastLED.addLeds<WS2812B, 5, GRB>(leds, MATRIX_SIZE);
    break;
  case 6:
    FastLED.addLeds<WS2812B, 6, GRB>(leds, MATRIX_SIZE);
    break;
  case 7:
    FastLED.addLeds<WS2812B, 7, GRB>(leds, MATRIX_SIZE);
    break;
  case 8:
    FastLED.addLeds<WS2812B, 8, GRB>(leds, MATRIX_SIZE);
    break;
  case 9:
    FastLED.addLeds<WS2812B, 9, GRB>(leds, MATRIX_SIZE);
    break;
  case 10:
    FastLED.addLeds<WS2812B, 10, GRB>(leds, MATRIX_SIZE);
    break;
  default:
    // 默认使用GPIO2
    FastLED.addLeds<WS2812B, 2, GRB>(leds, MATRIX_SIZE);
    Serial.printf("警告: 不支持的GPIO引脚 %d，使用默认GPIO2\n", ledPin);
    break;
  }

  FastLED.setBrightness(brightness);
  clearMatrix();
  FastLED.show();

  Serial.printf("LED矩阵初始化完成，使用GPIO%d\n", ledPin);
}

void MatrixDisplay::update() {
  if (autoMode && millis() - lastAutoSwitch > autoModeInterval) {
    // 自动切换模式
    currentMode = static_cast<DisplayMode>((currentMode + 1) % 7);
    lastAutoSwitch = millis();
  }

  switch (currentMode) {
  case MODE_AUDIO_SPECTRUM:
    updateAudioSpectrum();
    break;
  case MODE_AUDIO_WAVEFORM:
    updateAudioWaveform();
    break;
  case MODE_TEXT_SCROLL:
    scrollText();
    break;
  case MODE_ANIMATION:
    runAnimation();
    break;
  case MODE_CUSTOM_PATTERN:
    drawCustomPattern();
    break;
  case MODE_OFF:
  default:
    clearMatrix();
    break;
  }

  FastLED.show();
}

void MatrixDisplay::xy(int x, int y, CRGB color) {
  if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
    leds[y * MATRIX_WIDTH + x] = color;
  }
}

void MatrixDisplay::clearMatrix() {
  fill_solid(leds, MATRIX_SIZE, CRGB::Black);
}

void MatrixDisplay::setMode(DisplayMode mode) {
  currentMode = mode;
  if (mode == MODE_TEXT_SCROLL || mode == MODE_TEXT_STATIC) {
    scrollPosition = 0;
    lastScrollTime = millis();
  }
}

void MatrixDisplay::setVisualizationType(VisualizationType type) {
  currentVizType = type;
}

void MatrixDisplay::updateAudioData(float *audioSamples, int sampleCount) {
  int copyCount = min(sampleCount, MATRIX_SIZE);
  memcpy(audioData, audioSamples, copyCount * sizeof(float));

  // 处理FFT
  if (currentMode == MODE_AUDIO_SPECTRUM) {
    processFFT(audioSamples, sampleCount);
  }
}

void MatrixDisplay::processFFT(float *audioSamples, int sampleCount) {
  // 准备FFT数据
  int fftSize = min(sampleCount, 64);

  // 分离实部和虚部数组
  double vReal[64];
  double vImag[64];

  for (int i = 0; i < fftSize; i++) {
    vReal[i] = audioSamples[i] * 1000.0; // real
    vImag[i] = 0.0;                      // imag
  }

  // 执行FFT
  fft->Windowing(vReal, fftSize, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  fft->Compute(vReal, vImag, fftSize, FFT_FORWARD);
  fft->ComplexToMagnitude(vReal, vImag, fftSize);

  // 计算幅度谱并映射到8个频段
  // 跳过直流分量(0)和极低频分量(1)，避免低频噪声
  for (int i = 0; i < MATRIX_SIZE && i < fftSize / 2; i++) {
    int fftIndex = i + 2; // 跳过前2个频率分量
    if (fftIndex < fftSize) {
      float magnitude = vReal[fftIndex];
      // 增加阈值过滤，避免噪声触发
      if (magnitude < 20)
        magnitude = 0; // 从10提高到20
      spectrumData[i] = (uint8_t)constrain(map(magnitude, 0, 300, 0, 8), 0,
                                           8); // 从500降低到300
    } else {
      spectrumData[i] = 0;
    }
  }
}

void MatrixDisplay::updateAudioSpectrum() {
  clearMatrix();

  switch (currentVizType) {
  case VIZ_BARS:
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t height = spectrumData[x];
      for (int y = 0; y < height; y++) {
        xy(x, MATRIX_HEIGHT - 1 - y, CHSV(x * 32, 255, 255));
      }
    }
    break;

  case VIZ_CIRCLE:
    drawCircularViz();
    break;

  case VIZ_PARTICLES:
    drawParticleViz();
    break;

  default:
    mapSpectrumToLEDs();
    break;
  }
}

void MatrixDisplay::updateAudioWaveform() {
  clearMatrix();

  // 绘制波形
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    int y = map(audioData[x], -1.0, 1.0, 0, MATRIX_HEIGHT - 1);
    y = constrain(y, 0, MATRIX_HEIGHT - 1);
    xy(x, y, CRGB::Cyan);

    // 添加镜像效果
    if (y > 0 && y < MATRIX_HEIGHT - 1) {
      xy(x, MATRIX_HEIGHT - 1 - y, CRGB::Blue);
    }
  }
}

void MatrixDisplay::drawCircularViz() {
  // 中心点
  int centerX = MATRIX_WIDTH / 2;
  int centerY = MATRIX_HEIGHT / 2;

  // 根据频谱数据绘制圆形可视化
  for (int i = 0; i < 8; i++) {
    float angle = (i / 8.0) * 2 * PI;
    int radius = spectrumData[i];

    int x = centerX + cos(angle) * radius;
    int y = centerY + sin(angle) * radius;

    if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
      xy(x, y, CHSV(i * 32, 255, 255));
    }
  }
}

void MatrixDisplay::drawParticleViz() {
  // 粒子效果
  static float particles[8][2];  // x, y positions
  static float velocities[8][2]; // vx, vy velocities

  // 初始化粒子
  for (int i = 0; i < 8; i++) {
    if (particles[i][0] == 0 && particles[i][1] == 0) {
      particles[i][0] = random(8);
      particles[i][1] = random(8);
      velocities[i][0] = random(-2, 2) * 0.1;
      velocities[i][1] = random(-2, 2) * 0.1;
    }

    // 根据音频数据更新粒子
    float audioInfluence = spectrumData[i] / 8.0;
    velocities[i][0] += audioInfluence * 0.1;
    velocities[i][1] += audioInfluence * 0.1;

    // 更新位置
    particles[i][0] += velocities[i][0];
    particles[i][1] += velocities[i][1];

    // 边界检测
    if (particles[i][0] < 0 || particles[i][0] >= 8)
      velocities[i][0] *= -1;
    if (particles[i][1] < 0 || particles[i][1] >= 8)
      velocities[i][1] *= -1;

    particles[i][0] = constrain(particles[i][0], 0, 7);
    particles[i][1] = constrain(particles[i][1], 0, 7);

    // 绘制粒子
    xy((int)particles[i][0], (int)particles[i][1], CHSV(i * 32, 255, 255));
  }
}

void MatrixDisplay::mapSpectrumToLEDs() {
  // 将频谱数据映射到整个矩阵
  for (int i = 0; i < MATRIX_SIZE; i++) {
    uint8_t intensity = spectrumData[i % 8];
    leds[i] = CHSV((i % 8) * 32, 255, intensity * 32);
  }
}

void MatrixDisplay::setText(const char *text) {
  strncpy(textBuffer, text, sizeof(textBuffer) - 1);
  textBuffer[sizeof(textBuffer) - 1] = '\0';
  textLength = strlen(textBuffer);
  scrollPosition = 0;
  lastScrollTime = millis();
}

void MatrixDisplay::scrollText() {
  if (textLength == 0)
    return;

  clearMatrix();

  // 计算滚动偏移
  if (millis() - lastScrollTime > scrollSpeed) {
    scrollPosition++;
    lastScrollTime = millis();

    // 重置滚动位置
    int totalWidth = textLength * 6; // 5像素字符 + 1像素间距
    if (scrollPosition > totalWidth + MATRIX_WIDTH) {
      scrollPosition = -MATRIX_WIDTH;
    }
  }

  // 绘制文字
  int charIndex = 0;
  int xOffset = -scrollPosition;

  while (charIndex < textLength && xOffset < MATRIX_WIDTH) {
    char c = textBuffer[charIndex];
    if (c >= 32 && c <= 127) { // 可打印字符
      drawChar(c, xOffset, 0, textColor);
    }
    xOffset += 6; // 字符宽度 + 间距
    charIndex++;
  }
}

void MatrixDisplay::drawChar(char c, int x, int y, CRGB color) {
  if (c < 32 || c > 127)
    return;

  const uint8_t *charData = font5x7[c - 32];

  for (int row = 0; row < 7; row++) {
    uint8_t rowData = charData[row];
    for (int col = 0; col < 5; col++) {
      if (rowData & (0x10 >> col)) { // 恢复原始的位操作
        xy(x + col, y + row, color);
      }
    }
  }
}

void MatrixDisplay::showStaticText(const char *text) {
  currentMode = MODE_TEXT_STATIC;
  setText(text);

  clearMatrix();

  // 居中显示文字
  int textWidth = strlen(text) * 6;
  int startX = max(0, (MATRIX_WIDTH - textWidth) / 2);

  for (int i = 0; i < strlen(text); i++) {
    drawChar(text[i], startX + i * 6, 0, textColor);
  }
}

void MatrixDisplay::setCustomPattern(uint8_t *pattern) {
  memcpy(customPattern, pattern, MATRIX_SIZE);
  currentMode = MODE_CUSTOM_PATTERN;
}

void MatrixDisplay::drawCustomPattern() {
  clearMatrix();

  for (int i = 0; i < MATRIX_SIZE; i++) {
    if (customPattern[i] > 0) {
      leds[i] = CHSV(customPattern[i] * 32, 255, 255);
    }
  }
}

void MatrixDisplay::runAnimation() {
  if (!animationRunning)
    return;

  clearMatrix();

  switch (animationFrame % 4) {
  case 0:
    showHeart();
    break;
  case 1:
    showSmiley();
    break;
  case 2:
    showEqualizer();
    break;
  case 3:
    // 彩虹效果
    for (int i = 0; i < MATRIX_SIZE; i++) {
      leds[i] = CHSV((animationFrame * 8 + i * 8) % 256, 255, 255);
    }
    break;
  }

  if (millis() - lastUpdateTime > 500) {
    animationFrame++;
    lastUpdateTime = millis();
  }
}

// 预设图案
void MatrixDisplay::showHeart() {
  uint8_t heart[MATRIX_SIZE] = {0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0,
                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0,
                                0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  setCustomPattern(heart);
}

void MatrixDisplay::showSmiley() {
  uint8_t smiley[MATRIX_SIZE] = {
      0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0,
      1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1,
      1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0};
  setCustomPattern(smiley);
}

void MatrixDisplay::showEqualizer() {
  clearMatrix();

  for (int x = 0; x < MATRIX_WIDTH; x++) {
    uint8_t height = random(1, MATRIX_HEIGHT);
    for (int y = 0; y < height; y++) {
      xy(x, MATRIX_HEIGHT - 1 - y, CHSV(x * 32, 255, 255));
    }
  }
}

void MatrixDisplay::showArrow(int direction) {
  clearMatrix();
  CRGB color = CRGB::Green;

  switch (direction) {
  case 0: // 上
    xy(3, 0, color);
    xy(4, 0, color);
    xy(3, 1, color);
    xy(4, 1, color);
    xy(2, 2, color);
    xy(3, 2, color);
    xy(4, 2, color);
    xy(5, 2, color);
    xy(3, 3, color);
    xy(4, 3, color);
    xy(3, 4, color);
    xy(4, 4, color);
    xy(3, 5, color);
    xy(4, 5, color);
    xy(3, 6, color);
    xy(4, 6, color);
    break;
  case 1: // 右
    xy(7, 3, color);
    xy(7, 4, color);
    xy(6, 3, color);
    xy(6, 4, color);
    xy(5, 2, color);
    xy(5, 3, color);
    xy(5, 4, color);
    xy(5, 5, color);
    xy(4, 3, color);
    xy(4, 4, color);
    xy(3, 3, color);
    xy(3, 4, color);
    xy(2, 3, color);
    xy(2, 4, color);
    xy(1, 3, color);
    xy(1, 4, color);
    break;
  case 2: // 下
    xy(3, 7, color);
    xy(4, 7, color);
    xy(3, 6, color);
    xy(4, 6, color);
    xy(2, 5, color);
    xy(3, 5, color);
    xy(4, 5, color);
    xy(5, 5, color);
    xy(3, 4, color);
    xy(4, 4, color);
    xy(3, 3, color);
    xy(4, 3, color);
    xy(3, 2, color);
    xy(4, 2, color);
    xy(3, 1, color);
    xy(4, 1, color);
    break;
  case 3: // 左
    xy(0, 3, color);
    xy(0, 4, color);
    xy(1, 3, color);
    xy(1, 4, color);
    xy(2, 2, color);
    xy(2, 3, color);
    xy(2, 4, color);
    xy(2, 5, color);
    xy(3, 3, color);
    xy(3, 4, color);
    xy(4, 3, color);
    xy(4, 4, color);
    xy(5, 3, color);
    xy(5, 4, color);
    xy(6, 3, color);
    xy(6, 4, color);
    break;
  }
}

void MatrixDisplay::showNumber(int num) {
  clearMatrix();

  // 简单的数字显示 (0-9)
  uint8_t digits[10][MATRIX_SIZE] = {
      // 0
      {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
       1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
       0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 1
      {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1,
       0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,
       1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 2
      {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
       1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1,
       0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 3
      {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1,
       1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
       0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 4
      {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0,
       1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1,
       1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 5
      {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
       0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
       0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 6
      {0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
       0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
       0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 7
      {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1,
       1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1,
       0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 8
      {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
       1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
       0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      // 9
      {0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1,
       1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,
       0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

  if (num >= 0 && num <= 9) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
      if (digits[num][i]) {
        leds[i] = CRGB::Red;
      }
    }
  }
}

void MatrixDisplay::setBrightness(uint8_t brightness) {
  this->brightness = brightness;
  FastLED.setBrightness(brightness);
}

void MatrixDisplay::setAutoMode(bool enabled, unsigned long intervalMs) {
  autoMode = enabled;
  autoModeInterval = intervalMs;
  if (enabled) {
    lastAutoSwitch = millis();
  }
}

void MatrixDisplay::startAnimation() {
  animationRunning = true;
  currentMode = MODE_ANIMATION;
  animationFrame = 0;
  lastUpdateTime = millis();
}

void MatrixDisplay::stopAnimation() { animationRunning = false; }

void MatrixDisplay::setTextColor(CRGB color) { textColor = color; }

void MatrixDisplay::setScrollSpeed(int speed) { scrollSpeed = speed; }

void MatrixDisplay::clearCustomPattern() {
  memset(customPattern, 0, sizeof(customPattern));
}

void MatrixDisplay::setAnimationFrame(int frame) { animationFrame = frame; }

void MatrixDisplay::showBeatDetection() {
  // 节拍检测效果
  static unsigned long lastBeat = 0;
  static bool beatState = false;

  float audioSum = 0;
  for (int i = 0; i < MATRIX_SIZE; i++) {
    audioSum += abs(audioData[i]);
  }

  float threshold = 0.5; // 节拍阈值
  if (audioSum > threshold && millis() - lastBeat > 100) {
    beatState = !beatState;
    lastBeat = millis();

    if (beatState) {
      // 节拍闪烁
      fill_solid(leds, MATRIX_SIZE, CRGB::White);
    } else {
      clearMatrix();
    }
  }
}

void MatrixDisplay::showFrequencyBands() {
  clearMatrix();

  // 显示不同频率段
  for (int band = 0; band < 4; band++) {
    int startIdx = band * 2;
    int endIdx = (band + 1) * 2;

    float bandSum = 0;
    for (int i = startIdx; i < endIdx; i++) {
      bandSum += spectrumData[i];
    }

    uint8_t intensity = (uint8_t)constrain(bandSum / 2, 0, 8);
    CRGB color = CHSV(band * 64, 255, intensity * 32);

    // 在不同区域显示频率段
    for (int y = 0; y < intensity; y++) {
      if (band < 2) {
        xy(band * 4 + 0, MATRIX_HEIGHT - 1 - y, color);
        xy(band * 4 + 1, MATRIX_HEIGHT - 1 - y, color);
      } else {
        xy((band - 2) * 4 + 0, MATRIX_HEIGHT - 1 - y, color);
        xy((band - 2) * 4 + 1, MATRIX_HEIGHT - 1 - y, color);
      }
    }
  }
}

void MatrixDisplay::showVolumeMeter() {
  clearMatrix();

  // 计算音量
  float volume = 0;
  for (int i = 0; i < MATRIX_SIZE; i++) {
    volume += abs(audioData[i]);
  }
  volume /= MATRIX_SIZE;

  uint8_t level = (uint8_t)constrain(volume * 8, 0, 8);

  // 绘制音量条
  for (int i = 0; i < level; i++) {
    CRGB color = CRGB::Green;
    if (i > 5)
      color = CRGB::Yellow;
    if (i > 6)
      color = CRGB::Red;

    xy(i % MATRIX_WIDTH, i / MATRIX_WIDTH, color);
  }
}
