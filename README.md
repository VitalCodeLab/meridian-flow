# ESP32-C3 WS2812B LED + TCM 经络控制器

## 简要说明

这是一个在 ESP32-C3 上运行的高性能 WS2812B LED 灯带控制器，同时集成了中医经络演示系统，具有以下特点：

- 流畅的彩色流动效果，支持平滑过渡和拖尾效果
- 多种音频响应模式（音量条、频谱、节拍脉冲、音高颜色 / Pitch 模式）
- TCM 中医经络 / 穴位 / 子午流注演示，与主灯光共用同一根灯带，可通过 JSON 配置启用哪些经络及其灯带区间
- 通过内置 Wi-Fi AP 的 Web 界面远程控制（根页面 `/` 与经络页面 `/tcm`）
- 亮度可调，默认设置为 60（范围 0-255）
- 代码模块化，便于维护和扩展

主要功能：

- 默认启动彩色流动效果（FLOW 模式）
- 通过实体按钮在 FLOW / STEP 两种指示模式间切换
- 音频可视化（音量条 / 频谱 / 节拍脉冲 / Pitch 音高显示）
- Pitch Detection 命中提示 + Pitch 模式中用 1~7 级长度和颜色展示当前音高
- 中医经络静态显示、循行动画、子午流注、常用穴位高亮
- Web 控制界面和简洁的 HTTP API

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

4. 连接与控制（Web UI & TCM）
   - 设备会启动为 Wi‑Fi AP（默认 SSID: `meridian-c3`）。
   - 连接到该 AP 后，在浏览器访问 `http://192.168.4.1/` 进入主控制页面；访问 `http://192.168.4.1/tcm` 进入经络演示页面（TCM）。
   - 在主页面可以调节亮度、启用/关闭音频效果、切换音频模式、Arm/Disarm Pitch Detection、勾选 Pitch→Length；也可以通过 `/api/tcm?enable=1/0` 或前端按钮切换 TCM 模式。

## 硬件接线

- **WS2812B 灯带**

  - 数据线: GPIO0 （在 `src/main.cpp` 中常量 `LED_PIN`）
  - 灯珠数量: `LED_COUNT`（默认 60）
  - 电源: 5V (建议使用外部电源，确保电源充足)
  - 地线: GND (确保与 ESP32 共地)

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
  - `gLedFull_mA`: 单颗 LED 满亮度电流（毫安）

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
  - 默认情况下音频效果关闭，且仅在满足以下条件时才采样音频并处理：
    - 启用了音频可视化效果（`/api/audio?enable=1`）；或
    - Arm 了 Pitch Detection；或
    - 启用了 Pitch→Length 映射。
  - `updateAudioLog()` 只有在音频效果启用时才输出日志，避免默认刷屏。

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

- **src/meridian_tcm.hpp / .cpp**

  - 中医经络系统核心逻辑
  - 定义经络与穴位数据结构、非阻塞经络循行动画、子午流注、常用穴位显示等

- **src/meridian_config.hpp**

  - SPIFFS JSON 配置加载
  - 从 `data/meridians*.json` 读取经络配置，支持 `enabled`、`startIndex`、`length` 等字段

- **src/tcm_page.h**

  - TCM 经络控制页面 HTML/JS 模板
  - 前端经络选择、子午流注开关、常用穴位按钮等

- **data/meridians.json / meridians_more.json / meridians_rest.json**

  - 中医经络和穴位配置文件
  - 可通过修改 `enabled`、`startIndex`、`length` 调整启用的经络和灯带布局

- **platformio.ini**
  - PlatformIO 项目配置文件
  - 包含编译选项、库依赖等

## 程序执行流程

### 启动流程 (setup)

1. 初始化串口通信
2. 检查硬件连接
3. 初始化按钮和音频分析器
4. 设置 LED 控制器和默认流动效果（FLOW 模式）
5. 初始化 TCM 经络系统（加载经络配置等）
6. 启动 Wi-Fi AP 和 Web 服务器

### 主循环 (loop)

1. 输入采集：
   - `button.poll()`: 轮询按钮状态
   - 当 **未启用 TCM 模式** 且存在音频相关需求时，才调用 `analyzer.tick()` 更新音频分析：
     - `controller.audioEnabled()` 为 true，或
     - `gPitchArmed` 为 true，或
     - `gPitchMapEnable` 为 true。
2. 功能处理：
   - 在满足条件时调用：
     - `updateAudioLog()`: 输出音频状态日志（仅在音频效果启用时打印）
     - `handleAudioEffects()`: 处理音频可视化和 Pitch→Length 映射
     - `handlePitchDetection()`: 处理音高检测命中提示
   - 始终调用：
     - `handleButtonActions()`: 处理按钮短按/长按（切换 FLOW/STEP、步进等）
3. Web 服务：
   - `server.handleClient()`: 处理 Web 请求（主页面 `/` 与 TCM 页面 `/tcm` 共用同一 WebServer）
4. 渲染与模式切换：
   - 当 `gTcmMode == false` 时：
     - 调用 `controller.tick()`，更新主灯光效果和音频可视化。
   - 当 `gTcmMode == true` 时：
     - 暂停主控制器渲染，调用 `tcmTick()` 以非阻塞方式推进经络循行动画。

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

## HTTP API 总览

> 所有 API 默认监听在设备 AP 的 IP 上（例如 `http://192.168.4.1`），下表省略主机部分，仅列出路径。

### 主控制与音频相关 API

| 路径              | 方法 | 主要参数                  | 说明                                                                                                        |
| ----------------- | ---- | ------------------------- | ----------------------------------------------------------------------------------------------------------- |
| `/api/state`      | GET  | 无                        | 返回当前整体状态（模式、亮度、功率估计、音频开关与当前模式、TCM 开关、Pitch 配置等），用于前端轮询更新 UI。 |
| `/api/brightness` | GET  | `value` (0-255)           | 设置全局亮度 `gBrightness`。                                                                                |
| `/api/power`      | GET  | `limit_ma`, `led_full_ma` | 配置电源电流限制与单颗 LED 估算电流。                                                                       |
| `/api/flow/start` | GET  | 无                        | 启动主 FLOW 模式的流动效果。                                                                                |
| `/api/flow/stop`  | GET  | 无                        | 停止主 FLOW 流动效果。                                                                                      |
| `/api/audio`      | GET  | `enable` (0/1)            | 启用/关闭音频可视化效果。关闭时会顺便关闭 Pitch Detection 与 Pitch→Length，并清除指示点。                   |
| `/api/audio/mode` | GET  | `mode` (0-3)              | 设置音频可视化模式：0=VUMeter，1=Spectrum，2=Beat Pulse，3=Pitch Color。                                    |
| `/api/pitch`      | GET  | `arm` (0/1)               | Arm/Disarm Pitch Detection（音高命中检测）。Disarm 时会清除由 Pitch 命中产生的点效果。                      |
| `/api/pitchmap`   | GET  | `enable` (0/1)            | 启用/关闭 Pitch→Length 映射逻辑（音高映射到长度参数）。                                                     |

### TCM 经络相关 API

| 路径                    | 方法 | 主要参数                        | 说明                                                                                                                          |
| ----------------------- | ---- | ------------------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `/api/tcm`              | GET  | `enable` (0/1)                  | 主控制与经络系统之间的模式切换：1=进入 TCM 模式（停止主控制器渲染），0=退出 TCM 模式（恢复主灯光效果，并停止任何 TCM 动画）。 |
| `/api/select`           | GET  | `meridian` (0-11)               | 选择当前经络（手太阴肺经等 12 经），并显示该经络的静态分布；切换经络时会停止当前 TCM 循行动画。                               |
| `/api/show`             | GET  | 无                              | 显示当前选中经络的静态状态（会停止 TCM 循行动画），自动开启 TCM 模式。                                                        |
| `/api/showall`          | GET  | 无                              | 显示所有经络的静态状态，自动开启 TCM 模式并停止 TCM 动画。                                                                    |
| `/api/flow`             | GET  | `speed` (10-100，可选，默认 30) | 按当前选中经络执行一次非阻塞循行动画，`speed` 越大动画越快。                                                                  |
| `/api/flowall`          | GET  | `speed` (10-100，可选)          | 对所有经络执行非阻塞循行动画。                                                                                                |
| `/api/flow/current`     | GET  | `speed` (10-100，可选)          | 按当前子午流注当令经络执行一次非阻塞循行动画。                                                                                |
| `/api/tcm/brightness`   | GET  | `value` (0-255)                 | 设置 TCM 经络系统内部使用的亮度（与主控制器的 `gBrightness` 分开）。                                                          |
| `/api/acupoint`         | GET  | `name` (穴位英文名)             | 闪烁显示指定穴位，并返回包含中/英文名、拼音、定位、功效、主治等字段的 JSON 信息。若仅找到但无详情，则返回简短提示。           |
| `/api/acupoints`        | GET  | 无                              | 返回所有穴位的简要列表（名称、中文名、所属经络、重要程度）。                                                                  |
| `/api/ziwuliuzhu`       | GET  | `enable` (0/1)                  | 启用/禁用子午流注自动当令经络逻辑。启用时会自动进入 TCM 模式。                                                                |
| `/api/auto`             | GET  | `enable` (0/1)                  | 启用/禁用自动切换经络（独立于子午流注，只做简单轮换）。                                                                       |
| `/api/auto/interval`    | GET  | `value` (5-60，秒)              | 设置经络自动切换的时间间隔（单位秒）。                                                                                        |
| `/api/current-meridian` | GET  | 无                              | 当子午流注启用时，返回当前当令经络的中文名与时间段说明；未启用时返回提示 JSON。                                               |

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
