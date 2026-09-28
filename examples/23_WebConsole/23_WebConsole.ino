// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Web Console Example (ESP8266) — v1.3.0
// ═══════════════════════════════════════════════════════════════

// ⚠️ guard: CI قد يمرّر الفلاج عبر --build-property
#ifndef VOLTRONIC_USE_WEB
  #define VOLTRONIC_USE_WEB
#endif

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include "VoltronicMAX.h"

// ═══════════════════════════════════════════════════════════════
//  إعدادات المستخدم
// ═══════════════════════════════════════════════════════════════
const char *WIFI_SSID = "YOUR_SSID";
const char *WIFI_PASS = "YOUR_PASS";

const char *WEB_USER = "admin";
const char *WEB_PASS = "changeme";

const char *WEB_PREFIX = "/inv";

// ─── UART pins (ESP8266) ───
#define INV_RX_PIN 4    // D2
#define INV_TX_PIN 5    // D1
#define INV_BAUD   2400

// ─── Battery Configuration ───
#define BAT_TYPE          VoltronicBatteryType::LiFePO4_16S
#define BAT_CAPACITY_AH   300.0f
#define BAT_VOLT_EMPTY    44.0f
#define BAT_VOLT_FULL     58.4f

// ─── Smart Charger ───
#define SC_TARGET_AC      40
#define SC_TARGET_TOTAL   80
#define SC_FLOAT_AC       2
#define SC_FLOAT_TOTAL    10
#define SC_TEMP_PROTECT   70

// ─── Power Mode ───
#define PM_SOC_EMERGENCY    10.0f
#define PM_SOC_POWER_SAVING 25.0f
#define PM_SOC_RECOVER      30.0f
#define PM_SOC_SURPLUS      88.0f
#define PM_GRID_MIN_V       150.0f

// ─── Language ───
#define INV_LANGUAGE  VoltronicLang::ARABIC

// ═══════════════════════════════════════════════════════════════
//  Globals
// ═══════════════════════════════════════════════════════════════
ESP8266WebServer web(80);
SoftwareSerial   invSerial(INV_RX_PIN, INV_TX_PIN, false);
VoltronicMAX     inverter(invSerial);

// ═══════════════════════════════════════════════════════════════
//  Setup
// ═══════════════════════════════════════════════════════════════
void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("╔════════════════════════════════════╗"));
  Serial.println(F("║  VoltronicMAX — Web Console v1.3.0 ║"));
  Serial.println(F("╚════════════════════════════════════╝"));

  // ─── UART ───
  invSerial.begin(INV_BAUD);
  inverter.begin(INV_BAUD);
  Serial.printf_P(PSTR("[Boot] UART ready: %u 8N1 (RX=D%d TX=D%d)\n"),
                  INV_BAUD, INV_RX_PIN, INV_TX_PIN);

  // ─── Battery defaults ───
  if (!inverter.storage.isLoaded())
  {
    inverter.battery.setType(BAT_TYPE);
    inverter.battery.setCapacity(BAT_CAPACITY_AH);
    inverter.battery.setVoltageEmpty(BAT_VOLT_EMPTY);
    inverter.battery.setVoltageFull(BAT_VOLT_FULL);
    inverter.battery.setInitialSoh(100);

    inverter.smartCharger.setMode(VoltronicSmartCharger::MODE_STD);
    inverter.smartCharger.setTargetAC(SC_TARGET_AC);
    inverter.smartCharger.setTargetTotal(SC_TARGET_TOTAL);
    inverter.smartCharger.setFloatAC(SC_FLOAT_AC);
    inverter.smartCharger.setFloatTotal(SC_FLOAT_TOTAL);
    inverter.smartCharger.setTempProtectC(SC_TEMP_PROTECT);

    inverter.powerMode.setSocEmergency(PM_SOC_EMERGENCY);
    inverter.powerMode.setSocPowerSaving(PM_SOC_POWER_SAVING);
    inverter.powerMode.setSocRecover(PM_SOC_RECOVER);
    inverter.powerMode.setSocSurplus(PM_SOC_SURPLUS);
    inverter.powerMode.setGridMinVoltage(PM_GRID_MIN_V);

    inverter.storage.save(inverter);
    Serial.println(F("[Boot] Default settings saved to EEPROM"));
  }
  else
  {
    Serial.println(F("[Boot] Settings loaded from EEPROM"));
  }

  // ─── Language ───
  inverter.lang.setLanguage(INV_LANGUAGE);
  Serial.printf_P(PSTR("[Boot] Language: %s\n"),
                  inverter.lang.isArabic() ? "Arabic" : "English");

  // ─── فحص ───
  Serial.print(F("[Boot] Testing inverter... "));
  char proto[16];
  if (inverter.queryProtocolID(proto, sizeof(proto)))
  {
    Serial.printf_P(PSTR("OK (%s)\n"), proto);
  }
  else
  {
    Serial.printf_P(PSTR("no response (%s)\n"), inverter.lastErrorName());
  }

  // ─── معلومات ثابتة ───
  char sn[16] = {0}, fw[16] = {0}, model[16] = {0};
  inverter.querySerialNumber(sn, sizeof(sn));
  inverter.queryFirmware(fw, sizeof(fw));
  inverter.queryModelName(model, sizeof(model));
  Serial.printf_P(PSTR("[Boot] S/N:%s FW:%s Model:%s\n"), sn, fw, model);

  // ─── جلب أولي ───
  inverter.queryRating();
  inverter.queryMode();
  inverter.queryGeneralStatus();
  inverter.queryGeneralStatus2();
  inverter.queryWarnings();

  // ─── WiFi ───
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf_P(PSTR("[Boot] Connecting to \"%s\""), WIFI_SSID);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print('.');
    if (millis() - start > 20000)
    {
      Serial.println(F("\n[Boot] WiFi timeout — restarting"));
      ESP.restart();
    }
  }
  Serial.println();
  Serial.printf_P(PSTR("[Boot] WiFi: %s\n"),
                  WiFi.localIP().toString().c_str());
  Serial.printf_P(PSTR("[Boot] Signal: %d dBm\n"), WiFi.RSSI());

  // ─── Web ───
  if (WEB_USER && WEB_USER[0] != '\0' &&
      WEB_PASS && WEB_PASS[0] != '\0')
  {
    inverter.attachWebServer(&web, WEB_USER, WEB_PASS);
    Serial.printf_P(PSTR("[Boot] Web auth: user=\"%s\"\n"), WEB_USER);
  }
  else
  {
    inverter.attachWebServer(&web);
    Serial.println(F("[Boot] Web auth: DISABLED"));
  }

  if (WEB_PREFIX && WEB_PREFIX[0] != '\0')
  {
    inverter.setWebPrefix(WEB_PREFIX);
  }

  web.begin();

  // ─── رابط ───
  Serial.println(F("────────────────────────────────────"));
  Serial.printf_P(PSTR("  Open: http://%s%s/\n"),
                  WiFi.localIP().toString().c_str(),
                  (WEB_PREFIX && WEB_PREFIX[0]) ? WEB_PREFIX : "");
  Serial.println(F("────────────────────────────────────"));
}

// ═══════════════════════════════════════════════════════════════
//  Loop
// ═══════════════════════════════════════════════════════════════
void loop()
{
  web.handleClient();

  // ─── WiFi monitor ───
  static uint32_t lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000)
  {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.reconnect();
    }
  }

  // ─── Inverter queries (كل 10s) ───
  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 10000)
  {
    lastQuery = millis();
    inverter.queryGeneralStatus();
    inverter.queryGeneralStatus2();
    inverter.queryWarnings();
    inverter.queryMode();
    inverter.queryRating();
  }

  // ─── Smart Charger + Power Mode (كل 30s) ───
  static uint32_t lastSmart = 0;
  if (millis() - lastSmart > 30000)
  {
    lastSmart = millis();
    inverter.smartCharger.update(inverter, inverter.battery);
    inverter.powerMode.update(inverter, inverter.battery);
  }

  // ─── Storage auto-save ───
  inverter.storage.tick(inverter);

  // ─── Heap monitor (كل 60s) ───
  static uint32_t lastHeapCheck = 0;
  if (millis() - lastHeapCheck > 60000)
  {
    lastHeapCheck = millis();
    Serial.printf_P(PSTR("[Heap] Free: %u bytes\n"), ESP.getFreeHeap());
  }

  yield();
}