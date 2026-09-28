// ═══════════════════════════════════════════════════════════════
//  Example 03 — General Status (QPIGS full)
// ═══════════════════════════════════════════════════════════════

#include <VoltronicMAX.h>

#if defined(ARDUINO_ARCH_ESP8266)
  #include <SoftwareSerial.h>
  #define INV_RX_PIN  D1
  #define INV_TX_PIN  D2
  SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN, false);
  #define INV_BAUD    2400
  #define IS_ESP32    0

#elif defined(ARDUINO_ARCH_ESP32)
  #define INV_RX_PIN  16
  #define INV_TX_PIN  17
  #define INV_BAUD    2400
  #define invSerial   Serial2
  #define IS_ESP32    1

#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD)
  #include <SoftwareSerial.h>
  #define INV_RX_PIN  10
  #define INV_TX_PIN  11
  SoftwareSerial invSerial(INV_RX_PIN, INV_TX_PIN);
  #define INV_BAUD    2400
  #define IS_ESP32    0
#endif

VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n=== 03_GeneralStatus ==="));

#if IS_ESP32
  Serial2.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
#else
  invSerial.begin(INV_BAUD);
#endif

  inverter.begin(INV_BAUD);
  delay(300);
}

void loop() {
  if (inverter.queryGeneralStatus()) {
    const QPIGSData &d = inverter.qpigs();

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
    Serial.printf_P(PSTR("Bus:     %u V\n"), d.busVoltage);
    Serial.println();
  } else {
    Serial.printf_P(PSTR("FAIL: %s\n"), inverter.lastErrorName());
  }
  delay(3000);
}