#pragma once
#include <Arduino.h>
#include "VoltronicTypes.h"

enum class VoltronicBatteryType : uint8_t
{
  User = 0,
  AGM = 1,
  Flooded = 2,
  Pylontech = 3,
  LiFePO4_15S = 4,
  LiFePO4_16S = 5,
};

class VoltronicBattery
{
 public:
  VoltronicBattery();

  // ═══ الإعدادات ═══
  void setType(VoltronicBatteryType type);
  void setCapacity(float ah);
  void setVoltageEmpty(float v);
  void setVoltageFull(float v);
  void setNominalVoltage(float v);
  void setInitialSoh(float soh);
  void setCycleCount(uint16_t c);
  void enable(bool en) { _enabled = en; }

  // ═══ جديد: تعويض المقاومة الداخلية ═══
  // 0 = معطّل. القيمة المعتادة 0.005 - 0.05 Ohm
  void setSystemResistance(float ohm) { _systemResistance = constrain(ohm, 0.0f, 0.2f); }
  float systemResistance() const { return _systemResistance; }

  // ═══ جديد: EMA Smoothing على الجهد ═══
  // alpha = 0 → تجميد، alpha = 1 → بدون smoothing
  void setVoltageSmoothing(float alpha) { _emaAlpha = constrain(alpha, 0.05f, 1.0f); }
  float voltageSmoothing() const { return _emaAlpha; }

  // ═══ جديد: حماية القفزات ═══
  // أقصى تغيّر مسموح في الجهد بين قراءتين (V)
  void setMaxVoltageJump(float v) { _maxVoltageJump = constrain(v, 0.5f, 10.0f); }

  // ═══ التحديث ═══
  void update(const QPIGSData& g);

  // ═══ النتائج ═══
  float soc() const { return _soc; }
  float soh() const { return _soh; }
  float netCurrent() const { return _netCurrent; }
  float netPower() const { return _netPower; }
  float remainingWh() const { return _remainingWh; }
  float remainingKWh() const { return _remainingWh / 1000.0f; }
  float remainingHours() const { return _remainingHours; }
  float timeToFull() const { return _timeToFull; }
  float cRate() const { return _cRate; }

  // ═══ جديد: قراءات الجهد الثلاث ═══
  float rawVoltage() const { return _rawVoltage; }            // من QPIGS مباشرة
  float compensatedVoltage() const { return _compVoltage; }   // بعد IR
  float voltage() const { return _smoothedVoltage; }          // بعد EMA (المستخدم في SOC)

  uint16_t chargeCurrent() const { return _chargeCurrent; }
  uint16_t dischargeCurrent() const { return _dischargeCurrent; }

  // ═══ معلومات ═══
  const char* status() const;
  const char* typeName() const;
  VoltronicBatteryType type() const { return _type; }
  float capacityAh() const { return _capacityAh; }
  float nominalV() const { return _nominalV; }
  float voltageEmpty() const { return _voltageEmpty; }
  float voltageFull() const { return _voltageFull; }
  uint16_t cycleCount() const { return _cycleCount; }
  bool enabled() const { return _enabled; }
  bool voltageOutOfRange() const { return _voltageOutOfRange; }

  static const char* typeToString(VoltronicBatteryType t);
  static VoltronicBatteryType typeFromString(const char* s);

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
  float _systemResistance = 0.0f;
  float _emaAlpha = 0.30f;
  float _maxVoltageJump = 3.0f;

  // ─── القراءات ───
  uint16_t _chargeCurrent = 0;
  uint16_t _dischargeCurrent = 0;

  // ─── Voltages ───
  float _rawVoltage = 0;
  float _compVoltage = 0;
  float _smoothedVoltage = 0;
  bool _hasFirstReading = false;
  bool _voltageOutOfRange = false;

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