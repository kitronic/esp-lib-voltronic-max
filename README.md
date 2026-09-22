# VoltronicMAX Arduino Library

مكتبة Arduino للتواصل مع إنفرترات **Voltronic Power Axpert MAX** (موديلات HV7.2kW و LV5kW) عبر بروتوكول **PI30 ASCII**.

> ⚠️ هذه المكتبة **ليست Modbus**. البروتوكول ASCII على بورت RS232 بسرعة 2400 8N1.

## المميزات
- قراءة حالة الإنفرتر اللحظية (`QPIGS`)
- قراءة الأوضاع، الإنذارات، إعدادات الجهاز
- كتابة الإعدادات (Output priority, Charger priority, Max charging current, ...)
- حساب CRC-16/XMODEM تلقائياً
- غير محجوب (Non-blocking friendly) — بدون `delay()` داخلي
- متوافقة مع ESP32 / ESP8266 / AVR

## التثبيت
### عبر Arduino Library Manager
1. **Sketch → Include Library → Manage Libraries**
2. ابحث عن `VoltronicMAX`
3. اضغط **Install**

### يدوياً
حمّل الريبو ZIP ثم **Sketch → Include Library → Add .ZIP Library**

## الاستخدام السريع
```cpp
#include <VoltronicMAX.h>

VoltronicMAX inverter(Serial2);

void setup() {
  Serial.begin(115200);
  inverter.begin(2400, 16, 17); // baud, RX, TX
}

void loop() {
  if (inverter.queryGeneralStatus()) {
    InverterData d = inverter.data();
    Serial.printf("GridV=%.1f OutV=%.1f Load=%u%% BattV=%.1f Mode=%s\n",
                  d.gridVoltage, d.acOutputVoltage,
                  d.loadPercent, d.batteryVoltage,
                  inverter.modeString());
  }

  delay(2000);
}