#include "VoltronicBattery.h"
#include <math.h>

// ═══════════════════════════════════════════════════════════════
//  منحنيات SOC
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
  // إعادة حساب remainingAh بناءً على SOC الحالي
  _remainingAh = (_soc / 100.0f) * _capacityAh;
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
//  ⚡ Coulomb Counting — تحديث SOC من تكامل التيار
// ═══════════════════════════════════════════════════════════════
void VoltronicBattery::_updateCoulomb(uint32_t dtMs, bool chargingToFloat)
{
  if (!_useCoulomb)
    return;

  // تهيئة أولى
  if (!_coulombInitialized)
  {
    if (_soc > 0.0f)
      _remainingAh = (_soc / 100.0f) * _capacityAh;
    else
    {
      // أول قراءة — استخدم الجهد
      _remainingAh = (_socFromVoltage(_smoothedVoltage) / 100.0f) * _capacityAh;
      _soc = (_remainingAh / _capacityAh) * 100.0f;
    }
    _coulombInitialized = true;
    return;
  }

  // تكامل التيار: Ah = A × h = A × (ms / 3600000)
  float dtHours = dtMs / 3600000.0f;
  float deltaAh = _netCurrent * dtHours;

  _remainingAh += deltaAh;

  // حدود
  if (_remainingAh > _capacityAh)
    _remainingAh = _capacityAh;
  if (_remainingAh < 0)
    _remainingAh = 0;

  _soc = (_remainingAh / _capacityAh) * 100.0f;

  const bool stableSample = dtMs > 0 && dtMs <= 15000;
  const bool fullCondition = stableSample && chargingToFloat &&
                             _rawVoltage >= _voltageFull - 0.3f && _netCurrent >= -0.5f &&
                             _netCurrent < 2.0f;
  if (fullCondition)
  {
    if (_fullVoltageMs < 60000)
      _fullVoltageMs += dtMs;
    if (_fullVoltageMs > 60000)
      _fullVoltageMs = 60000;
  }
  else
    _fullVoltageMs = 0;

  if (_fullVoltageMs >= 60000)
  {
    _soc = 100.0f;
    _remainingAh = _capacityAh;
  }

  const bool emptyCondition =
      stableSample && _rawVoltage <= _voltageEmpty + 0.2f && _netCurrent <= 0.5f;
  if (emptyCondition)
  {
    if (_emptyVoltageMs < 60000)
      _emptyVoltageMs += dtMs;
    if (_emptyVoltageMs > 60000)
      _emptyVoltageMs = 60000;
  }
  else
    _emptyVoltageMs = 0;

  if (_emptyVoltageMs >= 60000)
  {
    _soc = 0.0f;
    _remainingAh = 0.0f;
  }
}

// ═══════════════════════════════════════════════════════════════
//  التحديث الرئيسي
// ═══════════════════════════════════════════════════════════════
void VoltronicBattery::update(const QPIGSData& g, uint32_t dtMs)
{
  if (!_enabled)
  {
    _fullVoltageMs = 0;
    _emptyVoltageMs = 0;
    return;
  }

  // ⚡ حساب dt
  uint32_t now = millis();
  if (_firstUpdate)
  {
    _lastUpdateMs = now;
    _firstUpdate = false;
    dtMs = 0;
  }
  else
  {
    dtMs = now - _lastUpdateMs;
    _lastUpdateMs = now;
    if (dtMs > 60000)
      dtMs = 60000;  // حد أقصى 60 ثانية (تجاهل القفزات)
  }

  _rawVoltage = g.batteryVoltage();
  _chargeCurrent = g.batteryChargingCurrent;
  _dischargeCurrent = g.batteryDischargeCurrent;
  _netCurrent = (float)_chargeCurrent - (float)_dischargeCurrent;

  // ═══ حماية جهد ═══
  const float vMin = _voltageEmpty - 6.0f;
  const float vMax = _voltageFull + 6.0f;
  if (_rawVoltage < vMin || _rawVoltage > vMax)
  {
    _voltageOutOfRange = true;
    _fullVoltageMs = 0;
    _emptyVoltageMs = 0;
    _netPower = _smoothedVoltage * _netCurrent;
    return;
  }
  _voltageOutOfRange = false;

  // ═══ Jump Protection ═══
  float vJumped = _rawVoltage;
  if (_hasFirstReading)
  {
    float diff = _rawVoltage - _smoothedVoltage;
    if (fabsf(diff) > _maxVoltageJump)
      vJumped = _smoothedVoltage + (_maxVoltageJump * (diff > 0 ? 1.0f : -1.0f));
  }

  // ═══ IR Compensation ═══
  _compVoltage = vJumped;
  if (_systemResistance > 0.001f)
    _compVoltage = vJumped - (_netCurrent * _systemResistance);

  // ═══ EMA Smoothing ═══
  if (!_hasFirstReading)
  {
    _smoothedVoltage = _compVoltage;
    _hasFirstReading = true;
  }
  else
  {
    _smoothedVoltage = _smoothedVoltage * (1.0f - _emaAlpha) + _compVoltage * _emaAlpha;
  }

  // ═══════════════════════════════════════════════════════════
  //  ⚡ SOC: Coulomb Counting + Voltage
  // ═══════════════════════════════════════════════════════════
  if (_useCoulomb)
  {
    _updateCoulomb(dtMs, g.sb2ChargingToFloat());
  }
  else
  {
    _fullVoltageMs = 0;
    _emptyVoltageMs = 0;
    _soc = _socFromVoltage(_smoothedVoltage);
    _remainingAh = (_soc / 100.0f) * _capacityAh;
  }

  // ═══ حسابات إضافية ═══
  _netPower = _smoothedVoltage * _netCurrent;
  _remainingWh = (_soc / 100.0f) * _capacityAh * _nominalV;
  _cRate = fabsf(_netCurrent) / _capacityAh;

  if (_netPower < -5.0f)
    _remainingHours = _remainingWh / (-_netPower);
  else
    _remainingHours = 0.0f;

  if (_netCurrent > 0.5f && _soc < 100.0f)
  {
    float remainingAhToFull = (1.0f - _soc / 100.0f) * _capacityAh;
    _timeToFull = remainingAhToFull / _netCurrent;
  }
  else
  {
    _timeToFull = 0.0f;
  }
}

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