#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 05 QPIRI Rating ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryRating())
  {
    const QPIRIData &r = inverter.qpiri();

    Serial.println(F("═══ QPIRI ═══"));
    Serial.printf("Grid Rating V:      %.1f V\n", r.gridRatingVoltage());
    Serial.printf("Grid Rating A:      %.1f A\n", r.gridRatingCurrent());
    Serial.printf("Output Rating V:    %.1f V\n", r.acOutputRatingVoltage());
    Serial.printf("Output Rating Hz:   %.1f Hz\n", r.acOutputRatingFrequency());
    Serial.printf("Output Rating A:    %.1f A\n", r.acOutputRatingCurrent());
    Serial.printf("Output Apparent:    %u VA\n", r.acOutputRatingApparentPower);
    Serial.printf("Output Active:      %u W\n", r.acOutputRatingActivePower);
    Serial.printf("Battery Rating V:   %.1f V\n", r.batteryRatingVoltage());
    Serial.printf("Recharge V:         %.1f V\n", r.batteryRechargeVoltage());
    Serial.printf("Under V:            %.1f V\n", r.batteryUnderVoltage());
    Serial.printf("Bulk V:             %.1f V\n", r.batteryBulkVoltage());
    Serial.printf("Float V:            %.1f V\n", r.batteryFloatVoltage());
    Serial.printf("Battery Type:       %u\n", r.batteryType);
    Serial.printf("Max AC Charge:      %u A\n", r.maxACChargingCurrent);
    Serial.printf("Max Charge:         %u A\n", r.maxChargingCurrent);
    Serial.printf("Input Range:        %u\n", r.inputVoltageRange);
    Serial.printf("Output Priority:    %u\n", r.outputSourcePriority);
    Serial.printf("Charger Priority:   %u\n", r.chargerSourcePriority);
    Serial.printf("Redischarge V:      %.1f V\n", r.batteryRedischargeVoltage());
  }
  else
  {
    Serial.printf("QPIRI failed, err=%u\n", inverter.lastError());
  }
  delay(10000);
}