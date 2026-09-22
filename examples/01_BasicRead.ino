#include <VoltronicMAX.h>

// ═══ اختيار المنفذ حسب المنصة ═══
#if defined(ESP8266)
  #include <SoftwareSerial.h>
  SoftwareSerial invSerial(D1, D2);       // ESP8266
#elif defined(ESP32)
  #define invSerial Serial2                // ESP32
#else
  #include <SoftwareSerial.h>
  SoftwareSerial invSerial(10, 11);        // AVR
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
    Serial.printf("Grid: %.1fV %.1fHz\n", d.gridVoltage(), d.gridFrequency());
    Serial.printf("Out:  %.1fV %.1fHz | Load %u%%\n",
                  d.acOutputVoltage(), d.acOutputFrequency(), d.loadPercent);
    Serial.printf("Batt: %.2fV %uA %u%% | Temp %uC\n",
                  d.batteryVoltage(), d.batteryChargingCurrent,
                  d.batteryCapacity, d.inverterTemperature);
    Serial.printf("PV1:  %.1fV %.1fA %uW\n",
                  d.pv1InputVoltage(), d.pv1InputCurrent(), d.pv1ChargingPower);
  } else {
    Serial.printf("Failed, err=%u\n", inverter.lastError());
  }
  delay(3000);
}