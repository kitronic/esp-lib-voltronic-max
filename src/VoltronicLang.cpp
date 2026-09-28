#include "VoltronicLang.h"

// ═══════════════════════════════════════════════════════════════
//  English table
// ═══════════════════════════════════════════════════════════════
const char *const VoltronicLang::TABLE_EN[] = {
    // Battery status
    "Charging",
    "Discharging",
    "Idle",
    // Battery types
    "User",
    "AGM",
    "Flooded",
    "Pylontech",
    "LiFePO4 15S",
    "LiFePO4 16S",
    // SC modes
    "Disabled",
    "Standard",
    "Fast",
    // SC stages
    "Disabled",
    "Temp Protect",
    "Float",
    "Near Full",
    "Charging",
    // SC statuses
    "Smart charging disabled",
    "Temp: AC to %uA",
    "Temp: Total to %uA",
    "Over-temp protection active",
    "Standard: AC to %uA (%.0f%%)",
    "Standard: AC to %uA",
    "Standard: Total to %uA (%.0f%%)",
    "Standard: Total to %uA",
    "Standard: steady @ %uA/%uA (%.0f%%)",
    "Fast: AC to %uA",
    "Fast: Total to %uA",
    "Fast: steady @ %uA/%uA",
    // Power modes
    "Normal",
    "Power Saving",
    "Emergency",
    "Solar Surplus",
    "Grid Surplus",
    "Combined Surplus",
    "Unknown",
    // Charging source
    "Solar + Grid",
    "Solar Only",
    "Grid Only",
    "Idle",
    // Warnings
    "OK",
    "Overload",
    "Over-temp",
    "Battery Low",
    "Grid Lost",
    "Fan Locked",
    "Warning",
    // Inverter modes
    "Power On",
    "Standby",
    "Line",
    "Battery",
    "Fault",
    "Power Saving",
    "Unknown",
    // Output priorities
    "Utility->Solar->Battery",
    "Solar->Utility->Battery",
    "Solar->Battery->Utility",
    // Charger priorities
    "Solar First",
    "Solar + Utility",
    "Only Solar",
    // Input ranges
    "Appliance",
    "UPS",
    // Common
    "Yes",
    "No",
    "On",
    "Off",
    "OK",
    "Error",
    "Locked",
    "Unlocked",
    // Charging status codes
    "No Charge",      // 0
    "Solar Charging", // 1
    "AC Charging",    // 2
    "Solar + AC",     // 3
    "Float",          // 4
    "Equalization",   // 5
    "Reserved",       // 6
    "Float/Timer",    // 7
};

// ═══════════════════════════════════════════════════════════════
//  Arabic table
// ═══════════════════════════════════════════════════════════════
const char *const VoltronicLang::TABLE_AR[] = {
    // Battery status
    "يشحن",
    "يفرغ",
    "ساكن",
    // Battery types
    "مخصص",
    "AGM",
    "سائل",
    "Pylontech",
    "LiFePO4 15S",
    "LiFePO4 16S",
    // SC modes
    "معطّل",
    "قياسي",
    "سريع",
    // SC stages
    "معطّل",
    "حماية حرارة",
    "تعويم",
    "قريب الامتلاء",
    "يشحن",
    // SC statuses
    "الشحن الذكي معطّل",
    "حماية: AC → %uA",
    "حماية: الإجمالي → %uA",
    "حماية الحرارة نشطة",
    "قياسي: AC → %uA (%.0f%%)",
    "قياسي: AC → %uA",
    "قياسي: الإجمالي → %uA (%.0f%%)",
    "قياسي: الإجمالي → %uA",
    "قياسي: مستقر @ %uA/%uA (%.0f%%)",
    "سريع: AC → %uA",
    "سريع: الإجمالي → %uA",
    "سريع: مستقر @ %uA/%uA",
    // Power modes
    "عادي",
    "توفير الطاقة",
    "طوارئ",
    "فائض شمسي",
    "فائض شبكة",
    "فائض مشترك",
    "غير معروف",
    // Charging source
    "شمسي + شبكة",
    "شمسي فقط",
    "شبكة فقط",
    "متوقف",
    // Warnings
    "سليم",
    "حمل زائد",
    "حرارة مرتفعة",
    "بطارية منخفضة",
    "انقطاع الشبكة",
    "مروحة متوقفة",
    "تحذير",
    // Inverter modes
    "تشغيل",
    "استعداد",
    "شبكة",
    "بطارية",
    "عطل",
    "توفير طاقة",
    "غير معروف",
    // Output priorities
    "شبكة → شمسي → بطارية",
    "شمسي → شبكة → بطارية",
    "شمسي → بطارية → شبكة",
    // Charger priorities
    "شمسي أولاً",
    "شمسي + شبكة",
    "شمسي فقط",
    // Input ranges
    "أجهزة",
    "UPS",
    // Common
    "نعم",
    "لا",
    "يعمل",
    "متوقف",
    "سليم",
    "خطأ",
    "مقفل",
    "غير مقفل",       
     // أكواد الشحن
    "لا شحن",          // 0
    "شحن شمسي",        // 1
    "شحن شبكة",        // 2
    "شحن شمسي + شبكة", // 3
    "تعويم",           // 4
    "معادلة",          // 5
    "محجوز",           // 6
    "تعويم / Timer",   // 7
};

// ═══════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════
const char *VoltronicLang::batteryTypeStr(uint8_t t) const
{
    static const Key keys[] = {
        BAT_TYPE_USER, BAT_TYPE_AGM, BAT_TYPE_FLOODED,
        BAT_TYPE_PYLON, BAT_TYPE_LIFEPO4_15, BAT_TYPE_LIFEPO4_16};
    return tr((t > 5) ? BAT_TYPE_USER : keys[t]);
}

const char *VoltronicLang::inverterModeStr(char m) const
{
    switch (m)
    {
    case 'P':
        return tr(INV_POWER_ON);
    case 'S':
        return tr(INV_STANDBY);
    case 'L':
        return tr(INV_LINE);
    case 'B':
        return tr(INV_BATTERY);
    case 'F':
        return tr(INV_FAULT);
    case 'H':
        return tr(INV_SAVING);
    default:
        return tr(INV_UNKNOWN);
    }
}

const char *VoltronicLang::outputPrioStr(uint8_t p) const
{
    switch (p)
    {
    case 0:
        return tr(PRIO_U_S_B);
    case 1:
        return tr(PRIO_S_U_B);
    case 2:
        return tr(PRIO_S_B_U);
    default:
        return "";
    }
}

const char *VoltronicLang::chargerPrioStr(uint8_t p) const
{
    switch (p)
    {
    case 1:
        return tr(CHG_SOLAR_FIRST);
    case 2:
        return tr(CHG_SOLAR_UTIL);
    case 3:
        return tr(CHG_SOLAR_ONLY);
    default:
        return "";
    }
}

const char *VoltronicLang::inputRangeStr(uint8_t r) const
{
    return (r == 0) ? tr(RANGE_APPLIANCE) : tr(RANGE_UPS);
}

const char *VoltronicLang::powerModeStr(uint8_t m) const
{
    switch (m)
    {
    case 1:
        return tr(PM_NORMAL);
    case 2:
        return tr(PM_SAVING);
    case 3:
        return tr(PM_EMERGENCY);
    case 4:
        return tr(PM_SOLAR);
    case 5:
        return tr(PM_GRID);
    case 6:
        return tr(PM_COMBINED);
    default:
        return tr(PM_UNKNOWN);
    }
}