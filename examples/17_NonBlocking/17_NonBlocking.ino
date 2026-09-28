#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

enum Step : uint8_t
{
  S_QPIGS = 0,
  S_QPIGS2,
  S_QPIRI,
  S_QMOD,
  S_QPIWS,
  S_DONE
};

uint8_t step = S_QPIGS;
uint32_t lastStepTime = 0;
const uint32_t STEP_INTERVAL = 500;

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 17 NonBlocking ---"));
  inverter.begin(2400);
}

void loop()
{
  uint32_t now = millis();
  if (now - lastStepTime < STEP_INTERVAL)
    return;
  lastStepTime = now;

  switch (step)
  {
  case S_QPIGS:
    if (inverter.queryGeneralStatus())
    {
      const QPIGSData &d = inverter.qpigs();
      Serial.printf("[QPIGS] Batt %.2fV Load %u%%\n",
                    d.batteryVoltage(), d.loadPercent);
    }
    step = S_QPIGS2;
    break;

  case S_QPIGS2:
    if (inverter.queryGeneralStatus2())
    {
      const QPIGS2Data &d = inverter.qpigs2();
      Serial.printf("[QPIGS2] PV2 %.1fV\n", d.pv2InputVoltage());
    }
    step = S_QPIRI;
    break;

  case S_QPIRI:
    if (inverter.queryRating())
    {
      Serial.printf("[QPIRI] MaxChg %uA\n", inverter.qpiri().maxChargingCurrent);
    }
    step = S_QMOD;
    break;

  case S_QMOD:
    if (inverter.queryMode())
    {
      Serial.printf("[QMOD] %s\n", inverter.modeString());
    }
    step = S_QPIWS;
    break;

  case S_QPIWS:
    if (inverter.queryWarnings())
    {
      WarningDecoded w = inverter.warnings();
      Serial.printf("[QPIWS] %s\n", w.any() ? "WARNING" : "OK");
    }
    step = S_DONE;
    break;

  case S_DONE:
    step = S_QPIGS;
    break;
  }
}