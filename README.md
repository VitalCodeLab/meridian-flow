# ESP32-C3 WS2812B LED 控制器

## 简要说明

这是一个在 ESP32-C3 上运行的高性能 WS2812B LED 灯带控制器，具有以下特点：
- 流畅的彩色流动效果，支持平滑过渡和拖尾效果
- 音频响应模式（音量条、频谱、节拍脉冲、音高颜色）
- 通过内置 Wi-Fi AP 的 Web 界面远程控制
- 亮度可调，默认设置为60（范围0-255）
- 代码采用模块化设计，便于维护和扩展

主要功能：
- 默认启动彩色流动效果
- 支持通过按钮切换不同模式
- 音频可视化效果
- 网页控制界面

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

## 硬件接线

- **WS2812B 灯带**
  - 数据线: GPIO0 （在 `src/main.cpp` 中常量 `LED_PIN`）
  - 灯珠数量: `LED_COUNT`（默认 60）
  - 电源: 5V (建议使用外部电源，确保电源充足)
  - 地线: GND (确保与ESP32共地)

- **音频输入**
  - 麦克风 (MAX9814) 输出: ADC 引脚 GPIO3 (`AUDIO_PIN`)
  - 电源: 3.3V
  - 地线: GND

- **控制按钮**
  - 按钮: GPIO9 (`BUTTON_PIN`)
  - 工作模式: 低电平有效 (`BUTTON_ACTIVE_LOW` 为 `true`)

## 主要配置

可在 `src/main.cpp` 中调整以下参数：

- **LED 设置**
  - `LED_PIN`: 灯带数据引脚（默认 GPIO0）
  - `LED_COUNT`: 灯珠数量（默认 60）
  - `gBrightness`: 全局亮度（0-255，默认 60）
  - `gPowerLimit_mA`: 电源限制（毫安）
  - `gLedFull_mA`: 单颗LED满亮度电流（毫安）

- **按钮设置**
  - `BUTTON_PIN`: 按钮引脚（默认 GPIO9）
  - `BUTTON_ACTIVE_LOW`: 按钮有效电平（默认 true，低电平有效）

- **音频效果设置**
  - `AUDIO_PIN`: 音频输入引脚（默认 GPIO3）
  - 音高检测相关参数：
    - `gPitchArmed`: 是否启用音高检测
    - `gPitchTargetHz`: 目标频率
    - `gPitchConfThresh`: 置信度阈值
    - `gPitchTolCents`: 音高容差（音分）
    - `gPitchMapEnable`: 启用频率映射
    - `gPitchMapScale`: 频率映射比例
    - `gPitchMapMinHz`: 最小频率
    - `gPitchMapMaxHz`: 最大频率

## 文件结构

- **src/main.cpp**
  - 程序入口，包含 `setup()` 和 `loop()` 函数
  - 全局配置和对象实例化
  - 各功能模块的初始化和调用

- **src/enhanced_led_controller.hpp / .cpp**
  - LED 灯带驱动核心
  - 实现多种灯光效果：
    - 彩色流动效果（支持平滑过渡和拖尾）
    - 点效果（用于音高检测反馈）
    - 音频可视化效果
  - 电源和亮度管理

- **src/audio_handler.h / .cpp**
  - 音频采集与分析
  - 主要功能：
    - `updateAudioLog()`: 更新音频数据
    - `handleAudioEffects()`: 处理音频效果
    - `handlePitchDetection()`: 音高检测处理

- **src/button_handler.h / .cpp**
  - 按钮去抖和事件处理
  - 主要功能：
    - `handleButtonActions()`: 处理按钮按下/长按事件
    - 模式切换和效果控制

- **src/hardware_check.h / .cpp**
  - 硬件连接检查
  - `checkHardwareConnections()`: 启动时检查硬件连接状态

- **src/webui.hpp / .cpp**
  - Web 界面和控制接口
  - 功能：
    - `startAp()`: 启动 Wi-Fi AP
    - `registerWeb()`: 注册 Web 路由和前端资源

- **platformio.ini**
  - PlatformIO 项目配置文件
  - 包含编译选项、库依赖等

## 程序执行流程

### 启动流程 (setup)
1. 初始化串口通信
2. 检查硬件连接
3. 初始化按钮和音频分析器
4. 设置 LED 控制器和效果
5. 启动 Wi-Fi AP 和 Web 服务器

### 主循环 (loop)
1. 输入采集：
   - `button.poll()`: 轮询按钮状态
   - `analyzer.tick()`: 更新音频分析
2. 功能处理：
   - `updateAudioLog()`: 更新音频数据
   - `handleAudioEffects()`: 处理音频效果
   - `handlePitchDetection()`: 处理音高检测
   - `handleButtonActions()`: 处理按钮事件
3. Web 服务：
   - `server.handleClient()`: 处理 Web 请求
4. 渲染：
   - `controller.tick()`: 更新 LED 显示

## 调试指南

### 常见问题排查

1. **灯带不亮**
   - 检查 `LED_PIN` 和 `LED_COUNT` 设置
   - 确保电源供应充足（建议使用外部电源）
   - 检查接线是否正确（数据线、电源、地线）
   - 查看串口日志中的硬件检查输出

2. **Web 界面无法访问**
   - 确认设备已启动 Wi-Fi AP（默认 SSID: `meridian-c3`）
   - 检查设备分配的 IP 地址（默认为 `192.168.4.1`）
   - 确保设备未进入深度睡眠模式

3. **音频效果异常**
   - 检查麦克风电源和接线
   - 确认 `AUDIO_PIN` 设置正确
   - 通过串口监视器查看音频分析器输出
   - 调整音频输入增益（如果麦克风支持）

4. **按钮无响应**
   - 检查按钮接线（信号、电源、地线）
   - 确认 `BUTTON_PIN` 和 `BUTTON_ACTIVE_LOW` 设置正确
   - 查看串口日志中的按钮事件

### 串口日志

- 波特率：115200
- 启动时显示硬件信息和初始化状态
- 运行中显示调试信息和错误信息

### 性能优化

- 如果出现闪烁或卡顿，可以尝试：
  - 降低 `gBrightness` 值
  - 减少 `LED_COUNT`
  - 增加 `flow.setInterval()` 值
  - 使用更高规格的电源

## 开发命令

### 常用命令

```bash
# 构建项目
pio run

# 上传固件到设备
pio run -t upload

# 启动串口监视器 (115200 baud)
pio device monitor -b 115200

# 清理构建文件
pio run -t clean
```

### VS Code 集成

如果使用 VS Code 和 PlatformIO 插件，可以使用以下功能：
- 构建: 点击底部状态栏的 ✓ 图标
- 上传: 点击底部状态栏的 → 图标
- 串口监视器: 点击底部状态栏的插头图标

## 扩展开发

### 添加新效果

1. 在 `enhanced_led_controller.hpp` 中定义新效果类
2. 实现 `EnhancedLEDEffect` 接口
3. 在 `EnhancedLEDController` 中添加效果管理
4. 在 `main.cpp` 中初始化和控制效果

### Web 界面扩展

1. 在 `webui.hpp/cpp` 中添加新的 API 路由
2. 更新前端页面（HTML/JS）
3. 添加新的控制参数和状态显示

### 贡献指南

1. Fork 项目仓库
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'Add some amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 提交 Pull Request

许可

- 本仓库默认无特定开源许可（请根据需要添加 LICENSE 文件）。

如需，能够：

- 生成项目的详尽 README（含 API 列表与 Web 接口说明）。
- 把 README 写成更简洁的版本用于设备内置文档或生成 wiki 页面。
