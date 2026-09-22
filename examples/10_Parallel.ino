#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 10 QPGSn Parallel ---"));
  inverter.begin(2400);
}

void loop() {
  for (uint8_t n = 0; n < 6; n++) {
    if (inverter.queryParallel(n)) {
      const ParallelInfo& p = inverter.parallel();
      if (!p.exists) continue;

      Serial.printf("═══ Device %u ═══\n", p.parallelNumber);
      Serial.printf("S/N: %s | Mode: %c | Fault: %u\n",
                    p.serialNumber, p.workMode, p.faultCode);
      Serial.printf("Grid: %.1fV %.1fHz | Out: %.1fV %.1fHz\n",
                    p.gridVoltage(), p.gridFrequency(),
                    p.acOutputVoltage(), p.acOutputFrequency());
      Serial.printf("Load: %u%% | Apparent: %uVA | Active: %uW\n",
                    p.loadPercent, p.acOutputApparentPower,
                    p.acOutputActivePower);
      Serial.printf("Batt: %.1fV %uA %u%%\n",
                    p.batteryVoltage(), p.batteryChargingCurrent,
                    p.batteryCapacity);
      Serial.printf("PV: %.1fV %.1fA | PV2: %.1fA\n",
                    p.pvInputVoltage(), p.pvInputCurrent(),
                    p.pvInputCurrent2());
      Serial.printf("Total: %uA | %uVA | %uW | %u%%\n",
                    p.totalChargingCurrent, p.totalACOutputApparentPower,
                    p.totalOutputActivePower, p.totalACOutputPercentage);
      Serial.printf("MaxChg: %uA | Range: %uA | AC: %uA\n",
                    p.maxChargerCurrent, p.maxChargerRange,
                    p.maxACChargerCurrent);
    }
    delay(500);
  }
  delay(10000);
}