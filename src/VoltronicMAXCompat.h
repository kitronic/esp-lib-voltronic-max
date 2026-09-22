#pragma once
// ═══════════════════════════════════════════════════════════════
//  Compat shim — يخلي كودك القديم يشتغل مع المكتبة الجديدة
//  بدون ما تغير استدعاءاتك
// ═══════════════════════════════════════════════════════════════

#include "VoltronicMAX.h"

// Global instance — يتعرف على SoftwareSerial أو HardwareSerial
// بما إن Stream هي base class، الاثنين يشتغلوا
extern VoltronicMAX* voltronicInstance;

inline void voltronicAttach(Stream& serial, uint32_t baud = 2400) {
  static VoltronicMAX inst(serial);
  voltronicInstance = &inst;
  inst.begin(baud);
  inst.setTimeout(800);
}

// ─── Wrappers متوافقة مع أسماء دوالك ───
inline String sendInverterCmd(String cmd) {
  if (!voltronicInstance) return "";
  voltronicInstance->sendRaw(cmd.c_str());
  if (voltronicInstance->lastError() != VoltronicMAX::ERR_NONE) return "";
  return String(voltronicInstance->lastResponse());
}

inline String sendInverterCmd(const char* cmd) {
  return sendInverterCmd(String(cmd));
}

// ─── CRC ───
inline uint16_t calculateCRC(const uint8_t* p, uint8_t len) {
  return voltronicCRC(p, len);
}

// ─── Parsers — نفس الأسماء ───
inline void parseQPIGS(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQPIGS(resp.c_str(), voltronicInstance->dataRef());
}

inline void parseQPIGS2(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQPIGS2(resp.c_str(), voltronicInstance->dataRef());
}

inline void parseQPIRI(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQPIRI(resp.c_str(), voltronicInstance->ratingRef());
}

inline void parseQMOD(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQMOD(resp.c_str(), voltronicInstance->dataRef());
}

inline void parseQPIWS(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQPIWS(resp.c_str(), voltronicInstance->dataRef());
}

inline void parseQFLAG(String resp) {
  if (!voltronicInstance) return;
  VoltronicParser::parseQFLAG(resp.c_str(), voltronicInstance->flagsRef());
}

inline void parseQID(String resp) {
  if (!voltronicInstance) return;
  if (resp.startsWith("(")) {
    // احفظه بمكانك
  }
}

// ─── Settings wrappers ───
inline void setOutputPriority(String p) {
  p.trim();
  if      (p == "UtilitySolarBat")  voltronicInstance->setOutputSourcePriority(0);
  else if (p == "SolarUtilityBat")  voltronicInstance->setOutputSourcePriority(1);
  else if (p == "SolarBatUtility")  voltronicInstance->setOutputSourcePriority(2);
}

inline void setChargerPriority(String p) {
  p.trim();
  if      (p == "Solar first")     voltronicInstance->setChargerSourcePriority(1);
  else if (p == "Solar + Utility") voltronicInstance->setChargerSourcePriority(2);
  else if (p == "Only solar")      voltronicInstance->setChargerSourcePriority(3);
}

inline void setInputVoltageRange(String r) {
  r.trim();
  if      (r == "Appliance") voltronicInstance->setGridWorkingRange(0);
  else if (r == "UPS")       voltronicInstance->setGridWorkingRange(1);
}

inline void setMaxChargeCurrent(int amps) {
  voltronicInstance->setMaxChargingCurrent(amps);
}

inline void setMaxUtilChargeCurrent(int amps) {
  voltronicInstance->setMaxUtilityChargingCurrent(amps);
}