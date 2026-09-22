#pragma once
#include <Arduino.h>
#include "VoltronicConfig.h"

// ═══════════════════════════════════════════════════════════════
//  Transport — يلف Stream (HardwareSerial أو SoftwareSerial)
//  بدون dynamic allocation، بدون String
// ═══════════════════════════════════════════════════════════════
class VoltronicTransport {
public:
  explicit VoltronicTransport(Stream& serial)
    : _serial(serial), _cfg(nullptr) {}

  void attachConfig(const VoltronicConfig* cfg) { _cfg = cfg; }

  void begin() {
    if (_cfg) _serial.begin(_cfg->baud, SERIAL_8N1);
    else      _serial.begin(VOLTRONIC_DEFAULT_BAUD, SERIAL_8N1);
  }

  // تنظيف أي بايتات عالقة
  void clear() {
    while (_serial.available()) _serial.read();
  }

  // كتابة frame جاهز (مع CRC و CR)
  void writeRaw(const uint8_t* data, size_t len) {
    _serial.write(data, len);
    _serial.flush();
  }

  // قراءة حتى CR — مع timeout و yield
  // يرجّع عدد البايتات المقروءة (بدون CR)
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

private:
  Stream& _serial;
  const VoltronicConfig* _cfg;
};