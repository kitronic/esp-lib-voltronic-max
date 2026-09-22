#pragma once
#include <Arduino.h>
#include "VoltronicTypes.h"

class VoltronicParser {
public:
  // ─── Helpers ───
  static void   skipSpaces(const char*& p);
  static bool   readToken(const char*& p, char* out, size_t maxLen);
  static bool   readUInt(const char*& p, uint32_t& out, uint8_t digits);
  static bool   readBits(const char*& p, uint8_t n, uint32_t& out);

  // ─── Internal numeric parsers ───
  static uint16_t strToU16x10(const char* s);   // "230.1" → 2301
  static uint16_t strToU16x100(const char* s);  // "52.00" → 5200
  static uint16_t strToU16(const char* s);      // "1234"  → 1234
  static uint8_t  strToU8(const char* s);       // "12"    → 12

  // ═══════════════════════════════════════════════════════════
  //  INQUIRY PARSERS — كلها موجودة
  // ═══════════════════════════════════════════════════════════
  static bool parseQPIGS  (const char* payload, QPIGSData&  out);
  static bool parseQPIGS2 (const char* payload, QPIGS2Data& out);
  static bool parseQPIRI  (const char* payload, QPIRIData&  out);
  static bool parseQMOD   (const char* payload, char& modeOut);
  static bool parseQPIWS  (const char* payload, uint32_t& warningsOut);
  static bool parseQFLAG  (const char* payload, QFLAGData&  out);
  static bool parseQPGSn  (const char* payload, ParallelInfo& out);
  static bool parseQBEQI  (const char* payload, BatteryEqualizationInfo& out);
  static bool parseQLED   (const char* payload, LedInfo& out);
  static bool parseQDI    (const char* payload, DefaultsInfo& out);
  static bool parseQBATCD (const char* payload, BatteryControlStatus& out);
  static bool parseQBATCDRaw(const char* payload, char& a, char& b, char& c);
  static bool parseQMCHGCR(const char* payload, SelectableValues& out);
  static bool parseQMUCHGCR(const char* payload, SelectableValues& out);
  static bool parseQOPPT  (const char* payload, TimeOrderInfo& out);
  static bool parseQCHPT  (const char* payload, TimeOrderInfo& out);
  static bool parseQBOOT  (const char* payload, bool& hasBootstrap);

  // ─── Mode helper ───
  static const char* modeToString(char m);
};