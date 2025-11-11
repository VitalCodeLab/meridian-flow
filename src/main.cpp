#include <Arduino.h>

// LED 引脚映射（D4 -> IO12, D5 -> IO13）
const uint8_t LED_D4 = 12;
const uint8_t LED_D5 = 13;

// 高电平有效
const bool LED_ACTIVE_HIGH = true;

// 板载按键固定为 GPIO9（常见 BOOT），按下为低电平
const uint8_t BUTTON_PIN = 9;
const bool BUTTON_ACTIVE_LOW = true;

// 去抖与闪烁间隔
const unsigned long DEBOUNCE_MS = 50;
const unsigned long BLINK_INTERVAL_MS = 300;

bool ledState = false;
bool blinking = true;
unsigned long lastBlinkMillis = 0;
int lastButtonRaw = HIGH;
unsigned long lastDebounceMillis = 0;

// 帮助函数：按板子电平规则写 LED
void ledWrite(uint8_t pin, bool on)
{
  if (LED_ACTIVE_HIGH)
    digitalWrite(pin, on ? HIGH : LOW);
  else
    digitalWrite(pin, on ? LOW : HIGH);
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_D4, OUTPUT);
  pinMode(LED_D5, OUTPUT);

  // 初始关闭两个 LED
  ledWrite(LED_D4, false);
  ledWrite(LED_D5, false);

  // 固定使用 GPIO9，按下为 LOW -> 使用 INPUT_PULLUP
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonRaw = digitalRead(BUTTON_PIN);

  Serial.printf("使用按键: GPIO%d, active %s\n", BUTTON_PIN, BUTTON_ACTIVE_LOW ? "LOW" : "HIGH");
  Serial.println("准备就绪：按下按钮切换闪烁 ON/OFF。");
}

void loop()
{
  // 读取原始按键电平
  int reading = digitalRead(BUTTON_PIN);
  // 适配 active-low -> 按下为 true
  bool pressed = BUTTON_ACTIVE_LOW ? (reading == LOW) : (reading == HIGH);

  // 简单去抖：检测原始电平变化并等待稳定
  if (reading != lastButtonRaw)
  {
    lastDebounceMillis = millis();
    lastButtonRaw = reading;
  }
  if ((millis() - lastDebounceMillis) > DEBOUNCE_MS)
  {
    static bool lastStablePressed = false;
    if (pressed != lastStablePressed)
    {
      lastStablePressed = pressed;
      if (pressed)
      {
        // 按下事件：切换闪烁状态
        blinking = !blinking;
        Serial.print("Blinking ");
        Serial.println(blinking ? "ON" : "OFF");
        if (!blinking)
        {
          // 关闭时保证 LED 关闭
          ledState = false;
          ledWrite(LED_D4, false);
          ledWrite(LED_D5, false);
        }
      }
    }
  }

  // 非阻塞闪烁处理
  if (blinking)
  {
    unsigned long now = millis();
    if (now - lastBlinkMillis >= BLINK_INTERVAL_MS)
    {
      lastBlinkMillis = now;
      ledState = !ledState;
      // 交替亮灭
      ledWrite(LED_D4, ledState);
      ledWrite(LED_D5, !ledState);
    }
  }

  delay(5);
}