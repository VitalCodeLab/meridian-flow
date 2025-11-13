# ESP32-C3 WS2812B LED Controller

简要说明

- 这是一个在 ESP32-C3 上运行的 WS2812B 灯带控制器，支持流动、点效、音频响应（音量条、频谱、节拍脉冲、音高颜色）以及通过内置 Wi‑Fi AP 的 Web 界面远程控制。
- 代码采用模块化拆分：音频、按键、板载 LED、硬件检查和渲染控制分离到各自文件，主入口在 `src/main.cpp`。

快速上手

1. 构建

   - 在项目根目录运行：
     ```
     pio run
     ```
   - 或在 VS Code 的 PlatformIO GUI 中点击 Build。

2. 烧录

   - 使用 PlatformIO：
     ```
     pio run -t upload
     ```
   - 或使用 VS Code PlatformIO Upload。

3. 打开串口日志（115200）

   ```
   pio device monitor -b 115200
   ```

   在 VS Code 中也可使用 PlatformIO 的串口监视器。

4. 连接与控制（Web UI）
   - 设备会启动为 Wi‑Fi AP（默认 SSID: `meridian-c3`）。
   - 连接到该 AP 后，在浏览器中访问设备（默认常见 IP 为 `192.168.4.1`）或直接使用页面路径提供的 Web UI（服务器监听 80 端口）。

硬件接线（以代码为准）

- WS2812B 灯带 数据线: GPIO0 （在 `src/main.cpp` 中常量 `LED_PIN`）
- 灯珠数量: `LED_COUNT`（默认 60）
- 麦克风 (MAX9814) 输出: ADC 引脚 GPIO3 (`AUDIO_PIN`)
- 按钮: GPIO9 (`BUTTON_PIN`)
- 板载指示灯: GPIO12 (L1), GPIO13 (L2)

主要配置（可在 `src/main.cpp` 中调整）

- LED 与按键:
  - LED_PIN, LED_COUNT, BUTTON_PIN, BUTTON_ACTIVE_LOW
- 亮度与功率:
  - gBrightness（0..255）, gPowerLimit_mA, gLedFull_mA
- 音高与映射:
  - gPitchArmed, gPitchTargetHz, gPitchConfThresh, gPitchTolCents
  - gPitchMapEnable, gPitchMapScale, gPitchMapMinHz, gPitchMapMaxHz
- 渲染目标:
  - gRenderOnboard（true = 仅板载 LED, false = 灯带）

主要文件与职责

- src/main.cpp
  - 程序入口（setup / loop），全局配置与对象实例化，按模块顺序调用各功能。
- src/enhanced_led_controller.hpp / .cpp
  - 灯带驱动与效果实现（流动、点、音频效果及功率/亮度管理）。
- src/audio_handler.h / .cpp
  - 音频采集与分析（OptimizedAudioAnalyzer），音频相关处理函数：`updateAudioLog()`、`handleAudioEffects()`、`handlePitchDetection()`。
- src/button_handler.h / .cpp
  - 去抖与按钮事件处理，函数：`handleButtonActions()`。
- src/hardware_check.h / .cpp
  - 启动时硬件检测：`checkHardwareConnections()`。
- src/webui.hpp / .cpp
  - Wi‑Fi AP 启动、WebServer 路由及前端资源注册：`startAp()`、`registerWeb()`。
- platformio.ini
  - PlatformIO 构建与上传配置。

运行时执行顺序（loop）

1. 输入采集：`button.poll()` 与 `analyzer.tick()`
2. 功能处理：`updateAudioLog()`、`handleAudioEffects()`、`handlePitchDetection()`、`handleButtonActions()`
3. Web 服务：`server.handleClient()`
4. 渲染：当 `gRenderOnboard == false` 时调用 `controller.tick()`
5. 板载 LED：`handleOnboardLEDs()`

调试提示

- 串口日志（115200）查看启动信息与错误。
- 若 Web UI 无响应，检查串口日志中 Wi‑Fi/AP 启动信息与 IP。
- 若灯带不亮，确认 `LED_PIN`/`LED_COUNT`、电源与接线并运行 `checkHardwareConnections()` 输出。
- 音频效果异常请先检查麦克风电源与 `AUDIO_PIN` 接法，并通过串口打印 `analyzer` 输出。

常见命令总结

- 构建: `pio run`
- 烧录: `pio run -t upload`
- 串口监视: `pio device monitor -b 115200`

扩展与贡献

- 若要添加新效果，请在 `enhanced_led_controller` 中实现并在 `main.cpp` 注册/调用。
- 新的 Web API 路由请在 `webui` 模块添加并更新前端页面。

许可

- 本仓库默认无特定开源许可（请根据需要添加 LICENSE 文件）。

如需，能够：

- 生成项目的详尽 README（含 API 列表与 Web 接口说明）。
- 把 README 写成更简洁的版本用于设备内置文档或生成 wiki 页面。
