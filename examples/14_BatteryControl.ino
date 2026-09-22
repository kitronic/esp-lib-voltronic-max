#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 14 QBATCD ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryBatteryControl()) {
    const BatteryControlStatus& b = inverter.batteryCtrl();
    Serial.printf("Discharge Completely: %u\n", b.dischargeCompletely);
    Serial.printf("Discharge Allowed:    %u\n", b.dischargeAllowed);
    Serial.printf("Charge Completely:    %u\n", b.chargeCompletely);
  }
  delay(10000);
}