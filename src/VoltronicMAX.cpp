#include "VoltronicMAX.h"
#include <string.h>
#include <stdlib.h>

// ═══════════════════════════════════════════════════════════════
//  (احتفظ بكل الكود السابق من begin, buildFrame, checkCRC, transact)
// ═══════════════════════════════════════════════════════════════

// ─── String queries ───
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

// ─── Data queries ───
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

// ─── Settings ───
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