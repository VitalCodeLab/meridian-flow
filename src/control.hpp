#pragma once
#include <Arduino.h>
#include "meridian.hpp"

// 前向声明
class EnhancedFlowEffect;

class DebouncedButton {
public:
  DebouncedButton(uint8_t pin, bool activeLow, unsigned long debounceMs, unsigned long longPressMs)
  : pin_(pin), activeLow_(activeLow), debounceMs_(debounceMs), longPressMs_(longPressMs) {}

  void begin() {
    pinMode(pin_, activeLow_ ? INPUT_PULLUP : INPUT);
    lastRaw_ = digitalRead(pin_);
    stablePressed_ = isPressedLevel(lastRaw_);
  }

  void poll() {
    int reading = digitalRead(pin_);
    if (reading != lastRaw_) { lastDebounceAt_ = millis(); lastRaw_ = reading; }
    if ((millis() - lastDebounceAt_) > debounceMs_) {
      bool nowPressed = isPressedLevel(reading);
      if (nowPressed != stablePressed_) {
        stablePressed_ = nowPressed;
        if (stablePressed_) { pressStartAt_ = millis(); }
        else {
          unsigned long held = millis() - pressStartAt_;
          if (held >= longPressMs_) longPressEvent_ = true; else shortPressEvent_ = true;
        }
      }
    }
  }

  bool consumeShortPress() { bool v = shortPressEvent_; shortPressEvent_ = false; return v; }
  bool consumeLongPress()  { bool v = longPressEvent_;  longPressEvent_ = false;  return v; }

private:
  bool isPressedLevel(int level) const { return activeLow_ ? (level == LOW) : (level == HIGH); }
  uint8_t pin_;
  bool activeLow_;
  unsigned long debounceMs_;
  unsigned long longPressMs_;
  int lastRaw_ = HIGH;
  bool stablePressed_ = false;
  unsigned long lastDebounceAt_ = 0;
  unsigned long pressStartAt_ = 0;
  bool shortPressEvent_ = false;
  bool longPressEvent_ = false;
};

// 已在文件开头前向声明 EnhancedFlowEffect

// 前向声明 OnboardMirror 类
class OnboardMirror {
public:
  OnboardMirror(uint8_t l1, uint8_t l2, Mode& modeRef, EnhancedFlowEffect& flowRef);
  void begin();
  void flashStep(unsigned ms);
  void tick();

private:
  uint8_t l1_, l2_;
  Mode& mode_;
  EnhancedFlowEffect& flow_;
  unsigned long lastBlink_ = 0;
  bool blinkState_ = false;
  unsigned long stepFlashUntil_ = 0;
};

