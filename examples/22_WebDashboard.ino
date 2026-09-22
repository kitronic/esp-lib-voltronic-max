#include <VoltronicMAX.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);
ESP8266WebServer server(80);

const char PAGE_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html><head>
<meta charset="utf-8">
<title>Voltronic MAX</title>
<meta http-equiv="refresh" content="3">
<style>
  body { font-family: Arial; margin: 20px; background: #1a1a1a; color: #eee; }
  h1 { color: #ffcc00; }
  .row { padding: 10px; border-bottom: 1px solid #333; }
  .label { display: inline-block; width: 180px; color: #999; }
  .val { font-weight: bold; color: #4caf50; }
</style>
</head><body>
<h1>Voltronic MAX — Live Data</h1>
<div class="row"><span class="label">Grid Voltage:</span><span class="val">%GRIDV% V</span></div>
<div class="row"><span class="label">Output Voltage:</span><span class="val">%OUTV% V</span></div>
<div class="row"><span class="label">Load:</span><span class="val">%LOAD% %%</span></div>
<div class="row"><span class="label">Battery Voltage:</span><span class="val">%BATTV% V</span></div>
<div class="row"><span class="label">Battery Capacity:</span><span class="val">%BATTC% %%</span></div>
<div class="row"><span class="label">PV1 Voltage:</span><span class="val">%PVV% V</span></div>
<div class="row"><span class="label">PV1 Power:</span><span class="val">%PVW% W</span></div>
<div class="row"><span class="label">Temperature:</span><span class="val">%TEMP% °C</span></div>
<div class="row"><span class="label">Mode:</span><span class="val">%MODE%</span></div>
</body></html>
)HTML";

void handleRoot() {
  String html = FPSTR(PAGE_HTML);
  const QPIGSData& d = inverter.qpigs();

  html.replace("%GRIDV%", String(d.gridVoltage(), 1));
  html.replace("%OUTV%",  String(d.acOutputVoltage(), 1));
  html.replace("%LOAD%",  String(d.loadPercent));
  html.replace("%BATTV%", String(d.batteryVoltage(), 2));
  html.replace("%BATTC%", String(d.batteryCapacity));
  html.replace("%PVV%",   String(d.pv1InputVoltage(), 1));
  html.replace("%PVW%",   String(d.pv1ChargingPower));
  html.replace("%TEMP%",  String(d.inverterTemperature));
  html.replace("%MODE%",  String(inverter.modeString()));

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("your_wifi", "your_pass");
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  inverter.begin(2400);
  inverter.startPolling();

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  inverter.poll();
}