#include "meridian_tcm.hpp"

// 实现 addSpecificAcupoint
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

// 实现 initZiwuliuzhu（采用完整子午流注时间表版本）
void TCMMeridianSystem::initZiwuliuzhu() {
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
  
  // 卯时到05时-7时：足太阴脾经
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
  
  // 未时到11时-13时：足太阳膀胱经
  ZiwuliuzhuTimeSlot slot7;
  slot7.meridian = BLADDER;
  slot7.startHour = 11;
  slot7.startMinute = 0;
  slot7.endHour = 13;
  slot7.endMinute = 0;
  slot7.description = "未时（11:00-13:00）：足太阳膀胱经当令";
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

// 实现 initMeridians（使用原有均分逻辑）
void TCMMeridianSystem::initMeridians() {
    // 计算每条经络的长度
    // 假设将灯带均分给12条经络
    uint16_t ledsPerMeridian = numLeds_ / 12;
    int remainder = numLeds_ % 12; // 使用有符号类型避免在 0-- 时溢出
    uint16_t currentIndex = 0;
    
    // 手太阴肺经 - 红色
    MeridianInfo lung;
    lung.type = LUNG;
    lung.name = "Lung Meridian";
    lung.chineseName = "手太阴肺经";
    lung.color = CRGB::Red;
    lung.enabled = true;
    lung.hasCustomRange = false;
    lung.startIndex = currentIndex;
    lung.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    lung.acupoints = {3, 9, 15, 21, 27};
    meridians_.push_back(lung);
    currentIndex += lung.length;
    
    // 手阳明大肠经 - 橙色
    MeridianInfo largeIntestine;
    largeIntestine.type = LARGE_INTESTINE;
    largeIntestine.name = "Large Intestine Meridian";
    largeIntestine.chineseName = "手阳明大肠经";
    largeIntestine.color = CRGB::Orange;
    largeIntestine.enabled = false;
    largeIntestine.hasCustomRange = false;
    largeIntestine.startIndex = currentIndex;
    largeIntestine.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    largeIntestine.acupoints = {4, 11, 18, 25, 32};
    meridians_.push_back(largeIntestine);
    currentIndex += largeIntestine.length;
    
    // 足阳明胃经 - 黄色
    MeridianInfo stomach;
    stomach.type = STOMACH;
    stomach.name = "Stomach Meridian";
    stomach.chineseName = "足阳明胃经";
    stomach.color = CRGB::Yellow;
    stomach.enabled = false;
    stomach.hasCustomRange = false;
    stomach.startIndex = currentIndex;
    stomach.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    stomach.acupoints = {5, 12, 19, 26, 33};
    meridians_.push_back(stomach);
    currentIndex += stomach.length;
    
    // 足太阴脾经 - 黄绿色
    MeridianInfo spleen;
    spleen.type = SPLEEN;
    spleen.name = "Spleen Meridian";
    spleen.chineseName = "足太阴脾经";
    spleen.color = CRGB(128, 255, 0);
    spleen.enabled = false;
    spleen.hasCustomRange = false;
    spleen.startIndex = currentIndex;
    spleen.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    spleen.acupoints = {6, 13, 20, 27, 34};
    meridians_.push_back(spleen);
    currentIndex += spleen.length;
    
    // 手少阴心经 - 绿色
    MeridianInfo heart;
    heart.type = HEART;
    heart.name = "Heart Meridian";
    heart.chineseName = "手少阴心经";
    heart.color = CRGB::Green;
    heart.enabled = false;
    heart.hasCustomRange = false;
    heart.startIndex = currentIndex;
    heart.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    heart.acupoints = {7, 14, 21, 28};
    meridians_.push_back(heart);
    currentIndex += heart.length;
    
    // 手太阳小肠经 - 青色
    MeridianInfo smallIntestine;
    smallIntestine.type = SMALL_INTESTINE;
    smallIntestine.name = "Small Intestine Meridian";
    smallIntestine.chineseName = "手太阳小肠经";
    smallIntestine.color = CRGB(0, 255, 128);
    smallIntestine.enabled = false;
    smallIntestine.hasCustomRange = false;
    smallIntestine.startIndex = currentIndex;
    smallIntestine.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    smallIntestine.acupoints = {8, 15, 22, 29};
    meridians_.push_back(smallIntestine);
    currentIndex += smallIntestine.length;
    
    // 足太阳膀胱经 - 蓝色
    MeridianInfo bladder;
    bladder.type = BLADDER;
    bladder.name = "Bladder Meridian";
    bladder.chineseName = "足太阳膀胱经";
    bladder.color = CRGB::Blue;
    bladder.enabled = false;
    bladder.hasCustomRange = false;
    bladder.startIndex = currentIndex;
    bladder.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    bladder.acupoints = {9, 16, 23, 30};
    meridians_.push_back(bladder);
    currentIndex += bladder.length;
    
    // 足少阴肾经 - 靛蓝色
    MeridianInfo kidney;
    kidney.type = KIDNEY;
    kidney.name = "Kidney Meridian";
    kidney.chineseName = "足少阴肾经";
    kidney.color = CRGB(75, 0, 130);
    kidney.enabled = false;
    kidney.hasCustomRange = false;
    kidney.startIndex = currentIndex;
    kidney.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    kidney.acupoints = {10, 17, 24, 31};
    meridians_.push_back(kidney);
    currentIndex += kidney.length;
    
    // 手厥阴心包经 - 紫色
    MeridianInfo pericardium;
    pericardium.type = PERICARDIUM;
    pericardium.name = "Pericardium Meridian";
    pericardium.chineseName = "手厥阴心包经";
    pericardium.color = CRGB::Purple;
    pericardium.enabled = false;
    pericardium.hasCustomRange = false;
    pericardium.startIndex = currentIndex;
    pericardium.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    pericardium.acupoints = {2, 8, 14, 20};
    meridians_.push_back(pericardium);
    currentIndex += pericardium.length;
    
    // 手少阳三焦经 - 品红色
    MeridianInfo tripleEnergizer;
    tripleEnergizer.type = TRIPLE_ENERGIZER;
    tripleEnergizer.name = "Triple Energizer Meridian";
    tripleEnergizer.chineseName = "手少阳三焦经";
    tripleEnergizer.color = CRGB::Magenta;
    tripleEnergizer.enabled = false;
    tripleEnergizer.hasCustomRange = false;
    tripleEnergizer.startIndex = currentIndex;
    tripleEnergizer.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    tripleEnergizer.acupoints = {3, 9, 15, 21};
    meridians_.push_back(tripleEnergizer);
    currentIndex += tripleEnergizer.length;
    
    // 足少阳胆经 - 粉色
    MeridianInfo gallbladder;
    gallbladder.type = GALLBLADDER;
    gallbladder.name = "Gallbladder Meridian";
    gallbladder.chineseName = "足少阳胆经";
    gallbladder.color = CRGB::Pink;
    gallbladder.enabled = false;
    gallbladder.hasCustomRange = false;
    gallbladder.startIndex = currentIndex;
    gallbladder.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    gallbladder.acupoints = {4, 10, 16, 22};
    meridians_.push_back(gallbladder);
    currentIndex += gallbladder.length;
    
    // 足厥阴肝经 - 暗红色
    MeridianInfo liver;
    liver.type = LIVER;
    liver.name = "Liver Meridian";
    liver.chineseName = "足厥阴肝经";
    liver.color = CRGB(139, 0, 0);
    liver.enabled = false;
    liver.hasCustomRange = false;
    liver.startIndex = currentIndex;
    liver.length = ledsPerMeridian + (remainder > 0 ? 1 : 0);
    if (remainder > 0) remainder--;
    liver.acupoints = {5, 11, 17, 23};
    meridians_.push_back(liver);
}

// 实现 initAcupoints：根据 meridians_ 填充 acupoints_，并附加示例穴位信息
void TCMMeridianSystem::initAcupoints() {
    // 为每个经络的穴位创建全局索引
    for (const auto& meridian : meridians_) {
      for (uint16_t localIdx : meridian.acupoints) {
        AcupointInfo acupoint;
        acupoint.globalIndex = meridian.startIndex + localIdx;
        acupoint.localIndex = localIdx;
        acupoint.meridian = meridian.type;
        
        // 示例名称
        char nameBuffer[50];
        sprintf(nameBuffer, "%s_point_%d", meridian.name, localIdx);
        acupoint.name = strdup(nameBuffer);
        
        char chineseNameBuffer[50];
        sprintf(chineseNameBuffer, "%s穴位%d", meridian.chineseName, localIdx);
        acupoint.chineseName = strdup(chineseNameBuffer);
        
        acupoints_.push_back(acupoint);
      }
    }

    // 为 /tcm 页面提供的常用穴位添加具名映射，使用各自经络中段 LED 作为示意位置
    const MeridianInfo* li = nullptr; // 大肠经
    const MeridianInfo* st = nullptr; // 胃经
    const MeridianInfo* sp = nullptr; // 脾经
    const MeridianInfo* ht = nullptr; // 心经
    const MeridianInfo* pc = nullptr; // 心包经
    const MeridianInfo* lv = nullptr; // 肝经

    for (const auto& meridian : meridians_) {
      switch (meridian.type) {
        case LARGE_INTESTINE: li = &meridian; break;
        case STOMACH:         st = &meridian; break;
        case SPLEEN:          sp = &meridian; break;
        case HEART:           ht = &meridian; break;
        case PERICARDIUM:     pc = &meridian; break;
        case LIVER:           lv = &meridian; break;
        default: break;
      }
    }

    if (li && li->length > 0) {
      uint16_t idx = li->length / 2; // 近似取本经中点
      addSpecificAcupoint(LARGE_INTESTINE, idx,
                          "Hegu", "合谷", "Hegu",
                          "手背第一、二掌骨间，当第二掌骨桡侧的中点处",
                          "疏风解表，调和营卫",
                          "头痛、牙痛、面瘫等头面部疾病", 5);
    }

    if (st && st->length > 0) {
      uint16_t idx = st->length / 2;
      addSpecificAcupoint(STOMACH, idx,
                          "Zusanli", "足三里", "Zusanli",
                          "外膝眼下三寸，胫骨前嵴旁一横指",
                          "健脾和胃，补中益气",
                          "胃痛、腹胀、消化不良、疲劳乏力", 5);
    }

    if (sp && sp->length > 0) {
      uint16_t idx = sp->length / 2;
      addSpecificAcupoint(SPLEEN, idx,
                          "Sanyinjiao", "三阴交", "Sanyinjiao",
                          "内踝尖上三寸，胫骨内侧后缘",
                          "健脾益肾，调补肝血",
                          "月经不调、失眠、腹胀腹痛等", 5);
    }

    if (ht && ht->length > 0) {
      uint16_t idx = ht->length / 2;
      addSpecificAcupoint(HEART, idx,
                          "Shenmen", "神门", "Shenmen",
                          "腕横纹上，尺侧腕屈肌腱的桡侧缘",
                          "宁心安神",
                          "心悸、失眠、健忘、焦虑", 5);
    }

    if (pc && pc->length > 0) {
      uint16_t idx = pc->length / 2;
      addSpecificAcupoint(PERICARDIUM, idx,
                          "Neiguan", "内关", "Neiguan",
                          "腕横纹上二寸，两筋之间",
                          "理气宽胸，安神",
                          "心绞痛、胸闷、呕吐、晕车", 5);
    }

    if (lv && lv->length > 0) {
      uint16_t idx = lv->length / 2;
      addSpecificAcupoint(LIVER, idx,
                          "Taichong", "太冲", "Taichong",
                          "足背第一、二跖骨间隙后方凹陷处",
                          "疏肝理气，平肝潜阳",
                          "头痛、眩晕、情志抑郁", 5);
    }

    // 下面这部分是原来在 hpp 中的示例“特定重要穴位”添加逻辑，
    // 如果后续不需要这么详细，可以逐步精简。
    // （为保持行为一致，这里先不删除原有 addSpecificAcupoint 调用。）
}

void TCMMeridianSystem::startSingleFlow(MeridianType type, uint8_t tailLength, uint16_t interval) {
  flowMode_ = FLOW_SINGLE;
  flowTailLength_ = tailLength;
  flowIntervalMs_ = interval;
  flowSingleMeridian_ = type;
  flowMeridianIndex_ = 0;
  flowHead_ = 0;
  flowInPause_ = false;
  lastFlowStepMs_ = millis();
}

void TCMMeridianSystem::startAllFlow(uint8_t tailLength, uint16_t interval) {
  flowMode_ = FLOW_ALL;
  flowTailLength_ = tailLength;
  flowIntervalMs_ = interval;
  flowMeridianIndex_ = 0;
  flowHead_ = 0;
  flowInPause_ = false;
  lastFlowStepMs_ = millis();
}

void TCMMeridianSystem::startCurrentTimeFlow(uint8_t tailLength, uint16_t interval) {
  MeridianType active = getCurrentActiveMeridian();
  startSingleFlow(active, tailLength, interval);
}

void TCMMeridianSystem::stopFlow() {
  flowMode_ = FLOW_NONE;
  flowInPause_ = false;
  flowHead_ = 0;
  flowMeridianIndex_ = 0;
}

void TCMMeridianSystem::tickFlow() {
  if (flowMode_ == FLOW_NONE) {
    return;
  }

  if (meridians_.empty()) {
    stopFlow();
    return;
  }

  uint32_t now = millis();

  if (flowInPause_) {
    if (now >= flowPauseEndMs_) {
      flowInPause_ = false;
      lastFlowStepMs_ = now;
    } else {
      return;
    }
  }

  if (now - lastFlowStepMs_ < flowIntervalMs_) {
    return;
  }
  lastFlowStepMs_ = now;

  MeridianInfo* meridian = nullptr;

  if (flowMode_ == FLOW_SINGLE) {
    for (auto& m : meridians_) {
      if (m.type == flowSingleMeridian_) {
        meridian = &m;
        break;
      }
    }
  } else if (flowMode_ == FLOW_ALL) {
    while (flowMeridianIndex_ < meridians_.size()) {
      auto& m = meridians_[flowMeridianIndex_];
      if (m.length == 0) {
        flowMeridianIndex_++;
        continue;
      }
      meridian = &m;
      break;
    }
  }

  if (!meridian || meridian->length == 0) {
    stopFlow();
    return;
  }

  for (uint16_t i = 0; i < meridian->length; i++) {
    uint16_t idx = meridian->startIndex + i;
    if (idx < numLeds_) {
      leds_[idx] = CRGB::Black;
    }
  }

  for (uint8_t t = 0; t < flowTailLength_; t++) {
    int16_t pos = (int16_t)flowHead_ - (int16_t)t;
    if (pos >= 0 && pos < (int16_t)meridian->length) {
      uint16_t idx = meridian->startIndex + (uint16_t)pos;
      if (idx < numLeds_) {
        uint8_t brightness = 255 - ((255 * t) / flowTailLength_);
        leds_[idx] = meridian->color;
        leds_[idx].nscale8(brightness);
      }
    }
  }

  for (uint16_t apIdx : meridian->acupoints) {
    if (apIdx >= meridian->length) {
      continue;
    }
    if (flowHead_ >= apIdx && (flowHead_ - apIdx) < flowTailLength_) {
      uint16_t idx = meridian->startIndex + apIdx;
      if (idx < numLeds_) {
        leds_[idx] = CRGB::White;
      }
    }
  }

  FastLED.show();

  flowHead_++;

  if (flowHead_ >= meridian->length + flowTailLength_) {
    if (flowMode_ == FLOW_SINGLE) {
      stopFlow();
    } else if (flowMode_ == FLOW_ALL) {
      flowHead_ = 0;
      flowMeridianIndex_++;
      if (flowMeridianIndex_ >= meridians_.size()) {
        stopFlow();
      } else {
        flowInPause_ = true;
        flowPauseEndMs_ = now + flowPauseMs_;
      }
    }
  }
}
