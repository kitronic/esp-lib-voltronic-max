#include "VoltronicParser.h"
#include <string.h>
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════
//  Numeric helpers
// ═══════════════════════════════════════════════════════════════
uint16_t VoltronicParser::strToU16x10(const char *s)
{
  uint32_t intPart = 0, fracPart = 0;
  uint8_t digits = 0;
  while (*s >= '0' && *s <= '9')
    intPart = intPart * 10 + (*s++ - '0');
  if (*s == '.')
  {
    s++;
    while (*s >= '0' && *s <= '9' && digits < 2)
    {
      fracPart = fracPart * 10 + (*s++ - '0');
      digits++;
    }
    if (digits == 1)
      fracPart *= 10;
  }
  return (uint16_t)(intPart * 10 + fracPart / 10);
}

uint16_t VoltronicParser::strToU16x100(const char *s)
{
  uint32_t intPart = 0, fracPart = 0;
  while (*s >= '0' && *s <= '9')
    intPart = intPart * 10 + (*s++ - '0');
  if (*s == '.')
  {
    s++;
    uint8_t digits = 0;
    while (*s >= '0' && *s <= '9' && digits < 2)
    {
      fracPart = fracPart * 10 + (*s++ - '0');
      digits++;
    }
    if (digits == 1)
      fracPart *= 10;
  }
  return (uint16_t)(intPart * 100 + fracPart);
}

uint16_t VoltronicParser::strToU16(const char *s)
{
  return (uint16_t)strtoul(s, nullptr, 10);
}

uint8_t VoltronicParser::strToU8(const char *s)
{
  return (uint8_t)strtoul(s, nullptr, 10);
}

// ═══════════════════════════════════════════════════════════════
//  QPGSn — Parallel info
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQPGSn(const char *payload, ParallelInfo &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[24];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };

  if (!next())
    return false;
  out.parallelNumber = (uint8_t)strtoul(tok, nullptr, 10);
  if (!next())
    return false;
  out.exists = (tok[0] == '1');
  if (!next())
    return false;
  strncpy(out.serialNumber, tok, sizeof(out.serialNumber) - 1);
  out.serialNumber[sizeof(out.serialNumber) - 1] = '\0';
  if (!next())
    return false;
  out.workMode = tok[0];
  if (!next())
    return false;
  out.faultCode = (uint8_t)strtoul(tok, nullptr, 10);
  if (!next())
    return false;
  out.gridVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.gridFrequency_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputFrequency_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputApparentPower = strToU16(tok);
  if (!next())
    return false;
  out.acOutputActivePower = strToU16(tok);
  if (!next())
    return false;
  out.loadPercent = strToU8(tok);
  if (!next())
    return false;
  out.batteryVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryChargingCurrent = strToU16(tok);
  if (!next())
    return false;
  out.batteryCapacity = strToU8(tok);
  if (!next())
    return false;
  out.pvInputVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.pvInputCurrent_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.totalChargingCurrent = strToU16(tok);
  if (!next())
    return false;
  out.totalACOutputApparentPower = strToU16(tok);
  if (!next())
    return false;
  out.totalOutputActivePower = strToU16(tok);
  if (!next())
    return false;
  out.totalACOutputPercentage = strToU8(tok);

  // Status bits b7..b0
  if (!next())
    return true;
  {
    uint8_t bits = 0;
    for (uint8_t i = 0; i < 8 && tok[i]; i++)
      if (tok[i] == '1')
        bits |= (1 << i);
    out.inverterStatus = bits;
  }

  if (!next())
    return true;
  out.outputMode = strToU8(tok);
  if (!next())
    return true;
  out.chargerSourcePriority = strToU8(tok);
  if (!next())
    return true;
  out.maxChargerCurrent = strToU8(tok);
  if (!next())
    return true;
  out.maxChargerRange = strToU8(tok);
  if (!next())
    return true;
  out.maxACChargerCurrent = strToU8(tok);
  if (!next())
    return true;
  out.pvInputCurrent2_x10 = strToU16x10(tok);

  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QBEQI — Battery equalization
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQBEQI(const char *payload, BatteryEqualizationInfo &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };

  if (!next())
    return false;
  out.enabled = (tok[0] == '1');
  if (!next())
    return false;
  out.timeMinutes = strToU16(tok);
  if (!next())
    return false;
  out.periodDays = strToU16(tok);
  if (!next())
    return false;
  out.voltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.overTimeMinutes = strToU16(tok);
  if (!next())
    return false;
  out.maxTimeMinutes = strToU16(tok);
  if (!next())
    return true;
  out.activeNow = strToU8(tok);

  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QLED — LED status
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQLED(const char *payload, LedInfo &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };

  if (!next())
    return false;
  out.enabled = (tok[0] == '1');
  if (!next())
    return false;
  out.speed = strToU8(tok);
  if (!next())
    return false;
  out.effect = strToU8(tok);
  if (!next())
    return false;
  out.brightness = strToU8(tok);
  if (!next())
    return false;
  out.colorCount = strToU8(tok);

  for (uint8_t i = 0; i < out.colorCount && i < 3; i++)
  {
    if (!next())
      break;
    out.red[i] = (uint8_t)strtoul(tok, nullptr, 10);
    if (!next())
      break;
    out.green[i] = (uint8_t)strtoul(tok, nullptr, 10);
    if (!next())
      break;
    out.blue[i] = (uint8_t)strtoul(tok, nullptr, 10);
  }
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QDI — Default settings
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQDI(const char *payload, DefaultsInfo &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };

  if (!next())
    return false;
  out.acOutputVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputFrequency_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.maxACChargingCurrent = strToU8(tok);
  if (!next())
    return false;
  out.batteryUnderVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryFloatVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryBulkVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryRechargeVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.maxChargingCurrent = strToU8(tok);
  if (!next())
    return false;
  out.acInputVoltageRange = strToU8(tok);
  if (!next())
    return false;
  out.outputSourcePriority = strToU8(tok);
  if (!next())
    return false;
  out.chargerSourcePriority = strToU8(tok);
  if (!next())
    return true;
  out.batteryType = strToU8(tok);

  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QBATCD
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQBATCDRaw(const char *payload, char &a, char &b, char &c)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;
  if (strlen(p) < 3)
    return false;
  a = p[0];
  b = p[1];
  c = p[2];
  return true;
}

bool VoltronicParser::parseQBATCD(const char *payload, BatteryControlStatus &out)
{
  char a, b, c;
  if (!parseQBATCDRaw(payload, a, b, c))
    return false;
  out.dischargeCompletely = (a == '1');
  out.dischargeAllowed = (b == '1');
  out.chargeCompletely = (c == '1');
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QMCHGCR / QMUCHGCR
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQMCHGCR(const char *payload, SelectableValues &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  out.count = 0;
  char tok[8];
  while (readToken(p, tok, sizeof(tok)) && out.count < SelectableValues::MAX_VALUES)
  {
    out.values[out.count++] = (uint8_t)strtoul(tok, nullptr, 10);
  }
  return out.count > 0;
}

bool VoltronicParser::parseQMUCHGCR(const char *payload, SelectableValues &out)
{
  return parseQMCHGCR(payload, out);
}

// ═══════════════════════════════════════════════════════════════
//  QOPPT / QCHPT
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQOPPT(const char *payload, TimeOrderInfo &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[8];
  for (uint8_t i = 0; i < 24; i++)
  {
    if (!readToken(p, tok, sizeof(tok)))
      return false;
    out.hours[i] = (uint8_t)strtoul(tok, nullptr, 10);
  }
  if (!readToken(p, tok, sizeof(tok)))
    return false;
  out.defaultVal = (uint8_t)strtoul(tok, nullptr, 10);
  if (!readToken(p, tok, sizeof(tok)))
    return false;
  out.order = (uint8_t)strtoul(tok, nullptr, 10);
  return true;
}

bool VoltronicParser::parseQCHPT(const char *payload, TimeOrderInfo &out)
{
  return parseQOPPT(payload, out);
}

// ═══════════════════════════════════════════════════════════════
//  QBOOT
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQBOOT(const char *payload, bool &hasBootstrap)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;
  hasBootstrap = (*p == '1');
  return true;
}

void VoltronicParser::skipSpaces(const char *&p)
{
  while (*p == ' ')
    p++;
}

bool VoltronicParser::readToken(const char *&p, char *out, size_t maxLen)
{
  skipSpaces(p);
  size_t i = 0;
  while (*p && *p != ' ' && i < maxLen - 1)
  {
    out[i++] = *p++;
  }
  out[i] = '\0';
  return i > 0;
}

bool VoltronicParser::readUInt(const char *&p, uint32_t &out, uint8_t digits)
{
  out = 0;
  for (uint8_t i = 0; i < digits; i++)
  {
    if (*p < '0' || *p > '9')
      return false;
    out = out * 10 + (uint32_t)(*p - '0');
    p++;
  }
  return true;
}

bool VoltronicParser::readBits(const char *&p, uint8_t n, uint32_t &out)
{
  out = 0;
  for (uint8_t i = 0; i < n; i++)
  {
    if (*p == '1')
      out |= (1UL << i);
    else if (*p != '0')
      return false;
    p++;
  }
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QPIGS — استعمل strToU16x10/100 مباشرة بدل lambda معطوبة
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQPIGS(const char *payload, QPIGSData &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };
  auto toU32 = [&](uint32_t &v)
  { v = (uint32_t)strtoul(tok, nullptr, 10); };

  uint32_t u;

  if (!next())
    return false;
  out.gridVoltage_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.gridFrequency_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.acOutputVoltage_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.acOutputFrequency_x10 = strToU16x10(tok);

  if (!next())
    return false;
  toU32(u);
  out.acOutputApparentPower = (uint16_t)u;

  if (!next())
    return false;
  toU32(u);
  out.acOutputActivePower = (uint16_t)u;

  if (!next())
    return false;
  toU32(u);
  out.loadPercent = (uint8_t)u;

  if (!next())
    return false;
  toU32(u);
  out.busVoltage = (uint16_t)u;

  if (!next())
    return false;
  out.batteryVoltage_x100 = strToU16x100(tok);

  if (!next())
    return false;
  toU32(u);
  out.batteryChargingCurrent = (uint16_t)u;

  if (!next())
    return false;
  toU32(u);
  out.batteryCapacity = (uint8_t)u;

  if (!next())
    return false;
  toU32(u);
  out.inverterTemperature = (uint16_t)u;

  if (!next())
    return false;
  out.pv1InputCurrent_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.pv1InputVoltage_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.sccBatteryVoltage_x100 = strToU16x100(tok);

  if (!next())
    return false;
  toU32(u);
  out.batteryDischargeCurrent = (uint16_t)u;

  // Status bits b7..b0
  if (!next())
    return false;
  {
    uint8_t bits = 0;
    for (uint8_t i = 0; i < 8 && tok[i]; i++)
      if (tok[i] == '1')
        bits |= (1 << i);
    out.deviceStatus = bits;
  }

  if (!next())
    return false;
  toU32(u);
  out.batteryVoltageOffsetForFans = (uint16_t)u;

  if (!next())
    return false;
  toU32(u);
  out.eepromVersion = (uint8_t)u;

  if (!next())
    return false;
  toU32(u);
  out.pv1ChargingPower = (uint16_t)u;

  // Status bits b10..b8
  if (!next())
  {
    out.deviceStatus2 = 0;
  }
  else
  {
    uint8_t bits = 0;
    for (uint8_t i = 0; i < 3 && tok[i]; i++)
      if (tok[i] == '1')
        bits |= (1 << i);
    out.deviceStatus2 = bits;
  }

  if (!next())
    return true;
  toU32(u);
  out.solarFeedToGridStatus = (uint8_t)u;

  if (!next())
    return true;
  toU32(u);
  out.countryRegulation = (uint8_t)u;

  if (!next())
    return true;
  toU32(u);
  out.solarFeedToGridPower = (uint16_t)u;

  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QPIGS2
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQPIGS2(const char *payload, QPIGS2Data &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };

  if (!next())
    return false;
  out.pv2InputCurrent_x10 = strToU16x10(tok);

  if (!next())
    return false;
  out.pv2InputVoltage_x10 = strToU16x10(tok);

  if (!next())
  {
    out.pv2ChargingPower = (uint16_t)((out.pv2InputVoltage_x10 / 10.0f) *
                                      (out.pv2InputCurrent_x10 / 10.0f));
    return true;
  }
  out.pv2ChargingPower = (uint16_t)strtoul(tok, nullptr, 10);
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QPIRI
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQPIRI(const char *payload, QPIRIData &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  char tok[16];
  auto next = [&]() -> bool
  { return readToken(p, tok, sizeof(tok)); };
  auto toU32 = [&](uint32_t &v)
  { v = (uint32_t)strtoul(tok, nullptr, 10); };

  uint32_t u;

  if (!next())
    return false;
  out.gridRatingVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.gridRatingCurrent_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputRatingVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputRatingFrequency_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.acOutputRatingCurrent_x10 = strToU16x10(tok);
  if (!next())
    return false;
  toU32(u);
  out.acOutputRatingApparentPower = (uint16_t)u;
  if (!next())
    return false;
  toU32(u);
  out.acOutputRatingActivePower = (uint16_t)u;
  if (!next())
    return false;
  out.batteryRatingVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryRechargeVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryUnderVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryBulkVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  out.batteryFloatVoltage_x10 = strToU16x10(tok);
  if (!next())
    return false;
  toU32(u);
  out.batteryType = (uint8_t)u;
  if (!next())
    return false;
  toU32(u);
  out.maxACChargingCurrent = (uint8_t)u;
  if (!next())
    return false;
  toU32(u);
  out.maxChargingCurrent = (uint8_t)u;
  if (!next())
    return false;
  toU32(u);
  out.inputVoltageRange = (uint8_t)u;
  if (!next())
    return false;
  toU32(u);
  out.outputSourcePriority = (uint8_t)u;
  if (!next())
    return false;
  toU32(u);
  out.chargerSourcePriority = (uint8_t)u;

  if (!next())
    return true;
  toU32(u);
  out.parallelMaxNum = (uint8_t)u;
  if (!next())
    return true;
  toU32(u);
  out.machineType = (uint8_t)u;
  if (!next())
    return true;
  toU32(u);
  out.outputMode = (uint8_t)u;
  if (!next())
    return true;
  out.batteryRedischargeVoltage_x10 = strToU16x10(tok);
  if (!next())
    return true;
  toU32(u);
  out.pvOKConditionForParallel = (uint8_t)u;
  if (!next())
    return true;
  toU32(u);
  out.pvPowerBalance = (uint8_t)u;

  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QMOD
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQMOD(const char *payload, char &modeOut)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;
  if (!*p)
    return false;
  modeOut = *p;
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QPIWS
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQPIWS(const char *payload, uint32_t &warningsOut)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  uint32_t flags = 0;
  for (uint8_t i = 0; i < 32 && p[i]; i++)
  {
    if (p[i] == '1')
      flags |= (1UL << i);
    else if (p[i] != '0')
      break;
  }
  warningsOut = flags;
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  QFLAG
// ═══════════════════════════════════════════════════════════════
bool VoltronicParser::parseQFLAG(const char *payload, QFLAGData &out)
{
  const char *p = payload;
  if (*p != '(')
    return false;
  p++;

  out.flags = 0;

  auto hasFlag = [&](char c) -> bool
  {
    bool enabled = false;
    for (const char *q = p; *q; q++)
    {
      if (*q == c && q > p)
      {
        char prev = *(q - 1);
        if (prev == 'E')
          enabled = true;
        else if (prev == 'D')
          enabled = false;
      }
    }
    return enabled;
  };

  if (hasFlag('a'))
    out.flags |= (1u << 0);
  if (hasFlag('b'))
    out.flags |= (1u << 1);
  if (hasFlag('d'))
    out.flags |= (1u << 2);
  if (hasFlag('j'))
    out.flags |= (1u << 3);
  if (hasFlag('k'))
    out.flags |= (1u << 4);
  if (hasFlag('u'))
    out.flags |= (1u << 5);
  if (hasFlag('v'))
    out.flags |= (1u << 6);
  if (hasFlag('x'))
    out.flags |= (1u << 7);
  if (hasFlag('y'))
    out.flags |= (1u << 8);
  if (hasFlag('z'))
    out.flags |= (1u << 9);

  return true;
}

const char *VoltronicParser::modeToString(char m)
{
  switch (m)
  {
  case 'P':
    return "PowerOn";
  case 'S':
    return "Standby";
  case 'L':
    return "Line";
  case 'B':
    return "Battery";
  case 'F':
    return "Fault";
  case 'H':
    return "PowerSaving";
  case 'D':
    return "Shutdown";
  default:
    return "Unknown";
  }
}