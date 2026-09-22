#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  Serial.println(F("--- 12 QLED ---"));
  inverter.begin(2400);
}

void loop()
{
  if (inverter.queryLed())
  {
    const LedInfo &l = inverter.led();
    Serial.printf("Enabled:    %d\n", l.enabled);
    Serial.printf("Speed:      %u\n", l.speed);
    Serial.printf("Effect:     %u\n", l.effect);
    Serial.printf("Brightness: %u\n", l.brightness);
    Serial.printf("Colors:     %u\n", l.colorCount);
    for (uint8_t i = 0; i < l.colorCount && i < 3; i++)
    {
      Serial.printf("  Color %u: R=%u G=%u B=%u\n",
                    i, l.red[i], l.green[i], l.blue[i]);
    }
  }
  delay(10000);
}