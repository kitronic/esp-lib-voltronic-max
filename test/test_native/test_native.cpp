// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Native unit tests
//  البناء:
//    cd test/test_native
//    g++ -std=c++11 -I../../src test_native.cpp \
//        ../../src/VoltronicParser.cpp -o test_native
//    ./test_native
// ═══════════════════════════════════════════════════════════════

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "Arduino.h"  

// نضمن أن parser يعمل بدون Arduino
#define ARDUINO 100
#include "VoltronicCRC.h"
#include "VoltronicTypes.h"
#include "VoltronicParser.h"

// ─── Test framework ───
static int g_pass = 0;
static int g_fail = 0;

#define TEST_ASSERT(cond, msg) do { \
  if (cond) { g_pass++; } \
  else { g_fail++; printf("  x FAIL: %s (line %d)\n", msg, __LINE__); } \
} while(0)

#define TEST_ASSERT_EQ(a, b, msg) do { \
  if ((long)(a) == (long)(b)) { g_pass++; } \
  else { g_fail++; printf("  x FAIL: %s -- got %ld, expected %ld (line %d)\n", \
                          msg, (long)(a), (long)(b), __LINE__); } \
} while(0)

#define TEST_ASSERT_FLOAT_EQ(a, b, eps, msg) do { \
  float _diff = (a) - (b); if (_diff < 0) _diff = -_diff; \
  if (_diff < (eps)) { g_pass++; } \
  else { g_fail++; printf("  x FAIL: %s -- got %.4f, expected %.4f (line %d)\n", \
                          msg, (double)(a), (double)(b), __LINE__); } \
} while(0)

// ═══════════════════════════════════════════════════════════════
//  CRC Tests
// ═══════════════════════════════════════════════════════════════
void test_crc() {
  printf("\n=== CRC Tests ===\n");

  const char* cmd1 = "QPI";
  uint16_t crc1 = voltronicCRC((const uint8_t*)cmd1, 3);
  printf("  QPI CRC   = 0x%04X\n", crc1);
  TEST_ASSERT(crc1 != 0, "QPI CRC not zero");

  const char* cmd2 = "QPIGS";
  uint16_t crc2 = voltronicCRC((const uint8_t*)cmd2, 5);
  printf("  QPIGS CRC = 0x%04X\n", crc2);
  TEST_ASSERT(crc2 != 0, "QPIGS CRC not zero");

  uint16_t crc3 = voltronicCRC((const uint8_t*)"", 0);
  TEST_ASSERT_EQ(crc3, 0x0000, "Empty CRC = 0");

  // Escape byte
  TEST_ASSERT_EQ(voltronicEscapeByte(0x0A), 0x0B, "Escape 0x0A");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x0D), 0x0E, "Escape 0x0D");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x28), 0x29, "Escape 0x28");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x55), 0x55, "Escape no-op");

  // Unescape
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x0B), 0x0A, "Unescape 0x0B");
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x0E), 0x0D, "Unescape 0x0E");
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x29), 0x28, "Unescape 0x29");
}

// ═══════════════════════════════════════════════════════════════
//  Numeric helpers
// ═══════════════════════════════════════════════════════════════
void test_numeric_helpers() {
  printf("\n=== Numeric Helpers ===\n");

  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("230.1"), 2301, "230.1 -> 2301");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("50.0"),  500,  "50.0 -> 500");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("5.5"),   55,   "5.5 -> 55");

  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("52.00"), 5200, "52.00 -> 5200");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("0.00"),  0,    "0.00 -> 0");

  TEST_ASSERT_EQ(VoltronicParser::strToU16("1234"), 1234, "1234 -> 1234");
  TEST_ASSERT_EQ(VoltronicParser::strToU8("90"),    90,   "90 -> 90");
}

// ═══════════════════════════════════════════════════════════════
//  QPIGS Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qpigs() {
  printf("\n=== QPIGS Parser ===\n");

  const char* resp = "(230.1 50.0 230.1 50.0 0350 0320 005 400 52.00 0010 090 0035 00.0 0.0 52.00 00000 00000000 00 00 00000 010 00 00000";

  QPIGSData d;
  bool ok = VoltronicParser::parseQPIGS(resp, d);

  TEST_ASSERT(ok, "QPIGS parse success");
  if (ok) {
    TEST_ASSERT_FLOAT_EQ(d.gridVoltage(), 230.1f, 0.05f, "Grid voltage");
    TEST_ASSERT_FLOAT_EQ(d.gridFrequency(), 50.0f, 0.05f, "Grid freq");
    TEST_ASSERT_FLOAT_EQ(d.acOutputVoltage(), 230.1f, 0.05f, "Output voltage");
    TEST_ASSERT_FLOAT_EQ(d.batteryVoltage(), 52.00f, 0.01f, "Battery voltage");
    TEST_ASSERT_EQ(d.loadPercent, 5, "Load percent");
    TEST_ASSERT_EQ(d.batteryChargingCurrent, 10, "Battery charge current");
    TEST_ASSERT_EQ(d.batteryCapacity, 90, "Battery capacity");
    TEST_ASSERT_EQ(d.inverterTemperature, 35, "Temperature");
  }
}

// ═══════════════════════════════════════════════════════════════
//  QMOD Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qmod() {
  printf("\n=== QMOD Parser ===\n");

  char mode = 0;
  TEST_ASSERT(VoltronicParser::parseQMOD("(L", mode), "QMOD parse");
  TEST_ASSERT_EQ(mode, 'L', "Mode = L (Line)");

  TEST_ASSERT(VoltronicParser::parseQMOD("(B", mode), "QMOD parse");
  TEST_ASSERT_EQ(mode, 'B', "Mode = B (Battery)");
}

// ═══════════════════════════════════════════════════════════════
//  QPIWS Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qpiws() {
  printf("\n=== QPIWS Parser ===\n");

  const char* resp1 = "(00000000000000000000000000000000";
  uint32_t warnings = 0xFFFFFFFF;
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp1, warnings), "QPIWS parse 1");
  TEST_ASSERT_EQ(warnings, 0, "No warnings");

  const char* resp2 = "(00000100000000000000000000000000";
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp2, warnings), "QPIWS parse 2");
  WarningDecoded w2; w2.raw = warnings;
  TEST_ASSERT(w2.lineFail(), "Line fail detected");
  TEST_ASSERT(!w2.overload(), "No overload");
}

// ═══════════════════════════════════════════════════════════════
//  QPIRI Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qpiri() {
  printf("\n=== QPIRI Parser ===\n");

  const char* resp = "(230.0 21.7 230.0 50.0 21.7 5000 5000 48.0 46.0 42.0 56.4 54.0 2 30 60 0 0 3 0 0 0 50.0 0 0";

  QPIRIData r;
  TEST_ASSERT(VoltronicParser::parseQPIRI(resp, r), "QPIRI parse");

  TEST_ASSERT_FLOAT_EQ(r.gridRatingVoltage(), 230.0f, 0.05f, "Grid V");
  TEST_ASSERT_FLOAT_EQ(r.acOutputRatingFrequency(), 50.0f, 0.05f, "Out Hz");
  TEST_ASSERT_EQ(r.batteryType, 2, "Battery type");
  TEST_ASSERT_EQ(r.maxChargingCurrent, 60, "Max charge");
  TEST_ASSERT_FLOAT_EQ(r.batteryBulkVoltage(), 56.4f, 0.05f, "Bulk V");
  TEST_ASSERT_FLOAT_EQ(r.batteryFloatVoltage(), 54.0f, 0.05f, "Float V");
}

// ═══════════════════════════════════════════════════════════════
//  QFLAG Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qflag() {
  printf("\n=== QFLAG Parser ===\n");

  const char* resp = "(EaDbDj";
  QFLAGData f;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp, f), "QFLAG parse");
  TEST_ASSERT(f.buzzerEnabled(), "Buzzer enabled");
  TEST_ASSERT(!f.powerSaving(), "Power saving off");
}

// ═══════════════════════════════════════════════════════════════
//  QBOOT Parser
// ═══════════════════════════════════════════════════════════════
void test_parse_qboot() {
  printf("\n=== QBOOT Parser ===\n");

  bool hasBoot = false;
  TEST_ASSERT(VoltronicParser::parseQBOOT("(1", hasBoot), "QBOOT 1");
  TEST_ASSERT(hasBoot, "Has bootloader");

  TEST_ASSERT(VoltronicParser::parseQBOOT("(0", hasBoot), "QBOOT 0");
  TEST_ASSERT(!hasBoot, "No bootloader");
}

// ═══════════════════════════════════════════════════════════════
//  Mode helper
// ═══════════════════════════════════════════════════════════════
void test_mode_string() {
  printf("\n=== Mode String ===\n");

  TEST_ASSERT(strcmp(VoltronicParser::modeToString('P'), "PowerOn") == 0, "P");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('L'), "Line") == 0,    "L");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('B'), "Battery") == 0, "B");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('F'), "Fault") == 0,   "F");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('X'), "Unknown") == 0, "X");
}

// ═══════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════
int main() {
  printf("\n==========================================\n");
  printf("  VoltronicMAX - Native Unit Tests\n");
  printf("==========================================\n");

  test_crc();
  test_numeric_helpers();
  test_parse_qpigs();
  test_parse_qmod();
  test_parse_qpiws();
  test_parse_qpiri();
  test_parse_qflag();
  test_parse_qboot();
  test_mode_string();

  printf("\n==========================================\n");
  printf("  Results\n");
  printf("    Passed: %-4d\n", g_pass);
  printf("    Failed: %-4d\n", g_fail);
  printf("==========================================\n\n");

  return g_fail == 0 ? 0 : 1;
}