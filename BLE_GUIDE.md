# ESP32 8x8 LED 矩阵 - BLE 控制指南

## 📱 BLE 连接说明

### 1. 设备信息

- **设备名称**: `ESP32_Matrix`
- **蓝牙类型**: BLE (低功耗蓝牙)
- **服务 UUID**: `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
- **特征 UUID**:
  - 接收: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
  - 发送: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`

### 2. 连接步骤

1. 打开手机/电脑的蓝牙设置
2. 使用 BLE 扫描应用搜索设备
3. 找到并选择 `ESP32_Matrix`
4. 连接到 UART 服务
5. 使用通知功能接收响应

## 📲 推荐 BLE 终端应用

### Android

- **Serial Bluetooth Terminal (BLE)** (推荐)
- **nRF Connect for Mobile** (调试用)
- **BLE Terminal**
- **LightBlue Explorer**

### iOS

- **LightBlue Explorer** (推荐)
- **nRF Connect for Mobile** (调试用)
- **BLE Terminal**
- **Serial Bluetooth Terminal**

### Windows/Mac/Linux

- **nRF Connect for Desktop**
- **BLE Scanner** (Mac)
- **BlueZ** (Linux 命令行)

## 🎮 BLE 控制命令详解

### 基本命令

```
help 或 h          # 显示帮助信息
status 或 s        # 显示当前系统状态
clear 或 c         # 清空LED屏幕
```

### 显示模式控制

```
mode <0-6>         # 设置显示模式
  0 = 关闭
  1 = 音频频谱
  2 = 音频波形
  3 = 文字滚动
  4 = 静态文字
  5 = 动画
  6 = 自定义图案

# 示例
mode 1             # 切换到音频频谱模式
mode 3             # 切换到文字滚动模式
```

### 文字显示控制

```
text <文字内容>    # 设置滚动文字
static <文字内容>  # 设置静态文字
speed <50-500>     # 设置滚动速度 (毫秒)

# 示例
text Hello ESP32   # 显示滚动文字 "Hello ESP32"
static 你好        # 静态显示 "你好"
speed 200          # 设置滚动速度为200ms
```

### 音频可视化控制

```
viz <0-3>          # 音频可视化类型
  0 = 频谱条
  1 = 圆形
  2 = 波形
  3 = 粒子

# 示例
viz 0              # 频谱条显示
viz 1              # 圆形可视化
```

### 预设图案

```
heart              # 显示爱心 ❤️
smiley             # 显示笑脸 😊
number <0-9>       # 显示数字 0-9
arrow <u/r/d/l>    # 显示箭头
  u = 上箭头 ⬆️
  r = 右箭头 ➡️
  d = 下箭头 ⬇️
  l = 左箭头 ⬅️
equalizer 或 eq    # 均衡器效果

# 示例
heart              # 显示爱心
number 8           # 显示数字8
arrow u            # 显示上箭头
```

### 音频特效

```
beat               # 节拍检测效果
frequency 或 freq  # 频率段显示
volume 或 vol      # 音量表显示

# 示例
beat               # 启动节拍检测
freq               # 显示频率段
```

### 系统设置

```
brightness <0-255> # 设置亮度 (0=最暗, 255=最亮)
demo [on/off]      # 演示模式控制

# 示例
brightness 128     # 设置中等亮度
demo on            # 开启演示模式
demo off           # 关闭演示模式
```

## 🔧 使用 nRF Connect 详细步骤

### 1. 扫描连接

1. 打开 nRF Connect 应用
2. 点击"SCAN"扫描设备
3. 找到 `ESP32_Matrix` 并点击 CONNECT
4. 等待连接成功

### 2. 找到 UART 服务

1. 在服务列表中找到 `UART Service` (UUID: 6E400001...)
2. 展开该服务，看到两个特征：
   - `TX Characteristic` (6E400003...) - 接收数据
   - `RX Characteristic` (6E400002...) - 发送命令

### 3. 启用通知

1. 点击 `TX Characteristic` 旁边的下拉箭头
2. 启用"Notify"或"Indicate"
3. 现在可以接收 ESP32 的响应

### 4. 发送命令

1. 点击 `RX Characteristic` 旁边的下拉箭头
2. 选择"Write"或"Write without response"
3. 在文本框中输入命令，如"help"
4. 点击发送

## 📱 使用 Serial Bluetooth Terminal

### Android 步骤

1. 下载并打开"Serial Bluetooth Terminal (BLE)"
2. 点击右上角菜单，选择"Connect device"
3. 选择"BLE"标签页
4. 扫描并选择 `ESP32_Matrix`
5. 连接成功后即可发送命令

### iOS 步骤

1. 下载并打开"LightBlue Explorer"
2. 点击"Devices"标签
3. 找到 `ESP32_Matrix` 并连接
4. 在服务中找到 UART 服务
5. 使用特征值进行通信

## 💡 使用技巧

### 1. 快速开始

连接 BLE 后，输入 `help` 查看所有可用命令。

### 2. 演示模式

- 系统启动后自动开启演示模式
- 每 5 秒自动切换不同效果
- 使用 `demo off` 可以关闭自动切换

### 3. 音频可视化

- 系统内置音频生成器，无需外部音频输入
- 可以实时看到频谱分析和可视化效果
- 尝试不同的 `viz` 模式获得最佳效果

### 4. 文字显示

- 支持中英文混合显示
- 滚动文字会自动循环
- 静态文字会在屏幕居中显示

### 5. 亮度调节

- 根据环境光线调节亮度
- 夜间建议使用较低亮度 (50-100)
- 白天可以使用较高亮度 (150-255)

## 🔧 故障排除

### 连接问题

**问题**: 无法找到 ESP32_Matrix 设备

- 解决: 确认 ESP32 已正常启动，串口监视器显示"BLE 控制已启动"
- 重启 ESP32 设备
- 确保手机支持 BLE

**问题**: 连接后无响应

- 解决: 检查是否启用了 TX 特征的通知
- 确认 BLE 终端应用设置
- 尝试发送 `help` 命令测试

### 命令问题

**问题**: 命令无响应

- 解决: 检查命令拼写
- 确保命令后按发送键
- 使用 `help` 查看正确命令格式

**问题**: 显示效果异常

- 解决: 检查 LED 矩阵接线
- 确认电源供电充足
- 尝试 `clear` 清屏后重新设置

### 性能问题

**问题**: 显示卡顿

- 解决: 降低滚动速度 `speed 300`
- 减少亮度设置 `brightness 100`
- 关闭演示模式 `demo off`

## 🎨 BLE vs 经典蓝牙对比

| 特性     | BLE      | 经典蓝牙     |
| -------- | -------- | ------------ |
| 功耗     | 极低     | 较高         |
| 连接速度 | 快速     | 中等         |
| 传输距离 | 中等     | 较远         |
| 兼容性   | 现代设备 | 广泛兼容     |
| 配对要求 | 简单     | 可能需要密码 |
| 稳定性   | 很好     | 很好         |

## 📱 手机控制示例

### Android Serial Bluetooth Terminal

1. 下载并打开应用
2. 选择 BLE 模式
3. 扫描并连接 `ESP32_Matrix`
4. 启用通知
5. 在输入框中输入命令

### iOS LightBlue Explorer

1. 下载并打开应用
2. 扫描并连接 `ESP32_Matrix`
3. 找到 UART 服务
4. 启用 TX 特征通知
5. 使用 RX 特征发送命令

## 🚀 高级用法

### 命令组合

可以在一行中发送多个命令，用分号分隔：

```
text Hello; brightness 150; viz 1
```

### 自动化脚本

可以创建 BLE 脚本文件，批量发送命令实现复杂效果。

### 实时控制

BLE 控制响应迅速，可以实时调整显示效果，非常适合现场演示和互动展示。

## 📊 BLE 技术参数

- **广播间隔**: 100ms
- **连接间隔**: 30ms-50ms
- **传输功率**: 最大 (ESP_PWR_LVL_P9)
- **MTU 大小**: 20 字节 (默认)
- **服务类型**: UART (串口模拟)

---

**享受你的 BLE 控制 LED 矩阵体验！** 🎵✨

如有问题，请检查串口监视器的调试信息以获得更多帮助。

## 📞 技术支持

如果遇到 BLE 连接问题，请：

1. 检查 ESP32-C3 固件版本
2. 确认手机系统支持 BLE
3. 使用 nRF Connect 进行调试
4. 查看串口监视器日志
