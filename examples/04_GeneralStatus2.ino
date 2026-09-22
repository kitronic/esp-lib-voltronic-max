#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 04 QPIGS2 (PV2) ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryGeneralStatus2()) {
    const QPIGS2Data& d = inverter.qpigs2();
    Serial.printf("PV2 Voltage: %.1f V\n", d.pv2InputVoltage());
    Serial.printf("PV2 Current: %.1f A\n", d.pv2InputCurrent());
    Serial.printf("PV2 Power:   %u W\n",   d.pv2ChargingPower);
  } else {
    Serial.printf("QPIGS2 failed, err=%u\n", inverter.lastError());
  }
  delay(3000);
}