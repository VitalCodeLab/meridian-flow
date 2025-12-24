#ifndef BLE_CONTROL_HPP
#define BLE_CONTROL_HPP

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "matrix_display.hpp"

class BLEControl {
private:
    NimBLEServer* pServer;
    NimBLECharacteristic* pTxCharacteristic;
    NimBLECharacteristic* pRxCharacteristic;
    NimBLEService* pService;
    MatrixDisplay* matrix;
    bool deviceConnected;
    bool oldDeviceConnected;
    uint8_t txValue;
    String deviceName;
    
    // BLE回调类
    class ServerCallbacks : public NimBLEServerCallbacks {
        BLEControl* parent;
    public:
        ServerCallbacks(BLEControl* p) : parent(p) {}
        void onConnect(NimBLEServer* pServer) override;
        void onDisconnect(NimBLEServer* pServer) override;
    };
    
    class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
        BLEControl* parent;
    public:
        CharacteristicCallbacks(BLEControl* p) : parent(p) {}
        void onWrite(NimBLECharacteristic* pCharacteristic) override;
        void onRead(NimBLECharacteristic* pCharacteristic) override;
    };
    
    ServerCallbacks* serverCallbacks;
    CharacteristicCallbacks* charCallbacks;
    
    // 命令处理
    void processCommand(const String& command);
    void showHelp();
    void showStatus();
    void parseModeCommand(const String& params);
    void parseTextCommand(const String& params, bool isStatic = false);
    void parseBrightnessCommand(const String& params);
    void parsePatternCommand(const String& params);
    void parseVizCommand(const String& params);
    void parseDemoCommand(const String& params);
    
    // 辅助函数
    String getModeName(MatrixDisplay::DisplayMode mode);
    String getVizTypeName(MatrixDisplay::VisualizationType type);
    void sendResponse(const String& response);
    void sendOK(const String& message = "");
    void sendError(const String& error);
    
public:
    BLEControl();
    ~BLEControl();
    
    // 初始化
    void begin(const String& name = "ESP32_Matrix");
    void update();
    void setMatrix(MatrixDisplay* display);
    
    // 连接状态
    bool isConnected() const { return deviceConnected; } 
    const char* getDeviceName() const { return deviceName.c_str(); }
    
    // 命令发送
    void sendNotification(const String& message);
    void sendTestASCII(); // ASCII测试函数
    
    // 自动重连
    void checkConnection();
};

#endif
