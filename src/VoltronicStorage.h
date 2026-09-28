#pragma once
#include <Arduino.h>

class VoltronicMAX;

// ═══════════════════════════════════════════════════════════════
//  VoltronicStorage — حفظ الإعدادات في EEPROM
//  يدعم ESP8266 + ESP32
// ═══════════════════════════════════════════════════════════════
class VoltronicStorage
{
 public:
  static constexpr uint32_t MAGIC = 0x564F4C54UL;  // "VOLT"
  static constexpr uint8_t VERSION = 1;

  VoltronicStorage();

  // ─── Lifecycle ───
  void begin(uint16_t eepromSize = 512);
  void load(VoltronicMAX &inv);
  void save(const VoltronicMAX &inv);
  void reset();

  // ─── حالة ───
  bool isLoaded() const
  {
    return _loaded;
  }
  bool isDirty() const
  {
    return _dirty;
  }
  uint32_t saves() const
  {
    return _saves;
  }

  // ─── auto-save ───
  void markDirty()
  {
    _dirty = true;
    _lastChange = millis();
  }
  void tick(VoltronicMAX &inv);  // احفظ تلقائياً بعد 5 ثواني من آخر تغيير

 private:
  struct Data
  {
    uint32_t magic;
    uint8_t version;
    uint8_t _pad1;
    uint16_t _pad2;

    // Battery
    uint8_t batteryType;
    uint8_t _padB;
    float batteryCapacityAh;
    float batteryVoltageEmpty;
    float batteryVoltageFull;
    float batterySoh;
    uint16_t batteryCycleCount;
    uint16_t _padB2;

    // Smart Charger
    uint8_t scMode;
    uint8_t scTargetAC;
    uint8_t scTargetTotal;
    uint8_t scFloatAC;
    uint8_t scFloatTotal;
    uint8_t scTempProtectC;
    uint16_t _padSC;

    // Power Mode
    uint8_t pmLocked;
    uint8_t _padPM[3];

    uint16_t crc;
    uint16_t _padEnd;

    // Power Mode thresholds
    uint8_t pmSocEmergency;     // 5-40
    uint8_t pmSocPowerSaving;   // 10-50
    uint8_t pmSocRecover;       // 20-60
    uint8_t pmSocSurplus;       // 70-99
    uint16_t pmGridMinVoltage;  // 100-220
  };

  Data _data;
  bool _loaded = false;
  bool _dirty = false;
  uint32_t _saves = 0;
  uint32_t _lastChange = 0;
  uint16_t _size = 512;

  static uint16_t _calcCRC(const Data &d);
  void _applyTo(VoltronicMAX &inv);
  void _captureFrom(const VoltronicMAX &inv);
};