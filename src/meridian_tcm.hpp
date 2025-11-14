#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <time.h>

// 先定义经络名称枚举
enum MeridianType {
  LUNG,           // 手太阴肺经
  LARGE_INTESTINE,// 手阳明大肠经
  STOMACH,        // 足阳明胃经
  SPLEEN,         // 足太阴脊经
  HEART,          // 手少阴心经
  SMALL_INTESTINE,// 手太阳小肠经
  BLADDER,        // 足太阳膜胱经
  KIDNEY,         // 足少阴肾经
  PERICARDIUM,    // 手厚阴心包经
  TRIPLE_ENERGIZER, // 手少阳三焦经
  GALLBLADDER,    // 足少阳胆经
  LIVER           // 足厚阴肝经
};

// 定义经络信息结构体
struct MeridianInfo {
  MeridianType type;        // 经络类型
  const char* name;        // 经络名称
  const char* chineseName; // 经络中文名称
  CRGB color;              // 经络颜色
  uint16_t startIndex;     // 起始索引
  uint16_t length;         // 长度
  std::vector<uint16_t> acupoints; // 穴位索引列表
};

// 定义穴位信息结构体
struct AcupointInfo {
  uint16_t globalIndex;     // 全局索引
  uint16_t localIndex;      // 在经络内的索引
  MeridianType meridian;   // 所属经络
  const char* name;        // 穴位名称
  const char* chineseName; // 穴位中文名称
  const char* pinyin;      // 拼音
  const char* location;    // 位置描述
  const char* functions;   // 功效
  const char* indications; // 适应症
  uint8_t importance;       // 重要性级别 (1-5)
};

/**
 * 子午流注时轴结构体
 * 记录经络活跃的时间段
 */
struct ZiwuliuzhuTimeSlot {
  MeridianType meridian;  // 经络类型
  uint8_t startHour;      // 开始小时 (0-23)
  uint8_t startMinute;    // 开始分钟 (0-59)
  uint8_t endHour;        // 结束小时 (0-23)
  uint8_t endMinute;      // 结束分钟 (0-59)
  const char* description; // 时间段描述
};

// 前向声明
class MeridianConfig;

// 现在包含配置文件
#include "meridian_config.hpp"

/**
 * 中医经络模拟控制类
 * 用于控制LED灯带模拟中医12经络的循行
 */
class TCMMeridianSystem {
public:
  // 构造函数
  TCMMeridianSystem(uint16_t numLeds, uint8_t pin)
      : leds_(nullptr), numLeds_(numLeds), pin_(pin), ziwuliuzhuEnabled_(false), currentMeridian_(LUNG), ownsLeds_(true) {
    leds_ = new CRGB[numLeds_];
    switch (pin) {
      case 0: FastLED.addLeds<WS2812B, 0>(leds_, numLeds_); break;
      case 2: FastLED.addLeds<WS2812B, 2>(leds_, numLeds_); break;
      case 3: FastLED.addLeds<WS2812B, 3>(leds_, numLeds_); break;
      case 4: FastLED.addLeds<WS2812B, 4>(leds_, numLeds_); break;
      case 5: FastLED.addLeds<WS2812B, 5>(leds_, numLeds_); break;
      case 6: FastLED.addLeds<WS2812B, 6>(leds_, numLeds_); break;
      case 7: FastLED.addLeds<WS2812B, 7>(leds_, numLeds_); break;
      case 8: FastLED.addLeds<WS2812B, 8>(leds_, numLeds_); break;
      case 9: FastLED.addLeds<WS2812B, 9>(leds_, numLeds_); break;
      case 10: FastLED.addLeds<WS2812B, 10>(leds_, numLeds_); break;
      default: FastLED.addLeds<WS2812B, 2>(leds_, numLeds_); break; // 默认使用GPIO2
    }
  }

  TCMMeridianSystem(CRGB *externalLeds, uint16_t numLeds)
      : leds_(externalLeds), numLeds_(numLeds), pin_(0), ziwuliuzhuEnabled_(false), currentMeridian_(LUNG), ownsLeds_(false) {
  }
  
  // 从配置文件初始化
  bool initFromConfig() {
    // 初始化SPIFFS
    if (!MeridianConfig::begin()) {
      Serial.println("无法初始化SPIFFS");
      return false;
    }
    
    // 列出文件
    MeridianConfig::listFiles();
    
    // 加载所有经络配置文件
    std::vector<const char*> configFiles = {
      "/meridians.json",
      "/meridians_more.json",
      "/meridians_rest.json"
    };
    
    // 清空现有数据
    meridians_.clear();
    ziwuliuzhuTimeSlots_.clear();
    acupoints_.clear();
    
    // 加载配置
    if (!MeridianConfig::loadAllMeridians(configFiles, meridians_, ziwuliuzhuTimeSlots_)) {
      Serial.println("加载经络配置失败");
      return false;
    }
    
    // 计算经络起始位置
    calculateMeridianStartIndices();
    
    // 初始化穴位
    initAcupointsFromConfig();
    
    Serial.printf("成功加载 %d 条经络和 %d 个穴位\n", 
                 meridians_.size(), acupoints_.size());
    
    return true;
  }
  
  ~TCMMeridianSystem() {
    if (ownsLeds_ && leds_ != nullptr) {
      delete[] leds_;
    }
  }
  
  // 初始化
  void begin() {
    FastLED.clear();
    FastLED.show();
    
    // 初始化子午流注时间表
    initZiwuliuzhu();
  }
  
  // 设置全局亮度
  void setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
  }
  
  // 显示特定经络
  void showMeridian(MeridianType type) {
    FastLED.clear();
    
    for (const auto& meridian : meridians_) {
      if (meridian.type == type) {
        for (uint16_t i = 0; i < meridian.length; i++) {
          uint16_t idx = meridian.startIndex + i;
          if (idx < numLeds_) {
            leds_[idx] = meridian.color;
          }
        }
        break;
      }
    }
    
    FastLED.show();
  }
  
  // 显示所有经络
  void showAllMeridians() {
    FastLED.clear();
    
    for (const auto& meridian : meridians_) {
      for (uint16_t i = 0; i < meridian.length; i++) {
        uint16_t idx = meridian.startIndex + i;
        if (idx < numLeds_) {
          leds_[idx] = meridian.color;
        }
      }
    }
    
    FastLED.show();
  }
  
  // 显示特定穴位
  void showAcupoint(const char* name, CRGB color = CRGB::White) {
    for (const auto& acupoint : acupoints_) {
      if (strcmp(acupoint.name, name) == 0) {
        leds_[acupoint.globalIndex] = color;
        FastLED.show();
        return;
      }
    }
  }
  
  // 显示单个像素
  void showPixel(uint16_t index, CRGB color) {
    if (index < numLeds_) {
      leds_[index] = color;
      FastLED.show();
    }
  }
  
  // 闪烁特定穴位
  void blinkAcupoint(const char* name, uint8_t times = 3, uint16_t interval = 200, CRGB color = CRGB::White) {
    for (const auto& acupoint : acupoints_) {
      if (strcmp(acupoint.name, name) == 0) {
        CRGB originalColor = leds_[acupoint.globalIndex];
        
        for (uint8_t i = 0; i < times; i++) {
          leds_[acupoint.globalIndex] = color;
          FastLED.show();
          delay(interval);
          
          leds_[acupoint.globalIndex] = CRGB::Black;
          FastLED.show();
          delay(interval);
        }
        
        leds_[acupoint.globalIndex] = originalColor;
        FastLED.show();
        return;
      }
    }
  }
  
  // 模拟经络循行效果
  void flowMeridian(MeridianType type, uint8_t tailLength = 5, uint16_t interval = 30) {
    MeridianInfo* meridian = nullptr;
    
    // 查找指定经络
    for (auto& m : meridians_) {
      if (m.type == type) {
        meridian = &m;
        break;
      }
    }
    
    if (!meridian) return;
    
    // 清除当前显示
    FastLED.clear();
    
    // 循行效果
    for (uint16_t head = 0; head < meridian->length + tailLength; head++) {
      // 清除上一帧
      for (uint16_t i = 0; i < meridian->length; i++) {
        uint16_t idx = meridian->startIndex + i;
        if (idx < numLeds_) {
          leds_[idx] = CRGB::Black;
        }
      }
      
      // 绘制流动效果
      for (uint8_t t = 0; t < tailLength; t++) {
        int16_t pos = head - t;
        if (pos >= 0 && pos < meridian->length) {
          uint16_t idx = meridian->startIndex + pos;
          if (idx < numLeds_) {
            // 计算尾部衰减
            uint8_t brightness = 255 - ((255 * t) / tailLength);
            leds_[idx] = meridian->color;
            leds_[idx].nscale8(brightness);
          }
        }
      }
      
      // 高亮显示经过的穴位
      for (uint16_t apIdx : meridian->acupoints) {
        if (apIdx >= meridian->length) {
          continue;
        }
        if (head >= apIdx && head - apIdx < tailLength) {
          // 穴位在尾部范围内，设置为白色高亮
          uint16_t idx = meridian->startIndex + apIdx;
          if (idx < numLeds_) {
            leds_[idx] = CRGB::White;
          }
        }
      }
      
      FastLED.show();
      delay(interval);
    }
  }
  
  // 模拟全身经络循环
  void flowAllMeridians(uint16_t interval = 30) {
    for (const auto& meridian : meridians_) {
      flowMeridian(meridian.type, 5, interval);
      delay(500); // 经络间短暂停顿
    }
  }
  
  // 获取经络信息
  const std::vector<MeridianInfo>& getMeridians() const {
    return meridians_;
  }
  
  // 获取穴位信息
  const std::vector<AcupointInfo>& getAcupoints() const {
    return acupoints_;
  }
  
  // 子午流注相关方法
  
  // 启用/禁用子午流注自动切换
  void enableZiwuliuzhu(bool enable) {
    ziwuliuzhuEnabled_ = enable;
  }
  
  // 检查子午流注是否启用
  bool isZiwuliuzhuEnabled() const {
    return ziwuliuzhuEnabled_;
  }
  
  // 根据当前时间获取活跃经络
  MeridianType getCurrentActiveMeridian() {
    if (!ziwuliuzhuEnabled_) {
      return currentMeridian_;
    }
    
    // 获取当前时间
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    uint8_t currentHour = timeinfo->tm_hour;
    uint8_t currentMinute = timeinfo->tm_min;
    
    // 循环检查每个时间段
    for (const auto& slot : ziwuliuzhuTimeSlots_) {
      if (isTimeInSlot(currentHour, currentMinute, slot)) {
        return slot.meridian;
      }
    }
    
    // 默认返回肺经
    return LUNG;
  }
  
  // 根据当前时间自动切换并流动经络
  void flowCurrentTimeMeridian(uint8_t tailLength = 5, uint16_t interval = 30) {
    if (!ziwuliuzhuEnabled_) return;
    
    MeridianType activeMeridian = getCurrentActiveMeridian();
    flowMeridian(activeMeridian, tailLength, interval);
  }
  
  // 获取当前活跃经络的时间段描述
  const char* getCurrentTimeSlotDescription() {
    if (!ziwuliuzhuEnabled_) {
      return "子午流注未启用";
    }
    
    // 获取当前时间
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    
    uint8_t currentHour = timeinfo->tm_hour;
    uint8_t currentMinute = timeinfo->tm_min;
    
    // 循环检查每个时间段
    for (const auto& slot : ziwuliuzhuTimeSlots_) {
      if (isTimeInSlot(currentHour, currentMinute, slot)) {
        return slot.description;
      }
    }
    
    return "无匹配时间段";
  }

  // 旧的初始化方法（保留作为备用）
  void initMeridians();
  void initAcupoints();
  void initZiwuliuzhu(); // 初始化子午流注时间表
  
  // 添加特定穴位
  void addSpecificAcupoint(MeridianType meridian, uint16_t localIndex, 
                          const char* name, const char* chineseName, const char* pinyin,
                          const char* location, const char* functions, 
                          const char* indications, uint8_t importance);
  
private:
  CRGB* leds_;                     // LED数组
  uint16_t numLeds_;               // LED总数
  uint8_t pin_;                    // LED数据引脚
  std::vector<MeridianInfo> meridians_;  // 经络信息
  std::vector<AcupointInfo> acupoints_;  // 穴位信息
  // 子午流注相关变量
  bool ziwuliuzhuEnabled_;
  MeridianType currentMeridian_;
  std::vector<ZiwuliuzhuTimeSlot> ziwuliuzhuTimeSlots_;
  bool ownsLeds_;
  
  // 检查当前时间是否在指定时间段内
  bool isTimeInSlot(uint8_t hour, uint8_t minute, const ZiwuliuzhuTimeSlot& slot) {
    // 转换为分钟计数便于比较
    int currentMinutes = hour * 60 + minute;
    int startMinutes = slot.startHour * 60 + slot.startMinute;
    int endMinutes = slot.endHour * 60 + slot.endMinute;
    
    // 处理跨天情况
    if (endMinutes < startMinutes) {
      endMinutes += 24 * 60; // 加上一天的分钟数
      if (currentMinutes < startMinutes) {
        currentMinutes += 24 * 60; // 如果当前时间小于开始时间，也加上一天
      }
    }
    
    return currentMinutes >= startMinutes && currentMinutes < endMinutes;
  }
  
  // 计算经络起始位置
  void calculateMeridianStartIndices() {
    // 计算每条经络的长度
    uint16_t ledsPerMeridian = numLeds_ / meridians_.size();
    uint16_t remainder = numLeds_ % meridians_.size();
    uint16_t currentIndex = 0;
    
    // 设置每条经络的起始位置和长度
    for (auto& meridian : meridians_) {
      meridian.startIndex = currentIndex;
      meridian.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
      currentIndex += meridian.length;
    }
  }

  // 从配置初始化穴位
  void initAcupointsFromConfig() {
    // 清空现有穴位
    acupoints_.clear();

    // 遍历所有经络
    for (const auto& meridian : meridians_) {
      // 遍历经络中的所有穴位索引
      for (uint16_t localIdx : meridian.acupoints) {
        if (localIdx >= meridian.length) {
          continue;
        }
        // 创建穴位信息对象
        AcupointInfo acupoint;
        acupoint.globalIndex = meridian.startIndex + localIdx;
        acupoint.localIndex = localIdx;
        acupoint.meridian = meridian.type;

        // 设置穴位名称和其他信息
        // 这里需要从配置文件中查找对应的穴位
        // 注意：这里的实现是简化的，实际上我们应该在加载配置时就完成这些工作
        // 添加到穴位列表
        acupoints_.push_back(acupoint);
      }
    }
  }

};  // TCMMeridianSystem类结束
