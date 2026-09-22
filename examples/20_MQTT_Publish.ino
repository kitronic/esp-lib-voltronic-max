#include <VoltronicMAX.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);
WiFiClient espClient;
PubSubClient mqtt(espClient);

const char *WIFI_SSID = "your_wifi";
const char *WIFI_PASS = "your_pass";
const char *MQTT_SERVER = "192.168.1.100";
const uint16_t MQTT_PORT = 1883;

uint32_t lastPublish = 0;

void pubStr(const char *t, const char *v)
{
  if (mqtt.connected())
    mqtt.publish(t, v, true);
}
void pubNum(const char *t, float v, uint8_t d)
{
  char buf[16];
  dtostrf(v, 0, d, buf);
  pubStr(t, buf);
}

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);

  inverter.begin(2400);
  inverter.startPolling();
}

void loop()
{
  if (!mqtt.connected())
    mqtt.connect("voltronic-max");
  mqtt.loop();

  inverter.poll();

  if (millis() - lastPublish > 5000)
  {
    lastPublish = millis();
    const QPIGSData &d = inverter.qpigs();

    pubNum("solar/inverter/battery_voltage", d.batteryVoltage(), 2);
    pubNum("solar/inverter/battery_capacity", (float)d.batteryCapacity, 0);
    pubNum("solar/inverter/pv_voltage", d.pv1InputVoltage(), 1);
    pubNum("solar/inverter/pv_power", (float)d.pv1ChargingPower, 0);
    pubNum("solar/inverter/load_percent", (float)d.loadPercent, 0);
    pubNum("solar/inverter/temperature", (float)d.inverterTemperature, 0);

    pubStr("solar/inverter/mode", inverter.modeString());
  }
}