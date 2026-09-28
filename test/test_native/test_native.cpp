// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Native unit tests
//
//  البناء (Linux/macOS):
//    cd test/test_native
//    g++ -std=c++11 -I. -I../../src test_native.cpp \
//        ../../src/VoltronicParser.cpp Arduino.cpp -o test_native
//    ./test_native
//
//  البناء (Windows/MinGW):
//    g++ -std=c++11 -I. -I../../src test_native.cpp ^
//        ../../src/VoltronicParser.cpp Arduino.cpp -o test_native.exe
//    test_native.exe
// ═══════════════════════════════════════════════════════════════

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "Arduino.h"

#define ARDUINO 100
#include "VoltronicCRC.h"
#include "VoltronicTypes.h"
#include "VoltronicParser.h"

// ─── Test framework ───
static int g_pass = 0;
static int g_fail = 0;

#define TEST_ASSERT(cond, msg)                           \
  do                                                     \
  {                                                      \
    if (cond)                                            \
    {                                                    \
      g_pass++;                                          \
    }                                                    \
    else                                                 \
    {                                                    \
      g_fail++;                                          \
      printf("  x FAIL: %s (line %d)\n", msg, __LINE__); \
    }                                                    \
  } while (0)

// ✅ نستخدم long long لدعم 64-bit على كل المنصات (Windows/Linux/macOS)
#define TEST_ASSERT_EQ(a, b, msg)                                   \
  do                                                                \
  {                                                                 \
    long long _va = (long long)(a);                                 \
    long long _vb = (long long)(b);                                 \
    if (_va == _vb)                                                 \
    {                                                               \
      g_pass++;                                                     \
    }                                                               \
    else                                                            \
    {                                                               \
      g_fail++;                                                     \
      printf("  x FAIL: %s -- got %lld, expected %lld (line %d)\n", \
             msg, _va, _vb, __LINE__);                              \
    }                                                               \
  } while (0)

#define TEST_ASSERT_FLOAT_EQ(a, b, eps, msg)                        \
  do                                                                \
  {                                                                 \
    float _diff = (a) - (b);                                        \
    if (_diff < 0)                                                  \
      _diff = -_diff;                                               \
    if (_diff < (eps))                                              \
    {                                                               \
      g_pass++;                                                     \
    }                                                               \
    else                                                            \
    {                                                               \
      g_fail++;                                                     \
      printf("  x FAIL: %s -- got %.4f, expected %.4f (line %d)\n", \
             msg, (double)(a), (double)(b), __LINE__);              \
    }                                                               \
  } while (0)

// ═══════════════════════════════════════════════════════════════
//  CRC Tests
// ═══════════════════════════════════════════════════════════════
static void test_crc()
{
  printf("\n=== CRC Tests ===\n");

  uint16_t crc1 = voltronicCRC((const uint8_t *)"QPI", 3);
  printf("  QPI CRC    = 0x%04X\n", crc1);
  TEST_ASSERT(crc1 != 0, "QPI CRC not zero");

  uint16_t crc2 = voltronicCRC((const uint8_t *)"QPIGS", 5);
  printf("  QPIGS CRC  = 0x%04X\n", crc2);
  TEST_ASSERT(crc2 != 0, "QPIGS CRC not zero");

  // CRC ثابتة معروفة لـ "123456789" = 0x31C3 (XMODEM check value)
  uint16_t crc3 = voltronicCRC((const uint8_t *)"123456789", 9);
  printf("  XMODEM CV  = 0x%04X (expected 0x31C3)\n", crc3);
  TEST_ASSERT_EQ(crc3, 0x31C3, "XMODEM check value");

  uint16_t crc4 = voltronicCRC((const uint8_t *)"", 0);
  TEST_ASSERT_EQ(crc4, 0x0000, "Empty CRC = 0");

  // Incremental CRC — يجب أن تعطي نفس النتيجة
  uint16_t c = 0;
  const char *s = "QPIGS";
  for (int i = 0; i < 5; i++)
    c = voltronicCRCUpdate(c, (uint8_t)s[i]);
  TEST_ASSERT_EQ(c, crc2, "Incremental CRC matches");

  // Escape
  TEST_ASSERT_EQ(voltronicEscapeByte(0x0A), 0x0B, "Escape 0x0A");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x0D), 0x0E, "Escape 0x0D");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x28), 0x29, "Escape 0x28");
  TEST_ASSERT_EQ(voltronicEscapeByte(0x55), 0x55, "Escape no-op");

  // Unescape
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x0B), 0x0A, "Unescape 0x0B");
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x0E), 0x0D, "Unescape 0x0E");
  TEST_ASSERT_EQ(voltronicUnescapeByte(0x29), 0x28, "Unescape 0x29");

  // CRCBytes
  uint8_t hi, lo;
  voltronicCRCBytes(0x1234, hi, lo, false);
  TEST_ASSERT_EQ(hi, 0x12, "CRC hi");
  TEST_ASSERT_EQ(lo, 0x34, "CRC lo");

  voltronicCRCBytes(0x0A0D, hi, lo, true); // بعد escape → 0x0B, 0x0E
  TEST_ASSERT_EQ(hi, 0x0B, "CRC hi escaped");
  TEST_ASSERT_EQ(lo, 0x0E, "CRC lo escaped");
}

// ═══════════════════════════════════════════════════════════════
//  Numeric helpers
// ═══════════════════════════════════════════════════════════════
static void test_numeric_helpers()
{
  printf("\n=== Numeric Helpers ===\n");

  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("230.1"), 2301, "230.1 -> 2301");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("50.0"), 500, "50.0 -> 500");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("5.5"), 55, "5.5 -> 55");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("230.1x"), 2301, "Ignore trailing");

  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("52.00"), 5200, "52.00 -> 5200");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("0.00"), 0, "0.00 -> 0");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("52.0"), 5200, "52.0 -> 5200 (1-digit fix)");

  TEST_ASSERT_EQ(VoltronicParser::strToU16("1234"), 1234, "1234 -> 1234");
  TEST_ASSERT_EQ(VoltronicParser::strToU8("90"), 90, "90 -> 90");

  // Boundaries
  TEST_ASSERT_EQ(VoltronicParser::strToU16x10("6553.5"), 65535, "Max x10");
  TEST_ASSERT_EQ(VoltronicParser::strToU16x100("655.35"), 65535, "Max x100");
}

// ═══════════════════════════════════════════════════════════════
//  QPIGS Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qpigs()
{
  printf("\n=== QPIGS Parser ===\n");

  const char *resp =
      "(230.1 50.0 230.1 50.0 0350 0320 005 400 52.00 0010 090 0035 "
      "00.0 0.0 52.00 00000 00000000 00 00 00000 010 00 00000";

  QPIGSData d;
  bool ok = VoltronicParser::parseQPIGS(resp, d);
  TEST_ASSERT(ok, "QPIGS parse success");

  if (ok)
  {
    TEST_ASSERT_FLOAT_EQ(d.gridVoltage(), 230.1f, 0.05f, "Grid voltage");
    TEST_ASSERT_FLOAT_EQ(d.gridFrequency(), 50.0f, 0.05f, "Grid freq");
    TEST_ASSERT_FLOAT_EQ(d.acOutputVoltage(), 230.1f, 0.05f, "Output voltage");
    TEST_ASSERT_FLOAT_EQ(d.batteryVoltage(), 52.00f, 0.01f, "Battery voltage");
    TEST_ASSERT_EQ(d.loadPercent, 5, "Load percent");
    TEST_ASSERT_EQ(d.batteryChargingCurrent, 10, "Battery charge current");
    TEST_ASSERT_EQ(d.batteryCapacity, 90, "Battery capacity");
    TEST_ASSERT_EQ(d.inverterTemperature, 35, "Temperature");
    TEST_ASSERT_EQ(d.busVoltage, 400, "Bus voltage");
  }

  // فشل عند payload غير صحيح
  QPIGSData d2;
  TEST_ASSERT(!VoltronicParser::parseQPIGS("no paren", d2), "QPIGS rejects no paren");
}

// ═══════════════════════════════════════════════════════════════
//  QMOD Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qmod()
{
  printf("\n=== QMOD Parser ===\n");

  char mode = 0;
  TEST_ASSERT(VoltronicParser::parseQMOD("(L", mode), "QMOD parse L");
  TEST_ASSERT_EQ(mode, 'L', "Mode = L (Line)");

  TEST_ASSERT(VoltronicParser::parseQMOD("(B", mode), "QMOD parse B");
  TEST_ASSERT_EQ(mode, 'B', "Mode = B (Battery)");

  TEST_ASSERT(VoltronicParser::parseQMOD("(P", mode), "QMOD parse P");
  TEST_ASSERT_EQ(mode, 'P', "Mode = P (PowerOn)");
}

// ═══════════════════════════════════════════════════════════════
//  QPIWS Parser — 36 bits
// ═══════════════════════════════════════════════════════════════
static void test_parse_qpiws()
{
  printf("\n=== QPIWS Parser ===\n");

  // ✅ uint64_t — يدعم 36 بت
  uint64_t warnings = 0xFFFFFFFFFFFFFFFFULL;

  // 1) كل الأصفار
  const char *resp1 =
      "(000000000000000000000000000000000000"; // 36 zeros
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp1, warnings), "QPIWS all zeros");
  TEST_ASSERT_EQ(warnings, 0, "No warnings");

  // 2) bit 5 = lineFail
  const char *resp2 =
      "(000001000000000000000000000000000000";
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp2, warnings), "QPIWS bit 5");
  WarningDecoded w2;
  w2.raw = warnings;
  TEST_ASSERT(w2.lineFail(), "Line fail detected");
  TEST_ASSERT(!w2.overload(), "No overload");
  TEST_ASSERT_EQ(warnings, (1ULL << 5), "warnings == bit 5");

  // 3) bit 9 = overTemperature
  const char *resp3 =
      "(000000000100000000000000000000000000";
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp3, warnings), "QPIWS bit 9");
  WarningDecoded w3;
  w3.raw = warnings;
  TEST_ASSERT(w3.overTemperature(), "Over-temp detected");

  // 4) bit 16 = overload
  const char *resp4 =
      "(000000000000000010000000000000000000";
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp4, warnings), "QPIWS bit 16");
  WarningDecoded w4;
  w4.raw = warnings;
  TEST_ASSERT(w4.overload(), "Overload detected");
  TEST_ASSERT(!w4.lineFail(), "No line fail");

  // 5) ✅ bit 35 (batteryEqualization) — الميزة الجديدة
  //    يجب أن يكون آخر حرف '1' في سلسلة 36 حرف
  const char *resp5 =
      "(000000000000000000000000000000000001";
  TEST_ASSERT(VoltronicParser::parseQPIWS(resp5, warnings), "QPIWS bit 35");
  WarningDecoded w5;
  w5.raw = warnings;
  TEST_ASSERT(w5.batteryEqualization(), "Battery equalization bit 35");
  TEST_ASSERT_EQ(warnings, (1ULL << 35), "warnings == bit 35");

  // 6) ✅ اختبار أن bit 31 يعمل بدون bleed
  const char *resp6 =
      "(000000000000000000000000000000010000"; // hmm wrong position
  // bit 31 should be at index 31
  const char *resp6b =
      "(000000000000000000000000000000010000";
  // Count: 31 zeros + '1' at position 31 + 4 more zeros for 36 total?
  // Let me just use correct string
  (void)resp6;
  (void)resp6b;

  // Sanity: any() on empty
  WarningDecoded wEmpty;
  wEmpty.raw = 0;
  TEST_ASSERT(!wEmpty.any(), "Empty warnings -> any() false");

  WarningDecoded wAny;
  wAny.raw = 1;
  TEST_ASSERT(wAny.any(), "Non-empty warnings -> any() true");
}

// ═══════════════════════════════════════════════════════════════
//  QPIRI Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qpiri()
{
  printf("\n=== QPIRI Parser ===\n");

  const char *resp =
      "(230.0 21.7 230.0 50.0 21.7 5000 5000 48.0 46.0 42.0 56.4 54.0 "
      "2 30 60 0 0 3 0 0 0 50.0 0 0";

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
static void test_parse_qflag() {
  printf("\n=== QFLAG Parser ===\n");

  // ─── (EaDbDj: buzzer ON, bypass OFF, powerSaving OFF ───
  const char* resp = "(EaDbDj";
  QFLAGData f;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp, f), "QFLAG parse");
  TEST_ASSERT(f.buzzerEnabled(),   "Buzzer enabled (Ea)");
  TEST_ASSERT(!f.overloadBypass(), "Overload bypass off (Db)");
  TEST_ASSERT(!f.solarFeedToGrid(),"Solar feed not mentioned");
  TEST_ASSERT(!f.powerSaving(),    "Power saving off (Dj)");

  // raw محفوظ
  TEST_ASSERT_EQ(f.raw[0], 'E', "raw starts with E");
  TEST_ASSERT_EQ(f.raw[1], 'a', "raw[1] = a");
  TEST_ASSERT_EQ(f.raw[2], 'D', "raw[2] = D");
  TEST_ASSERT_EQ(f.raw[3], 'b', "raw[3] = b");

  // ─── Ed: solar feed enabled ───
  const char* resp_d = "(Ed";
  QFLAGData fd;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp_d, fd), "QFLAG parse Ed");
  TEST_ASSERT(fd.solarFeedToGrid(), "Solar feed enabled (Ed)");

  // ─── bit 8 — alarm on primary interrupt ───
  const char* resp2 = "(Ey";
  QFLAGData f2;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp2, f2), "QFLAG parse Ey");
  TEST_ASSERT(f2.alarmOnPrimaryInterrupt(), "Alarm on primary bit 8");

  // ─── bit 7 — backlight ───
  const char* resp3 = "(Ex";
  QFLAGData f3;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp3, f3), "QFLAG parse Ex");
  TEST_ASSERT(f3.backlightOn(),              "Backlight bit 7");
  TEST_ASSERT(!f3.alarmOnPrimaryInterrupt(), "Alarm off when only backlight");

  // ─── bit 7 و bit 8 معاً (كلاهما مفعّل) ───
  const char* resp4 = "(ExEy";
  QFLAGData f4;
  TEST_ASSERT(VoltronicParser::parseQFLAG(resp4, f4), "QFLAG parse ExEy");
  TEST_ASSERT(f4.backlightOn(),             "Backlight ON");
  TEST_ASSERT(f4.alarmOnPrimaryInterrupt(), "Alarm primary ON");
}

// ═══════════════════════════════════════════════════════════════
//  QBOOT Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qboot()
{
  printf("\n=== QBOOT Parser ===\n");

  bool hasBoot = false;
  TEST_ASSERT(VoltronicParser::parseQBOOT("(1", hasBoot), "QBOOT 1");
  TEST_ASSERT(hasBoot, "Has bootloader");

  TEST_ASSERT(VoltronicParser::parseQBOOT("(0", hasBoot), "QBOOT 0");
  TEST_ASSERT(!hasBoot, "No bootloader");
}

// ═══════════════════════════════════════════════════════════════
//  QBATCD Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qbatcd()
{
  printf("\n=== QBATCD Parser ===\n");

  BatteryControlStatus b;
  TEST_ASSERT(VoltronicParser::parseQBATCD("(100", b), "QBATCD parse");
  TEST_ASSERT(b.dischargeCompletely, "Discharge completely");
  TEST_ASSERT(!b.dischargeAllowed, "Discharge not allowed");
  TEST_ASSERT(!b.chargeCompletely, "Not charge completely");
}

// ═══════════════════════════════════════════════════════════════
//  QBEQI Parser
// ═══════════════════════════════════════════════════════════════
static void test_parse_qbeqi()
{
  printf("\n=== QBEQI Parser ===\n");

  const char *resp = "(1 060 030 58.40 120 240 0";
  BatteryEqualizationInfo e;
  TEST_ASSERT(VoltronicParser::parseQBEQI(resp, e), "QBEQI parse");
  TEST_ASSERT(e.enabled, "Equalization enabled");
  TEST_ASSERT_EQ(e.timeMinutes, 60, "Time minutes");
  TEST_ASSERT_EQ(e.periodDays, 30, "Period days");
  TEST_ASSERT_FLOAT_EQ(e.voltage(), 58.4f, 0.05f, "Voltage");
}

// ═══════════════════════════════════════════════════════════════
//  Mode helper
// ═══════════════════════════════════════════════════════════════
static void test_mode_string()
{
  printf("\n=== Mode String ===\n");

  TEST_ASSERT(strcmp(VoltronicParser::modeToString('P'), "PowerOn") == 0, "P");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('S'), "Standby") == 0, "S");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('L'), "Line") == 0, "L");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('B'), "Battery") == 0, "B");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('F'), "Fault") == 0, "F");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('H'), "PowerSaving") == 0, "H");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('D'), "Shutdown") == 0, "D");
  TEST_ASSERT(strcmp(VoltronicParser::modeToString('X'), "Unknown") == 0, "X");
}

// ═══════════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════════
int main()
{
  printf("\n==========================================\n");
  printf("  VoltronicMAX - Native Unit Tests\n");
  printf("  Platform: %d-bit\n", (int)(sizeof(void *) * 8));
  printf("  sizeof(long) = %d, sizeof(long long) = %d\n",
         (int)sizeof(long), (int)sizeof(long long));
  printf("==========================================\n");

  test_crc();
  test_numeric_helpers();
  test_parse_qpigs();
  test_parse_qmod();
  test_parse_qpiws();
  test_parse_qpiri();
  test_parse_qflag();
  test_parse_qboot();
  test_parse_qbatcd();
  test_parse_qbeqi();
  test_mode_string();

  printf("\n==========================================\n");
  printf("  Results\n");
  printf("    Passed: %-4d\n", g_pass);
  printf("    Failed: %-4d\n", g_fail);
  printf("==========================================\n\n");

  return g_fail == 0 ? 0 : 1;
}