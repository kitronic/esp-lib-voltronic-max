// ═══════════════════════════════════════════════════════════════
//  Example 01 — Basic Read
//  يدعم ESP8266, ESP32, AVR, SAMD
// ═══════════════════════════════════════════════════════════════

#include <VoltronicMAX.h>

// ═══════════════════════════════════════════════════════════════
//  Platform-specific UART setup
// ═══════════════════════════════════════════════════════════════
#if defined(ESP8266)
// ─── ESP8266: SoftwareSerial ───
#include <SoftwareSerial.h>
#define INV_RX_PIN D1 // GPIO5
#define INV_TX_PIN D2 // GPIO4
SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN, false);
#define INV_BAUD 2400

#elif defined(ESP32)
  // ─── ESP32: HardwareSerial (Serial2) ───
#define INV_RX_PIN 16
#define INV_TX_PIN 17
#define INV_BAUD 2400
#define invSerial Serial2

#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
  // ─── AVR / SAMD: SoftwareSerial ───
#include <SoftwareSerial.h>
#define INV_RX_PIN 10
#define INV_TX_PIN 11
SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN);
#define INV_BAUD 2400

#else
#error "Unsupported platform"
#endif

VoltronicMAX inverter(invSerial);

// ═══════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println(F("=== 01_BasicRead ==="));

#if defined(ESP32)
  Serial2.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
#else
  invSerial.begin(INV_BAUD);
#endif

  inverter.begin(INV_BAUD);
  Serial.printf_P(PSTR("UART ready: %u 8N1 (RX=GPIO%d, TX=GPIO%d)\n"),
                  INV_BAUD, INV_RX_PIN, INV_TX_PIN);
  Serial.println(F("Querying QPIGS every 3s...\n"));
  delay(300);
}

// ═══════════════════════════════════════════════════════════════
void loop()
{
  if (inverter.queryGeneralStatus())
  {
    const QPIGSData &d = inverter.qpigs();

    Serial.println(F("─── QPIGS ───"));
    Serial.printf_P(PSTR("Grid:    %.1f V  %.1f Hz\n"),
                    d.gridVoltage(), d.gridFrequency());
    Serial.printf_P(PSTR("Output:  %.1f V  %.1f Hz\n"),
                    d.acOutputVoltage(), d.acOutputFrequency());
    Serial.printf_P(PSTR("Load:    %u VA  %u W  (%u%%)\n"),
                    d.acOutputApparentPower, d.acOutputActivePower, d.loadPercent);
    Serial.printf_P(PSTR("Battery: %.2f V  %u A  %u%%  %u C\n"),
                    d.batteryVoltage(), d.batteryChargingCurrent,
                    d.batteryCapacity, d.inverterTemperature);
    Serial.printf_P(PSTR("PV1:     %.1f V  %.1f A  %u W\n"),
                    d.pv1InputVoltage(), d.pv1InputCurrent(), d.pv1ChargingPower);
    Serial.println(F("─────────────\n"));
  }
  else
  {
    Serial.printf_P(PSTR("[QPIGS] FAIL: %s\n"), inverter.lastErrorName());
  }
  delay(3000);
}