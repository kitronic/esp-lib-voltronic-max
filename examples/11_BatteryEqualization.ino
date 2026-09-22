#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 11 QBEQI ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryBatteryEqualization()) {
    const BatteryEqualizationInfo& e = inverter.equalization();
    Serial.printf("Enabled:        %d\n", e.enabled);
    Serial.printf("Time:           %u min\n", e.timeMinutes);
    Serial.printf("Period:         %u days\n", e.periodDays);
    Serial.printf("Voltage:        %.1f V\n", e.voltage());
    Serial.printf("Over Time:      %u min\n", e.overTimeMinutes);
    Serial.printf("Max Time:       %u min\n", e.maxTimeMinutes);
    Serial.printf("Active Now:     %u\n", e.activeNow);
  }
  delay(10000);
}