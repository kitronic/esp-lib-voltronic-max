#pragma once
#include <Arduino.h>
#include "VoltronicTypes.h"

// ═══════════════════════════════════════════════════════════════
//  VoltronicBattery — حساب SOC/SOH/الطاقة من قراءات QPIGS
//  أنواع البطاريات المدعومة + إمكانية تخصيص كامل
// ═══════════════════════════════════════════════════════════════

enum class VoltronicBatteryType : uint8_t
{
  User = 0,         // مخصص (خطي بين empty/full)
  AGM = 1,          // Lead-acid AGM 48V
  Flooded = 2,      // Lead-acid Flooded 48V
  Pylontech = 3,    // LiFePO4 15S (Pylontech)
  LiFePO4_15S = 4,  // LiFePO4 15S عام (48V)
  LiFePO4_16S = 5,  // LiFePO4 16S (51.2V)
};

class VoltronicBattery
{
 public:
  VoltronicBattery();

  // ═══ الإعدادات (من المستخدم) ═══
  void setType(VoltronicBatteryType type);
  void setCapacity(float ah);     // 10..2000 Ah
  void setVoltageEmpty(float v);  // SOC = 0%
  void setVoltageFull(float v);   // SOC = 100%
  void setNominalVoltage(float v);
  void setInitialSoh(float soh);  // 0..100
  void setCycleCount(uint16_t c);
  void enable(bool en)
  {
    _enabled = en;
  }

  // ═══ التحديث (تلقائي بعد QPIGS) ═══
  void update(const QPIGSData &g);

  // ═══ النتائج ═══
  float soc() const
  {
    return _soc;
  }  // %
  float soh() const
  {
    return _soh;
  }  // %
  float netCurrent() const
  {
    return _netCurrent;
  }  // A
  float netPower() const
  {
    return _netPower;
  }  // W
  float remainingWh() const
  {
    return _remainingWh;
  }  // Wh
  float remainingKWh() const
  {
    return _remainingWh / 1000.0f;
  }
  float remainingHours() const
  {
    return _remainingHours;
  }
  float timeToFull() const
  {
    return _timeToFull;
  }
  float cRate() const
  {
    return _cRate;
  }  // C
  float voltage() const
  {
    return _voltage;
  }
  uint16_t chargeCurrent() const
  {
    return _chargeCurrent;
  }
  uint16_t dischargeCurrent() const
  {
    return _dischargeCurrent;
  }

  // ═══ معلومات ═══
  const char *status() const;  // Charging / Discharging / Idle
  const char *typeName() const;
  VoltronicBatteryType type() const
  {
    return _type;
  }
  float capacityAh() const
  {
    return _capacityAh;
  }
  float nominalV() const
  {
    return _nominalV;
  }
  float voltageEmpty() const
  {
    return _voltageEmpty;
  }
  float voltageFull() const
  {
    return _voltageFull;
  }
  uint16_t cycleCount() const
  {
    return _cycleCount;
  }
  bool enabled() const
  {
    return _enabled;
  }

  // ═══ اسم النوع كنص (للواجهات) ═══
  static const char *typeToString(VoltronicBatteryType t);
  static VoltronicBatteryType typeFromString(const char *s);

 private:
  // ─── الإعدادات ───
  VoltronicBatteryType _type = VoltronicBatteryType::User;
  bool _enabled = true;
  float _capacityAh = 100.0f;
  float _nominalV = 48.0f;
  float _voltageEmpty = 42.0f;
  float _voltageFull = 54.0f;
  float _soh = 100.0f;
  uint16_t _cycleCount = 0;

  // ─── القراءات ───
  float _voltage = 0;
  uint16_t _chargeCurrent = 0;
  uint16_t _dischargeCurrent = 0;

  // ─── النتائج ───
  float _soc = 0;
  float _netCurrent = 0;
  float _netPower = 0;
  float _remainingWh = 0;
  float _remainingHours = 0;
  float _timeToFull = 0;
  float _cRate = 0;

  void _applyDefaultsForType();
  float _socFromVoltage(float v) const;
};