#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 01 BasicRead ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryGeneralStatus())
  {
    const QPIGSData &d = inverter.qpigs();
    Serial.printf("Grid: %.1fV %.1fHz\n", d.gridVoltage(), d.gridFrequency());
    Serial.printf("Out:  %.1fV %.1fHz | Load %u%%\n",
                  d.acOutputVoltage(), d.acOutputFrequency(), d.loadPercent);
    Serial.printf("Batt: %.2fV %uA %u%% | Temp %uC\n",
                  d.batteryVoltage(), d.batteryChargingCurrent,
                  d.batteryCapacity, d.inverterTemperature);
    Serial.printf("PV1:  %.1fV %.1fA %uW\n",
                  d.pv1InputVoltage(), d.pv1InputCurrent(), d.pv1ChargingPower);
  }
  else
  {
    Serial.printf("Failed, err=%u\n", inverter.lastError());
  }
  delay(3000);
}