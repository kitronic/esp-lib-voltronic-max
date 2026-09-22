#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 15 QMCHGCR / QMUCHGCR ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryMaxChargingCurrents())
  {
    const SelectableValues &v = inverter.maxChgOptions();
    Serial.print(F("Max charging options:"));
    for (uint8_t i = 0; i < v.count; i++)
      Serial.printf(" %uA", v.values[i]);
    Serial.println();
  }

  if (inverter.queryMaxUtilityChargingCurrents())
  {
    const SelectableValues &v = inverter.maxUtilChgOptions();
    Serial.print(F("Max utility options:"));
    for (uint8_t i = 0; i < v.count; i++)
      Serial.printf(" %uA", v.values[i]);
    Serial.println();
  }

  delay(15000);
}