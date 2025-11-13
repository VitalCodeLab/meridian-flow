## 问题概览
- 框架：`Arduino`（`platformio.ini:11-17`）
- 重复入口：同时存在两组 `setup()/loop()`
  - `src/main.cpp:132` 与 `src/main.cpp:405`
  - `src/tcm_demo.cpp:673` 与 `src/tcm_demo.cpp:720`
- 跨文件全局变量链接错误：头文件使用 `extern`，但在 `src/main.cpp` 中被 `static` 限定或类型不一致
  - 典型例：`src/audio_handler.h:16` 声明 `extern uint16_t LED_COUNT;`，而 `src/main.cpp:38` 定义为 `static const uint16_t LED_COUNT = 60;`
  - 典型例：`src/led_handler.h:16` 声明 `extern AudioAnalyzer analyzer;`，而 `src/main.cpp:108` 定义为 `OptimizedAudioAnalyzer analyzer(AUDIO_PIN);`

## 修复步骤
1) 移除重复入口（必做）
- 在 `platformio.ini` 添加：`src_filter = -<tcm_demo.cpp>`，将演示文件排除出构建，保留 `src/main.cpp` 作为唯一入口。

2) 统一跨文件变量的外部链接与类型（必做）
- 头文件改动（对齐类型与 `const`）：
  - `src/audio_handler.h:16` 改为 `extern const uint16_t LED_COUNT;`
  - `src/led_handler.h:16` 改为 `extern OptimizedAudioAnalyzer analyzer;`
  - 检查并保持以下声明与定义一致：
    - `hardware_check.h:5-13` 的 `LED_PIN/AUDIO_PIN/BUTTON_PIN/BUTTON_ACTIVE_LOW/ONBOARD_L1/ONBOARD_L2/LED_COUNT` 均为 `extern const`
    - `hardware_check.h:12`