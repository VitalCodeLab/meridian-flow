# BLE 乱码问题修复指南

## 🔧 Serial Bluetooth Terminal (BLE) 乱码解决方案

### 问题现象

- 发送命令后收到乱码：`??????` 或 `[][][][]`
- 中文字符显示异常
- 部分字符丢失或重复

### 🛠️ 解决方案

#### 方案 1: 调整应用设置

**Serial Bluetooth Terminal (BLE) 设置：**

1. **编码设置**

   ```
   Settings → Encoding → UTF-8
   ```

2. **字符集设置**

   ```
   Settings → Character Set → Unicode (UTF-8)
   ```

3. **换行符设置**
   ```
   Settings → Line Ending → CR+LF
   ```

#### 方案 2: 使用正确的连接方式

**连接步骤：**

1. 打开 "Serial Bluetooth Terminal (BLE)"
2. 点击右上角菜单 → "Connect device"
3. 选择 "BLE" 标签页
4. 扫描并选择 "ESP32_Matrix"
5. **重要**: 连接后等待 2-3 秒再发送命令

#### 方案 3: 测试简单命令

**先测试英文命令：**

```
help
status
clear
```

**再测试中文命令：**

```
text Hello
text 你好
```

#### 方案 4: 使用 nRF Connect 调试

**如果仍然乱码，使用 nRF Connect：**

1. 下载 "nRF Connect for Mobile"
2. 连接 "ESP32_Matrix"
3. 找到 UART Service
4. 启用 TX Characteristic 的通知
5. 使用 RX Characteristic 发送命令

#### 方案 5: 检查 ESP32 串口输出

**查看调试信息：**

```bash
pio device monitor -p COM_PORT -b 115200
```

**正常输出应该显示：**

```
BLE> Connected to ESP32 Matrix!
BLE> Type 'help' for commands
```

### 🔍 调试步骤

#### 步骤 1: 确认 BLE 连接

```
在串口监视器中查看：
"BLE客户端连接"
"BLE设备已连接"
```

#### 步骤 2: 测试基本通信

发送命令: `help`
应该收到命令列表，如果收到乱码说明编码问题

#### 步骤 3: 测试中英文混合

```
发送: text Hello World
期望: OK: 滚动文字设置: Hello World

发送: text 你好世界
期望: OK: 滚动文字设置: 你好世界
```

### ⚙️ 代码层面的修复

#### 1. MTU 协商

```cpp
NimBLEDevice::setMTU(247); // 已添加
```

#### 2. 发送延迟

```cpp
delay(10); // 已添加，确保传输完成
```

#### 3. UTF-8 编码

```cpp
String utf8Response = response; // 已添加
```

### 📱 其他 BLE 终端应用推荐

#### Android

1. **BLE Terminal** - 简单稳定
2. **nRF Connect** - 功能强大
3. **LightBlue** - 调试专用

#### iOS

1. **LightBlue Explorer** - 推荐
2. **nRF Connect** - 功能全面
3. **BLE Scanner** - 简单易用

### 🚨 常见问题

#### 问题 1: 连接后无响应

**解决**: 等待 2-3 秒再发送命令，让 BLE 连接稳定

#### 问题 2: 部分字符丢失

**解决**: 检查 MTU 设置，确保使用最大 MTU

#### 问题 3: 中文完全乱码

**解决**:

1. 确认应用编码设置为 UTF-8
2. 使用 nRF Connect 测试
3. 检查 ESP32 串口输出

#### 问题 4: 命令无响应

**解决**:

1. 确认启用了 TX 特征通知
2. 检查 RX 特征写入权限
3. 查看串口监视器调试信息

### 🎯 最佳实践

#### 推荐工作流程

1. 使用 nRF Connect 确认 BLE 功能正常
2. 使用 Serial Bluetooth Terminal (BLE)进行日常控制
3. 遇到问题时查看串口监视器

#### 代码调试

```cpp
// 在BLE发送前添加调试
Serial.print("发送BLE消息: ");
Serial.println(response);
```

#### 连接稳定性

- 连接后等待 500ms 再发送命令
- 避免快速连续发送命令
- 长消息分批发送

### 📋 测试清单

- [ ] BLE 连接成功
- [ ] 英文命令正常响应
- [ ] 中文命令正常显示
- [ ] 频谱可视化正常
- [ ] 文字滚动正常
- [ ] 亮度调节正常
- [ ] 模式切换正常

---

**如果问题仍然存在，请：**

1. 检查串口监视器输出
2. 使用 nRF Connect 进行详细调试
3. 确认手机 BLE 功能正常
4. 尝试重启 ESP32 设备

**技术支持**: 查看串口监视器获取详细调试信息
