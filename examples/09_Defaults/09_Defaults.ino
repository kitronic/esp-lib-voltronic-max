#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 09 QDI Defaults ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryDefaults())
  {
    const DefaultsInfo &d = inverter.defaults();
    Serial.printf("AC Out V:        %.1f V\n", d.acOutputVoltage_x10 / 10.0f);
    Serial.printf("AC Out Hz:       %.1f Hz\n", d.acOutputFrequency_x10 / 10.0f);
    Serial.printf("Max AC Charge:   %u A\n", d.maxACChargingCurrent);
    Serial.printf("Batt Under V:    %.1f V\n", d.batteryUnderVoltage_x10 / 10.0f);
    Serial.printf("Batt Float V:    %.1f V\n", d.batteryFloatVoltage_x10 / 10.0f);
    Serial.printf("Batt Bulk V:     %.1f V\n", d.batteryBulkVoltage_x10 / 10.0f);
    Serial.printf("Batt Recharge V: %.1f V\n", d.batteryRechargeVoltage_x10 / 10.0f);
    Serial.printf("Max Charge:      %u A\n", d.maxChargingCurrent);
    Serial.printf("Input Range:     %u\n", d.acInputVoltageRange);
    Serial.printf("Output Priority: %u\n", d.outputSourcePriority);
    Serial.printf("Charger Priority:%u\n", d.chargerSourcePriority);
    Serial.printf("Battery Type:    %u\n", d.batteryType);
  }
  delay(10000);
}