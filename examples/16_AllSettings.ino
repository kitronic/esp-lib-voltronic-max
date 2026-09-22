#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 16 All Settings ---"));
  inverter.begin(2400);
  inverter.setTimeout(2000);

  // Priorities
  if (inverter.setOutputSourcePriority(VC_OUT_PRIO_SOLAR_FIRST))
    Serial.println(F("OK: output priority = Solar first"));

  if (inverter.setChargerSourcePriority(VC_CHG_PRIO_SOLAR_UTILITY))
    Serial.println(F("OK: charger priority = Solar + Utility"));

  if (inverter.setGridWorkingRange(0))
    Serial.println(F("OK: grid range = Appliance"));

  // Currents
  if (inverter.setMaxChargingCurrent(60))
    Serial.println(F("OK: max charge = 60A"));

  if (inverter.setMaxUtilityChargingCurrent(30))
    Serial.println(F("OK: max utility = 30A"));

  if (inverter.setMaxDischargingCurrent(100))
    Serial.println(F("OK: max discharge = 100A"));

  // Output
  if (inverter.setOutputVoltage(230))
    Serial.println(F("OK: output = 230V"));

  if (inverter.setOutputFrequency(50))
    Serial.println(F("OK: frequency = 50Hz"));

  if (inverter.setOutputMode(0))
    Serial.println(F("OK: output mode = single"));

  // Battery
  if (inverter.setBatteryType(VC_BAT_TYPE_AGM))
    Serial.println(F("OK: battery type = AGM"));

  if (inverter.setBatteryCutoffVoltage(44.0f))
    Serial.println(F("OK: cutoff = 44.0V"));

  if (inverter.setBatteryRechargeVoltage(46.0f))
    Serial.println(F("OK: recharge = 46.0V"));

  if (inverter.setBatteryRedischargeVoltage(54.0f))
    Serial.println(F("OK: redischarge = 54.0V"));

  if (inverter.setBatteryCvVoltage(56.4f))
    Serial.println(F("OK: CV = 56.4V"));

  if (inverter.setBatteryFloatVoltage(54.0f))
    Serial.println(F("OK: float = 54.0V"));

  if (inverter.setMaxCvChargingTime(60))
    Serial.println(F("OK: max CV time = 60min"));

  // Flags
  if (inverter.setFlag('a'))
    Serial.println(F("OK: buzzer enabled"));

  if (inverter.clearFlag('j'))
    Serial.println(F("OK: power saving disabled"));

  // Battery equalization
  if (inverter.setBatteryEqualizationEnabled(true))
    Serial.println(F("OK: equalization enabled"));

  if (inverter.setBatteryEqualizationTime(60))
    Serial.println(F("OK: equalization time = 60min"));

  if (inverter.setBatteryEqualizationPeriod(30))
    Serial.println(F("OK: equalization period = 30 days"));

  if (inverter.setBatteryEqualizationVoltage(58.4f))
    Serial.println(F("OK: equalization voltage = 58.4V"));

  if (inverter.setBatteryEqualizationOverTime(120))
    Serial.println(F("OK: equalization over time = 120min"));

  // Date/Time
  if (inverter.setDateTime("260922120000"))
    Serial.println(F("OK: date/time set"));

  // Battery control
  if (inverter.setBatteryControl(1, 1, 1))
    Serial.println(F("OK: battery control 1,1,1"));

  // Energy (careful!)
  // if (inverter.resetEnergy()) Serial.println(F("OK: energy reset"));
  // if (inverter.eraseLog())    Serial.println(F("OK: log erased"));

  Serial.println(F("=== Settings applied ==="));
}

void loop() {}