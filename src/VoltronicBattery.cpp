#include "VoltronicBattery.h"
#include <math.h>

// ═══════════════════════════════════════════════════════════════
//  منحنيات الجهد → SOC
// ═══════════════════════════════════════════════════════════════

static const float CURVE_LIFEPO4_15S[][2] = {
    {42.0f, 0},
    {46.0f, 10},
    {48.0f, 20},
    {49.5f, 30},
    {50.0f, 40},
    {50.5f, 50},
    {51.0f, 60},
    {51.5f, 70},
    {52.0f, 80},
    {53.0f, 90},
    {54.0f, 100},
};

static const float CURVE_LIFEPO4_16S[][2] = {
    {44.8f, 0},
    {49.1f, 10},
    {51.2f, 20},
    {52.8f, 30},
    {53.3f, 40},
    {53.9f, 50},
    {54.4f, 60},
    {54.9f, 70},
    {55.5f, 80},
    {56.5f, 90},
    {57.6f, 100},
};

static const float CURVE_LEADACID_48V[][2] = {
    {45.6f, 0},
    {46.8f, 10},
    {47.5f, 20},
    {48.0f, 30},
    {48.4f, 40},
    {48.8f, 50},
    {49.2f, 60},
    {49.6f, 70},
    {50.0f, 80},
    {50.4f, 90},
    {50.9f, 100},
};

// ═══════════════════════════════════════════════════════════════
VoltronicBattery::VoltronicBattery()
{
  setType(VoltronicBatteryType::User);
}

// ═══════════════════════════════════════════════════════════════
//  الإعدادات
// ═══════════════════════════════════════════════════════════════
void VoltronicBattery::setType(VoltronicBatteryType t)
{
  _type = t;
  _applyDefaultsForType();
}

void VoltronicBattery::_applyDefaultsForType()
{
  switch (_type)
  {
    case VoltronicBatteryType::LiFePO4_15S:
      _nominalV = 48.0f;
      _voltageEmpty = 42.0f;
      _voltageFull = 54.0f;
      break;
    case VoltronicBatteryType::LiFePO4_16S:
    case VoltronicBatteryType::Pylontech:
      _nominalV = 51.2f;
      _voltageEmpty = 44.8f;
      _voltageFull = 57.6f;
      break;
    case VoltronicBatteryType::AGM:
    case VoltronicBatteryType::Flooded:
      _nominalV = 48.0f;
      _voltageEmpty = 45.6f;
      _voltageFull = 50.9f;
      break;
    default:
      break;
  }
}

void VoltronicBattery::setCapacity(float ah)
{
  if (ah < 10.0f)
    ah = 10.0f;
  if (ah > 2000.0f)
    ah = 2000.0f;
  _capacityAh = ah;
}
void VoltronicBattery::setVoltageEmpty(float v)
{
  _voltageEmpty = v;
}
void VoltronicBattery::setVoltageFull(float v)
{
  _voltageFull = v;
}
void VoltronicBattery::setNominalVoltage(float v)
{
  _nominalV = v;
}
void VoltronicBattery::setInitialSoh(float s)
{
  _soh = constrain(s, 0.0f, 100.0f);
}
void VoltronicBattery::setCycleCount(uint16_t c)
{
  _cycleCount = c;
}

// ═══════════════════════════════════════════════════════════════
//  التحديث
// ═══════════════════════════════════════════════════════════════
void VoltronicBattery::update(const QPIGSData& g)
{
  if (!_enabled)
    return;

  // ─── 1. القراءة الخام ───
  _rawVoltage = g.batteryVoltage();
  _chargeCurrent = g.batteryChargingCurrent;
  _dischargeCurrent = g.batteryDischargeCurrent;

  // ─── 2. صافي التيار (صح، مش if) ───
  _netCurrent = (float)_chargeCurrent - (float)_dischargeCurrent;

  // ─── 3. حماية: الجهد خارج النطاق المعقول ───
  const float vMin = _voltageEmpty - 6.0f;
  const float vMax = _voltageFull + 6.0f;
  if (_rawVoltage < vMin || _rawVoltage > vMax)
  {
    _voltageOutOfRange = true;
    // لا نحدّث SOC، لكن نحدّث التيارات
    _netPower = _smoothedVoltage * _netCurrent;
    return;
  }
  _voltageOutOfRange = false;

  // ─── 4. حماية القفزات المفاجئة ───
  float vJumped = _rawVoltage;
  if (_hasFirstReading)
  {
    float diff = _rawVoltage - _smoothedVoltage;
    if (fabsf(diff) > _maxVoltageJump)
    {
      // نقصّ التغيير لأقصى مسموح في اتجاه واحد
      vJumped = _smoothedVoltage + (_maxVoltageJump * (diff > 0 ? 1.0f : -1.0f));
    }
  }

  // ─── 5. تعويض المقاومة الداخلية (IR Compensation) ───
  //  V_ocv ≈ V_measured − (I_net × R_system)
  //  عند الشحن (I_net > 0): V_measured أعلى → ننقص
  //  عند التفريغ (I_net < 0): V_measured أقل → نزيد
  _compVoltage = vJumped;
  if (_systemResistance > 0.001f)
  {
    _compVoltage = vJumped - (_netCurrent * _systemResistance);
  }

  // ─── 6. EMA Smoothing على الجهد (وليس على SOC) ───
  if (!_hasFirstReading)
  {
    _smoothedVoltage = _compVoltage;
    _hasFirstReading = true;
  }
  else
  {
    _smoothedVoltage = _smoothedVoltage * (1.0f - _emaAlpha) + _compVoltage * _emaAlpha;
  }

  // ─── 7. SOC من الجهد الناعم ───
  _soc = _socFromVoltage(_smoothedVoltage);

  // ─── 8. صافي القدرة ───
  _netPower = _smoothedVoltage * _netCurrent;

  // ─── 9. الطاقة المتبقية (بـ nominalV وليس voltage الحالي) ───
  _remainingWh = (_soc / 100.0f) * _capacityAh * _nominalV;

  // ─── 10. C-Rate ───
  _cRate = fabsf(_netCurrent) / _capacityAh;

  // ─── 11. ساعات التفريغ ───
  if (_netPower < -5.0f)
    _remainingHours = _remainingWh / (-_netPower);
  else
    _remainingHours = 0.0f;

  // ─── 12. ساعات حتى الامتلاء ───
  if (_netCurrent > 0.5f && _soc < 100.0f)
  {
    float remainingAh = (1.0f - _soc / 100.0f) * _capacityAh;
    _timeToFull = remainingAh / _netCurrent;
  }
  else
  {
    _timeToFull = 0.0f;
  }
}

// ═══════════════════════════════════════════════════════════════
//  SOC من الجهد
// ═══════════════════════════════════════════════════════════════
float VoltronicBattery::_socFromVoltage(float v) const
{
  const float (*curve)[2] = nullptr;
  size_t n = 0;

  switch (_type)
  {
    case VoltronicBatteryType::LiFePO4_15S:
      curve = CURVE_LIFEPO4_15S;
      n = sizeof(CURVE_LIFEPO4_15S) / sizeof(CURVE_LIFEPO4_15S[0]);
      break;
    case VoltronicBatteryType::LiFePO4_16S:
    case VoltronicBatteryType::Pylontech:
      curve = CURVE_LIFEPO4_16S;
      n = sizeof(CURVE_LIFEPO4_16S) / sizeof(CURVE_LIFEPO4_16S[0]);
      break;
    case VoltronicBatteryType::AGM:
    case VoltronicBatteryType::Flooded:
      curve = CURVE_LEADACID_48V;
      n = sizeof(CURVE_LEADACID_48V) / sizeof(CURVE_LEADACID_48V[0]);
      break;
    default:
      if (v <= _voltageEmpty)
        return 0.0f;
      if (v >= _voltageFull)
        return 100.0f;
      return (v - _voltageEmpty) / (_voltageFull - _voltageEmpty) * 100.0f;
  }

  if (v <= curve[0][0])
    return 0.0f;
  if (v >= curve[n - 1][0])
    return 100.0f;

  for (size_t i = 0; i < n - 1; i++)
  {
    float v1 = curve[i][0];
    float v2 = curve[i + 1][0];
    if (v >= v1 && v <= v2)
    {
      float soc1 = curve[i][1];
      float soc2 = curve[i + 1][1];
      float t = (v - v1) / (v2 - v1);
      return soc1 + t * (soc2 - soc1);
    }
  }
  return 0.0f;
}

// ═══════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════
const char* VoltronicBattery::status() const
{
  if (_netCurrent > 0.5f)
    return "Charging";
  if (_netCurrent < -0.5f)
    return "Discharging";
  return "Idle";
}

const char* VoltronicBattery::typeName() const
{
  return typeToString(_type);
}

const char* VoltronicBattery::typeToString(VoltronicBatteryType t)
{
  switch (t)
  {
    case VoltronicBatteryType::User:
      return "User";
    case VoltronicBatteryType::AGM:
      return "AGM";
    case VoltronicBatteryType::Flooded:
      return "Flooded";
    case VoltronicBatteryType::Pylontech:
      return "Pylontech";
    case VoltronicBatteryType::LiFePO4_15S:
      return "LiFePO4 15S";
    case VoltronicBatteryType::LiFePO4_16S:
      return "LiFePO4 16S";
    default:
      return "Unknown";
  }
}

VoltronicBatteryType VoltronicBattery::typeFromString(const char* s)
{
  if (!s)
    return VoltronicBatteryType::User;
  if (strcmp(s, "AGM") == 0)
    return VoltronicBatteryType::AGM;
  if (strcmp(s, "Flooded") == 0)
    return VoltronicBatteryType::Flooded;
  if (strcmp(s, "Pylontech") == 0)
    return VoltronicBatteryType::Pylontech;
  if (strcmp(s, "LiFePO4 15S") == 0)
    return VoltronicBatteryType::LiFePO4_15S;
  if (strcmp(s, "LiFePO4 16S") == 0)
    return VoltronicBatteryType::LiFePO4_16S;
  return VoltronicBatteryType::User;
}