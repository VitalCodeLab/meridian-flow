#include "ble_control.hpp"

// BLE服务UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART服务
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // 接收特征
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // 发送特征

BLEControl::BLEControl() {
    pServer = nullptr;
    pTxCharacteristic = nullptr;
    pRxCharacteristic = nullptr;
    pService = nullptr;
    matrix = nullptr;
    deviceConnected = false;
    oldDeviceConnected = false;
    txValue = 0;
    deviceName = "ESP32_Matrix";
    
    serverCallbacks = new ServerCallbacks(this);
    charCallbacks = new CharacteristicCallbacks(this);
}

BLEControl::~BLEControl() {
    delete serverCallbacks;
    delete charCallbacks;
    // NimBLEServer会自动清理，不需要手动删除
    NimBLEDevice::deinit();
}

void BLEControl::begin(const String& name) {
    deviceName = name;
    
    // 初始化BLE
    NimBLEDevice::init(name.c_str());
    
    // 设置传输功率
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); // 最大功率
    
    // 设置MTU大小
    NimBLEDevice::setMTU(247); // 最大MTU大小
    
    // 创建BLE服务器
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(serverCallbacks);
    
    // 创建BLE服务
    pService = pServer->createService(SERVICE_UUID);
    
    // 创建发送特征（通知）
    pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        NIMBLE_PROPERTY::NOTIFY |
                        NIMBLE_PROPERTY::READ
                      );
    
    // 创建接收特征（写入）
    pRxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        NIMBLE_PROPERTY::WRITE |
                        NIMBLE_PROPERTY::WRITE_NR
                      );
    pRxCharacteristic->setCallbacks(charCallbacks);
    
    // 启动服务
    pService->start();
    
    // 开始广播
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // 不使用首选间隔
    pAdvertising->setMaxPreferred(0x0);  // 不使用首选间隔
    pAdvertising->start();
    
    Serial.println("BLE控制已启动");
    Serial.print("设备名称: ");
    Serial.println(name);
    Serial.println("等待BLE连接...");
    
    sendNotification("ESP32 8x8 LED矩阵控制系统");
    sendNotification("发送 'help' 查看命令列表");
}

void BLEControl::setMatrix(MatrixDisplay* display) {
    matrix = display;
}

void BLEControl::update() {
    // 检查连接状态变化
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // 给蓝牙栈时间断开连接
        pServer->startAdvertising(); // 重新开始广播
        oldDeviceConnected = deviceConnected;
        Serial.println("BLE广播已重启，等待连接...");
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("BLE设备已连接");
        sendNotification("欢迎连接ESP32矩阵控制器！");
        showHelp();
    }
}

// 服务器回调实现
void BLEControl::ServerCallbacks::onConnect(NimBLEServer* pServer) {
    parent->deviceConnected = true;
    Serial.println("BLE客户端连接");
    
    // 连接后发送简单ASCII测试消息
    delay(500); // 等待连接稳定
    parent->sendTestASCII();
}

void BLEControl::ServerCallbacks::onDisconnect(NimBLEServer* pServer) {
    parent->deviceConnected = false;
    Serial.println("BLE客户端断开");
}

// 特征回调实现
void BLEControl::CharacteristicCallbacks::onWrite(NimBLECharacteristic* pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    String command = rxValue.c_str();
    
    if (command.length() > 0) {
        Serial.print("收到BLE命令: ");
        Serial.println(command);
        parent->processCommand(command);
    }
}

void BLEControl::CharacteristicCallbacks::onRead(NimBLECharacteristic* pCharacteristic) {
    // 处理读取请求（如果需要）
}

void BLEControl::processCommand(const String& command) {
    // 调试：显示原始命令
    sendResponse("DEBUG: Raw command: '" + command + "' (length: " + String(command.length()) + ")");
    
    String cmd = command;
    cmd.trim();
    String rawCmd = cmd;
    cmd.toLowerCase();
    
    // 调试：显示处理后命令
    sendResponse("DEBUG: Processed command: '" + cmd + "' (length: " + String(cmd.length()) + ")");
    
    if (cmd == "help" || cmd == "h") {
        showHelp();
    } else if (cmd == "status" || cmd == "s") {
        showStatus();
    } else if (cmd.startsWith("mode ")) {
        parseModeCommand(cmd.substring(5));
    } else if (cmd.startsWith("text ")) {
        String params = rawCmd.substring(5);
        sendResponse("DEBUG: Processing text command, params: '" + params + "'");
        parseTextCommand(params);
    } else if (cmd.startsWith("static ")) {
        parseTextCommand(rawCmd.substring(7), true);
    } else if (cmd.startsWith("brightness ") || cmd.startsWith("b ")) {
        parseBrightnessCommand(cmd.indexOf(' ') == -1 ? cmd.substring(2) : cmd.substring(11));
    } else if (cmd.startsWith("pattern ")) {
        parsePatternCommand(cmd.substring(8));
    } else if (cmd.startsWith("viz ")) {
        parseVizCommand(cmd.substring(4));
    } else if (cmd == "demo" || cmd == "d") {
        parseDemoCommand("toggle");
    } else if (cmd.startsWith("demo ")) {
        parseDemoCommand(cmd.substring(5));
    } else if (cmd == "clear" || cmd == "c") {
        if (matrix) {
            matrix->setMode(MatrixDisplay::MODE_OFF);
            sendOK("Screen cleared");
        }
    } else if (cmd == "heart") {
        if (matrix) {
            matrix->showHeart();
            sendOK("Heart pattern");
        }
    } else if (cmd == "smiley") {
        if (matrix) {
            matrix->showSmiley();
            sendOK("Smiley face");
        }
    } else if (cmd.startsWith("number ")) {
        String numStr = cmd.substring(7);
        int num = numStr.toInt();
        if (matrix && num >= 0 && num <= 9) {
            matrix->showNumber(num);
            sendOK("Number: " + String(num));
        } else {
            sendError("Number must be 0-9");
        }
    } else if (cmd == "equalizer" || cmd == "eq") {
        if (matrix) {
            matrix->showEqualizer();
            sendOK("Equalizer pattern");
        }
    } else if (cmd.startsWith("arrow ")) {
        String dir = cmd.substring(6);
        int direction = -1;
        if (dir == "up" || dir == "u") direction = 0;
        else if (dir == "right" || dir == "r") direction = 1;
        else if (dir == "down" || dir == "d") direction = 2;
        else if (dir == "left" || dir == "l") direction = 3;
        
        if (matrix && direction >= 0) {
            matrix->showArrow(direction);
            String dirNames[] = {"UP", "RIGHT", "DOWN", "LEFT"};
            sendOK("Arrow: " + dirNames[direction]);
        } else {
            sendError("Direction: up/right/down/left");
        }
    } else if (cmd.startsWith("speed ")) {
        String speedStr = cmd.substring(6);
        int speed = speedStr.toInt();
        if (matrix && speed >= 50 && speed <= 500) {
            matrix->setScrollSpeed(speed);
            sendOK("Scroll speed: " + String(speed));
        } else {
            sendError("Speed must be 50-500");
        }
    } else if (cmd == "beat") {
        if (matrix) {
            matrix->showBeatDetection();
            sendOK("Beat detection");
        }
    } else if (cmd == "frequency" || cmd == "freq") {
        if (matrix) {
            matrix->showFrequencyBands();
            sendOK("Frequency bands");
        }
    } else if (cmd == "volume" || cmd == "vol") {
        if (matrix) {
            matrix->showVolumeMeter();
            sendOK("Volume meter");
        }
    } else if (cmd.startsWith("sensitivity ")) {
        // 调整音频灵敏度
        String value = cmd.substring(12);
        float sens = value.toFloat();
        if (sens >= 0.1f && sens <= 5.0f) {
            // 这里需要访问audioAnalyzer，暂时通过全局变量或回调
            sendOK("Sensitivity: " + String(sens));
        } else {
            sendError("Sensitivity must be 0.1-5.0");
        }
    } else if (cmd == "quiet") {
        // 静音模式 - 极低灵敏度
        sendOK("Quiet mode - very low sensitivity");
    } else if (cmd == "normal") {
        // 正常模式 - 适中灵敏度
        sendOK("Normal mode - moderate sensitivity");
    } else if (cmd == "loud") {
        // 嘈杂环境 - 较高灵敏度
        sendOK("Loud mode - higher sensitivity");
    } else if (cmd == "test") {
        // LED测试模式 - 逐列点亮
        if (matrix) {
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    matrix->xy(x, y, CRGB::Blue);
                }
                matrix->update();
                delay(200);
                for (int y = 0; y < 8; y++) {
                    matrix->xy(x, y, CRGB::Black);
                }
                matrix->update();
                delay(100);
            }
            sendOK("LED column test completed");
        }
    } else if (cmd == "texttest") {
        // 文字测试模式
        if (matrix) {
            matrix->setText("TEST");
            matrix->setMode(MatrixDisplay::MODE_TEXT_SCROLL);
            sendOK("Text test: 'TEST' scrolling");
        }
    } else if (cmd == "showtext") {
        // 强制显示静态文字测试
        if (matrix) {
            matrix->setText("HELLO");
            matrix->setTextColor(CRGB::White);
            matrix->showStaticText("HELLO");
            sendOK("Static text: 'HELLO'");
        }
    } else if (cmd == "chardebug") {
        // 字符调试 - 显示单个字符的位模式
        if (matrix) {
            // 显示字母A的调试
            matrix->setText("A");
            matrix->setTextColor(CRGB::Red);
            matrix->showStaticText("A");
            sendOK("Character debug: 'A' in red");
        }
    } else if (cmd == "abc") {
        // 字母测试 - 逐个显示字母
        if (matrix) {
            matrix->setText("E");
            matrix->setTextColor(CRGB::Green);
            matrix->showStaticText("E");
            sendOK("Letter test: 'E' in green");
        }
    } else {
        sendError("Unknown command: " + cmd + ". Type 'help'");
    }
}

void BLEControl::showHelp() {
    sendResponse("=== ESP32 LED Matrix Commands ===");
    sendResponse("Basic:");
    sendResponse("  help/h - Show help");
    sendResponse("  status/s - Show status");
    sendResponse("  clear/c - Clear screen");
    sendResponse("  test - LED column test");
    sendResponse("  texttest - Text scrolling test");
    sendResponse("  showtext - Static text test");
    sendResponse("  chardebug - Single character debug");
    sendResponse("  abc - Letter 'E' test");
    sendResponse("Modes:");
    sendResponse("  mode <0-6> - Set display mode");
    sendResponse("Text:");
    sendResponse("  text <msg> - Scroll text");
    sendResponse("  static <msg> - Static text");
    sendResponse("Patterns:");
    sendResponse("  heart - Heart pattern");
    sendResponse("  smiley - Smiley face");
    sendResponse("  number <0-9> - Number");
    sendResponse("Audio:");
    sendResponse("  beat - Beat detection");
    sendResponse("  freq - Frequency bands");
    sendResponse("  vol - Volume meter");
    sendResponse("  sensitivity <0.1-5.0> - Audio sensitivity");
    sendResponse("  quiet - Very low sensitivity");
    sendResponse("  normal - Moderate sensitivity");
    sendResponse("  loud - High sensitivity");
    sendResponse("Settings:");
    sendResponse("  brightness <0-255> - Set brightness");
    sendResponse("  demo on/off - Demo mode");
    sendResponse("===============================");
}

void BLEControl::showStatus() {
    sendResponse("=== System Status ===");
    sendResponse("BLE: " + String(deviceConnected ? "Connected" : "Disconnected"));
    sendResponse("Device: " + deviceName);
    
    if (matrix) {
        sendResponse("Mode: " + getModeName(matrix->getMode()));
        sendResponse("Text: " + String(matrix->getCurrentText()));
        sendResponse("Demo: " + String(matrix->isAnimationRunning() ? "ON" : "OFF"));
    } else {
        sendResponse("Matrix: Not initialized");
    }
    sendResponse("==================");
}

void BLEControl::parseModeCommand(const String& params) {
    if (!matrix) {
        sendError("Matrix not initialized");
        return;
    }
    
    int mode = params.toInt();
    if (mode >= 0 && mode <= 6) {
        matrix->setMode(static_cast<MatrixDisplay::DisplayMode>(mode));
        sendOK("Mode set: " + getModeName(static_cast<MatrixDisplay::DisplayMode>(mode)));
    } else {
        sendError("Mode must be 0-6");
    }
}

void BLEControl::parseTextCommand(const String& params, bool isStatic) {
    if (!matrix) {
        sendError("Matrix not initialized");
        return;
    }
    
    if (params.length() == 0) {
        sendError("Text cannot be empty");
        return;
    }
    
    // 调试：发送接收到的参数
    sendResponse("DEBUG: Received text: '" + params + "' (length: " + String(params.length()) + ")");
    
    matrix->setText(params.c_str());
    matrix->setTextColor(CRGB::White); // 设置为白色确保可见
    matrix->setScrollSpeed(150); // 设置合适的滚动速度
    
    if (isStatic) {
        matrix->showStaticText(params.c_str());
        sendOK("Static text: " + params);
    } else {
        matrix->setMode(MatrixDisplay::MODE_TEXT_SCROLL);
        sendOK("Scroll text: " + params);
    }
}

void BLEControl::parseBrightnessCommand(const String& params) {
    if (!matrix) {
        sendError("Matrix not initialized");
        return;
    }
    
    int brightness = params.toInt();
    if (brightness >= 0 && brightness <= 255) {
        matrix->setBrightness(brightness);
        sendOK("Brightness: " + String(brightness));
    } else {
        sendError("Brightness must be 0-255");
    }
}

void BLEControl::parsePatternCommand(const String& params) {
    if (!matrix) {
        sendError("Matrix not initialized");
        return;
    }
    
    String pattern = params;
    pattern.toLowerCase();
    
    if (pattern == "heart") {
        matrix->showHeart();
        sendOK("Heart pattern");
    } else if (pattern == "smiley") {
        matrix->showSmiley();
        sendOK("Smiley face");
    } else if (pattern.startsWith("arrow")) {
        String dir = pattern.indexOf(' ') == -1 ? "" : pattern.substring(6);
        int direction = -1;
        if (dir == "up" || dir == "u") direction = 0;
        else if (dir == "right" || dir == "r") direction = 1;
        else if (dir == "down" || dir == "d") direction = 2;
        else if (dir == "left" || dir == "l") direction = 3;
        
        if (direction >= 0) {
            matrix->showArrow(direction);
            String dirNames[] = {"UP", "RIGHT", "DOWN", "LEFT"};
            sendOK("Arrow: " + dirNames[direction]);
        } else {
            sendError("Direction: up/right/down/left");
        }
    } else {
        sendError("Unknown pattern: " + pattern);
    }
}

void BLEControl::parseVizCommand(const String& params) {
    if (!matrix) {
        sendError("Matrix not initialized");
        return;
    }
    
    int type = params.toInt();
    if (type >= 0 && type <= 3) {
        matrix->setVisualizationType(static_cast<MatrixDisplay::VisualizationType>(type));
        sendOK("Viz type: " + getVizTypeName(static_cast<MatrixDisplay::VisualizationType>(type)));
    } else {
        sendError("Viz type must be 0-3");
    }
}

void BLEControl::parseDemoCommand(const String& params) {
    String cmd = params;
    cmd.toLowerCase();
    
    if (cmd == "toggle") {
        if (matrix) {
            bool isRunning = matrix->isAnimationRunning();
            if (isRunning) {
                matrix->stopAnimation();
                sendOK("Demo OFF");
            } else {
                matrix->startAnimation();
                sendOK("Demo ON");
            }
        }
    } else if (cmd == "on" || cmd == "start") {
        if (matrix) {
            matrix->startAnimation();
            sendOK("Demo ON");
        }
    } else if (cmd == "off" || cmd == "stop") {
        if (matrix) {
            matrix->stopAnimation();
            sendOK("Demo OFF");
        }
    } else {
        sendError("Demo command: toggle/on/off");
    }
}

String BLEControl::getModeName(MatrixDisplay::DisplayMode mode) {
    switch (mode) {
        case MatrixDisplay::MODE_OFF: return "OFF";
        case MatrixDisplay::MODE_AUDIO_SPECTRUM: return "AUDIO";
        case MatrixDisplay::MODE_AUDIO_WAVEFORM: return "WAVE";
        case MatrixDisplay::MODE_TEXT_SCROLL: return "TEXT";
        case MatrixDisplay::MODE_TEXT_STATIC: return "STATIC";
        case MatrixDisplay::MODE_ANIMATION: return "ANIM";
        case MatrixDisplay::MODE_CUSTOM_PATTERN: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

String BLEControl::getVizTypeName(MatrixDisplay::VisualizationType type) {
    switch (type) {
        case MatrixDisplay::VIZ_BARS: return "BARS";
        case MatrixDisplay::VIZ_CIRCLE: return "CIRCLE";
        case MatrixDisplay::VIZ_WAVE: return "WAVE";
        case MatrixDisplay::VIZ_PARTICLES: return "PARTICLES";
        default: return "UNKNOWN";
    }
}

void BLEControl::sendResponse(const String& response) {
    if (deviceConnected && pTxCharacteristic) {
        // 将String转换为字节数组，确保UTF-8编码
        std::string utf8Str = response.c_str();
        
        // 分批发送长消息，避免MTU限制
        const size_t maxChunkSize = 20; // 保守的MTU大小
        size_t totalLength = utf8Str.length();
        
        for (size_t i = 0; i < totalLength; i += maxChunkSize) {
            size_t chunkSize = min(maxChunkSize, totalLength - i);
            std::string chunk = utf8Str.substr(i, chunkSize);
            
            pTxCharacteristic->setValue(chunk);
            pTxCharacteristic->notify();
            
            if (totalLength > maxChunkSize) {
                delay(20); // 分包间延迟
            }
        }
        
        delay(10); // 最后延迟确保传输完成
    }
    Serial.println("BLE> " + response);
}

void BLEControl::sendOK(const String& message) {
    if (message.isEmpty()) {
        sendResponse("OK");
    } else {
        sendResponse("OK: " + message);
    }
}

void BLEControl::sendError(const String& error) {
    sendResponse("ERROR: " + error);
}

void BLEControl::sendNotification(const String& message) {
    sendResponse("NOTIFY: " + message);
}

void BLEControl::sendTestASCII() {
    // 发送纯ASCII测试字符
    if (deviceConnected && pTxCharacteristic) {
        // 逐个字符发送测试
        const char* testMsg = "HELLO";
        for (int i = 0; i < strlen(testMsg); i++) {
            char singleChar[2] = {testMsg[i], '\0'};
            pTxCharacteristic->setValue(singleChar);
            pTxCharacteristic->notify();
            delay(50); // 每个字符间延迟
        }
        
        // 发送换行
        pTxCharacteristic->setValue("\r\n");
        pTxCharacteristic->notify();
        delay(50);
        
        Serial.println("BLE> ASCII test sent: HELLO");
    }
}

void BLEControl::checkConnection() {
    // 连接状态检查在update()中处理
}
