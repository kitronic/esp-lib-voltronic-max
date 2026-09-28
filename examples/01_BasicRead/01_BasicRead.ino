// ═══════════════════════════════════════════════════════════════
//  Example 01 — Basic Read
//  يدعم ESP8266, ESP32, AVR, SAMD
// ═══════════════════════════════════════════════════════════════

#include <VoltronicMAX.h>

// ═══════════════════════════════════════════════════════════════
//  Platform detection (ARDUINO_ARCH_*)
// ═══════════════════════════════════════════════════════════════
#if defined(ARDUINO_ARCH_ESP8266)
#include <SoftwareSerial.h>
#define INV_RX_PIN D1
#define INV_TX_PIN D2
SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN, false);
#define INV_BAUD 2400
#define IS_ESP32 0
#define HAS_PRINTF 1

#elif defined(ARDUINO_ARCH_ESP32)
#define INV_RX_PIN 16
#define INV_TX_PIN 17
#define INV_BAUD 2400
#define invSerial Serial2
#define IS_ESP32 1
#define HAS_PRINTF 1

#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
#include <SoftwareSerial.h>
#define INV_RX_PIN 10
#define INV_TX_PIN 11
SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN);
#define INV_BAUD 2400
#define IS_ESP32 0
#define HAS_PRINTF 0

#else
#error "Unsupported platform"
#endif

// ─── Logging macro ───
#if HAS_PRINTF
#define LOG(fmt, ...) Serial.printf_P(PSTR(fmt), ##__VA_ARGS__)
#else
#define LOG(fmt, ...) \
  do                  \
  {                   \
  } while (0)
#endif

VoltronicMAX inverter(invSerial);

// ═══════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("=== 01_BasicRead ==="));

#if IS_ESP32
  Serial2.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
#else
  invSerial.begin(INV_BAUD);
#endif

  inverter.begin(INV_BAUD);
  Serial.println(F("UART ready. Querying QPIGS every 3s...\n"));
  delay(300);
}

// ═══════════════════════════════════════════════════════════════
void loop()
{
  if (inverter.queryGeneralStatus())
  {
    const QPIGSData &d = inverter.qpigs();

    LOG("Grid:    %.1f V  %.1f Hz\n",
        d.gridVoltage(), d.gridFrequency());
    LOG("Output:  %.1f V  %.1f Hz\n",
        d.acOutputVoltage(), d.acOutputFrequency());
    LOG("Load:    %u VA  %u W  (%u%%)\n",
        d.acOutputApparentPower, d.acOutputActivePower, d.loadPercent);
    LOG("Battery: %.2f V  %u A  %u%%  %u C\n",
        d.batteryVoltage(), d.batteryChargingCurrent,
        d.batteryCapacity, d.inverterTemperature);
    LOG("PV1:     %.1f V  %.1f A  %u W\n",
        d.pv1InputVoltage(), d.pv1InputCurrent(), d.pv1ChargingPower);
    LOG("─────────────\n");

#if !HAS_PRINTF
    // AVR/SAMD: نص بدون قيم
    Serial.println(F("QPIGS OK"));
#endif
  }
  else
  {
    LOG("[QPIGS] FAIL: %s\n", inverter.lastErrorName());
#if !HAS_PRINTF
    Serial.println(F("QPIGS FAIL"));
#endif
  }
  delay(3000);
}