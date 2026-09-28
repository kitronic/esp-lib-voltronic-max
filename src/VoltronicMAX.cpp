#include "VoltronicMAX.h"
#include <string.h>
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════
VoltronicMAX::VoltronicMAX(Stream &serial)
    : _transport(serial) {}

// ═══════════════════════════════════════════════════════════════
//  Initialization
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::begin(const VoltronicConfig &cfg)
{
  _localCfg = cfg;
  _transport.attachConfig(&_localCfg);
  _transport.clear();
  storage.begin(512);  // ← جديد
  storage.load(*this); // ← جديد
  return true;
}

bool VoltronicMAX::begin(uint32_t baud)
{
  VoltronicConfig cfg;
  cfg.baud = baud;
  return begin(cfg);
}

void VoltronicMAX::setTimeout(uint16_t ms) { _localCfg.responseTimeoutMs = ms; }
void VoltronicMAX::setRetries(uint8_t r) { _localCfg.retries = r; }
// ═══════════════════════════════════════════════════════════════
//  Error name helper
// ═══════════════════════════════════════════════════════════════
const char *VoltronicMAX::lastErrorName() const
{
  switch (_lastError)
  {
  case ERR_NONE:
    return "NONE";
  case ERR_TIMEOUT:
    return "TIMEOUT";
  case ERR_SHORT:
    return "SHORT";
  case ERR_CRC:
    return "CRC";
  case ERR_NAK:
    return "NAK";
  case ERR_BAD_RESP:
    return "BAD_RESP";
  case ERR_PARSE:
    return "PARSE";
  case ERR_BUSY:
    return "BUSY";
  case ERR_TOO_LONG:
    return "TOO_LONG";
  default:
    return "UNKNOWN";
  }
}
// ═══════════════════════════════════════════════════════════════
//  Frame building
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::buildFrame(const char *cmd, uint8_t *out, size_t &outLen)
{
  size_t cmdLen = strlen(cmd);

  // out buffer = VOLTRONIC_CMD_BUF_SIZE + 4 → نحتاج cmdLen + 3
  if (cmdLen + 3 > VOLTRONIC_CMD_BUF_SIZE + 4)
  {
    _lastError = ERR_TOO_LONG;
    return false;
  }

  memcpy(out, cmd, cmdLen);

  uint16_t crc = voltronicCRC((const uint8_t *)cmd, cmdLen);

  uint8_t hi, lo;
  voltronicCRCBytes(crc, hi, lo, _localCfg.applyCrcEscape);

  out[cmdLen + 0] = hi;
  out[cmdLen + 1] = lo;
  out[cmdLen + 2] = 0x0D;
  outLen = cmdLen + 3;
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  CRC validation
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::checkCRC()
{
  if (_respLen < 3)
  {
    _lastError = ERR_SHORT;
    return false;
  }

  uint8_t rawHi = (uint8_t)_respBuf[_respLen - 2];
  uint8_t rawLo = (uint8_t)_respBuf[_respLen - 1];

  uint16_t calc = voltronicCRC((const uint8_t *)_respBuf, _respLen - 2);

  // ─── Try 1: direct ───
  uint16_t recv1 = ((uint16_t)rawHi << 8) | rawLo;
  if (recv1 == calc)
  {
    _respBuf[_respLen - 2] = '\0';
    _respLen -= 2;
    return true;
  }

  // ─── Try 2: unescape ───
  if (_localCfg.applyCrcEscape)
  {
    uint8_t unh = voltronicUnescapeByte(rawHi);
    uint8_t unl = voltronicUnescapeByte(rawLo);
    uint16_t recv2 = ((uint16_t)unh << 8) | unl;
    if (recv2 == calc)
    {
      _respBuf[_respLen - 2] = '\0';
      _respLen -= 2;
      return true;
    }
  }

  _lastError = ERR_CRC;
  return false;
}

// ═══════════════════════════════════════════════════════════════
//  ACK / NAK detection — ✅ مصحّح: يتجاوز '(' في البداية
// ═══════════════════════════════════════════════════════════════
const char *VoltronicMAX::payloadStart() const
{
  return (_respBuf[0] == '(') ? (_respBuf + 1) : _respBuf;
}

bool VoltronicMAX::isAck() const
{
  const char *p = payloadStart();
  return p[0] == 'A' && p[1] == 'C' && p[2] == 'K';
}

bool VoltronicMAX::isNak() const
{
  const char *p = payloadStart();
  return p[0] == 'N' && p[1] == 'A' && p[2] == 'K';
}

// ═══════════════════════════════════════════════════════════════
//  Filter printable — يُطبَّق بعد CRC على payload
// ═══════════════════════════════════════════════════════════════
void VoltronicMAX::applyPrintableFilter()
{
  if (!_localCfg.filterPrintable)
    return;
  for (size_t i = 0; i < _respLen; i++)
  {
    uint8_t c = (uint8_t)_respBuf[i];
    if (c < 0x20 && c != '\t')
      _respBuf[i] = ' ';
    else if (c > 0x7E)
      _respBuf[i] = '.';
  }
}

// ═══════════════════════════════════════════════════════════════
//  Transaction
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::transact(const char *cmd, bool expectAck)
{
  _lastError = ERR_NONE;
  _respLen = 0;

  size_t cmdLen = strlen(cmd);
  if (cmdLen >= VOLTRONIC_CMD_BUF_SIZE)
  {
    _lastError = ERR_TOO_LONG;
    return false;
  }

  strncpy(_cmdBuf, cmd, sizeof(_cmdBuf) - 1);
  _cmdBuf[sizeof(_cmdBuf) - 1] = '\0';

  uint8_t frame[VOLTRONIC_CMD_BUF_SIZE + 4];
  size_t frameLen;
  if (!buildFrame(_cmdBuf, frame, frameLen))
    return false;

  uint8_t attempts = _localCfg.retries + 1;

  for (uint8_t a = 0; a < attempts; a++)
  {
    if (_localCfg.clearBufferBeforeSend)
      _transport.clear();
    _transport.writeRaw(frame, frameLen);

    _respLen = _transport.readUntilCR((uint8_t *)_respBuf, sizeof(_respBuf) - 1);
    if (_respLen == 0)
    {
      _lastError = ERR_TIMEOUT;
      continue;
    }
    _respBuf[_respLen] = '\0';

    if (!checkCRC())
      continue;

    // ✅ الفلتر بعد CRC — لا يؤثر على الحساب
    applyPrintableFilter();

    if (expectAck)
    {
      if (isAck())
      {
        _lastError = ERR_NONE;
        return true;
      }
      if (isNak())
      {
        _lastError = ERR_NAK;
        continue;
      }
      _lastError = ERR_BAD_RESP;
      continue;
    }

    _lastError = ERR_NONE;
    return true;
  }
  return false;
}

bool VoltronicMAX::sendRaw(const char *cmd)
{
  return transact(cmd, false);
}

bool VoltronicMAX::sendRawSetting(const char *cmd)
{
  return transact(cmd, true);
}

// ═══════════════════════════════════════════════════════════════
//  String queries
// ═══════════════════════════════════════════════════════════════
#define VC_STR_QUERY(cmdConst, validBit) \
  do                                     \
  {                                      \
    if (!transact(cmdConst, false))      \
      return false;                      \
    const char *s = payloadStart();      \
    strncpy(out, s, len - 1);            \
    out[len - 1] = '\0';                 \
    _validBits |= (validBit);            \
    return true;                         \
  } while (0)

bool VoltronicMAX::queryProtocolID(char *out, size_t len) { VC_STR_QUERY(VC_QPI, VVB_QID); }
bool VoltronicMAX::querySerialNumber(char *out, size_t len) { VC_STR_QUERY(VC_QID, VVB_QID); }
bool VoltronicMAX::querySerialNumberLong(char *out, size_t len) { VC_STR_QUERY(VC_QSID, VVB_QSID); }
bool VoltronicMAX::queryFirmware(char *out, size_t len) { VC_STR_QUERY(VC_QVFW, VVB_QVFW); }
bool VoltronicMAX::queryFirmware2(char *out, size_t len) { VC_STR_QUERY(VC_QVFW3, VVB_QVFW3); }
bool VoltronicMAX::queryBluetoothVersion(char *out, size_t len) { VC_STR_QUERY(VC_VERFW, VVB_VERFW); }
bool VoltronicMAX::queryModelName(char *out, size_t len) { VC_STR_QUERY(VC_QMN, VVB_QMN); }
bool VoltronicMAX::queryGeneralModelName(char *out, size_t len) { VC_STR_QUERY(VC_QGMN, VVB_QGMN); }
bool VoltronicMAX::queryTime(char *out, size_t len) { VC_STR_QUERY(VC_QT, VVB_QT); }

#undef VC_STR_QUERY

// ═══════════════════════════════════════════════════════════════
//  Data queries
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::queryGeneralStatus()
{
  if (!transact(VC_QPIGS, false))
    return false;
  if (!VoltronicParser::parseQPIGS(_respBuf, _qpigs))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QPIGS;
  battery.update(_qpigs); // ← جديد: يحدّث الحسابات
  return true;
}

bool VoltronicMAX::queryGeneralStatus2()
{
  if (!transact(VC_QPIGS2, false))
    return false;
  if (!VoltronicParser::parseQPIGS2(_respBuf, _qpigs2))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QPIGS2;
  return true;
}

bool VoltronicMAX::queryRating()
{
  if (!transact(VC_QPIRI, false))
    return false;
  if (!VoltronicParser::parseQPIRI(_respBuf, _qpiri))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QPIRI;
  smartCharger.update(*this, battery); // ← جديد
  powerMode.update(*this, battery);    // ← جديد
  return true;
}

bool VoltronicMAX::queryMode()
{
  if (!transact(VC_QMOD, false))
    return false;
  if (!VoltronicParser::parseQMOD(_respBuf, _mode))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QMOD;
  return true;
}

bool VoltronicMAX::queryWarnings()
{
  if (!transact(VC_QPIWS, false))
    return false;
  if (!VoltronicParser::parseQPIWS(_respBuf, _warningsRaw))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QPIWS;
  return true;
}

bool VoltronicMAX::queryFlags()
{
  if (!transact(VC_QFLAG, false))
    return false;
  if (!VoltronicParser::parseQFLAG(_respBuf, _qflag))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QFLAG;
  return true;
}

bool VoltronicMAX::queryDefaults()
{
  if (!transact(VC_QDI, false))
    return false;
  if (!VoltronicParser::parseQDI(_respBuf, _defaults))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QDI;
  return true;
}

bool VoltronicMAX::queryParallel(uint8_t n)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%u", VC_QPGS, n);
  if (!transact(cmd, false))
    return false;
  if (!VoltronicParser::parseQPGSn(_respBuf, _parallel))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QPGS;
  return true;
}

bool VoltronicMAX::queryBatteryEqualization()
{
  if (!transact(VC_QBEQI, false))
    return false;
  if (!VoltronicParser::parseQBEQI(_respBuf, _beqi))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QBEQI;
  return true;
}

bool VoltronicMAX::queryLed()
{
  if (!transact(VC_QLED, false))
    return false;
  if (!VoltronicParser::parseQLED(_respBuf, _led))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QLED;
  return true;
}

bool VoltronicMAX::queryBatteryControl()
{
  if (!transact(VC_QBATCD, false))
    return false;
  if (!VoltronicParser::parseQBATCD(_respBuf, _batcd))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QBATCD;
  return true;
}

bool VoltronicMAX::queryBoot(bool &hasBootstrap)
{
  if (!transact(VC_QBOOT, false))
    return false;
  if (!VoltronicParser::parseQBOOT(_respBuf, hasBootstrap))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _hasBoot = hasBootstrap;
  _validBits |= VVB_QBOOT;
  return true;
}

bool VoltronicMAX::queryMaxChargingCurrents()
{
  if (!transact(VC_QMCHGCR, false))
    return false;
  if (!VoltronicParser::parseQMCHGCR(_respBuf, _maxChg))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QMCHGCR;
  return true;
}

bool VoltronicMAX::queryMaxUtilityChargingCurrents()
{
  if (!transact(VC_QMUCHGCR, false))
    return false;
  if (!VoltronicParser::parseQMUCHGCR(_respBuf, _maxUtilChg))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QMUCHGCR;
  return true;
}

bool VoltronicMAX::queryOutputPriorityTimeOrder()
{
  if (!transact(VC_QOPPT, false))
    return false;
  if (!VoltronicParser::parseQOPPT(_respBuf, _outputTO))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QOPPT;
  return true;
}

bool VoltronicMAX::queryChargerPriorityTimeOrder()
{
  if (!transact(VC_QCHPT, false))
    return false;
  if (!VoltronicParser::parseQCHPT(_respBuf, _chargerTO))
  {
    _lastError = ERR_PARSE;
    return false;
  }
  _validBits |= VVB_QCHPT;
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  Settings
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::setFlag(char flag)
{
  char cmd[4] = {'P', 'E', flag, 0};
  return transact(cmd, true);
}

bool VoltronicMAX::clearFlag(char flag)
{
  char cmd[4] = {'P', 'D', flag, 0};
  return transact(cmd, true);
}

bool VoltronicMAX::resetDefaults() { return transact(VC_PF, true); }

bool VoltronicMAX::setMaxChargingCurrent(uint16_t a)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_MNCHGC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxUtilityChargingCurrent(uint16_t a)
{
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_MUCHGC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxDischargingCurrent(uint16_t a)
{
  char cmd[20];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBATMAXDISC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputVoltage(uint16_t v)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_V, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputFrequency(uint8_t hz)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_F, hz);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputSourcePriority(uint8_t p)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_POP, p);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputMode(uint8_t m)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_POPM, m);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryRechargeVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBCV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryRedischargeVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBDV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryCutoffVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PSDV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryCvVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PCVV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryFloatVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBFT, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setChargerSourcePriority(uint8_t p)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_PCP, p);
  return transact(cmd, true);
}
bool VoltronicMAX::setGridWorkingRange(uint8_t r)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_PGR, r);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryType(uint8_t t)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%02u", VC_PBT, t);
  return transact(cmd, true);
}
bool VoltronicMAX::setParallelChargerPriority(uint8_t m, uint8_t p)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%u%02u", VC_PPCP, m, p);
  return transact(cmd, true);
}
bool VoltronicMAX::resetEnergy() { return transact(VC_RTEY, true); }
bool VoltronicMAX::eraseLog() { return transact(VC_RTDL, true); }

bool VoltronicMAX::setBatteryEqualizationEnabled(bool en)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%u", VC_PBEQE, en ? 1 : 0);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationTime(uint16_t min)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationPeriod(uint16_t days)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQP, days);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationVoltage(float v)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%.2f", VC_PBEQV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationOverTime(uint16_t min)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQOT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::activateBatteryEqualization(bool active)
{
  char cmd[8];
  snprintf(cmd, sizeof(cmd), "%s%u", VC_PBEQA, active ? 1 : 0);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxCvChargingTime(uint16_t min)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%03u", VC_PCVT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::setDateTime(const char *s)
{
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s%s", VC_DAT, s);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryControl(uint8_t a, uint8_t b, uint8_t c)
{
  char cmd[12];
  snprintf(cmd, sizeof(cmd), "%s%u%u%u", VC_PBATCD, a, b, c);
  return transact(cmd, true);
}

// ═══════════════════════════════════════════════════════════════
//  Non-blocking polling
// ═══════════════════════════════════════════════════════════════
void VoltronicMAX::startPolling(const VoltronicPollSchedule &schedule)
{
  _pollSchedule = schedule;
  _pollEnabled = true;
  _pollIndex = 0;
  _pollCycles = 0;
  _pollCycleDone = false;
  uint32_t now = millis();
  for (uint8_t i = 0; i < 9; i++)
    _pollLastFire[i] = now;
}

void VoltronicMAX::startPolling()
{
  VoltronicPollSchedule def;
  startPolling(def);
}

void VoltronicMAX::stopPolling()
{
  _pollEnabled = false;
}

uint16_t VoltronicMAX::getPollInterval(uint8_t idx) const
{
  switch (idx)
  {
  case 0:
    return _pollSchedule.qpigsMs;
  case 1:
    return _pollSchedule.qpigs2Ms;
  case 2:
    return _pollSchedule.qpiriMs;
  case 3:
    return _pollSchedule.qmodMs;
  case 4:
    return _pollSchedule.qpiwsMs;
  case 5:
    return _pollSchedule.qflagMs;
  case 6:
    return _pollSchedule.qidMs;
  case 7:
    return _pollSchedule.qbeqiMs;
  case 8:
    return _pollSchedule.qbatcdMs;
  default:
    return 0;
  }
}

bool VoltronicMAX::execPollQuery(uint8_t idx)
{
  switch (idx)
  {
  case 0:
    return queryGeneralStatus();
  case 1:
    return queryGeneralStatus2();
  case 2:
    return queryRating();
  case 3:
    return queryMode();
  case 4:
    return queryWarnings();
  case 5:
    return queryFlags();
  case 6:
  {
    char buf[24];
    return querySerialNumber(buf, sizeof(buf));
  }
  case 7:
    return queryBatteryEqualization();
  case 8:
    return queryBatteryControl();
  default:
    return false;
  }
}

bool VoltronicMAX::poll()
{
  _pollCycleDone = false;
  if (!_pollEnabled)
    return true;

  uint32_t now = millis();
  const uint8_t total = 9;

  for (uint8_t i = 0; i < total; i++)
  {
    uint8_t idx = (_pollIndex + i) % total;
    uint16_t interval = getPollInterval(idx);
    if (interval == 0)
      continue;

    if (now - _pollLastFire[idx] >= interval)
    {
      _pollLastFire[idx] = now;
      execPollQuery(idx);
      _pollIndex = (idx + 1) % total;

      bool anyDue = false;
      for (uint8_t j = 0; j < total; j++)
      {
        uint16_t iv = getPollInterval(j);
        if (iv == 0)
          continue;
        if (now - _pollLastFire[j] >= iv)
        {
          anyDue = true;
          break;
        }
      }
      if (!anyDue && _pollIndex == 0)
      {
        _pollCycles++;
        _pollCycleDone = true;
      }
      return false;
    }
  }
  return true;
}
// ═══════════════════════════════════════════════════════════════
//  Web console implementation (opt-in)
// ═══════════════════════════════════════════════════════════════
#if defined(VOLTRONIC_USE_WEB)

// ═══════════════════════════════════════════════════════════════
//  VoltronicWeb.h — Web Console + Parser
// ═══════════════════════════════════════════════════════════════

// ─── جدول الأوامر (PROGMEM) ───
struct VoltWebCmdEntry
{
  const char *cmd;
  const char *label;
  uint8_t type;
  const char *defParam;
};

static const VoltWebCmdEntry VWEB_CMDS[] PROGMEM = {
    {VC_QPI, "Protocol ID", 0, ""},
    {VC_QID, "Serial number", 0, ""},
    {VC_QSID, "Serial (long)", 0, ""},
    {VC_QVFW, "Firmware version", 0, ""},
    {VC_QVFW3, "Firmware v2", 0, ""},
    {VC_VERFW, "Bluetooth version", 0, ""},
    {VC_QMN, "Model name", 0, ""},
    {VC_QGMN, "General model", 0, ""},
    {VC_QBOOT, "Boot status", 0, ""},
    {VC_QMOD, "Device mode", 0, ""},
    {VC_QPIGS, "General status", 0, ""},
    {VC_QPIGS2, "General status 2", 0, ""},
    {VC_QPIRI, "Rated info", 0, ""},
    {VC_QFLAG, "Status flags", 0, ""},
    {VC_QPIWS, "Warning status", 0, ""},
    {VC_QT, "Inverter time", 0, ""},
    {VC_QBEQI, "Battery equalize", 0, ""},
    {VC_QBATCD, "Battery control", 0, ""},
    {VC_QLED, "LED status", 0, ""},
    {VC_QDI, "Defaults", 0, ""},
    {VC_QMCHGCR, "Max charge currents", 0, ""},
    {VC_QMUCHGCR, "Max utility currents", 0, ""},
    {VC_QOPPT, "Output prio time", 0, ""},
    {VC_QCHPT, "Charger prio time", 0, ""},
    {VC_QPGS, "Parallel info (n)", 0, "0"},
    {VC_PF, "Factory reset", 1, ""},
    {VC_PE, "Enable flag (a/b/d/...)", 1, "a"},
    {VC_PD, "Disable flag (a/b/d/...)", 1, "a"},
    {VC_MNCHGC, "Max charge A", 1, "060"},
    {VC_MUCHGC, "Max utility A", 1, "060"},
    {VC_PBATMAXDISC, "Max discharge A", 1, "060"},
    {VC_V, "Output voltage", 1, "230"},
    {VC_F, "Output freq Hz", 1, "50"},
    {VC_POP, "Output priority", 1, "0"},
    {VC_PCP, "Charger priority", 1, "2"},
    {VC_PGR, "Grid range", 1, "1"},
    {VC_PBT, "Battery type", 1, "2"},
    {VC_POPM, "Output mode", 1, "0"},
    {VC_PBCV, "Battery recharge V", 1, "56.4"},
    {VC_PBDV, "Battery redischarge V", 1, "54.0"},
    {VC_PSDV, "Battery cutoff V", 1, "48.0"},
    {VC_PCVV, "Battery CV V", 1, "56.4"},
    {VC_PBFT, "Battery float V", 1, "54.0"},
    {VC_PPCP, "Parallel priority (M PP)", 1, "100"},
    {VC_RTEY, "Reset energy", 1, ""},
    {VC_RTDL, "Erase log", 1, ""},
    {VC_PBEQE, "Equalize enable", 1, "1"},
    {VC_PBEQT, "Equalize time min", 1, "060"},
    {VC_PBEQP, "Equalize period days", 1, "030"},
    {VC_PBEQV, "Equalize voltage", 1, "58.40"},
    {VC_PBEQOT, "Equalize over time min", 1, "120"},
    {VC_PBEQA, "Activate equalize", 1, "0"},
    {VC_PCVT, "Max CV time min", 1, "060"},
    {VC_DAT, "Set datetime", 1, "26010112000000"},
    {VC_PBATCD, "Battery control (a b c)", 1, "000"},
};
static const size_t VWEB_CMDS_N = sizeof(VWEB_CMDS) / sizeof(VWEB_CMDS[0]);

// ═══════════════════════════════════════════════════════════════
//  HTML (PROGMEM)
// ═══════════════════════════════════════════════════════════════
static const char VHTML_P1[] PROGMEM =
    "<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Voltronic MAX</title>"
    "<style>"
    "*{box-sizing:border-box}"
    "body{font-family:system-ui,-apple-system,sans-serif;max-width:900px;"
    "margin:16px auto;padding:0 14px;background:#eef1f5;color:#1a1a1a;"
    "font-size:14px;line-height:1.5}"
    "h1{color:#2563eb;margin:0 0 14px;font-size:1.4rem}"
    "h2{font-size:.7rem;color:#6b7280;text-transform:uppercase;"
    "letter-spacing:.08em;margin:0 0 10px;font-weight:700}"
    ".card{background:#fff;padding:14px;border-radius:10px;"
    "box-shadow:0 1px 6px rgba(0,0,0,.06);margin-bottom:12px}"
    ".filter{margin-bottom:10px;display:flex;gap:8px;flex-wrap:wrap}"
    ".filter input{flex:1;min-width:180px;padding:7px 10px;border:1px solid #d1d5db;"
    "border-radius:6px;font-size:.85rem;font-family:inherit}"
    ".filter button{padding:7px 12px;border:0;background:#e5e7eb;cursor:pointer;"
    "border-radius:6px;font-size:.8rem}"
    ".filter button.active{background:#2563eb;color:#fff}"
    ".cmd{display:grid;grid-template-columns:1fr auto auto;gap:8px;"
    "align-items:center;padding:7px 0;border-bottom:1px solid #f1f3f5}"
    ".cmd:last-child{border-bottom:0}"
    ".cmd .info{min-width:0}"
    ".cmd b{font-weight:600;font-size:.88rem;display:block}"
    ".cmd code{background:#f1f3f5;padding:1px 6px;border-radius:3px;"
    "font-size:.68rem;color:#2563eb;font-family:ui-monospace,monospace}"
    ".cmd input{padding:6px 9px;border:1px solid #d1d5db;border-radius:5px;"
    "font-size:.8rem;width:110px;font-family:ui-monospace,monospace}"
    ".cmd button{padding:6px 12px;border:0;background:#2563eb;color:#fff;"
    "border-radius:5px;font-size:.78rem;cursor:pointer;font-weight:600}"
    ".cmd button:hover{background:#1d4ed8}"
    ".row{display:flex;gap:6px}.row input{flex:1;width:auto;padding:8px 10px;"
    "border:1px solid #d1d5db;border-radius:6px;font-family:ui-monospace,monospace}"
    ".row button{padding:8px 16px;border:0;background:#2563eb;color:#fff;"
    "border-radius:6px;cursor:pointer;font-weight:600}"
    "#out{display:flex;flex-direction:column;gap:10px}"
    ".resp{background:#fff;border-radius:8px;border-left:4px solid #2563eb;"
    "padding:10px 12px;box-shadow:0 1px 4px rgba(0,0,0,.06)}"
    ".resp.err{border-left-color:#dc2626}"
    ".resp.ok{border-left-color:#16a34a}"
    ".resp-head{display:flex;align-items:center;gap:8px;margin-bottom:8px;"
    "flex-wrap:wrap}"
    ".resp-cmd{background:#2563eb;color:#fff;padding:2px 8px;border-radius:4px;"
    "font-family:ui-monospace,monospace;font-size:.75rem;font-weight:700}"
    ".resp-time{color:#9ca3af;font-size:.7rem;margin-left:auto}"
    ".badge{padding:2px 8px;border-radius:4px;font-size:.68rem;font-weight:700}"
    ".badge.ok{background:#dcfce7;color:#15803d}"
    ".badge.err{background:#fee2e2;color:#b91c1c}"
    ".kv{width:100%;border-collapse:collapse;font-size:.82rem;margin:6px 0}"
    ".kv td{padding:4px 8px;border-bottom:1px solid #f1f3f5}"
    ".kv td:first-child{color:#6b7280;width:55%}"
    ".kv td:last-child{font-family:ui-monospace,monospace;font-weight:600;"
    "color:#111827;text-align:right}"
    ".raw{margin-top:8px;background:#0f172a;color:#7dd3fc;padding:8px 10px;"
    "border-radius:6px;font-size:.72rem;font-family:ui-monospace,monospace;"
    "word-break:break-all;white-space:pre-wrap}"
    ".raw-label{font-size:.68rem;color:#94a3b8;text-transform:uppercase;"
    "margin-bottom:3px;font-weight:700}"
    ".foot{font-size:.72rem;color:#9ca3af;text-align:center;margin-top:20px}"
    "#msgBanner{position:fixed;top:14px;left:50%;transform:translateX(-50%);"
    "padding:10px 22px;border-radius:8px;font-weight:600;z-index:9999;"
    "box-shadow:0 2px 12px rgba(0,0,0,.15);opacity:0;transition:opacity .3s;"
    "pointer-events:none;font-size:.85rem}"
    "</style></head><body>"
    "<h1>⚡ Voltronic MAX</h1>";

static const char VHTML_P2[] PROGMEM =
    // ═══════════════ Commands ═══════════════
    "<div class='card'>"
    "<h2>Commands</h2>"
    "<div class='filter'>"
    "<input type='text' id='srch' placeholder='Filter commands…' oninput='renderCmds()'>"
    "<button id='fAll' class='active' onclick='setFilter(0)'>All</button>"
    "<button id='fQ'   onclick='setFilter(1)'>Query</button>"
    "<button id='fS'   onclick='setFilter(2)'>Setting</button>"
    "</div>"
    "<div id='cmds'>loading…</div>"
    "</div>"

    // ═══════════════ Battery ═══════════════
    "<div class='card'>"
    "<h2>Battery</h2>"
    "<div style='display:flex;flex-direction:column;gap:8px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Type</label>"
    "<select id='batType' style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<option>User</option><option>AGM</option><option>Flooded</option>"
    "<option>Pylontech</option><option>LiFePO4 15S</option><option>LiFePO4 16S</option>"
    "</select>"
    "<label style='font-size:.8rem;color:#6b7280'>Capacity (Ah)</label>"
    "<input type='number' id='batCap' value='100' min='10' max='2000' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Voltage Empty (V)</label>"
    "<input type='number' id='batVEmpty' value='42.0' step='0.1' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Voltage Full (V)</label>"
    "<input type='number' id='batVFull' value='54.0' step='0.1' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<button onclick='saveBattery()' style='padding:8px;border:0;background:#16a34a;"
    "color:#fff;border-radius:5px;cursor:pointer;font-weight:600'>Save Battery</button>"
    "</div></div>"

    // ═══════════════ Smart Charger ═══════════════
    "<div class='card'>"
    "<h2>Smart Charger</h2>"
    "<div style='display:flex;flex-direction:column;gap:8px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Mode</label>"
    "<select id='scMode' style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<option value='0'>Disabled</option>"
    "<option value='1'>Standard (Step Down)</option>"
    "<option value='2'>Fast (Float Only)</option>"
    "</select>"
    "<label style='font-size:.8rem;color:#6b7280'>Target AC (A)</label>"
    "<input type='number' id='scAC' value='40' min='2' max='80' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Target Total (A)</label>"
    "<input type='number' id='scTotal' value='80' min='10' max='80' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Float AC (A)</label>"
    "<input type='number' id='scFloatAC' value='2' min='2' max='20' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Float Total (A)</label>"
    "<input type='number' id='scFloatTotal' value='10' min='2' max='40' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<label style='font-size:.8rem;color:#6b7280'>Temp Protect (°C)</label>"
    "<input type='number' id='scTemp' value='70' min='50' max='90' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"
    "<button onclick='saveSmartCharger()' style='padding:8px;border:0;background:#16a34a;"
    "color:#fff;border-radius:5px;cursor:pointer;font-weight:600'>Save Smart Charger</button>"
    "</div></div>"

    // ═══════════════ Power Mode Thresholds ═══════════════
    "<div class='card'>"
    "<h2>Power Mode Thresholds</h2>"
    "<div style='display:flex;flex-direction:column;gap:8px'>"

    "<label style='font-size:.8rem;color:#6b7280'>⚠️ SOC Emergency Mode (%)</label>"
    "<input type='number' id='pmSocEm' value='10' min='5' max='40' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"

    "<label style='font-size:.8rem;color:#6b7280'>🔋 SOC Power Saving Mode (%)</label>"
    "<input type='number' id='pmSocPs' value='25' min='10' max='50' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"

    "<label style='font-size:.8rem;color:#6b7280'>🔌 SOC Recover (Exit Lock) (%)</label>"
    "<input type='number' id='pmSocRec' value='30' min='20' max='60' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"

    "<label style='font-size:.8rem;color:#6b7280'>☀️ SOC Surplus Mode (%)</label>"
    "<input type='number' id='pmSocSurp' value='88' min='70' max='99' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"

    "<label style='font-size:.8rem;color:#6b7280'>⚡ Grid Min Voltage (V)</label>"
    "<input type='number' id='pmGridV' value='150' min='100' max='220' "
    "style='padding:6px;border:1px solid #d1d5db;border-radius:5px'>"

    "<button onclick='savePowerMode()' style='padding:8px;border:0;"
    "background:#16a34a;color:#fff;border-radius:5px;cursor:pointer;"
    "font-weight:600'>Save Power Mode</button>"
    "</div></div>"

    // ═══════════════ Live Status ═══════════════
    "<div class='card'>"
    "<h2>Live Status</h2>"
    "<div id='statusBox' style='font-size:.85rem;line-height:1.9'>loading…</div>"
    "</div>"

    // ═══════════════ Raw command ═══════════════
    "<div class='card'>"
    "<h2>Raw command</h2>"
    "<div class='row'>"
    "<input type='text' id='raw' placeholder='QPIGS   or   PBCV56.4'>"
    "<button onclick='sendRaw()'>Send</button>"
    "</div>"
    "</div>"

    // ═══════════════ Output ═══════════════
    "<div class='card'>"
    "<h2>Output</h2>"
    "<div id='out'><div style='color:#9ca3af;font-style:italic'>No output yet…</div></div>"
    "</div>"

    "<div class='foot'>Kitronic &middot; Voltronic MAX Web Console</div>"

    // ═══════════════════════════════════════════════════════════
    //  JavaScript
    // ═══════════════════════════════════════════════════════════
    "<script>"
    "let CMDS=[];let FILTER=0;"
    "const BASE=(document.querySelector('base')||{}).href||'/';"
    "const $=id=>document.getElementById(id);"
    "const esc=s=>String(s).replace(/[<>&]/g,c=>({'<':'&lt;','>':'&gt;','&':'&amp;'}[c]));"

    // ─── Banner message ───
    "function showMsg(text, ok){"
    "let el=$('msgBanner');"
    "if(!el){el=document.createElement('div');el.id='msgBanner';"
    "document.body.appendChild(el);}"
    "el.textContent=text;"
    "el.style.background=ok?'#dcfce7':'#fee2e2';"
    "el.style.color=ok?'#15803d':'#b91c1c';"
    "el.style.opacity='1';"
    "clearTimeout(window._msgTimer);"
    "window._msgTimer=setTimeout(()=>{el.style.opacity='0';},3000);"
    "}"

    // ─── Load commands ───
    "async function load(){"
    "try{"
    "const r=await fetch(BASE+'api/commands');"
    "CMDS=await r.json();"
    "renderCmds();"
    "}catch(e){$('cmds').innerHTML='<span style=\"color:#c00\">load error: '+esc(e.message)+'</span>';}"
    "}"

    // ─── Render list ───
    "function setFilter(f){FILTER=f;"
    "['fAll','fQ','fS'].forEach((id,i)=>$(id).classList.toggle('active',i===f));"
    "renderCmds();}"
    "function renderCmds(){"
    "const q=($('srch').value||'').toLowerCase();"
    "const list=CMDS.filter(c=>{"
    "if(FILTER===1&&c.type!==0)return false;"
    "if(FILTER===2&&c.type!==1)return false;"
    "if(q&&!(c.label.toLowerCase().includes(q)||c.cmd.toLowerCase().includes(q)))return false;"
    "return true;"
    "});"
    "if(!list.length){$('cmds').innerHTML='<div style=\"color:#9ca3af;padding:10px\">No matches.</div>';return;}"
    "$('cmds').innerHTML=list.map((c,i)=>{"
    "const origIdx=CMDS.indexOf(c);"
    "const inp=c.type===1?'<input type=\"text\" id=\"p'+origIdx+'\" value=\"'+esc(c.def||'')+'\">':'<span></span>';"
    "return '<div class=\"cmd\"><div class=\"info\"><b>'+esc(c.label)+'</b>'"
    "+'<code>'+esc(c.cmd)+'</code></div>'+inp"
    "+'<button onclick=\"run('+origIdx+')\">Send</button></div>';"
    "}).join('');"
    "}"

    // ─── Execute ───
    "async function run(i){const c=CMDS[i];"
    "const p=c.type===1?$('p'+i).value:'';"
    "await send(c.cmd,p,c.type);}"
    "function sendRaw(){const v=$('raw').value.trim();if(!v)return;"
    "const sp=v.split(/\\s+/,2);"
    "const isSet=/^P[A-Z]/.test(sp[0])&&!/^Q/.test(sp[0]);"
    "send(sp[0],sp[1]||'',isSet?1:0);}"

    "async function send(cmd,param,type){"
    "const body='cmd='+encodeURIComponent(cmd)+'&param='+encodeURIComponent(param||'')+'&type='+(type||0);"
    "try{"
    "const r=await fetch(BASE+'api/cmd',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
    "const j=await r.json();"
    "showResult(cmd,param,j);"
    "}catch(e){"
    "const out=$('out');"
    "if(out.firstChild&&out.firstChild.style)out.innerHTML='';"
    "out.insertAdjacentHTML('afterbegin',"
    "'<div class=\"resp err\"><div class=\"resp-head\">'"
    "+'<span class=\"resp-cmd\">'+esc(cmd)+'</span>'"
    "+'<span class=\"badge err\">FETCH</span></div>'"
    "+'<div class=\"raw\">'+esc(e.message)+'</div></div>');"
    "}"
    "}"

    // ═══════════════════════════════════════════════════════════
    //  PARSERS
    // ═══════════════════════════════════════════════════════════
    "function clean(s){return String(s||'').replace(/^\\s*\\(/,'').trim();}"
    "function tok(s){return clean(s).split(/\\s+/);}"

    "const MODES={P:'Power On',S:'Standby',L:'Line',B:'Battery',"
    "F:'Fault',H:'Power Saving',D:'Shutdown'};"
    "const BAT_TYPES=['AGM','Flooded','User','Pylontech'];"
    "const OUT_PRIO=['Utility→Solar→Batt','Solar→Utility→Batt','Solar→Batt→Utility'];"
    "const CHG_PRIO={1:'Solar First',2:'Solar + Utility',3:'Only Solar'};"
    "const GRID_RANGE=['Appliance','UPS'];"

    // ─── parseDeviceStatus ───
    "function parseDeviceStatus(s){"
    "s=String(s||'');"
    "if(s.length<8)return [['Raw',s]];"
    "const b=(i)=>s[7-i]==='1';"
    "const flags=[];"
    "flags.push(['Bit 7: SBU Priority Version', b(7)?'نعم':'لا']);"
    "flags.push(['Bit 6: Configuration Changed', b(6)?'نعم':'لا']);"
    "flags.push(['Bit 5: SCC Firmware Updated', b(5)?'نعم':'لا']);"
    "flags.push(['Bit 4: الحمل (Load ON)', b(4)?'🟢 يعمل':'⚫ متوقف']);"
    "flags.push(['Bit 3: Battery Steady', b(3)?'نعم':'لا']);"
    "const code = (b(2)?4:0) | (b(1)?2:0) | (b(0)?1:0);"
    "const chgNames = ["
    "'لا شحن',"
    "'شحن شمسي (SCC)',"
    "'شحن شبكة (AC)',"
    "'شحن شمسي + شبكة',"
    "'Float (تعويم)',"
    "'Equalization (معادلة)',"
    "'محجوز',"
    "'Float / Timer (تعويم)'"
    "];"
    "flags.push(['Charging Status Code', code]);"
    "flags.push(['Charging Status', chgNames[code]||'غير معروف']);"
    "flags.push(['Raw Binary', s]);"
    "flags.push(['Raw Hex', '0x'+parseInt(s,2).toString(16).toUpperCase().padStart(2,'0')]);"
    "return flags;"
    "}"

    // ─── parseQPIGS ───
    "function parseQPIGS(resp){"
    "const t=tok(resp);if(t.length<20)return null;"
    "const result = ["
    "['Grid Voltage',t[0]+' V'],"
    "['Grid Frequency',t[1]+' Hz'],"
    "['AC Output Voltage',t[2]+' V'],"
    "['AC Output Frequency',t[3]+' Hz'],"
    "['AC Output Apparent Power',t[4]+' VA'],"
    "['AC Output Active Power',t[5]+' W'],"
    "['Load Percent',t[6]+' %'],"
    "['Bus Voltage',t[7]+' V'],"
    "['Battery Voltage',t[8]+' V'],"
    "['Battery Charging Current',t[9]+' A'],"
    "['Battery Capacity',t[10]+' %'],"
    "['Inverter Temperature',t[11]+' °C'],"
    "['PV1 Input Current',t[12]+' A'],"
    "['PV1 Input Voltage',t[13]+' V'],"
    "['SCC Battery Voltage',t[14]+' V'],"
    "['Battery Discharge Current',t[15]+' A']"
    "];"
    "const dev = parseDeviceStatus(t[16]);"
    "for(const d of dev) result.push(d);"
    "result.push(['Fan Voltage Offset',t[17]]);"
    "result.push(['EEPROM Version',t[18]]);"
    "result.push(['PV1 Charging Power',t[19]+' W']);"
    "return result;"
    "}"

    "function parseQPIGS2(resp){const t=tok(resp);if(t.length<2)return null;return ["
    "['PV2 Input Current',t[0]+' A'],['PV2 Input Voltage',t[1]+' V'],"
    "['PV2 Charging Power',(t[2]||'0')+' W']];}"

    "function parseQPIRI(resp){const t=tok(resp);if(t.length<17)return null;return ["
    "['Grid Rating Voltage',t[0]+' V'],['Grid Rating Current',t[1]+' A'],"
    "['AC Output Rating Voltage',t[2]+' V'],['AC Output Rating Frequency',t[3]+' Hz'],"
    "['AC Output Rating Current',t[4]+' A'],['AC Output Apparent Power',t[5]+' VA'],"
    "['AC Output Active Power',t[6]+' W'],['Battery Rating Voltage',t[7]+' V'],"
    "['Battery Recharge Voltage',t[8]+' V'],['Battery Under Voltage',t[9]+' V'],"
    "['Battery Bulk Voltage',t[10]+' V'],['Battery Float Voltage',t[11]+' V'],"
    "['Battery Type',BAT_TYPES[+t[12]]||t[12]],['Max AC Charging Current',t[13]+' A'],"
    "['Max Charging Current',t[14]+' A'],['Input Voltage Range',GRID_RANGE[+t[15]]||t[15]],"
    "['Output Source Priority',OUT_PRIO[+t[16]]||t[16]],"
    "['Charger Source Priority',CHG_PRIO[+t[17]]||t[17]],"
    "['Parallel Max Num',t[18]||'—'],['Machine Type',t[19]||'—'],"
    "['Output Mode',t[20]||'—'],"
    "['Battery Redischarge Voltage',(t[21]||'—')+(t[21]?' V':'')]];}"

    "function parseQMOD(resp){const c=clean(resp).charAt(0);"
    "return [['Mode Char',c],['Mode',MODES[c]||'Unknown']];}"

    "function parseQFLAG(resp){const s=clean(resp);"
    "const kv={'a':'Buzzer','b':'Overload Bypass','d':'Solar Feed to Grid',"
    "'j':'Power Saving','k':'LCD Default','u':'Overload Restart',"
    "'v':'Over-temp Restart','x':'Backlight','y':'Alarm Primary','z':'Fault Code Record'};"
    "const r=[['Raw',s]];"
    "for(let i=0;i<s.length-1;i++){"
    "const p=s[i],c=s[i+1];"
    "if((p==='E'||p==='D')&&kv[c])r.push([kv[c],p==='E'?'Enabled':'Disabled']);"
    "}return r;}"

    "function parseQPIWS(resp){const s=clean(resp);"
    "const names=['PV Loss','Inverter Fault','Bus Over','Bus Under','Bus Soft Fail',"
    "'Line Fail','OPV Short','Inv Voltage Low','Inv Voltage High','Over Temperature',"
    "'Fan Locked','Batt Voltage High','Batt Low Alarm','(reserved 13)','Batt Under Shutdown',"
    "'Batt Derating','Overload','EEPROM Fault','Inv Over Current','Inv Soft Fail',"
    "'Self-Test Fail','OP DC Voltage Over','Batt Open','Current Sensor Fail'];"
    "const r=[['Raw',s]];let any=false;"
    "for(let i=0;i<Math.min(s.length,32);i++){"
    "if(s[i]==='1'){r.push(['⚠ '+names[i]||('Bit '+i),'ACTIVE']);any=true;}}"
    "if(!any)r.push(['Status','All OK ✓']);"
    "return r;}"

    "function parseQT(resp){const s=clean(resp);"
    "if(s.length<12)return [['Raw',s]];"
    "return [['Date','20'+s.substr(0,2)+'-'+s.substr(2,2)+'-'+s.substr(4,2)],"
    "['Time',s.substr(6,2)+':'+s.substr(8,2)+':'+s.substr(10,2)],['Raw',s]];}"

    "function parseQBEQI(resp){const t=tok(resp);if(t.length<6)return null;return ["
    "['Enabled',t[0]==='1'?'Yes':'No'],['Time (min)',t[1]],['Period (days)',t[2]],"
    "['Voltage',t[3]+' V'],['Over Time (min)',t[4]],['Max Time (min)',t[5]],"
    "['Active Now',t[6]||'—']];}"

    "function parseQBATCD(resp){const s=clean(resp);"
    "return [['Discharge Completely',s[0]==='1'?'Yes':'No'],"
    "['Discharge Allowed',s[1]==='1'?'Yes':'No'],"
    "['Charge Completely',s[2]==='1'?'Yes':'No']];}"

    "function parseQBOOT(resp){const c=clean(resp).charAt(0);"
    "return [['Has Bootstrap',c==='1'?'Yes':'No']];}"

    "function parseQMCHGCR(resp){const t=tok(resp);"
    "return [['Count',t.length],['Values',t.join(', ')]];}"

    "function parseQDI(resp){const t=tok(resp);if(t.length<12)return null;return ["
    "['AC Output Voltage',t[0]+' V'],['AC Output Frequency',t[1]+' Hz'],"
    "['Max AC Charging Current',t[2]+' A'],['Battery Under Voltage',t[3]+' V'],"
    "['Battery Float Voltage',t[4]+' V'],['Battery Bulk Voltage',t[5]+' V'],"
    "['Battery Recharge Voltage',t[6]+' V'],['Max Charging Current',t[7]+' A'],"
    "['Input Voltage Range',GRID_RANGE[+t[8]]||t[8]],"
    "['Output Source Priority',OUT_PRIO[+t[9]]||t[9]],"
    "['Charger Source Priority',CHG_PRIO[+t[10]]||t[10]],"
    "['Battery Type',BAT_TYPES[+t[11]]||t[11]]];}"

    "function parseQPGSn(resp){const t=tok(resp);if(t.length<10)return null;"
    "return [['Parallel Number',t[0]],['Exists',t[1]==='1'?'Yes':'No'],"
    "['Serial',t[2]],['Work Mode',MODES[t[3]]||t[3]],['Fault Code',t[4]],"
    "['Grid Voltage',t[5]+' V'],['Grid Freq',t[6]+' Hz'],"
    "['AC Output Voltage',t[7]+' V'],['AC Output Freq',t[8]+' Hz'],"
    "['Load',t[11]+' %']];}"

    "function parseQLED(resp){const t=tok(resp);return [['Raw','('+t.join(' ')+')']];}"
    "function parseQOPPT(resp){return [['Time Order (24h)',clean(resp)]];}"

    "function fmtResponse(cmd,resp){"
    "if(!resp)return null;"
    "const c=clean(resp);"
    "if(c.startsWith('ACK'))return [['Result','✓ ACK (accepted)']];"
    "if(c.startsWith('NAK'))return [['Result','✗ NAK (rejected)']];"
    "if(cmd==='QPIGS')return parseQPIGS(resp);"
    "if(cmd==='QPIGS2')return parseQPIGS2(resp);"
    "if(cmd==='QPIRI')return parseQPIRI(resp);"
    "if(cmd==='QMOD')return parseQMOD(resp);"
    "if(cmd==='QFLAG')return parseQFLAG(resp);"
    "if(cmd==='QPIWS')return parseQPIWS(resp);"
    "if(cmd==='QT')return parseQT(resp);"
    "if(cmd==='QBEQI')return parseQBEQI(resp);"
    "if(cmd==='QBATCD')return parseQBATCD(resp);"
    "if(cmd==='QBOOT')return parseQBOOT(resp);"
    "if(cmd==='QMCHGCR'||cmd==='QMUCHGCR')return parseQMCHGCR(resp);"
    "if(cmd==='QDI')return parseQDI(resp);"
    "if(cmd.startsWith('QPGS'))return parseQPGSn(resp);"
    "if(cmd==='QLED')return parseQLED(resp);"
    "if(cmd==='QOPPT'||cmd==='QCHPT')return parseQOPPT(resp);"
    "return null;}"

    "function showResult(cmd,param,j){"
    "const out=$('out');"
    "if(out.firstChild&&out.firstChild.style)out.innerHTML='';"
    "const fullCmd=cmd+(param||'');"
    "const ok=j.ok;"
    "const resp=j.response||'';"
    "const parsed=ok?fmtResponse(cmd,resp):null;"
    "const t=new Date().toLocaleTimeString();"
    "let html='<div class=\"resp '+(ok?'ok':'err')+'\">';"
    "html+='<div class=\"resp-head\">';"
    "html+='<span class=\"resp-cmd\">'+esc(fullCmd)+'</span>';"
    "html+='<span class=\"badge '+(ok?'ok':'err')+'\">'+(ok?'OK':'ERR')+'</span>';"
    "if(j.error)html+='<span class=\"badge err\">'+esc(j.error)+'</span>';"
    "html+='<span class=\"resp-time\">'+t+'</span>';"
    "html+='</div>';"
    "if(parsed&&parsed.length){"
    "html+='<table class=\"kv\">';"
    "for(const kv of parsed){html+='<tr><td>'+esc(kv[0])+'</td><td>'+esc(kv[1])+'</td></tr>';}"
    "html+='</table>';}"
    "if(resp){html+='<div class=\"raw\"><div class=\"raw-label\">RAW</div>'+esc(resp)+'</div>';}"
    "html+='</div>';"
    "out.insertAdjacentHTML('afterbegin',html);"
    "while(out.children.length>20)out.removeChild(out.lastChild);"
    "}"

    // ═══════════════ Save Battery ═══════════════
    "async function saveBattery(){"
    "const body='type='+encodeURIComponent($('batType').value)"
    "+'&cap='+encodeURIComponent($('batCap').value)"
    "+'&vEmpty='+encodeURIComponent($('batVEmpty').value)"
    "+'&vFull='+encodeURIComponent($('batVFull').value);"
    "try{"
    "const r=await fetch(BASE+'api/battery',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
    "const j=await r.json();"
    "showMsg(j.ok?('Battery saved ('+j.cap+' Ah)'):('Error: '+(j.error||'failed')), j.ok);"
    "}catch(e){showMsg('Error: '+e.message, false);}"
    "}"

    // ═══════════════ Save Smart Charger ═══════════════
    "async function saveSmartCharger(){"
    "const body='mode='+encodeURIComponent($('scMode').value)"
    "+'&ac='+encodeURIComponent($('scAC').value)"
    "+'&total='+encodeURIComponent($('scTotal').value)"
    "+'&fAC='+encodeURIComponent($('scFloatAC').value)"
    "+'&fTotal='+encodeURIComponent($('scFloatTotal').value)"
    "+'&temp='+encodeURIComponent($('scTemp').value);"
    "try{"
    "const r=await fetch(BASE+'api/smartcharger',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
    "const j=await r.json();"
    "showMsg(j.ok?'Smart charger saved':('Error: '+(j.error||'failed')), j.ok);"
    "}catch(e){showMsg('Error: '+e.message, false);}"
    "}"

    // ═══════════════ Save Power Mode ═══════════════
    "async function savePowerMode(){"
    "const body='em='+$('pmSocEm').value"
    "+'&ps='+$('pmSocPs').value"
    "+'&rec='+$('pmSocRec').value"
    "+'&surp='+$('pmSocSurp').value"
    "+'&gv='+$('pmGridV').value;"
    "try{"
    "const r=await fetch(BASE+'api/powermode',{method:'POST',"
    "headers:{'Content-Type':'application/x-www-form-urlencoded'},body});"
    "const j=await r.json();"
    "showMsg(j.ok?'Power mode saved':('Error: '+(j.error||'failed')), j.ok);"
    "}catch(e){showMsg('Error: '+e.message, false);}"
    "}"

    // ═══════════════ Load inputs (مرة واحدة) ═══════════════
    "let inputsLoaded=false;"
    "async function loadInputs(){"
    "if(inputsLoaded)return;"
    "try{"
    "const r=await fetch(BASE+'api/status');"
    "const s=await r.json();"
    "$('batType').value=s.batType||'User';"
    "$('batCap').value=s.batCap||100;"
    "$('batVEmpty').value=s.batVEmpty||42;"
    "$('batVFull').value=s.batVFull||54;"
    "$('scMode').value=s.scMode||1;"
    "$('scAC').value=s.scAC||40;"
    "$('scTotal').value=s.scTotal||80;"
    "$('scFloatAC').value=s.scFloatAC||2;"
    "$('scFloatTotal').value=s.scFloatTotal||10;"
    "$('scTemp').value=s.scTemp||70;"
    "$('pmSocEm').value=s.pmEm||10;"
    "$('pmSocPs').value=s.pmPs||25;"
    "$('pmSocRec').value=s.pmRec||30;"
    "$('pmSocSurp').value=s.pmSurp||88;"
    "$('pmGridV').value=s.pmGridV||150;"
    "inputsLoaded=true;"
    "}catch(e){}"
    "}"

    // ═══════════════ Update Status Box (كل 5s) ═══════════════
    "async function updateStatusBox(){"
    "try{"
    "const r=await fetch(BASE+'api/status');"
    "const s=await r.json();"
    "$('statusBox').innerHTML="
    "'<b>SOC:</b> '+(s.soc||0).toFixed(1)+'%<br>'"
    "+'<b>SOH:</b> '+(s.soh||100).toFixed(1)+'%<br>'"
    "+'<b>Voltage:</b> '+(s.voltage||0).toFixed(2)+' V<br>'"
    "+'<b>Net:</b> '+(s.netPower||0).toFixed(0)+' W<br>'"
    "+'<b>Remaining:</b> '+(s.remKWh||0).toFixed(2)+' kWh<br>'"
    "+'<b>Stage:</b> '+(s.stage||'-')+'<br>'"
    "+'<b>Status:</b> '+(s.status||'-')+'<br>'"
    "+'<b>Power Mode:</b> '+(s.powerMode||'-')+'<br>'"
    "+'<b>Storage saves:</b> '+(s.saves||0);"
    "}catch(e){$('statusBox').textContent='err: '+e.message;}"
    "}"

    // ═══════════════ Init ═══════════════
    "loadInputs();"
    "setInterval(updateStatusBox, 5000);"
    "updateStatusBox();"
    "load();"
    "</script></body></html>";

// ═══════════════════════════════════════════════════════════════
//  JSON escape
// ═══════════════════════════════════════════════════════════════
static size_t voltJsonEscape(char *dst, size_t dstLen, const char *src)
{
  size_t w = 0;
  if (!src)
  {
    if (dstLen)
      dst[0] = '\0';
    return 0;
  }
  while (*src && w + 8 < dstLen)
  {
    uint8_t c = (uint8_t)*src++;
    switch (c)
    {
    case '"':
      dst[w++] = '\\';
      dst[w++] = '"';
      break;
    case '\\':
      dst[w++] = '\\';
      dst[w++] = '\\';
      break;
    case '\n':
      dst[w++] = '\\';
      dst[w++] = 'n';
      break;
    case '\r':
      dst[w++] = '\\';
      dst[w++] = 'r';
      break;
    default:
      if (c < 0x20)
        w += snprintf(dst + w, dstLen - w, "\\u%04x", c);
      else
        dst[w++] = (char)c;
    }
  }
  dst[w] = '\0';
  return w;
}

// ═══════════════════════════════════════════════════════════════
//  Config
// ═══════════════════════════════════════════════════════════════
void VoltronicMAX::attachWebServer(VoltWebServer *server)
{
  if (!server)
    return;
  _webServer = server;
  _webAuthOn = false;
  _webUser[0] = '\0';
  _webPass[0] = '\0';
  _webPrefix[0] = '\0';
  _webRegisterRoutes();
  Serial.println(F("[Voltronic] Web registered (no auth)"));
}

void VoltronicMAX::attachWebServer(VoltWebServer *server,
                                   const char *user, const char *pass)
{
  if (!user || !pass || !*user || !*pass)
  {
    attachWebServer(server);
    return;
  }
  _webServer = server;
  strncpy(_webUser, user, sizeof(_webUser) - 1);
  _webUser[sizeof(_webUser) - 1] = '\0';
  strncpy(_webPass, pass, sizeof(_webPass) - 1);
  _webPass[sizeof(_webPass) - 1] = '\0';
  _webAuthOn = true;
  _webPrefix[0] = '\0';
  _webRegisterRoutes();
  Serial.println(F("[Voltronic] Web registered (auth ON)"));
}

void VoltronicMAX::setWebPrefix(const char *prefix)
{
  if (!prefix)
  {
    _webPrefix[0] = '\0';
    return;
  }
  strncpy(_webPrefix, prefix, sizeof(_webPrefix) - 1);
  _webPrefix[sizeof(_webPrefix) - 1] = '\0';
  size_t n = strlen(_webPrefix);
  if (n > 0 && _webPrefix[n - 1] == '/')
    _webPrefix[n - 1] = '\0';
}

bool VoltronicMAX::_webRequireAuth()
{
  if (!_webAuthOn)
    return true;
  if (_webServer->authenticate(_webUser, _webPass))
    return true;
  _webServer->requestAuthentication();
  return false;
}

// ─── GET / ───
void VoltronicMAX::_webHandlePage()
{
  if (!_webRequireAuth())
    return;

  _webServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  _webServer->send(200, "text/html", "");
  _webServer->sendContent_P(VHTML_P1);

  char baseBuf[48];
  snprintf_P(baseBuf, sizeof(baseBuf), PSTR("<base href='%s/'>"), _webPrefix);
  _webServer->sendContent(baseBuf);

  _webServer->sendContent_P(VHTML_P2);
  _webServer->sendContent("");
}

// ─── GET /api/commands ───
void VoltronicMAX::_webHandleList()
{
  if (!_webRequireAuth())
    return;
  _webServer->setContentLength(CONTENT_LENGTH_UNKNOWN);
  _webServer->send(200, "application/json", "");
  _webServer->sendContent(F("["));
  for (size_t i = 0; i < VWEB_CMDS_N; i++)
  {
    VoltWebCmdEntry e;
    memcpy_P(&e, &VWEB_CMDS[i], sizeof(e));
    char row[200];
    snprintf_P(row, sizeof(row),
               PSTR("%s{\"cmd\":\"%s\",\"label\":\"%s\",\"type\":%u,\"def\":\"%s\"}"),
               (i ? "," : ""), e.cmd, e.label, (unsigned)e.type, e.defParam);
    _webServer->sendContent(row);
  }
  _webServer->sendContent(F("]"));
  _webServer->sendContent("");
}

// ─── POST /api/cmd ───
void VoltronicMAX::_webHandleExecute()
{
  if (!_webRequireAuth())
    return;
  if (!_webServer->hasArg("cmd"))
  {
    _webServer->send_P(400, PSTR("application/json"),
                       PSTR("{\"error\":\"missing cmd\"}"));
    return;
  }
  String cmd = _webServer->arg("cmd");
  String param = _webServer->arg("param");
  uint8_t type = (uint8_t)_webServer->arg("type").toInt();

  String full = cmd;
  if (param.length())
    full += param;

  bool ok = (type == 1) ? sendRawSetting(full.c_str()) : sendRaw(full.c_str());

  const char *raw = lastResponse();
  static char esc[512];
  voltJsonEscape(esc, sizeof(esc), raw ? raw : "");

  static char out[768];
  snprintf_P(out, sizeof(out),
             PSTR("{\"ok\":%s,\"cmd\":\"%s\",\"response\":\"%s\",\"error\":\"%s\"}"),
             ok ? "true" : "false", full.c_str(), esc, ok ? "" : lastErrorName());

  _webServer->send(200, "application/json", out);
}

// ─── POST /api/battery ───
void VoltronicMAX::_webHandleBattery()
{
  if (!_webRequireAuth())
    return;

  if (_webServer->hasArg("type"))
    battery.setType(VoltronicBattery::typeFromString(_webServer->arg("type").c_str()));
  if (_webServer->hasArg("cap"))
    battery.setCapacity(_webServer->arg("cap").toFloat());
  if (_webServer->hasArg("vEmpty"))
    battery.setVoltageEmpty(_webServer->arg("vEmpty").toFloat());
  if (_webServer->hasArg("vFull"))
    battery.setVoltageFull(_webServer->arg("vFull").toFloat());

  storage.save(*this);

  char buf[128];
  snprintf_P(buf, sizeof(buf),
             PSTR("{\"ok\":true,\"cap\":%.0f,\"type\":\"%s\"}"),
             battery.capacityAh(), battery.typeName());
  _webServer->send(200, "application/json", buf);

  Serial.printf_P(PSTR("[Web] Battery saved: %.0fAh, type=%s\n"),
                  battery.capacityAh(), battery.typeName());
}

// ─── POST /api/smartcharger ───
void VoltronicMAX::_webHandleSmartCharger()
{
  if (!_webRequireAuth())
    return;

  if (_webServer->hasArg("mode"))
    smartCharger.setMode((VoltronicSmartCharger::Mode)_webServer->arg("mode").toInt());
  if (_webServer->hasArg("ac"))
    smartCharger.setTargetAC(_webServer->arg("ac").toInt());
  if (_webServer->hasArg("total"))
    smartCharger.setTargetTotal(_webServer->arg("total").toInt());
  if (_webServer->hasArg("fAC"))
    smartCharger.setFloatAC(_webServer->arg("fAC").toInt());
  if (_webServer->hasArg("fTotal"))
    smartCharger.setFloatTotal(_webServer->arg("fTotal").toInt());
  if (_webServer->hasArg("temp"))
    smartCharger.setTempProtectC(_webServer->arg("temp").toInt());

  storage.save(*this);

  _webServer->send_P(200, PSTR("application/json"), PSTR("{\"ok\":true}"));

  Serial.printf_P(PSTR("[Web] SmartCharger saved: mode=%u AC=%u T=%u\n"),
                  (unsigned)smartCharger.mode(),
                  smartCharger.targetAC(),
                  smartCharger.targetTotal());
}

// ─── GET /api/status ───
void VoltronicMAX::_webHandleStatus()
{
  if (!_webRequireAuth())
    return;

  char buf[768];
  snprintf_P(buf, sizeof(buf),
             PSTR("{\"batType\":\"%s\",\"batCap\":%.0f,\"batVEmpty\":%.1f,\"batVFull\":%.1f,"
                  "\"scMode\":%u,\"scAC\":%u,\"scTotal\":%u,\"scFloatAC\":%u,\"scFloatTotal\":%u,"
                  "\"scTemp\":%u,\"soc\":%.1f,\"soh\":%.1f,\"voltage\":%.2f,\"netPower\":%.0f,"
                  "\"remKWh\":%.3f,\"stage\":\"%s\",\"status\":\"%s\","
                  "\"powerMode\":\"%s\",\"saves\":%u,"
                  "\"pmEm\":%.0f,\"pmPs\":%.0f,\"pmRec\":%.0f,\"pmSurp\":%.0f,\"pmGridV\":%.0f}"),
             battery.typeName(), battery.capacityAh(),
             battery.voltageEmpty(), battery.voltageFull(),
             (unsigned)smartCharger.mode(),
             smartCharger.targetAC(), smartCharger.targetTotal(),
             smartCharger.floatAC(), smartCharger.floatTotal(),
             smartCharger.tempProtectC(),
             battery.soc(), battery.soh(), battery.voltage(),
             battery.netPower(), battery.remainingKWh(),
             smartCharger.stage(), smartCharger.status(),
             powerMode.modeNameEn(), storage.saves(),
             powerMode.socEmergency(),
             powerMode.socPowerSaving(),
             powerMode.socRecover(),
             powerMode.socSurplus(),
             powerMode.gridMinVoltage());

  _webServer->send(200, "application/json", buf);
}

// ─── POST /api/powermode ───
void VoltronicMAX::_webHandlePowerMode()
{
  if (!_webRequireAuth())
    return;

  if (_webServer->hasArg("em"))
    powerMode.setSocEmergency(_webServer->arg("em").toFloat());
  if (_webServer->hasArg("ps"))
    powerMode.setSocPowerSaving(_webServer->arg("ps").toFloat());
  if (_webServer->hasArg("rec"))
    powerMode.setSocRecover(_webServer->arg("rec").toFloat());
  if (_webServer->hasArg("surp"))
    powerMode.setSocSurplus(_webServer->arg("surp").toFloat());
  if (_webServer->hasArg("gv"))
    powerMode.setGridMinVoltage(_webServer->arg("gv").toFloat());

  storage.save(*this);
  _webServer->send_P(200, PSTR("application/json"), PSTR("{\"ok\":true}"));
}

// ─── Routes ───
void VoltronicMAX::_webRegisterRoutes()
{
  if (!_webServer)
    return;

  static char pRoot[24], pList[48], pCmd[48], pBat[32], pSC[32], pSt[32], pPM[32];
  snprintf_P(pRoot, sizeof(pRoot), PSTR("%s/"), _webPrefix);
  snprintf_P(pList, sizeof(pList), PSTR("%s/api/commands"), _webPrefix);
  snprintf_P(pCmd, sizeof(pCmd), PSTR("%s/api/cmd"), _webPrefix);
  snprintf_P(pBat, sizeof(pBat), PSTR("%s/api/battery"), _webPrefix);
  snprintf_P(pSC, sizeof(pSC), PSTR("%s/api/smartcharger"), _webPrefix);
  snprintf_P(pSt, sizeof(pSt), PSTR("%s/api/status"), _webPrefix);
  snprintf_P(pPM, sizeof(pPM), PSTR("%s/api/powermode"), _webPrefix);

  _webServer->on(pRoot, HTTP_GET, [this]()
                 { _webHandlePage(); });
  _webServer->on(pList, HTTP_GET, [this]()
                 { _webHandleList(); });
  _webServer->on(pCmd, HTTP_POST, [this]()
                 { _webHandleExecute(); });
  _webServer->on(pBat, HTTP_POST, [this]()
                 { _webHandleBattery(); });
  _webServer->on(pSC, HTTP_POST, [this]()
                 { _webHandleSmartCharger(); });
  _webServer->on(pSt, HTTP_GET, [this]()
                 { _webHandleStatus(); });
  _webServer->on(pPM, HTTP_POST, [this]()
                 { _webHandlePowerMode(); });

  Serial.printf_P(PSTR("[Voltronic] Web routes at \"%s\"\n"), _webPrefix);
}

#endif // VOLTRONIC_USE_WEB