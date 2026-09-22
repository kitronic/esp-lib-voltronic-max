#include "VoltronicMAX.h"
#include <string.h>
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════════
VoltronicMAX::VoltronicMAX(Stream& serial)
  : _transport(serial) {}

// ═══════════════════════════════════════════════════════════════
//  Initialization
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::begin(const VoltronicConfig& cfg) {
  _localCfg = cfg;
  _transport.attachConfig(&_localCfg);
  _transport.clear();
  return true;
}

bool VoltronicMAX::begin(uint32_t baud) {
  VoltronicConfig cfg;
  cfg.baud = baud;
  return begin(cfg);
}

void VoltronicMAX::setTimeout(uint16_t ms) { _localCfg.responseTimeoutMs = ms; }
void VoltronicMAX::setRetries(uint8_t r)   { _localCfg.retries = r; }

// ═══════════════════════════════════════════════════════════════
//  Frame building
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::buildFrame(const char* cmd, uint8_t* out, size_t& outLen) {
  size_t cmdLen = strlen(cmd);
  if (cmdLen + 3 > VOLTRONIC_RESP_BUF_SIZE) {
    _lastError = ERR_TOO_LONG;
    return false;
  }

  memcpy(out, cmd, cmdLen);

  uint16_t crc = voltronicCRC((const uint8_t*)cmd, cmdLen);

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
bool VoltronicMAX::checkCRC() {
  if (_respLen < 3) { _lastError = ERR_SHORT; return false; }

  uint8_t rawHi = (uint8_t)_respBuf[_respLen - 2];
  uint8_t rawLo = (uint8_t)_respBuf[_respLen - 1];

  uint16_t calc = voltronicCRC((const uint8_t*)_respBuf, _respLen - 2);

  // Try 1: direct
  uint16_t recv1 = ((uint16_t)rawHi << 8) | rawLo;
  if (recv1 == calc) {
    _respBuf[_respLen - 2] = '\0';
    _respLen -= 2;
    return true;
  }

  // Try 2: with unescape
  if (_localCfg.applyCrcEscape) {
    uint8_t unh = voltronicUnescapeByte(rawHi);
    uint8_t unl = voltronicUnescapeByte(rawLo);
    uint16_t recv2 = ((uint16_t)unh << 8) | unl;
    if (recv2 == calc) {
      _respBuf[_respLen - 2] = '\0';
      _respLen -= 2;
      return true;
    }
  }

  _lastError = ERR_CRC;
  return false;
}

bool VoltronicMAX::isAck() const {
  return _respLen >= 3 && _respBuf[0] == 'A' && _respBuf[1] == 'C' && _respBuf[2] == 'K';
}

bool VoltronicMAX::isNak() const {
  return _respLen >= 3 && _respBuf[0] == 'N' && _respBuf[1] == 'A' && _respBuf[2] == 'K';
}

const char* VoltronicMAX::payloadStart() const {
  return (_respBuf[0] == '(') ? (_respBuf + 1) : _respBuf;
}

// ═══════════════════════════════════════════════════════════════
//  Transaction
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::transact(const char* cmd, bool expectAck) {
  _lastError = ERR_NONE;
  _respLen = 0;

  strncpy(_cmdBuf, cmd, sizeof(_cmdBuf) - 1);
  _cmdBuf[sizeof(_cmdBuf) - 1] = '\0';

  uint8_t frame[VOLTRONIC_CMD_BUF_SIZE + 4];
  size_t frameLen;
  if (!buildFrame(cmd, frame, frameLen)) return false;

  uint8_t attempts = _localCfg.retries + 1;

  for (uint8_t a = 0; a < attempts; a++) {
    if (_localCfg.clearBufferBeforeSend) _transport.clear();
    _transport.writeRaw(frame, frameLen);

    _respLen = _transport.readUntilCR((uint8_t*)_respBuf, sizeof(_respBuf) - 1);
    if (_respLen == 0) { _lastError = ERR_TIMEOUT; continue; }
    _respBuf[_respLen] = '\0';

    if (!checkCRC()) continue;

    if (expectAck) {
      if (isAck()) return true;
      if (isNak()) { _lastError = ERR_NAK; continue; }
      _lastError = ERR_BAD_RESP;
      continue;
    }
    return true;
  }
  return false;
}

bool VoltronicMAX::sendRaw(const char* cmd) {
  return transact(cmd, false);
}

// ═══════════════════════════════════════════════════════════════
//  String queries
// ═══════════════════════════════════════════════════════════════
#define VC_STR_QUERY(cmdConst) do { \
  if (!transact(cmdConst, false)) return false; \
  const char* s = payloadStart(); \
  strncpy(out, s, len - 1); \
  out[len - 1] = '\0'; \
  return true; \
} while(0)

bool VoltronicMAX::queryProtocolID(char* out, size_t len)       { VC_STR_QUERY(VC_QPI); }
bool VoltronicMAX::querySerialNumber(char* out, size_t len)     { VC_STR_QUERY(VC_QID); }
bool VoltronicMAX::querySerialNumberLong(char* out, size_t len) { VC_STR_QUERY(VC_QSID); }
bool VoltronicMAX::queryFirmware(char* out, size_t len)         { VC_STR_QUERY(VC_QVFW); }
bool VoltronicMAX::queryFirmware2(char* out, size_t len)        { VC_STR_QUERY(VC_QVFW3); }
bool VoltronicMAX::queryBluetoothVersion(char* out, size_t len) { VC_STR_QUERY(VC_VERFW); }
bool VoltronicMAX::queryModelName(char* out, size_t len)        { VC_STR_QUERY(VC_QMN); }
bool VoltronicMAX::queryGeneralModelName(char* out, size_t len) { VC_STR_QUERY(VC_QGMN); }
bool VoltronicMAX::queryTime(char* out, size_t len)             { VC_STR_QUERY(VC_QT); }

#undef VC_STR_QUERY

// ═══════════════════════════════════════════════════════════════
//  Data queries
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::queryGeneralStatus() {
  if (!transact(VC_QPIGS, false)) return false;
  if (!VoltronicParser::parseQPIGS(_respBuf, _qpigs)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QPIGS;
  return true;
}

bool VoltronicMAX::queryGeneralStatus2() {
  if (!transact(VC_QPIGS2, false)) return false;
  if (!VoltronicParser::parseQPIGS2(_respBuf, _qpigs2)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QPIGS2;
  return true;
}

bool VoltronicMAX::queryRating() {
  if (!transact(VC_QPIRI, false)) return false;
  if (!VoltronicParser::parseQPIRI(_respBuf, _qpiri)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QPIRI;
  return true;
}

bool VoltronicMAX::queryMode() {
  if (!transact(VC_QMOD, false)) return false;
  if (!VoltronicParser::parseQMOD(_respBuf, _mode)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QMOD;
  return true;
}

bool VoltronicMAX::queryWarnings() {
  if (!transact(VC_QPIWS, false)) return false;
  if (!VoltronicParser::parseQPIWS(_respBuf, _warningsRaw)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QPIWS;
  return true;
}

bool VoltronicMAX::queryFlags() {
  if (!transact(VC_QFLAG, false)) return false;
  if (!VoltronicParser::parseQFLAG(_respBuf, _qflag)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QFLAG;
  return true;
}

bool VoltronicMAX::queryDefaults() {
  if (!transact(VC_QDI, false)) return false;
  if (!VoltronicParser::parseQDI(_respBuf, _defaults)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QDI;
  return true;
}

bool VoltronicMAX::queryParallel(uint8_t n) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%u", VC_QPGS, n);
  if (!transact(cmd, false)) return false;
  if (!VoltronicParser::parseQPGSn(_respBuf, _parallel)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QPGS;
  return true;
}

bool VoltronicMAX::queryBatteryEqualization() {
  if (!transact(VC_QBEQI, false)) return false;
  if (!VoltronicParser::parseQBEQI(_respBuf, _beqi)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QBEQI;
  return true;
}

bool VoltronicMAX::queryLed() {
  if (!transact(VC_QLED, false)) return false;
  if (!VoltronicParser::parseQLED(_respBuf, _led)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QLED;
  return true;
}

bool VoltronicMAX::queryBatteryControl() {
  if (!transact(VC_QBATCD, false)) return false;
  if (!VoltronicParser::parseQBATCD(_respBuf, _batcd)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QBATCD;
  return true;
}

bool VoltronicMAX::queryBoot(bool& hasBootstrap) {
  if (!transact(VC_QBOOT, false)) return false;
  if (!VoltronicParser::parseQBOOT(_respBuf, hasBootstrap)) { _lastError = ERR_PARSE; return false; }
  _hasBoot = hasBootstrap;
  _validBits |= VVB_QBOOT;
  return true;
}

bool VoltronicMAX::queryMaxChargingCurrents() {
  if (!transact(VC_QMCHGCR, false)) return false;
  if (!VoltronicParser::parseQMCHGCR(_respBuf, _maxChg)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QMCHGCR;
  return true;
}

bool VoltronicMAX::queryMaxUtilityChargingCurrents() {
  if (!transact(VC_QMUCHGCR, false)) return false;
  if (!VoltronicParser::parseQMUCHGCR(_respBuf, _maxUtilChg)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QMUCHGCR;
  return true;
}

bool VoltronicMAX::queryOutputPriorityTimeOrder() {
  if (!transact(VC_QOPPT, false)) return false;
  if (!VoltronicParser::parseQOPPT(_respBuf, _outputTO)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QOPPT;
  return true;
}

bool VoltronicMAX::queryChargerPriorityTimeOrder() {
  if (!transact(VC_QCHPT, false)) return false;
  if (!VoltronicParser::parseQCHPT(_respBuf, _chargerTO)) { _lastError = ERR_PARSE; return false; }
  _validBits |= VVB_QCHPT;
  return true;
}

// ═══════════════════════════════════════════════════════════════
//  Settings
// ═══════════════════════════════════════════════════════════════
bool VoltronicMAX::setFlag(char flag) {
  char cmd[4] = { 'P', 'E', flag, 0 };
  return transact(cmd, true);
}
bool VoltronicMAX::clearFlag(char flag) {
  char cmd[4] = { 'P', 'D', flag, 0 };
  return transact(cmd, true);
}
bool VoltronicMAX::resetDefaults() { return transact(VC_PF, true); }

bool VoltronicMAX::setMaxChargingCurrent(uint16_t a) {
  char cmd[16]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_MNCHGC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxUtilityChargingCurrent(uint16_t a) {
  char cmd[16]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_MUCHGC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxDischargingCurrent(uint16_t a) {
  char cmd[16]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBATMAXDISC, a);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputVoltage(uint16_t v) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_V, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputFrequency(uint8_t hz) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_F, hz);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputSourcePriority(uint8_t p) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_POP, p);
  return transact(cmd, true);
}
bool VoltronicMAX::setOutputMode(uint8_t m) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_POPM, m);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryRechargeVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBCV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryRedischargeVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBDV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryCutoffVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PSDV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryCvVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PCVV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryFloatVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.1f", VC_PBFT, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setChargerSourcePriority(uint8_t p) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_PCP, p);
  return transact(cmd, true);
}
bool VoltronicMAX::setGridWorkingRange(uint8_t r) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_PGR, r);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryType(uint8_t t) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%02u", VC_PBT, t);
  return transact(cmd, true);
}
bool VoltronicMAX::setParallelChargerPriority(uint8_t m, uint8_t p) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%u%02u", VC_PPCP, m, p);
  return transact(cmd, true);
}
bool VoltronicMAX::resetEnergy() { return transact(VC_RTEY, true); }
bool VoltronicMAX::eraseLog()    { return transact(VC_RTDL, true); }

bool VoltronicMAX::setBatteryEqualizationEnabled(bool en) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%u", VC_PBEQE, en ? 1 : 0);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationTime(uint16_t min) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationPeriod(uint16_t days) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQP, days);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationVoltage(float v) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%.2f", VC_PBEQV, v);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryEqualizationOverTime(uint16_t min) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_PBEQOT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::activateBatteryEqualization(bool active) {
  char cmd[8]; snprintf(cmd, sizeof(cmd), "%s%u", VC_PBEQA, active ? 1 : 0);
  return transact(cmd, true);
}
bool VoltronicMAX::setMaxCvChargingTime(uint16_t min) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%03u", VC_PCVT, min);
  return transact(cmd, true);
}
bool VoltronicMAX::setDateTime(const char* s) {
  char cmd[32]; snprintf(cmd, sizeof(cmd), "%s%s", VC_DAT, s);
  return transact(cmd, true);
}
bool VoltronicMAX::setBatteryControl(uint8_t a, uint8_t b, uint8_t c) {
  char cmd[12]; snprintf(cmd, sizeof(cmd), "%s%u%u%u", VC_PBATCD, a, b, c);
  return transact(cmd, true);
}

// ═══════════════════════════════════════════════════════════════
//  Non-blocking polling
// ═══════════════════════════════════════════════════════════════
void VoltronicMAX::startPolling(const VoltronicPollSchedule& schedule) {
  _pollSchedule = schedule;
  _pollEnabled = true;
  _pollIndex = 0;
  _pollCycles = 0;
  _pollCycleDone = false;
  uint32_t now = millis();
  for (uint8_t i = 0; i < 9; i++) _pollLastFire[i] = now;
}

void VoltronicMAX::startPolling() {
  VoltronicPollSchedule def;
  startPolling(def);
}

void VoltronicMAX::stopPolling() {
  _pollEnabled = false;
}

uint16_t VoltronicMAX::getPollInterval(uint8_t idx) const {
  switch (idx) {
    case 0: return _pollSchedule.qpigsMs;
    case 1: return _pollSchedule.qpigs2Ms;
    case 2: return _pollSchedule.qpiriMs;
    case 3: return _pollSchedule.qmodMs;
    case 4: return _pollSchedule.qpiwsMs;
    case 5: return _pollSchedule.qflagMs;
    case 6: return _pollSchedule.qidMs;
    case 7: return _pollSchedule.qbeqiMs;
    case 8: return _pollSchedule.qbatcdMs;
    default: return 0;
  }
}

bool VoltronicMAX::execPollQuery(uint8_t idx) {
  switch (idx) {
    case 0: return queryGeneralStatus();
    case 1: return queryGeneralStatus2();
    case 2: return queryRating();
    case 3: return queryMode();
    case 4: return queryWarnings();
    case 5: return queryFlags();
    case 6: { char buf[24]; return querySerialNumber(buf, sizeof(buf)); }
    case 7: return queryBatteryEqualization();
    case 8: return queryBatteryControl();
    default: return false;
  }
}

bool VoltronicMAX::poll() {
  _pollCycleDone = false;
  if (!_pollEnabled) return true;

  uint32_t now = millis();
  uint8_t total = 9;

  for (uint8_t i = 0; i < total; i++) {
    uint8_t idx = (_pollIndex + i) % total;
    uint16_t interval = getPollInterval(idx);
    if (interval == 0) continue;

    if (now - _pollLastFire[idx] >= interval) {
      _pollLastFire[idx] = now;
      execPollQuery(idx);
      _pollIndex = (idx + 1) % total;

      bool anyDue = false;
      for (uint8_t j = 0; j < total; j++) {
        uint16_t iv = getPollInterval(j);
        if (iv == 0) continue;
        if (now - _pollLastFire[j] >= iv) { anyDue = true; break; }
      }
      if (!anyDue && _pollIndex == 0) {
        _pollCycles++;
        _pollCycleDone = true;
      }
      return false;
    }
  }
  return true;
}