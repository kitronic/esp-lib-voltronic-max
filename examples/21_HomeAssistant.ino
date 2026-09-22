#include <VoltronicMAX.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);
WiFiClient espClient;
PubSubClient mqtt(espClient);

const char *DEVICE_ID = "voltronic_max_01";

void publishDiscovery()
{
  mqtt.publish(
      "homeassistant/sensor/voltronic_max_01/battery_voltage/config",
      "{\"name\":\"Battery Voltage\","
      "\"state_topic\":\"voltronic/battery_voltage\","
      "\"unit_of_measurement\":\"V\","
      "\"device_class\":\"voltage\","
      "\"state_class\":\"measurement\","
      "\"unique_id\":\"voltronic_max_01_bv\","
      "\"device\":{\"identifiers\":[\"voltronic_max_01\"],"
      "\"name\":\"Voltronic MAX\",\"manufacturer\":\"Voltronic\"}}",
      true);

  mqtt.publish(
      "homeassistant/sensor/voltronic_max_01/battery_soc/config",
      "{\"name\":\"Battery SOC\","
      "\"state_topic\":\"voltronic/battery_soc\","
      "\"unit_of_measurement\":\"%\","
      "\"device_class\":\"battery\","
      "\"state_class\":\"measurement\","
      "\"unique_id\":\"voltronic_max_01_soc\","
      "\"device\":{\"identifiers\":[\"voltronic_max_01\"]}}",
      true);

  mqtt.publish(
      "homeassistant/sensor/voltronic_max_01/pv_power/config",
      "{\"name\":\"PV Power\","
      "\"state_topic\":\"voltronic/pv_power\","
      "\"unit_of_measurement\":\"W\","
      "\"device_class\":\"power\","
      "\"state_class\":\"measurement\","
      "\"unique_id\":\"voltronic_max_01_pv\","
      "\"device\":{\"identifiers\":[\"voltronic_max_01\"]}}",
      true);
}

void setup()
{
  Serial.begin(115200);
  invSerial.begin(2400);
  WiFi.begin("your_wifi", "your_pass");
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
  mqtt.setServer("192.168.1.100", 1883);

  inverter.begin(2400);
  inverter.startPolling();
}

void loop()
{
  if (!mqtt.connected())
  {
    if (mqtt.connect(DEVICE_ID))
      publishDiscovery();
  }
  mqtt.loop();
  inverter.poll();

  static uint32_t t = 0;
  if (millis() - t > 5000)
  {
    t = millis();
    const QPIGSData &d = inverter.qpigs();
    char buf[16];

    dtostrf(d.batteryVoltage(), 0, 2, buf);
    mqtt.publish("voltronic/battery_voltage", buf, true);

    snprintf(buf, sizeof(buf), "%u", d.batteryCapacity);
    mqtt.publish("voltronic/battery_soc", buf, true);

    snprintf(buf, sizeof(buf), "%u", d.pv1ChargingPower);
    mqtt.publish("voltronic/pv_power", buf, true);
  }
}