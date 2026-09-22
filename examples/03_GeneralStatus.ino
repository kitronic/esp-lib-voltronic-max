#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 03 QPIGS Full Status ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryGeneralStatus())
  {
    const QPIGSData &d = inverter.qpigs();

    Serial.println(F("═══ QPIGS ═══"));
    Serial.printf("Grid Voltage:          %.1f V\n", d.gridVoltage());
    Serial.printf("Grid Frequency:        %.1f Hz\n", d.gridFrequency());
    Serial.printf("AC Output Voltage:     %.1f V\n", d.acOutputVoltage());
    Serial.printf("AC Output Frequency:   %.1f Hz\n", d.acOutputFrequency());
    Serial.printf("Output Apparent:       %u VA\n", d.acOutputApparentPower);
    Serial.printf("Output Active:         %u W\n", d.acOutputActivePower);
    Serial.printf("Load Percent:          %u %%\n", d.loadPercent);
    Serial.printf("Bus Voltage:           %u V\n", d.busVoltage);
    Serial.printf("Battery Voltage:       %.2f V\n", d.batteryVoltage());
    Serial.printf("Battery Charge Curr:   %u A\n", d.batteryChargingCurrent);
    Serial.printf("Battery Capacity:      %u %%\n", d.batteryCapacity);
    Serial.printf("Inverter Temp:         %u C\n", d.inverterTemperature);
    Serial.printf("PV1 Current:           %.1f A\n", d.pv1InputCurrent());
    Serial.printf("PV1 Voltage:           %.1f V\n", d.pv1InputVoltage());
    Serial.printf("SCC Battery Voltage:   %.2f V\n", d.sccBatteryVoltage());
    Serial.printf("Battery Discharge:     %u A\n", d.batteryDischargeCurrent);
    Serial.printf("Device Status:         0x%02X\n", d.deviceStatus);
    Serial.printf("  Load On:             %d\n", d.sbLoadOn());
    Serial.printf("  Charging Status:     %u\n", d.sbChargingStatus());
    Serial.printf("Battery Offset Fans:   %u\n", d.batteryVoltageOffsetForFans);
    Serial.printf("EEPROM Version:        %u\n", d.eepromVersion);
    Serial.printf("PV1 Charging Power:    %u W\n", d.pv1ChargingPower);
    Serial.printf("Device Status 2:       0x%02X\n", d.deviceStatus2);
    Serial.printf("Feed to Grid Status:   %u\n", d.solarFeedToGridStatus);
    Serial.printf("Country Regulation:    %u\n", d.countryRegulation);
    Serial.printf("Feed to Grid Power:    %u W\n", d.solarFeedToGridPower);
  }
  else
  {
    Serial.printf("QPIGS failed, err=%u\n", inverter.lastError());
  }
  delay(5000);
}