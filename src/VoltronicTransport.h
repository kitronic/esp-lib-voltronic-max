#pragma once
#include <Arduino.h>
#include "VoltronicConfig.h"

// ═══════════════════════════════════════════════════════════════
//  Transport — يلف Stream (HardwareSerial أو SoftwareSerial)
//
//  ⚠️ ملاحظة: Stream مجردة، ما فيها begin().
//  المستخدم لازم يعمل invSerial.begin(2400) قبل inverter.begin().
// ═══════════════════════════════════════════════════════════════
class VoltronicTransport
{
public:
  explicit VoltronicTransport(Stream &serial)
      : _serial(serial), _cfg(nullptr) {}

  void attachConfig(const VoltronicConfig *cfg) { _cfg = cfg; }

  void begin()
  {
    // لا شي — المستخدم يهيّئ Serial بنفسه
  }

  void clear()
  {
    while (_serial.available())
      _serial.read();
  }

  void writeRaw(const uint8_t *data, size_t len)
  {
    _serial.write(data, len);
    _serial.flush();
  }

  // ═══════════════════════════════════════════════════════════
  //  قراءة حتى CR — بدون filter
  //  ⚠️ مهم: نحتفظ بكل bytes (بما فيها CRC غير المطبوع)
  // ═══════════════════════════════════════════════════════════
  size_t readUntilCR(uint8_t *buffer, size_t maxLen)
  {
    if (!_cfg)
      return 0;

    uint32_t start = millis();
    size_t idx = 0;
    bool foundStart = false;

    while ((millis() - start) < _cfg->responseTimeoutMs)
    {
      while (_serial.available())
      {
        int b = _serial.read();
        if (b < 0)
          continue;

        // ═══ نهاية الفريم ═══
        if (b == 0x0D)
          return idx;

        // ═══ تخطى noise قبل '(' ═══
        if (!foundStart)
        {
          if (b == '(')
          {
            foundStart = true;
          }
          else
          {
            continue;
          }
        }

        // ═══ ⚠️ لا filter — كل bytes مقبولة (بما فيها CRC) ═══
        if (idx < maxLen)
          buffer[idx++] = (uint8_t)b;
      }

#if VOLTRONIC_YIELD_IN_READ
      yield();
#endif
    }
    return idx;
  }

  // للاستعمال الداخلي
  Stream &stream() { return _serial; }

private:
  Stream &_serial;
  const VoltronicConfig *_cfg;
};