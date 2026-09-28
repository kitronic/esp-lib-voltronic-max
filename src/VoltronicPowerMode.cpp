#include "VoltronicPowerMode.h"
#include "VoltronicMAX.h"
#include "VoltronicLang.h"
VoltronicPowerMode::VoltronicPowerMode() {}

// ═══════════════════════════════════════════════════════════════
//  التحديث — كل العتبات من إعدادات المستخدم
// ═══════════════════════════════════════════════════════════════
void VoltronicPowerMode::update(VoltronicMAX &inv, const VoltronicBattery &bat)
{
    const QPIGSData &g = inv.qpigs();
    const QPIGS2Data &g2 = inv.qpigs2();

    float soc = bat.soc();

    // ✅ عتبة الشبكة من الإعدادات
    bool gridOk = g.gridVoltage() > _gridMinVoltage;

    bool discharging = g.batteryDischargeCurrent > 2;

    // ✅ فحص PV1 + PV2
    bool hasSolar = (g.pv1ChargingPower > 50) ||
                    (g2.pv2ChargingPower > 50);

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
            _mode = EMERGENCY_MAX;
            _locked = true;
        }
        // POWER_SAVING لو:
        //   SOC < _socPowerSaving
        else if (soc < _socPowerSaving)
        {
            _mode = POWER_SAVING;
            _locked = false;
        }
        // NORMAL لو:
        //   SOC > _socPowerSaving
        else
        {
            _mode = NORMAL;
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
    if (_mode == SOLAR_SURPLUS || _mode == GRID_SURPLUS || _mode == COMBINED_SURPLUS)
    {
        if (soc >= (_socSurplus - 3.0f) && !sustained)
            full = true;
    }

    if (full && gridOk && hasSolar)
        _mode = COMBINED_SURPLUS;
    else if (full && hasSolar)
        _mode = SOLAR_SURPLUS;
    else if (full && gridOk)
        _mode = GRID_SURPLUS;
    else
        _mode = NORMAL;
}

// ═══════════════════════════════════════════════════════════════
//  Names
// ═══════════════════════════════════════════════════════════════
const char *VoltronicPowerMode::modeNameAr() const
{
    switch (_mode)
    {
    case NORMAL:
        return "عادي";
    case POWER_SAVING:
        return "توفير الطاقة";
    case EMERGENCY_MAX:
        return "طوارئ";
    case SOLAR_SURPLUS:
        return "فائض شمسي";
    case GRID_SURPLUS:
        return "فائض شبكة";
    case COMBINED_SURPLUS:
        return "فائض مشترك";
    default:
        return "غير معروف";
    }
}

const char *VoltronicPowerMode::modeNameEn() const
{
    switch (_mode)
    {
    case NORMAL:
        return "Normal";
    case POWER_SAVING:
        return "Power Saving";
    case EMERGENCY_MAX:
        return "Emergency";
    case SOLAR_SURPLUS:
        return "Solar Surplus";
    case GRID_SURPLUS:
        return "Grid Surplus";
    case COMBINED_SURPLUS:
        return "Combined Surplus";
    default:
        return "Unknown";
    }
}