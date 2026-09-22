#pragma once
#include <Arduino.h>
#include "VoltronicConfig.h"

// ═══════════════════════════════════════════════════════════════
//  Transport — يلف Stream (HardwareSerial أو SoftwareSerial)
//
//  ⚠️ ملاحظة مهمة:
//  Stream هي class مجردة وما فيها begin(). يجب على المستخدم
//  استدعاء Serial.begin(baud) قبل استخدام المكتبة.
//
//  مثال:
//    Serial2.begin(2400, SERIAL_8N1, 16, 17);  // ESP32
//    inverter.begin(2400);                     // يخزن الإعدادات فقط
// ═══════════════════════════════════════════════════════════════
class VoltronicTransport {
public:
  explicit VoltronicTransport(Stream& serial)
    : _serial(serial), _cfg(nullptr) {}

  void attachConfig(const VoltronicConfig* cfg) { _cfg = cfg; }

  // ما نستدعي begin() على Stream لأنها مجردة
  // المستخدم مسؤول عن تهيئة Serial بنفسه
  void begin() {
    // لا شي — الباود والفريم يضبطهم المستخدم
  }

  void clear() {
    while (_serial.available()) _serial.read();
  }

  void writeRaw(const uint8_t* data, size_t len) {
    _serial.write(data, len);
    _serial.flush();
  }

  size_t readUntilCR(uint8_t* buffer, size_t maxLen) {
    if (!_cfg) return 0;

    uint32_t start = millis();
    size_t idx = 0;

    while ((millis() - start) < _cfg->responseTimeoutMs) {
      while (_serial.available()) {
        int b = _serial.read();
        if (b < 0) continue;
        if (b == 0x0D) return idx;

        #if VOLTRONIC_FILTER_PRINTABLE
          if (b < 32 || b > 126) continue;
        #endif

        if (idx < maxLen) buffer[idx++] = (uint8_t)b;
      }

      #if VOLTRONIC_YIELD_IN_READ
        yield();
      #endif
    }
    return idx;
  }

  // للاستعمال الداخلي فقط
  Stream& stream() { return _serial; }

private:
  Stream& _serial;
  const VoltronicConfig* _cfg;
};