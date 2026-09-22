#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);
char buf[64];

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 02 DeviceInfo ---"));
  inverter.begin(2400);
  inverter.setTimeout(1500);
}

void loop() {
  if (inverter.queryProtocolID(buf, sizeof(buf)))
    Serial.printf("Protocol ID: %s\n", buf);

  if (inverter.querySerialNumber(buf, sizeof(buf)))
    Serial.printf("Serial: %s\n", buf);

  if (inverter.querySerialNumberLong(buf, sizeof(buf)))
    Serial.printf("Serial Long: %s\n", buf);

  if (inverter.queryFirmware(buf, sizeof(buf)))
    Serial.printf("Main FW: %s\n", buf);

  if (inverter.queryFirmware2(buf, sizeof(buf)))
    Serial.printf("Secondary FW: %s\n", buf);

  if (inverter.queryBluetoothVersion(buf, sizeof(buf)))
    Serial.printf("BT FW: %s\n", buf);

  if (inverter.queryModelName(buf, sizeof(buf)))
    Serial.printf("Model: %s\n", buf);

  if (inverter.queryGeneralModelName(buf, sizeof(buf)))
    Serial.printf("General Model: %s\n", buf);

  bool hasBoot = false;
  if (inverter.queryBoot(hasBoot))
    Serial.printf("Bootloader: %s\n", hasBoot ? "yes" : "no");

  if (inverter.queryTime(buf, sizeof(buf)))
    Serial.printf("Time: %s\n", buf);

  delay(10000);
}