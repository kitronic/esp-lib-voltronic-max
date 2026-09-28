#include <VoltronicMAX.h>

#if defined(ESP8266)
  #include <SoftwareSerial.h>
  SoftwareSerial invSerial(D1, D2);
#elif defined(ESP32)
  #define invSerial Serial2
#else
  #include <SoftwareSerial.h>
  SoftwareSerial invSerial(10, 11);
#endif

VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);

#if defined(ESP32)
  Serial2.begin(2400, SERIAL_8N1, 16, 17);
#else
  invSerial.begin(2400);
#endif

  inverter.begin(2400);
  Serial.println(F("--- 01 BasicRead ---"));
}

void loop() {
  if (inverter.queryGeneralStatus()) {
    const QPIGSData& d = inverter.qpigs();

    Serial.print(F("Grid: "));
    Serial.print(d.gridVoltage(), 1);
    Serial.print(F("V "));
    Serial.print(d.gridFrequency(), 1);
    Serial.println(F("Hz"));

    Serial.print(F("Out:  "));
    Serial.print(d.acOutputVoltage(), 1);
    Serial.print(F("V "));
    Serial.print(d.acOutputFrequency(), 1);
    Serial.print(F("Hz | Load "));
    Serial.print(d.loadPercent);
    Serial.println(F("%"));

    Serial.print(F("Batt: "));
    Serial.print(d.batteryVoltage(), 2);
    Serial.print(F("V "));
    Serial.print(d.batteryChargingCurrent);
    Serial.print(F("A "));
    Serial.print(d.batteryCapacity);
    Serial.print(F("% | Temp "));
    Serial.print(d.inverterTemperature);
    Serial.println(F("C"));

    Serial.print(F("PV1:  "));
    Serial.print(d.pv1InputVoltage(), 1);
    Serial.print(F("V "));
    Serial.print(d.pv1InputCurrent(), 1);
    Serial.print(F("A "));
    Serial.print(d.pv1ChargingPower);
    Serial.println(F("W"));
  } else {
    Serial.print(F("Failed, err="));
    Serial.println(inverter.lastError());
  }
  delay(3000);
}