#include "control.hpp"
#include "enhanced_led_controller.hpp"

// OnboardMirror 类的实现
OnboardMirror::OnboardMirror(uint8_t l1, uint8_t l2, Mode& modeRef, EnhancedFlowEffect& flowRef)
  : l1_(l1), l2_(l2), mode_(modeRef), flow_(flowRef) {}

void OnboardMirror::begin() {
  pinMode(l1_, OUTPUT); pinMode(l2_, OUTPUT);
  digitalWrite(l1_, LOW); digitalWrite(l2_, LOW);
}

void OnboardMirror::flashStep(unsigned ms) { 
  stepFlashUntil_ = millis() + ms; 
}

void OnboardMirror::tick() {
  unsigned long now = millis();
  if (mode_ == FLOW) {
    if (flow_.running()) {
      if (now - lastBlink_ >= 300) { lastBlink_ = now; blinkState_ = !blinkState_; }
      digitalWrite(l1_, blinkState_ ? HIGH : LOW);
    } else { digitalWrite(l1_, HIGH); }
    digitalWrite(l2_, LOW);
  } else { // STEP
    bool l2 = true;
    if (now < stepFlashUntil_) l2 = !l2;
    digitalWrite(l2_, l2 ? HIGH : LOW);
    digitalWrite(l1_, LOW);
  }
}
