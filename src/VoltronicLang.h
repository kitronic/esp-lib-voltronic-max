#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  VoltronicLang — عربي/إنجليزي مركزي
//  ⚠️ احفظ الملف UTF-8 بدون BOM
// ═══════════════════════════════════════════════════════════════
class VoltronicLang
{
 public:
  enum Language : uint8_t
  {
    ENGLISH = 0,
    ARABIC = 1,
  };

  enum Key : uint8_t
  {
    // Battery status
    BAT_CHARGING = 0,
    BAT_DISCHARGING,
    BAT_IDLE,
    // Battery types
    BAT_TYPE_USER,
    BAT_TYPE_AGM,
    BAT_TYPE_FLOODED,
    BAT_TYPE_PYLON,
    BAT_TYPE_LIFEPO4_15,
    BAT_TYPE_LIFEPO4_16,
    // Smart Charger modes
    SC_MODE_OFF,
    SC_MODE_STD,
    SC_MODE_FAST,
    // Smart Charger stages
    SC_STAGE_OFF,
    SC_STAGE_TEMP,
    SC_STAGE_FLOAT,
    SC_STAGE_NEAR,
    SC_STAGE_CHARGE,
    // Smart Charger status (format strings)
    SC_STATUS_OFF,
    SC_STATUS_TEMP_AC,
    SC_STATUS_TEMP_TOTAL,
    SC_STATUS_TEMP_ACTIVE,
    SC_STATUS_STD_AC_DN,
    SC_STATUS_STD_AC_UP,
    SC_STATUS_STD_T_DN,
    SC_STATUS_STD_T_UP,
    SC_STATUS_STD_STEADY,
    SC_STATUS_FAST_AC,
    SC_STATUS_FAST_TOTAL,
    SC_STATUS_FAST_STEADY,
    // Power modes
    PM_NORMAL,
    PM_SAVING,
    PM_EMERGENCY,
    PM_SOLAR,
    PM_GRID,
    PM_COMBINED,
    PM_UNKNOWN,
    // Charging source
    SRC_SOLAR_GRID,
    SRC_SOLAR,
    SRC_GRID,
    SRC_IDLE,
    // Warnings
    WARN_OK,
    WARN_OVERLOAD,
    WARN_OVERTEMP,
    WARN_BATT_LOW,
    WARN_GRID_LOST,
    WARN_FAN,
    WARN_GENERAL,
    // Inverter modes
    INV_POWER_ON,
    INV_STANDBY,
    INV_LINE,
    INV_BATTERY,
    INV_FAULT,
    INV_SAVING,
    INV_UNKNOWN,
    // Output priorities
    PRIO_U_S_B,
    PRIO_S_U_B,
    PRIO_S_B_U,
    // Charger priorities
    CHG_SOLAR_FIRST,
    CHG_SOLAR_UTIL,
    CHG_SOLAR_ONLY,
    // Input ranges
    RANGE_APPLIANCE,
    RANGE_UPS,
    // Common
    YN_YES,
    YN_NO,
    ON_OFF_ON,
    ON_OFF_OFF,
    SAVE_OK,
    SAVE_ERROR,
    LOCKED,
    UNLOCKED,
    // Charging Status codes (QPIGS deviceStatus bits 2-0)
    CHG_CODE_0_NONE,
    CHG_CODE_1_SOLAR,
    CHG_CODE_2_AC,
    CHG_CODE_3_SOLAR_AC,
    CHG_CODE_4_FLOAT,
    CHG_CODE_5_EQ,
    CHG_CODE_6_RESERVED,
    CHG_CODE_7_FLOAT_TIMER,
    _KEY_COUNT
  };

  VoltronicLang()
    : _lang(ENGLISH)
  {
  }

  // ─── الإعداد ───
  void setLanguage(Language l)
  {
    _lang = (l == ARABIC) ? ARABIC : ENGLISH;
  }
  Language language() const
  {
    return _lang;
  }
  bool isArabic() const
  {
    return _lang == ARABIC;
  }

  // ─── الترجمة ───
  const char* tr(Key k) const
  {
    if (k >= _KEY_COUNT)
      return "?";
    return (_lang == ARABIC) ? TABLE_AR[k] : TABLE_EN[k];
  }

  // ─── تحويلات جاهزة ───
  const char* batteryTypeStr(uint8_t t) const;
  const char* inverterModeStr(char m) const;
  const char* outputPrioStr(uint8_t p) const;
  const char* chargerPrioStr(uint8_t p) const;
  const char* inputRangeStr(uint8_t r) const;
  const char* powerModeStr(
      uint8_t m) const;  // ─── Charging Status Code (bits 2-0 من QPIGS.deviceStatus) ───
  const char* chargingCodeStr(uint8_t code) const
  {
    switch (code)
    {
      case 0:
        return tr(CHG_CODE_0_NONE);
      case 1:
        return tr(CHG_CODE_1_SOLAR);
      case 2:
        return tr(CHG_CODE_2_AC);
      case 3:
        return tr(CHG_CODE_3_SOLAR_AC);
      case 4:
        return tr(CHG_CODE_4_FLOAT);
      case 5:
        return tr(CHG_CODE_5_EQ);
      case 6:
        return tr(CHG_CODE_6_RESERVED);
      case 7:
        return tr(CHG_CODE_7_FLOAT_TIMER);
      default:
        return "?";
    }
  }

 private:
  Language _lang;

  static const char* const TABLE_EN[_KEY_COUNT];
  static const char* const TABLE_AR[_KEY_COUNT];
};