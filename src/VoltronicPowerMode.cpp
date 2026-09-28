#include "VoltronicPowerMode.h"
#include "VoltronicLang.h"
#include "VoltronicMAX.h"
VoltronicPowerMode::VoltronicPowerMode()
{
}

// ═══════════════════════════════════════════════════════════════
//  التحديث — كل العتبات من إعدادات المستخدم
// ═══════════════════════════════════════════════════════════════
void VoltronicPowerMode::update(VoltronicMAX& inv, const VoltronicBattery& bat)
{
  const QPIGSData& g = inv.qpigs();
  const QPIGS2Data& g2 = inv.qpigs2();

  float soc = bat.soc();

  // ✅ عتبة الشبكة من الإعدادات
  bool gridOk = g.gridVoltage() > _gridMinVoltage;

  bool discharging = g.batteryDischargeCurrent > 2;

  // ✅ فحص PV1 + PV2
  bool hasSolar = (g.pv1ChargingPower > 50) || (g2.pv2ChargingPower > 50);

  bool isFloat = g.sb2ChargingToFloat();
  bool readyFull = (soc >= _socSurplus) || isFloat;

  // ─── تتبع التفريغ المستمر ───
  if (g.batteryDischargeCurrent > 8)
  {
    if (_dischargeStart == 0)
      _dischargeStart = millis();
  }
  else
  {
    _dischargeStart = 0;
  }
  bool sustained = (_dischargeStart > 0) && (millis() - _dischargeStart > 60000);

  // ═══════════════════════════════════════════════════════════
  //  Emergency — حسب إعدادات المستخدم
  // ═══════════════════════════════════════════════════════════
  if (!gridOk)
  {
    // أدخل EMERGENCY لو:
    //   SOC < _socEmergency  → دائماً
    //   _locked AND SOC < _socRecover → نبقى مقفولين
    if (soc < _socEmergency || (_locked && soc < _socRecover))
    {
      _mode = PM_EMERGENCY_MAX;
      _locked = true;
    }
    // POWER_SAVING لو:
    //   SOC < _socPowerSaving
    else if (soc < _socPowerSaving)
    {
      _mode = PM_POWER_SAVING;
      _locked = false;
    }
    // NORMAL لو:
    //   SOC > _socPowerSaving
    else
    {
      _mode = PM_NORMAL;
      _locked = false;
    }
    return;
  }
  _locked = false;

  // ═══════════════════════════════════════════════════════════
  //  Surplus / Normal
  // ═══════════════════════════════════════════════════════════
  bool full = readyFull && !discharging;

  // منع القفز
  if (_mode == PM_SOLAR_SURPLUS || _mode == PM_GRID_SURPLUS || _mode == PM_COMBINED_SURPLUS)
  {
    if (soc >= (_socSurplus - 3.0f) && !sustained)
      full = true;
  }

  if (full && gridOk && hasSolar)
    _mode = PM_COMBINED_SURPLUS;
  else if (full && hasSolar)
    _mode = PM_SOLAR_SURPLUS;
  else if (full && gridOk)
    _mode = PM_GRID_SURPLUS;
  else
    _mode = PM_NORMAL;
}

// ═══════════════════════════════════════════════════════════════
//  Names
// ═══════════════════════════════════════════════════════════════
const char* VoltronicPowerMode::modeNameAr() const
{
  switch (_mode)
  {
    case PM_NORMAL:
      return "عادي";
    case PM_POWER_SAVING:
      return "توفير الطاقة";
    case PM_EMERGENCY_MAX:
      return "طوارئ";
    case PM_SOLAR_SURPLUS:
      return "فائض شمسي";
    case PM_GRID_SURPLUS:
      return "فائض شبكة";
    case PM_COMBINED_SURPLUS:
      return "فائض مشترك";
    default:
      return "غير معروف";
  }
}

const char* VoltronicPowerMode::modeNameEn() const
{
  switch (_mode)
  {
    case PM_NORMAL:
      return "Normal";
    case PM_POWER_SAVING:
      return "Power Saving";
    case PM_EMERGENCY_MAX:
      return "Emergency";
    case PM_SOLAR_SURPLUS:
      return "Solar Surplus";
    case PM_GRID_SURPLUS:
      return "Grid Surplus";
    case PM_COMBINED_SURPLUS:
      return "Combined Surplus";
    default:
      return "Unknown";
  }
}