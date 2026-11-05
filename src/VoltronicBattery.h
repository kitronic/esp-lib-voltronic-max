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
  void enable(bool en)
  {
    _enabled = en;
  }

  // ═══ IR Compensation + EMA ═══
  void setSystemResistance(float ohm)
  {
    _systemResistance = constrain(ohm, 0.0f, 0.2f);
  }
  float systemResistance() const
  {
    return _systemResistance;
  }

  void setVoltageSmoothing(float alpha)
  {
    _emaAlpha = constrain(alpha, 0.05f, 1.0f);
  }
  float voltageSmoothing() const
  {
    return _emaAlpha;
  }

  void setMaxVoltageJump(float v)
  {
    _maxVoltageJump = constrain(v, 0.5f, 10.0f);
  }
  float maxVoltageJump() const
  {
    return _maxVoltageJump;
  }

  // ═══ ⚡ جديد: Coulomb Counting + معايرة ═══
  void setSocFromCoulomb(bool en)
  {
    _useCoulomb = en;
  }
  bool socFromCoulomb() const
  {
    return _useCoulomb;
  }

  void setSocCalibrationThreshold(float pct)
  {
    _socCalibThreshold = constrain(pct, 0.0f, 100.0f);
  }
  float socCalibrationThreshold() const
  {
    return _socCalibThreshold;
  }

  // ═══ ⚡ جديد: SOC مبدئي (للاستعادة من EEPROM) ═══
  void setInitialSoc(float soc)
  {
    _soc = constrain(soc, 0.0f, 100.0f);
    _remainingAh = (_soc / 100.0f) * _capacityAh;
  }
  float initialSoc() const
  {
    return _soc;
  }
  float remainingAh() const
  {
    return _remainingAh;
  }

  // ═══ التحديث ═══
  void update(const QPIGSData& g, uint32_t dtMs = 1000);

  // ═══ النتائج ═══
  float soc() const
  {
    return _soc;
  }
  float soh() const
  {
    return _soh;
  }
  float netCurrent() const
  {
    return _netCurrent;
  }
  float netPower() const
  {
    return _netPower;
  }
  float remainingWh() const
  {
    return _remainingWh;
  }
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
  }

  float rawVoltage() const
  {
    return _rawVoltage;
  }
  float compensatedVoltage() const
  {
    return _compVoltage;
  }
  float voltage() const
  {
    return _smoothedVoltage;
  }

  uint16_t chargeCurrent() const
  {
    return _chargeCurrent;
  }
  uint16_t dischargeCurrent() const
  {
    return _dischargeCurrent;
  }

  const char* status() const;
  const char* typeName() const;
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
  bool voltageOutOfRange() const
  {
    return _voltageOutOfRange;
  }
  bool socFromCoulombActive() const
  {
    return _useCoulomb && _coulombInitialized;
  }

  static const char* typeToString(VoltronicBatteryType t);
  static VoltronicBatteryType typeFromString(const char* s);

 private:
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

  // ⚡ Coulomb counting
  bool _useCoulomb = true;
  bool _coulombInitialized = false;
  float _remainingAh = 0.0f;
  float _socCalibThreshold = 98.0f;
  uint32_t _fullVoltageMs = 0;
  uint32_t _emptyVoltageMs = 0;
  uint32_t _lastUpdateMs = 0;
  bool _firstUpdate = true;

  uint16_t _chargeCurrent = 0;
  uint16_t _dischargeCurrent = 0;

  float _rawVoltage = 0;
  float _compVoltage = 0;
  float _smoothedVoltage = 0;
  bool _hasFirstReading = false;
  bool _voltageOutOfRange = false;

  float _soc = 0;
  float _netCurrent = 0;
  float _netPower = 0;
  float _remainingWh = 0;
  float _remainingHours = 0;
  float _timeToFull = 0;
  float _cRate = 0;

  void _applyDefaultsForType();
  float _socFromVoltage(float v) const;
  void _updateCoulomb(uint32_t dtMs, bool chargingToFloat);
};