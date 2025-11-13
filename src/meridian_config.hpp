#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <vector>
#include <SPIFFS.h>

// 引用外部定义的类型
enum MeridianType;
struct MeridianInfo;
struct AcupointInfo;
struct ZiwuliuzhuTimeSlot;

/**
 * 经络配置管理类
 * 负责从JSON配置文件中读取经络和穴位信息
 */
class MeridianConfig {
public:
  // 初始化SPIFFS文件系统
  static bool begin() {
    if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS挂载失败！");
      return false;
    }
    Serial.println("SPIFFS挂载成功");
    return true;
  }
  
  // 从JSON文件加载经络配置
  static bool loadMeridians(const char* filename, std::vector<MeridianInfo>& meridians, std::vector<ZiwuliuzhuTimeSlot>& timeSlots) {
    // 打开文件
    File file = SPIFFS.open(filename, "r");
    if (!file) {
      Serial.printf("无法打开文件: %s\n", filename);
      return false;
    }
    
    // 分配JSON文档内存
    // 使用动态内存分配，因为文件可能很大
    const size_t capacity = 32768; // 32KB
    DynamicJsonDocument doc(capacity);
    
    // 解析JSON
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
      Serial.printf("解析JSON失败: %s\n", error.c_str());
      file.close();
      return false;
    }
    
    // 关闭文件
    file.close();
    
    // 处理经络数据
    JsonArray meridiansArray = doc["meridians"];
    for (JsonObject meridianObj : meridiansArray) {
      MeridianInfo meridian;
      
      // 基本信息
      meridian.type = static_cast<MeridianType>(meridianObj["id"].as<int>());
      meridian.name = strdup(meridianObj["name"].as<const char*>());
      meridian.chineseName = strdup(meridianObj["chineseName"].as<const char*>());
      
      // 颜色 - 从十六进制字符串转换为CRGB
      String colorStr = meridianObj["color"].as<String>();
      uint32_t colorHex;
      if (colorStr.startsWith("0x")) {
        colorHex = strtoul(colorStr.c_str() + 2, NULL, 16); // 跳过"0x"前缀
      } else {
        colorHex = strtoul(colorStr.c_str(), NULL, 16); // 直接解析
      }
      meridian.color = CRGB(
        (colorHex >> 16) & 0xFF,  // R
        (colorHex >> 8) & 0xFF,   // G
        colorHex & 0xFF           // B
      );
      
      // 子午流注信息
      JsonObject ziwuObj = meridianObj["ziwuliuzhu"];
      ZiwuliuzhuTimeSlot timeSlot;
      timeSlot.meridian = meridian.type;
      timeSlot.startHour = ziwuObj["startHour"].as<uint8_t>();
      timeSlot.startMinute = ziwuObj["startMinute"].as<uint8_t>();
      timeSlot.endHour = ziwuObj["endHour"].as<uint8_t>();
      timeSlot.endMinute = ziwuObj["endMinute"].as<uint8_t>();
      timeSlot.description = strdup(ziwuObj["description"].as<const char*>());
      
      // 添加到时间槽列表
      timeSlots.push_back(timeSlot);
      
      // 穴位信息
      JsonArray acupointsArray = meridianObj["acupoints"];
      std::vector<uint16_t> acupointIndices;
      
      for (JsonObject acupointObj : acupointsArray) {
        uint16_t localIndex = acupointObj["id"].as<uint16_t>();
        acupointIndices.push_back(localIndex);
        
        // 创建穴位信息对象
        AcupointInfo acupoint;
        acupoint.globalIndex = meridian.startIndex + localIndex; // 全局索引将在后续设置
        acupoint.localIndex = localIndex;
        acupoint.meridian = meridian.type;
        acupoint.name = strdup(acupointObj["name"].as<const char*>());
        acupoint.chineseName = strdup(acupointObj["chineseName"].as<const char*>());
        
        // 可选字段
        if (acupointObj.containsKey("pinyin")) {
          acupoint.pinyin = strdup(acupointObj["pinyin"].as<const char*>());
        } else {
          acupoint.pinyin = "";
        }
        
        if (acupointObj.containsKey("location")) {
          acupoint.location = strdup(acupointObj["location"].as<const char*>());
        } else {
          acupoint.location = "";
        }
        
        if (acupointObj.containsKey("functions")) {
          acupoint.functions = strdup(acupointObj["functions"].as<const char*>());
        } else {
          acupoint.functions = "";
        }
        
        if (acupointObj.containsKey("indications")) {
          acupoint.indications = strdup(acupointObj["indications"].as<const char*>());
        } else {
          acupoint.indications = "";
        }
        
        if (acupointObj.containsKey("importance")) {
          acupoint.importance = acupointObj["importance"].as<uint8_t>();
        } else {
          acupoint.importance = 3; // 默认重要性
        }
        
        // 添加到穴位列表
        // 注意：这里我们只是准备数据，实际添加会在调用方完成
      }
      
      // 设置穴位索引
      meridian.acupoints = acupointIndices;
      
      // 添加到经络列表
      meridians.push_back(meridian);
    }
    
    return true;
  }
  
  // 从多个JSON文件加载经络配置
  static bool loadAllMeridians(const std::vector<const char*>& filenames, 
                              std::vector<MeridianInfo>& meridians, 
                              std::vector<ZiwuliuzhuTimeSlot>& timeSlots) {
    bool success = true;
    
    for (const char* filename : filenames) {
      if (!loadMeridians(filename, meridians, timeSlots)) {
        Serial.printf("加载文件失败: %s\n", filename);
        success = false;
      }
    }
    
    return success;
  }
  
  // 列出SPIFFS中的所有文件
  static void listFiles() {
    File root = SPIFFS.open("/");
    if (!root) {
      Serial.println("无法打开根目录");
      return;
    }
    
    Serial.println("SPIFFS文件列表:");
    File file = root.openNextFile();
    while (file) {
      Serial.print("  ");
      Serial.print(file.name());
      Serial.print(" (");
      Serial.print(file.size());
      Serial.println(" bytes)");
      file = root.openNextFile();
    }
  }
};
