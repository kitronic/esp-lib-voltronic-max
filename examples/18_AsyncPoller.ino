#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

uint32_t tStatus = 0;
uint32_t tRating = 0;
uint32_t tWarnings = 0;

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 18 AsyncPoller ---"));
  inverter.begin(2400);
}

void loop()
{
  uint32_t now = millis();

  if (now - tStatus > 2000)
  {
    tStatus = now;
    if (inverter.queryGeneralStatus())
    {
      const QPIGSData &d = inverter.qpigs();
      Serial.printf("[2s] Batt %.2fV %u%% | PV %uW | Load %u%%\n",
                    d.batteryVoltage(), d.batteryCapacity,
                    d.pv1ChargingPower, d.loadPercent);
    }
  }

  if (now - tRating > 60000)
  {
    tRating = now;
    if (inverter.queryRating())
    {
      const QPIRIData &r = inverter.qpiri();
      Serial.printf("[60s] MaxChg %uA BattType %u\n",
                    r.maxChargingCurrent, r.batteryType);
    }
  }

  if (now - tWarnings > 300000)
  {
    tWarnings = now;
    if (inverter.queryWarnings())
    {
      WarningDecoded w = inverter.warnings();
      Serial.printf("[5m] Warnings: 0x%08lX\n", (unsigned long)w.raw);
    }
  }
}