#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 06 QMOD ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryMode()) {
    Serial.printf("Mode: %c (%s)\n",
                  inverter.mode(), inverter.modeString());
  }
  delay(2000);
}