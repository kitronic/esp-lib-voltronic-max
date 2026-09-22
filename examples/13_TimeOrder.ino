#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 13 QOPPT / QCHPT ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryOutputPriorityTimeOrder()) {
    const TimeOrderInfo& t = inverter.outputTimeOrder();
    Serial.print(F("Output priority hours:"));
    for (uint8_t i = 0; i < 24; i++) Serial.printf(" %u", t.hours[i]);
    Serial.printf("\nDefault: %u  Order: %u\n", t.defaultVal, t.order);
  }

  if (inverter.queryChargerPriorityTimeOrder()) {
    const TimeOrderInfo& t = inverter.chargerTimeOrder();
    Serial.print(F("Charger priority hours:"));
    for (uint8_t i = 0; i < 24; i++) Serial.printf(" %u", t.hours[i]);
    Serial.printf("\nDefault: %u  Order: %u\n", t.defaultVal, t.order);
  }

  delay(10000);
}