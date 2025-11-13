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
  TCMMeridianSystem(uint16_t numLeds, uint8_t pin) : numLeds_(numLeds), pin_(pin), ziwuliuzhuEnabled_(false) {
    // 初始化FastLED
    leds_ = new CRGB[numLeds];
    // 根据传入的pin参数初始化
    switch(pin) {
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
    delete[] leds_;
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
  
  // 实现addSpecificAcupoint函数
void TCMMeridianSystem::addSpecificAcupoint(MeridianType meridianType, uint16_t localIndex, 
                        const char* name, const char* chineseName, const char* pinyin,
                        const char* location, const char* functions, 
                        const char* indications, uint8_t importance) {
  // 查找对应的经络
  MeridianInfo* meridian = nullptr;
  for (auto& m : meridians_) {
    if (m.type == meridianType) {
      meridian = &m;
      break;
    }
  }
  
  if (!meridian) return;
  
  // 创建穴位信息对象
  AcupointInfo acupoint;
  acupoint.globalIndex = meridian->startIndex + localIndex;
  acupoint.localIndex = localIndex;
  acupoint.meridian = meridianType;
  acupoint.name = name;
  acupoint.chineseName = chineseName;
  acupoint.pinyin = pinyin;
  acupoint.location = location;
  acupoint.functions = functions;
  acupoint.indications = indications;
  acupoint.importance = importance;
  
  // 添加到穴位列表
  acupoints_.push_back(acupoint);
}

// 实现initZiwuliuzhu函数
void TCMMeridianSystem::initZiwuliuzhu() {
  // 初始化子午流注时间表
  ziwuliuzhuTimeSlots_.clear();
  
  // 添加时间段
  ZiwuliuzhuTimeSlot slot;
  slot.meridian = LUNG;
  slot.startHour = 3;
  slot.startMinute = 0;
  slot.endHour = 5;
  slot.endMinute = 0;
  slot.description = "子时（23:00-01:00）";
  ziwuliuzhuTimeSlots_.push_back(slot);
  
  slot.meridian = LARGE_INTESTINE;
  slot.startHour = 5;
  slot.startMinute = 0;
  slot.endHour = 7;
  slot.endMinute = 0;
  slot.description = "丑时（01:00-03:00）";
  ziwuliuzhuTimeSlots_.push_back(slot);
  
  slot.meridian = STOMACH;
  slot.startHour = 7;
  slot.startMinute = 0;
  slot.endHour = 9;
  slot.endMinute = 0;
  slot.description = "寅时（03:00-05:00）";
  ziwuliuzhuTimeSlots_.push_back(slot);
  
  slot.meridian = SPLEEN;
  slot.startHour = 9;
  slot.startMinute = 0;
  slot.endHour = 11;
  slot.endMinute = 0;
  slot.description = "卫时（05:00-07:00）";
  ziwuliuzhuTimeSlots_.push_back(slot);
  
  // 设置当前经络为肺经（默认）
  currentMeridian_ = LUNG;
}
  
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

// 实现initMeridians函数
void TCMMeridianSystem::initMeridians() {
    // 计算每条经络的长度
    // 假设我们将3米灯带(432颗灯珠)均分给12条经络，每条约36颗
    uint16_t ledsPerMeridian = numLeds_ / 12;
    uint16_t remainder = numLeds_ % 12;
    uint16_t currentIndex = 0;
    
    // 手太阴肺经 - 红色
    MeridianInfo lung;
    lung.type = LUNG;
    lung.name = "Lung Meridian";
    lung.chineseName = "手太阴肺经";
    lung.color = CRGB::Red;
    lung.startIndex = currentIndex;
    lung.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    lung.acupoints = {3, 9, 15, 21, 27}; // 示例穴位位置
    meridians_.push_back(lung);
    currentIndex += lung.length;
    
    // 手阳明大肠经 - 橙色
    MeridianInfo largeIntestine;
    largeIntestine.type = LARGE_INTESTINE;
    largeIntestine.name = "Large Intestine Meridian";
    largeIntestine.chineseName = "手阳明大肠经";
    largeIntestine.color = CRGB::Orange;
    largeIntestine.startIndex = currentIndex;
    largeIntestine.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    largeIntestine.acupoints = {4, 11, 18, 25, 32}; // 示例穴位位置
    meridians_.push_back(largeIntestine);
    currentIndex += largeIntestine.length;
    
    // 足阳明胃经 - 黄色
    MeridianInfo stomach;
    stomach.type = STOMACH;
    stomach.name = "Stomach Meridian";
    stomach.chineseName = "足阳明胃经";
    stomach.color = CRGB::Yellow;
    stomach.startIndex = currentIndex;
    stomach.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    stomach.acupoints = {5, 12, 19, 26, 33}; // 示例穴位位置
    meridians_.push_back(stomach);
    currentIndex += stomach.length;
    
    // 足太阴脾经 - 黄绿色
    MeridianInfo spleen;
    spleen.type = SPLEEN;
    spleen.name = "Spleen Meridian";
    spleen.chineseName = "足太阴脾经";
    spleen.color = CRGB(128, 255, 0); // 黄绿色
    spleen.startIndex = currentIndex;
    spleen.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    spleen.acupoints = {6, 13, 20, 27, 34}; // 示例穴位位置
    meridians_.push_back(spleen);
    currentIndex += spleen.length;
    
    // 手少阴心经 - 绿色
    MeridianInfo heart;
    heart.type = HEART;
    heart.name = "Heart Meridian";
    heart.chineseName = "手少阴心经";
    heart.color = CRGB::Green;
    heart.startIndex = currentIndex;
    heart.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    heart.acupoints = {7, 14, 21, 28}; // 示例穴位位置
    meridians_.push_back(heart);
    currentIndex += heart.length;
    
    // 手太阳小肠经 - 青色
    MeridianInfo smallIntestine;
    smallIntestine.type = SMALL_INTESTINE;
    smallIntestine.name = "Small Intestine Meridian";
    smallIntestine.chineseName = "手太阳小肠经";
    smallIntestine.color = CRGB(0, 255, 128); // 青色
    smallIntestine.startIndex = currentIndex;
    smallIntestine.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    smallIntestine.acupoints = {8, 15, 22, 29}; // 示例穴位位置
    meridians_.push_back(smallIntestine);
    currentIndex += smallIntestine.length;
    
    // 足太阳膀胱经 - 蓝色
    MeridianInfo bladder;
    bladder.type = BLADDER;
    bladder.name = "Bladder Meridian";
    bladder.chineseName = "足太阳膀胱经";
    bladder.color = CRGB::Blue;
    bladder.startIndex = currentIndex;
    bladder.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    bladder.acupoints = {9, 16, 23, 30}; // 示例穴位位置
    meridians_.push_back(bladder);
    currentIndex += bladder.length;
    
    // 足少阴肾经 - 靛蓝色
    MeridianInfo kidney;
    kidney.type = KIDNEY;
    kidney.name = "Kidney Meridian";
    kidney.chineseName = "足少阴肾经";
    kidney.color = CRGB(75, 0, 130); // 靛蓝色
    kidney.startIndex = currentIndex;
    kidney.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    kidney.acupoints = {10, 17, 24, 31}; // 示例穴位位置
    meridians_.push_back(kidney);
    currentIndex += kidney.length;
    
    // 手厥阴心包经 - 紫色
    MeridianInfo pericardium;
    pericardium.type = PERICARDIUM;
    pericardium.name = "Pericardium Meridian";
    pericardium.chineseName = "手厥阴心包经";
    pericardium.color = CRGB::Purple;
    pericardium.startIndex = currentIndex;
    pericardium.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    pericardium.acupoints = {2, 8, 14, 20}; // 示例穴位位置
    meridians_.push_back(pericardium);
    currentIndex += pericardium.length;
    
    // 手少阳三焦经 - 品红色
    MeridianInfo tripleEnergizer;
    tripleEnergizer.type = TRIPLE_ENERGIZER;
    tripleEnergizer.name = "Triple Energizer Meridian";
    tripleEnergizer.chineseName = "手少阳三焦经";
    tripleEnergizer.color = CRGB::Magenta;
    tripleEnergizer.startIndex = currentIndex;
    tripleEnergizer.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    tripleEnergizer.acupoints = {3, 9, 15, 21}; // 示例穴位位置
    meridians_.push_back(tripleEnergizer);
    currentIndex += tripleEnergizer.length;
    
    // 足少阳胆经 - 粉色
    MeridianInfo gallbladder;
    gallbladder.type = GALLBLADDER;
    gallbladder.name = "Gallbladder Meridian";
    gallbladder.chineseName = "足少阳胆经";
    gallbladder.color = CRGB::Pink;
    gallbladder.startIndex = currentIndex;
    gallbladder.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    gallbladder.acupoints = {4, 10, 16, 22}; // 示例穴位位置
    meridians_.push_back(gallbladder);
    currentIndex += gallbladder.length;
    
    // 足厥阴肝经 - 暗红色
    MeridianInfo liver;
    liver.type = LIVER;
    liver.name = "Liver Meridian";
    liver.chineseName = "足厥阴肝经";
    liver.color = CRGB(139, 0, 0); // 暗红色
    liver.startIndex = currentIndex;
    liver.length = ledsPerMeridian + (remainder-- > 0 ? 1 : 0);
    liver.acupoints = {5, 11, 17, 23}; // 示例穴位位置
    meridians_.push_back(liver);
  }
  
  // 初始化子午流注时间表
  void initZiwuliuzhu() {
    // 清空时间表
    ziwuliuzhuTimeSlots_.clear();
    
    // 子时到23时-1时：手太阴肺经
    ZiwuliuzhuTimeSlot slot1;
    slot1.meridian = LUNG;
    slot1.startHour = 23;
    slot1.startMinute = 0;
    slot1.endHour = 1;
    slot1.endMinute = 0;
    slot1.description = "子时（23:00-01:00）：手太阴肺经当令";
    ziwuliuzhuTimeSlots_.push_back(slot1);
    
    // 丑时到01时-3时：手阳明大肠经
    ZiwuliuzhuTimeSlot slot2;
    slot2.meridian = LARGE_INTESTINE;
    slot2.startHour = 1;
    slot2.startMinute = 0;
    slot2.endHour = 3;
    slot2.endMinute = 0;
    slot2.description = "丑时（01:00-03:00）：手阳明大肠经当令";
    ziwuliuzhuTimeSlots_.push_back(slot2);
    
    // 寅时到03时-5时：足阳明胃经
    ZiwuliuzhuTimeSlot slot3;
    slot3.meridian = STOMACH;
    slot3.startHour = 3;
    slot3.startMinute = 0;
    slot3.endHour = 5;
    slot3.endMinute = 0;
    slot3.description = "寅时（03:00-05:00）：足阳明胃经当令";
    ziwuliuzhuTimeSlots_.push_back(slot3);
    
    // 卯时到05时-7时：足太阴脑经
    ZiwuliuzhuTimeSlot slot4;
    slot4.meridian = SPLEEN;
    slot4.startHour = 5;
    slot4.startMinute = 0;
    slot4.endHour = 7;
    slot4.endMinute = 0;
    slot4.description = "卯时（05:00-07:00）：足太阴脾经当令";
    ziwuliuzhuTimeSlots_.push_back(slot4);
    
    // 巳时到07时-9时：手少阴心经
    ZiwuliuzhuTimeSlot slot5;
    slot5.meridian = HEART;
    slot5.startHour = 7;
    slot5.startMinute = 0;
    slot5.endHour = 9;
    slot5.endMinute = 0;
    slot5.description = "巳时（07:00-09:00）：手少阴心经当令";
    ziwuliuzhuTimeSlots_.push_back(slot5);
    
    // 午时到09时-11时：手太阳小肠经
    ZiwuliuzhuTimeSlot slot6;
    slot6.meridian = SMALL_INTESTINE;
    slot6.startHour = 9;
    slot6.startMinute = 0;
    slot6.endHour = 11;
    slot6.endMinute = 0;
    slot6.description = "午时（09:00-11:00）：手太阳小肠经当令";
    ziwuliuzhuTimeSlots_.push_back(slot6);
    
    // 未时到11时-13时：足太阳膜光经
    ZiwuliuzhuTimeSlot slot7;
    slot7.meridian = BLADDER;
    slot7.startHour = 11;
    slot7.startMinute = 0;
    slot7.endHour = 13;
    slot7.endMinute = 0;
    slot7.description = "未时（11:00-13:00）：足太阳膜光经当令";
    ziwuliuzhuTimeSlots_.push_back(slot7);
    
    // 申时到13时-15时：足少阴肾经
    ZiwuliuzhuTimeSlot slot8;
    slot8.meridian = KIDNEY;
    slot8.startHour = 13;
    slot8.startMinute = 0;
    slot8.endHour = 15;
    slot8.endMinute = 0;
    slot8.description = "申时（13:00-15:00）：足少阴肾经当令";
    ziwuliuzhuTimeSlots_.push_back(slot8);
    
    // 酉时到15时-17时：手厥阴心包经
    ZiwuliuzhuTimeSlot slot9;
    slot9.meridian = PERICARDIUM;
    slot9.startHour = 15;
    slot9.startMinute = 0;
    slot9.endHour = 17;
    slot9.endMinute = 0;
    slot9.description = "酉时（15:00-17:00）：手厥阴心包经当令";
    ziwuliuzhuTimeSlots_.push_back(slot9);
    
    // 戌时到17时-19时：手少阳三焦经
    ZiwuliuzhuTimeSlot slot10;
    slot10.meridian = TRIPLE_ENERGIZER;
    slot10.startHour = 17;
    slot10.startMinute = 0;
    slot10.endHour = 19;
    slot10.endMinute = 0;
    slot10.description = "戌时（17:00-19:00）：手少阳三焦经当令";
    ziwuliuzhuTimeSlots_.push_back(slot10);
    
    // 亥时到19时-21时：足少阳胆经
    ZiwuliuzhuTimeSlot slot11;
    slot11.meridian = GALLBLADDER;
    slot11.startHour = 19;
    slot11.startMinute = 0;
    slot11.endHour = 21;
    slot11.endMinute = 0;
    slot11.description = "亥时（19:00-21:00）：足少阳胆经当令";
    ziwuliuzhuTimeSlots_.push_back(slot11);
    
    // 亥时到21时-23时：足厥阴肝经
    ZiwuliuzhuTimeSlot slot12;
    slot12.meridian = LIVER;
    slot12.startHour = 21;
    slot12.startMinute = 0;
    slot12.endHour = 23;
    slot12.endMinute = 0;
    slot12.description = "亥时（21:00-23:00）：足厥阴肝经当令";
    ziwuliuzhuTimeSlots_.push_back(slot12);
    
    // 设置当前经络为肺经（默认）
    currentMeridian_ = LUNG;
  }
  
// 实现initAcupoints函数
void TCMMeridianSystem::initAcupoints() {
    // 为每个经络的穴位创建全局索引
    for (const auto& meridian : meridians_) {
      for (uint16_t localIdx : meridian.acupoints) {
        AcupointInfo acupoint;
        acupoint.globalIndex = meridian.startIndex + localIdx;
        acupoint.localIndex = localIdx;
        acupoint.meridian = meridian.type;
        
        // 这里应该设置实际的穴位名称，这里只是示例
        char nameBuffer[50];
        sprintf(nameBuffer, "%s_point_%d", meridian.name, localIdx);
        acupoint.name = strdup(nameBuffer);
        
        char chineseNameBuffer[50];
        sprintf(chineseNameBuffer, "%s穴位%d", meridian.chineseName, localIdx);
        acupoint.chineseName = strdup(chineseNameBuffer);
        
        acupoints_.push_back(acupoint);
      }
    }
    
    // 添加一些特定的重要穴位
    // 这里只是示例，实际应该根据中医经络穴位图来定位
    
    // 肺经穴位（手太阴肺经）
    addSpecificAcupoint(LUNG, 1, "Zhongfu", "中府", "zhong fu", 
                      "胸部，锁骨下，距前正中线6寸，在第1胸肌间隙内。", 
                      "守卫肺气，定嗓止喉，导病下行。", 
                      "哮嗽，咳嗽，气喜上逼，胸满，肺病嗽血。", 4);
    
    addSpecificAcupoint(LUNG, 2, "Yunmen", "云门", "yun men", 
                      "胸部，锁骨下，距前正中线6寸，在第2胸肌间隙内。", 
                      "守卫肺气，定嗓止喉，导病下行。", 
                      "哮嗽，咳嗽，气喜上逼，胸满，肺病嗽血。", 3);
    
    addSpecificAcupoint(LUNG, 3, "Tianfu", "天府", "tian fu", 
                      "上臂内侧，肩臂关节下3寸，肩二头肌与股二头肌之间。", 
                      "清泛肺热，定嗓止哮，安神定志。", 
                      "哮嗽，咳嗽，肺病，胸满，喉痛，失音，心惊悶。", 4);
    
    addSpecificAcupoint(LUNG, 5, "Chize", "尺泽", "chi ze", 
                      "肢窝横纹中，股二头肌腰肚的桥窝处。", 
                      "泛肺清热，定嗓止哮，导病下行。", 
                      "哮嗽，咳嗽，胸满，热病不汗，肛门痛痛。", 5);
    
    addSpecificAcupoint(LUNG, 7, "Lieque", "列缺", "lie que", 
                      "腕部桥窝上，桌骨茅突上方，桌骨茅突与桥窝之间，当桌骨桥窝上方1.5寸。", 
                      "守卫肺气，定嗓止喉，导病下行。", 
                      "头痛，颈项强直，哮嗽，咳嗽，发热恶寒，手腕痛痛。", 5);
    
    addSpecificAcupoint(LUNG, 9, "Taiyuan", "太渊", "tai yuan", 
                      "腕部，桌骨茅突与腔横纹之间，桥动脉所在处。", 
                      "守卫肺气，定嗓止喉，导病下行。", 
                      "哮嗽，咳嗽，胸满，气喜上逼，喉痛，失音。", 5);
    
    addSpecificAcupoint(LUNG, 10, "Yuji", "鱼际", "yu ji", 
                      "手掌部，第1掌骨中点的桥窝处。", 
                      "清泛肺热，定嗓止哮，导病下行。", 
                      "哮嗽，咳嗽，喉痛，失音，喉病。", 4);
    
    addSpecificAcupoint(LUNG, 11, "Shaoshang", "少商", "shao shang", 
                      "手拇指桥侧，距指甲角0.1寸。", 
                      "清泛肺热，开窗空窝，苏醒开窗。", 
                      "哮嗽，咳嗽，喉痛，失音，水肿，中暗。", 3);
    
    // 大肠经穴位（手阳明大肠经）
    addSpecificAcupoint(LARGE_INTESTINE, 1, "Shangyang", "商阳", "shang yang", 
                      "食指桥侧，距指甲角0.1寸。", 
                      "清泛肺热，开窗空窝，苏醒开窗。", 
                      "发热恶寒，喉痛，鼻塞，鼻血，齿痛，中暗。", 3);
    
    addSpecificAcupoint(LARGE_INTESTINE, 4, "Hegu", "合谷", "he gu", 
                      "第1掌骨与第2掌骨间，约当第1掌骨中点处。", 
                      "疏风清热，止痛开窗，调和气血。", 
                      "头痛，牙痛，眼痛，鼻出血，喉痛，面肿，发热恶寒。", 5);
    
    addSpecificAcupoint(LARGE_INTESTINE, 10, "Shousanli", "手三里", "shou san li", 
                      "前臂背侧，腕尺关节下2寸，桌骨桌骨茅突与股骨外上蹄之间。", 
                      "疏风清热，活素利节，止痛开窗。", 
                      "手臂痛痛，腕痛，肩周痛，齿痛，面痛。", 4);
    
    addSpecificAcupoint(LARGE_INTESTINE, 11, "Quchi", "曲池", "qu chi", 
                      "肢窝横纹外侧端，股骨外上蹄与桌骨之间。", 
                      "清热解表，消肿止痛，疗病开窗。", 
                      "发热恶寒，病痛，面肿，齿痛，肩臂痛痛，手臂痛痛。", 5);
    
    addSpecificAcupoint(LARGE_INTESTINE, 15, "Jianyu", "肩骨", "jian yu", 
                      "肩关节部，肩峰与肩峦之间凹陷处，当三角肌前缘与肩峦之间。", 
                      "疏风清热，活素利节，止痛开窗。", 
                      "肩周痛，半身不遂，上肢痣痛，手臂不能举。", 4);
    
    addSpecificAcupoint(LARGE_INTESTINE, 20, "Yingxiang", "迎香", "ying xiang", 
                      "鼻唇沟外，当鼻翼外缘中点处。", 
                      "疏风清热，开窗利鼻，止痛开窗。", 
                      "鼻塞，鼻出血，鼻痛，面肿，面痛。", 4);
    
    // 胃经穴位（足阳明胃经）
    addSpecificAcupoint(STOMACH, 25, "Tianshu", "天枢", "tian shu", 
                      "腹部，腹中线旁开约2寸，当脂下缘。", 
                      "理气健脑，调理肠胃，消食导滑。", 
                      "腹痛，腹泡，便秘，泰泰，痛经，病痛。", 4);
    
    addSpecificAcupoint(STOMACH, 36, "Zusanli", "足三里", "zu san li", 
                      "小腿外侧，膝关节下3寸，距胋骨外缘一横指。", 
                      "和胃健脑，补中益气，强身健体。", 
                      "胃痛，胃肠不和，泰泰，便秘，消化不良，全身乏力。", 5);
    
    addSpecificAcupoint(STOMACH, 40, "Fenglong", "丰隆", "feng long", 
                      "小腿外侧，距胋骨前缘一横指，当胋骨下8寸。", 
                      "化疸消病，健脑和胃，化疸化湿。", 
                      "哮嗽，咳嗽，胸满，病痛，消化不良。", 4);
    
    addSpecificAcupoint(STOMACH, 44, "Neiting", "内庭", "nei ting", 
                      "足背，第2、第3足趾间，趾蹄连合部前方凹陷处。", 
                      "清胃泛热，开窗利咽，消食导滑。", 
                      "发热恶寒，喉痛，齿痛，面肿，消化不良。", 4);
    
    // 脾经穴位（足太阴脾经）
    addSpecificAcupoint(SPLEEN, 3, "Taibai", "太白", "tai bai", 
                      "足内侧缘，第1趾蹄关节后下方赤白肉际处。", 
                      "健脑和胃，化湿消肿，补中益气。", 
                      "腹痛，腹泡，泰泰，便秘，消化不良。", 4);
    
    addSpecificAcupoint(SPLEEN, 6, "Sanyinjiao", "三阴交", "san yin jiao", 
                      "小腿内侧，内蹄骨后缘上，距内蹄骨顶点3寸。", 
                      "健脑益气，补血活血，补肾固精。", 
                      "泰泰，便秘，腹痛，消化不良，阴部痛痛，月经不调，不孕。", 5);
    
    addSpecificAcupoint(SPLEEN, 9, "Yinlingquan", "阴陵泉", "yin ling quan", 
                      "小腿内侧，股骨内侧蹄下缘，股骨内侧蹄与小腿股骨之间。", 
                      "健脑利湿，利尿消肿，补中益气。", 
                      "小便不利，泰泰，便秘，腹泡，腹痛，消化不良。", 4);
    
    addSpecificAcupoint(SPLEEN, 10, "Xuehai", "血海", "xue hai", 
                      "大腿内侧，股直肌上，距髁骨上缘2寸。", 
                      "活血化湿，调经养血，清热凉血。", 
                      "月经不调，经间腹痛，病痛，阴部痛痛，不孕。", 4);
    
    // 心经穴位（手少阴心经）
    addSpecificAcupoint(HEART, 3, "Shaohai", "少海", "shao hai", 
                      "肢窝横纹内侧端，屈肢时形成的凹陷中。", 
                      "清心泛热，宁神定志，活素利节。", 
                      "心惊悶，失眠多梦，心情不宁，肢窝痛痛。", 4);
    
    addSpecificAcupoint(HEART, 5, "Tongli", "通里", "tong li", 
                      "腕部掉骨沟内，腕横纹上方1.5寸。", 
                      "清心泛热，宁神定志，开窗利咽。", 
                      "心惊悶，失眠多梦，心情不宁，喉痛，失音。", 4);
    
    addSpecificAcupoint(HEART, 7, "Shenmen", "神门", "shen men", 
                      "腕部，腕横纹尖端，小指侧腔屏骨桥窝处。", 
                      "清心泛热，宁神定志，安神益智。", 
                      "心惊悶，失眠多梦，心情不宁，心惨，健忘。", 5);
    
    addSpecificAcupoint(HEART, 9, "Shaochong", "少冲", "shao chong", 
                      "小指桥侧，距指甲角0.1寸。", 
                      "清心泛热，开窗空窝，苏醒开窗。", 
                      "心惊悶，失眠多梦，心情不宁，中暗。", 3);
    
    // 小肠经穴位（手太阳小肠经）
    addSpecificAcupoint(SMALL_INTESTINE, 3, "Houxi", "后溪", "hou xi", 
                      "手外侧，小指本节后，当第5掌骨小指侧后方的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "头痛，颈项强直，耳鸣，耳肖，小指痛痛。", 4);
    
    addSpecificAcupoint(SMALL_INTESTINE, 6, "Yanglao", "养老", "yang lao", 
                      "腕部尖端，小指侧，当尖骨茅突与腰肚之间的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "眼痛，眼花，视力下降，腕痛，手臂痛痛。", 4);
    
    addSpecificAcupoint(SMALL_INTESTINE, 11, "Tianzong", "天宗", "tian zong", 
                      "肝肩胸部，当肝甲下端，肝甲下端与肝骨之间。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "肩周痛，肩胸痛，肩关节活动受限。", 4);
    
    addSpecificAcupoint(SMALL_INTESTINE, 19, "Tinggong", "听宫", "ting gong", 
                      "耳前，当下颜关节前，张口时形成的凹陷中。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "耳鸣，耳肖，听力下降，耳痛，面神经痫痛。", 4);
    
    // 膀胱经穴位（足太阳膀胱经）
    addSpecificAcupoint(BLADDER, 11, "Dazhu", "大朱", "da zhu", 
                      "背部，第1胸椎棱突下，旁开春有1.5寸。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "头痛，颈项强直，发热恶寒，肩周痛，肩胸痛。", 4);
    
    addSpecificAcupoint(BLADDER, 23, "Shenshu", "肾俯", "shen shu", 
                      "腰部，第2腰椎棱突下，旁开春有1.5寸。", 
                      "温补肾阳，固精益气，强腰健肾。", 
                      "腰痛，腰酸，腰膝酸痛，小便不利，遂尿，遂尿多。", 5);
    
    addSpecificAcupoint(BLADDER, 25, "Dachangshu", "大肠俯", "da chang shu", 
                      "腰部，第4腰椎棱突下，旁开春有1.5寸。", 
                      "理气健脑，调理肠胃，消食导滑。", 
                      "腹痛，腹泡，便秘，泰泰，痛经，病痛。", 4);
    
    addSpecificAcupoint(BLADDER, 40, "Weizhong", "委中", "wei zhong", 
                      "膝窝横纹中点。", 
                      "清热凉血，强腰健肾，活素利节。", 
                      "腰痛，腰酸，腰膝酸痛，下肢痣痛，病痛。", 5);
    
    addSpecificAcupoint(BLADDER, 57, "Chengshan", "承山", "cheng shan", 
                      "小腿后侧，当腿肚肠下端与趾蹄肠的连接处。", 
                      "清热凉血，强腰健肾，活素利节。", 
                      "腰痛，腰酸，腰膝酸痛，小腿痣痛，病痛。", 4);
    
    addSpecificAcupoint(BLADDER, 60, "Kunlun", "昆仑", "kun lun", 
                      "足外蹄后方，当外蹄骨与趾蹄肠之间的凹陷处。", 
                      "清热凉血，强腰健肾，活素利节。", 
                      "头痛，颈项强直，腰痛，腰酸，足趾痛痛。", 4);
    
    // 肾经穴位（足少阴肾经）
    addSpecificAcupoint(KIDNEY, 1, "Yongquan", "涌泉", "yong quan", 
                      "足底，当足前部凹陷处，约在足前1/3处。", 
                      "温补肾阳，固精益气，强腰健肾。", 
                      "头痛，头晕，失眠多梦，心惊悶，中暗。", 5);
    
    addSpecificAcupoint(KIDNEY, 3, "Taixi", "太溪", "tai xi", 
                      "足内蹄后方，当内蹄骨与趾蹄肠之间的凹陷处。", 
                      "温补肾阳，固精益气，强腰健肾。", 
                      "腰痛，腰酸，腰膝酸痛，小便不利，遂尿，遂尿多。", 5);
    
    addSpecificAcupoint(KIDNEY, 6, "Zhaohai", "照海", "zhao hai", 
                      "足内蹄下方，当内蹄骨下方的凹陷处。", 
                      "温补肾阳，固精益气，宁神定志。", 
                      "失眠多梦，心惊悶，心情不宁，遂尿，遂尿多。", 4);
    
    addSpecificAcupoint(KIDNEY, 7, "Fuliu", "复溜", "fu liu", 
                      "小腿内侧，当内蹄骨上方2寸。", 
                      "温补肾阳，固精益气，强腰健肾。", 
                      "腰痛，腰酸，腰膝酸痛，小便不利，遂尿，遂尿多。", 4);
    
    // 心包经穴位（手厥阴心包经）
    addSpecificAcupoint(PERICARDIUM, 3, "Quze", "曲泽", "qu ze", 
                      "肢窝横纹中，股二头肌腰肚的桥窝处。", 
                      "清心泛热，宁神定志，导病下行。", 
                      "心惊悶，失眠多梦，心情不宁，心惨，肢窝痛痛。", 4);
    
    addSpecificAcupoint(PERICARDIUM, 6, "Neiguan", "内关", "nei guan", 
                      "前臂掌侧，腕尺关节上2寸，当掌长肌与桌骨之间。", 
                      "宁心安神，和胃利气，消食导滑。", 
                      "心惊悶，失眠多梦，心情不宁，胃痛，恶心呕吐，消化不良。", 5);
    
    addSpecificAcupoint(PERICARDIUM, 7, "Daling", "大陵", "da ling", 
                      "腕部，腕横纹中点，当掌长肌腔屏骨之间。", 
                      "清心泛热，宁神定志，导病下行。", 
                      "心惊悶，失眠多梦，心情不宁，心惨，手掌热。", 4);
    
    addSpecificAcupoint(PERICARDIUM, 8, "Laogong", "劳宫", "lao gong", 
                      "手掌中央，当第3掌骨与第4掌骨之间，屏指尺寸处。", 
                      "清心泛热，宁神定志，导病下行。", 
                      "心惊悶，失眠多梦，心情不宁，心惨，手掌热。", 4);
    
    // 三焦经穴位（手少阳三焦经）
    addSpecificAcupoint(TRIPLE_ENERGIZER, 3, "Zhongzhu", "中渝", "zhong zhu", 
                      "手背部，第4掌骨与第5掌骨之间，当腔横纹后方。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "耳鸣，耳肖，听力下降，耳痛，手背痛痛。", 4);
    
    addSpecificAcupoint(TRIPLE_ENERGIZER, 5, "Waiguan", "外关", "wai guan", 
                      "前臂背侧，腕尺关节上2寸，当桌骨与尺骨之间。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "头痛，颈项强直，耳鸣，耳肖，手臂痛痛。", 5);
    
    addSpecificAcupoint(TRIPLE_ENERGIZER, 17, "Yifeng", "翼风", "yi feng", 
                      "耳后，当乳突与下颈之间的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "耳鸣，耳肖，听力下降，耳痛，面神经痫痛。", 4);
    
    // 胆经穴位（足少阳胆经）
    addSpecificAcupoint(GALLBLADDER, 20, "Fengchi", "风池", "feng chi", 
                      "颈部，当染发际与胸锥肌上端之间的凹陷处。", 
                      "疏风清热，除热开窗，活素利节。", 
                      "头痛，头晕，颈项强直，眼痛，眼花，耳鸣。", 5);
    
    addSpecificAcupoint(GALLBLADDER, 30, "Huantiao", "环跳", "huan tiao", 
                      "臀部，当股骨大转子与臀缝之间的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "腰痛，腰酸，腰膝酸痛，下肢痣痛，臀部痛痛。", 4);
    
    addSpecificAcupoint(GALLBLADDER, 34, "Yanglingquan", "阳陵泉", "yang ling quan", 
                      "小腿外侧，当股骨外侧蹄下方的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "腰痛，腰酸，腰膝酸痛，下肢痣痛，膝关节痛痛。", 5);
    
    addSpecificAcupoint(GALLBLADDER, 40, "Qiuxu", "丘墅", "qiu xu", 
                      "足外蹄前方，当足外蹄骨前下方的凹陷处。", 
                      "疏经活素，除热开窗，活素利节。", 
                      "头痛，头晕，颈项强直，足蹄痛痛。", 4);
    
    // 肝经穴位
    addSpecificAcupoint(LIVER, 1, "Dadun", "大敦", "da dun", 
                      "足大指外侧，距指甲角0.1寸。", 
                      "疏肝理气，清肝泛热，开窗空窝。", 
                      "小便不利，遂尿，遂尿多，阴部痛痛，阴囊肿痛。", 3);
    
    addSpecificAcupoint(LIVER, 2, "Xingjian", "行间", "xing jian", 
                      "足大指与第2足趾之间，趾蹄连合部前方。", 
                      "清肝泛热，疏肝理气，开窗空窝。", 
                      "头痛，头晕，眼痛，眼花，阴部痛痛，阴囊肿痛。", 4);
    
    addSpecificAcupoint(LIVER, 3, "Taichong", "太冲", "tai chong", 
                      "足背，第1趾骨与第2趾骨之间，趾蹄连合部后方2寸。", 
                      "疏肝理气，平肝息风，清肝泛热。", 
                      "头痛，头晕，眼痛，眼花，失眠多梦，心惊悶，心情不宁。", 5);
    
    addSpecificAcupoint(LIVER, 8, "Ququan", "曲泉", "qu quan", 
                      "膝关节内侧，当膝关节内侧横纹上端。", 
                      "清肝泛热，清热凉血，强腰健肾。", 
                      "小便不利，遂尿，遂尿多，阴部痛痛，阴囊肿痛。", 4);
  }
  
  // 添加特定穴位
  void addSpecificAcupoint(MeridianType type, uint16_t localIndex, const char* name, const char* chineseName,
                          const char* pinyin = "", const char* location = "", 
                          const char* functions = "", const char* indications = "", uint8_t importance = 3) {
    for (const auto& meridian : meridians_) {
      if (meridian.type == type) {
        // 检查是否已存在
        for (auto& acupoint : acupoints_) {
          if (acupoint.meridian == type && acupoint.localIndex == localIndex) {
            // 更新现有穴位信息
            acupoint.name = name;
            acupoint.chineseName = chineseName;
            acupoint.pinyin = pinyin;
            acupoint.location = location;
            acupoint.functions = functions;
            acupoint.indications = indications;
            acupoint.importance = importance;
            return;
          }
        }
        
        // 添加新穴位
        AcupointInfo acupoint;
        acupoint.globalIndex = meridian.startIndex + localIndex;
        acupoint.localIndex = localIndex;
        acupoint.meridian = type;
        acupoint.name = name;
        acupoint.chineseName = chineseName;
        acupoint.pinyin = pinyin;
        acupoint.location = location;
        acupoint.functions = functions;
        acupoint.indications = indications;
        acupoint.importance = importance;
        
        acupoints_.push_back(acupoint);
        return;
      }
    }
  }
};
