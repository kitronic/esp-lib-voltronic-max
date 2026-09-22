#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 07 QPIWS ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryWarnings()) {
    WarningDecoded w = inverter.warnings();
    Serial.printf("Raw: 0x%08lX\n", (unsigned long)w.raw);

    if (!w.any()) { Serial.println(F("No warnings.")); }
    else {
      if (w.pvLoss())               Serial.println(F("PV loss"));
      if (w.inverterFault())        Serial.println(F("Inverter fault"));
      if (w.busOver())              Serial.println(F("Bus over"));
      if (w.busUnder())             Serial.println(F("Bus under"));
      if (w.busSoftFail())          Serial.println(F("Bus soft fail"));
      if (w.lineFail())             Serial.println(F("Line fail"));
      if (w.opvShort())             Serial.println(F("OPV short"));
      if (w.inverterVoltageLow())   Serial.println(F("Inverter V low"));
      if (w.inverterVoltageHigh())  Serial.println(F("Inverter V high"));
      if (w.overTemperature())      Serial.println(F("Over temp"));
      if (w.fanLocked())            Serial.println(F("Fan locked"));
      if (w.batteryVoltageHigh())   Serial.println(F("Batt V high"));
      if (w.batteryLowAlarm())      Serial.println(F("Batt low"));
      if (w.batteryUnderShutdown()) Serial.println(F("Batt under shutdown"));
      if (w.batteryDerating())      Serial.println(F("Batt derating"));
      if (w.overload())             Serial.println(F("Overload"));
      if (w.eepromFault())          Serial.println(F("EEPROM fault"));
      if (w.inverterOverCurrent())  Serial.println(F("Inverter OC"));
      if (w.inverterSoftFail())     Serial.println(F("Inverter soft fail"));
      if (w.selfTestFail())         Serial.println(F("Self-test fail"));
      if (w.opDcVoltageOver())      Serial.println(F("OP DC V over"));
      if (w.batOpen())              Serial.println(F("Battery open"));
      if (w.currentSensorFail())    Serial.println(F("Current sensor fail"));
      if (w.batteryWeak())          Serial.println(F("Battery weak"));
      if (w.batteryEqualization())  Serial.println(F("Battery equalization"));
    }
  }
  delay(3000);
}