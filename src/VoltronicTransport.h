#pragma once
#include <Arduino.h>
#include "VoltronicConfig.h"

// ===============================================================
//  VoltronicTransport - wraps a Stream (HardwareSerial/SoftwareSerial)
//
//  Stream is abstract - it has no begin() method.
//  The user MUST initialize the serial port before inverter.begin():
//
//    ESP8266:
//      SoftwareSerial invSerial(D1, D2);
//      invSerial.begin(2400);
//      VoltronicMAX inverter(invSerial);
//      inverter.begin(cfg);
//
//    ESP32:
//      Serial2.begin(2400, SERIAL_8N1, 16, 17);
//      VoltronicMAX inverter(Serial2);
//      inverter.begin(cfg);
// ===============================================================
class VoltronicTransport
{
public:
  explicit VoltronicTransport(Stream &serial)
      : _serial(serial), _cfg(nullptr) {}

  // Bind config (called from VoltronicMAX::begin)
  void attachConfig(const VoltronicConfig *cfg) { _cfg = cfg; }

  // No-op - user initializes Serial
  void begin() {}

  // Drain input buffer
  void clear()
  {
    while (_serial.available())
      _serial.read();
  }

  // Send a complete frame
  void writeRaw(const uint8_t *data, size_t len)
  {
    _serial.write(data, len);
    _serial.flush();
  }

  // ===============================================================
  //  Read until CR (0x0D)
  //
  //  We keep ALL bytes including non-printable CRC bytes
  //  because they are needed to verify the CRC later.
  //
  //  Behavior:
  //    1. Skip noise before the first '('
  //    2. Collect all bytes after '(' until 0x0D
  //    3. Return count of collected bytes (excluding CR)
  //    4. Return 0 on timeout
  // ===============================================================
  size_t readUntilCR(uint8_t *buffer, size_t maxLen)
  {
    if (!_cfg)
      return 0;

    const uint32_t start = millis();
    const uint32_t timeout = _cfg->responseTimeoutMs;
    size_t idx = 0;
    bool foundStart = false;

    while ((millis() - start) < timeout)
    {
      while (_serial.available())
      {
        int b = _serial.read();
        if (b < 0)
          continue;

        // End of frame
        if (b == 0x0D)
          return idx;

        // Skip noise before '('
        if (!foundStart)
        {
          if (b == '(')
            foundStart = true;
          else
            continue;
        }

        // No filtering - keep all bytes (including CRC)
        if (idx < maxLen)
          buffer[idx++] = (uint8_t)b;
      }

      // Runtime yield (for ESP8266 WDT)
      if (_cfg->yieldDuringRead)
        yield();
    }
    return idx;
  }

  // Direct access (for debugging only)
  Stream &stream() { return _serial; }

private:
  Stream &_serial;
  const VoltronicConfig *_cfg;
};