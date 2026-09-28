#pragma once
#include <Arduino.h>
#include "VoltronicBattery.h"

class VoltronicMAX;

class VoltronicPowerMode
{
 public:
  enum Mode : uint8_t
  {
    PM_UNKNOWN = 0,
    PM_NORMAL = 1,
    PM_POWER_SAVING = 2,
    PM_EMERGENCY_MAX = 3,
    PM_SOLAR_SURPLUS = 4,
    PM_GRID_SURPLUS = 5,
    PM_COMBINED_SURPLUS = 6,
  };

  VoltronicPowerMode();

  // ═══════════════════════════════════════════════════════════
  //  إعدادات المستخدم (نِسَب SOC)
  // ═══════════════════════════════════════════════════════════
  void setSocEmergency(float pct)
  {
    _socEmergency = _clamp(pct, 5, 40);
  }
  void setSocPowerSaving(float pct)
  {
    _socPowerSaving = _clamp(pct, 10, 50);
  }
  void setSocRecover(float pct)
  {
    _socRecover = _clamp(pct, 20, 60);
  }
  void setSocSurplus(float pct)
  {
    _socSurplus = _clamp(pct, 70, 99);
  }
  void setGridMinVoltage(float v)
  {
    _gridMinVoltage = _clamp(v, 100, 220);
  }

  float socEmergency() const
  {
    return _socEmergency;
  }
  float socPowerSaving() const
  {
    return _socPowerSaving;
  }
  float socRecover() const
  {
    return _socRecover;
  }
  float socSurplus() const
  {
    return _socSurplus;
  }
  float gridMinVoltage() const
  {
    return _gridMinVoltage;
  }

  // ─── التحديث ───
  void update(VoltronicMAX& inv, const VoltronicBattery& bat);

  // ─── الحالة ───
  Mode mode() const
  {
    return _mode;
  }
  bool emergencyLocked() const
  {
    return _locked;
  }
  void setEmergencyLocked(bool b)
  {
    _locked = b;
  }
  void resetLock()
  {
    _locked = false;
    _dischargeStart = 0;
  }

  const char* modeNameEn() const;
  const char* modeNameAr() const;

 private:
  Mode _mode = PM_NORMAL;
  bool _locked = false;
  unsigned long _dischargeStart = 0;

  // ─── عتبات SOC (%) ───
  float _socEmergency = 10.0f;    // أقل من هذا → طوارئ
  float _socPowerSaving = 25.0f;  // أقل من هذا → توفير طاقة
  float _socRecover = 30.0f;      // أعلى من هذا → عودة للعادي
  float _socSurplus = 88.0f;      // أعلى من هذا → فائض

  // ─── عتبة جهد الشبكة ───
  float _gridMinVoltage = 150.0f;

  static float _clamp(float v, float lo, float hi)
  {
    if (v < lo)
      return lo;
    if (v > hi)
      return hi;
    return v;
  }
};