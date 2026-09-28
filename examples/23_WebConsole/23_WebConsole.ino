// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Web Console Example (ESP8266) — v1.3.0
//
//  يوفر:
//    - واجهة ويب كاملة للتحكم بالإنفرتر
//    - إرسال الأوامر الجاهزة بضغطة زر
//    - إرسال أوامر خام (raw)
//    - Battery + Smart Charger + Power Mode (v1.3.0)
//
//  الرابط بعد التشغيل:
//    http://<IP>/inv/
//
//  ⚙️ الإعداد:
//    1. عدّل WIFI_SSID و WIFI_PASS
//    2. تأكد من توصيلات UART (RX/TX)
//    3. ارفع السكتش
//    4. افتح Serial Monitor لعرض IP
// ═══════════════════════════════════════════════════════════════

// ⚠️ يجب أن يكون قبل include
#define VOLTRONIC_USE_WEB

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "VoltronicMAX.h"

// ═══════════════════════════════════════════════════════════════
//  إعدادات المستخدم
// ═══════════════════════════════════════════════════════════════

// ─── WiFi ───
const char *WIFI_SSID = "YOUR_SSID";
const char *WIFI_PASS = "YOUR_PASS";

// ─── Web Auth (اتركها فارغة لتعطيل الحماية) ───
const char *WEB_USER = "admin";
const char *WEB_PASS = "changeme";

// ─── Web Prefix ───
const char *WEB_PREFIX = "/inv";

// ─── UART pins (ESP8266) ───
#define INV_RX_PIN 4 // D2
#define INV_TX_PIN 5 // D1

// ─── Baud ───
#define INV_BAUD 2400

// ═══════════════════════════════════════════════════════════════
//  Battery Configuration (v1.3.0)
//  ⚠️ عدّل هذه القيم حسب بطاريتك
// ═══════════════════════════════════════════════════════════════
#define BAT_TYPE VoltronicBatteryType::LiFePO4_16S
#define BAT_CAPACITY_AH 300.0f // سعة البطارية
#define BAT_VOLT_EMPTY 44.0f   // جهد فارغ
#define BAT_VOLT_FULL 58.4f    // جهد ممتلئ

// ─── Smart Charger defaults ───
#define SC_TARGET_AC 40    // حد AC
#define SC_TARGET_TOTAL 80 // حد Total
#define SC_FLOAT_AC 2      // Float AC
#define SC_FLOAT_TOTAL 10  // Float Total
#define SC_TEMP_PROTECT 70 // حماية الحرارة

// ─── Power Mode thresholds ───
#define PM_SOC_EMERGENCY 10.0f
#define PM_SOC_POWER_SAVING 25.0f
#define PM_SOC_RECOVER 30.0f
#define PM_SOC_SURPLUS 88.0f
#define PM_GRID_MIN_V 150.0f

// ─── Language ───
#define INV_LANGUAGE VoltronicLang::ARABIC

// ═══════════════════════════════════════════════════════════════
//  Globals
// ═══════════════════════════════════════════════════════════════
ESP8266WebServer web(80);
VoltronicMAX inverter(&Serial1);

// ═══════════════════════════════════════════════════════════════
//  Setup
// ═══════════════════════════════════════════════════════════════
void setup()
{
  // ─── Serial للـ debug ───
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("╔════════════════════════════════════╗"));
  Serial.println(F("║  VoltronicMAX — Web Console v1.3.0 ║"));
  Serial.println(F("╚════════════════════════════════════╝"));

  // ─── UART للإنفرتر ───
  Serial1.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
  inverter.begin(INV_BAUD);
  Serial.printf_P(PSTR("[Boot] UART ready: %u 8N1 (RX=D%d TX=D%d)\n"),
                  INV_BAUD, INV_RX_PIN, INV_TX_PIN);

  // ─── Battery Configuration ───
  //  ⚠️ storage.load() تم استدعاؤه تلقائياً في inverter.begin()
  //  إذا كانت EEPROM فارغة، نضع قيم افتراضية:
  if (!inverter.storage.isLoaded())
  {
    inverter.battery.setType(BAT_TYPE);
    inverter.battery.setCapacity(BAT_CAPACITY_AH);
    inverter.battery.setVoltageEmpty(BAT_VOLT_EMPTY);
    inverter.battery.setVoltageFull(BAT_VOLT_FULL);
    inverter.battery.setInitialSoh(100);

    inverter.smartCharger.setMode(VoltronicSmartCharger::STANDARD);
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

  // ─── فحص سريع أن الإنفرتر يستجيب ───
  Serial.print(F("[Boot] Testing inverter... "));
  char proto[16];
  if (inverter.queryProtocolID(proto, sizeof(proto)))
  {
    Serial.printf_P(PSTR("OK (%s)\n"), proto);
  }
  else
  {
    Serial.printf_P(PSTR("no response (%s)\n"), inverter.lastErrorName());
    Serial.println(F("[Boot] Check wiring / RX-TX swap"));
  }

  // ─── معلومات ثابتة ───
  char sn[16] = {0}, fw[16] = {0}, model[16] = {0};
  inverter.querySerialNumber(sn, sizeof(sn));
  inverter.queryFirmware(fw, sizeof(fw));
  inverter.queryModelName(model, sizeof(model));
  Serial.printf_P(PSTR("[Boot] S/N:%s FW:%s Model:%s\n"), sn, fw, model);

  // ─── جلب QPIRI مبدئياً (يحتاجه SmartCharger) ───
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
  Serial.printf_P(PSTR("[Boot] WiFi connected: %s\n"),
                  WiFi.localIP().toString().c_str());
  Serial.printf_P(PSTR("[Boot] Signal: %d dBm\n"), WiFi.RSSI());

  // ─── Web Server ───
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

  // ─── روابط جاهزة ───
  Serial.println(F("────────────────────────────────────"));
  Serial.println(F("  Ready. Open in browser:"));
  Serial.printf_P(PSTR("    http://%s%s/\n"),
                  WiFi.localIP().toString().c_str(),
                  (WEB_PREFIX && WEB_PREFIX[0]) ? WEB_PREFIX : "");
  Serial.println(F("────────────────────────────────────"));
}

// ═══════════════════════════════════════════════════════════════
//  Loop
// ═══════════════════════════════════════════════════════════════
void loop()
{
  // ─── معالجة طلبات الويب ───
  web.handleClient();

  // ─── WiFi monitor ───
  static uint32_t lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000)
  {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println(F("[WiFi] Disconnected — reconnecting"));
      WiFi.reconnect();
    }
  }

  // ═══════════════════════════════════════════════════════════
  //  Inverter queries (كل 10s)
  // ═══════════════════════════════════════════════════════════
  static uint32_t lastQuery = 0;
  if (millis() - lastQuery > 10000)
  {
    lastQuery = millis();

    inverter.queryGeneralStatus();  // QPIGS  → battery.update()
    inverter.queryGeneralStatus2(); // QPIGS2 → PV2
    inverter.queryWarnings();       // QPIWS
    inverter.queryMode();           // QMOD
    inverter.queryRating();         // QPIRI (للـ SmartCharger)
  }

  // ═══════════════════════════════════════════════════════════
  //  Smart Charger + Power Mode (كل 30s)
  //  ⚠️ يجب استدعاؤهما دورياً — أو تفعيلهما من Web UI
  // ═══════════════════════════════════════════════════════════
  static uint32_t lastSmart = 0;
  if (millis() - lastSmart > 30000)
  {
    lastSmart = millis();

    // Smart Charger: يضبط MCHGC/MUCHGC حسب SOC + حرارة
    inverter.smartCharger.update(inverter, inverter.battery);

    // Power Mode: يقيّم الوضع الحالي
    inverter.powerMode.update(inverter, inverter.battery);
  }

  // ═══════════════════════════════════════════════════════════
  //  Storage auto-save (يُحفظ بعد 5s من آخر تغيير)
  // ═══════════════════════════════════════════════════════════
  inverter.storage.tick(inverter);

  // ─── Heap monitor (كل 60s) ───
  static uint32_t lastHeapCheck = 0;
  if (millis() - lastHeapCheck > 60000)
  {
    lastHeapCheck = millis();
    Serial.printf_P(PSTR("[Heap] Free: %u bytes\n"), ESP.getFreeHeap());
  }

  // ─── yield للـ WDT ───
  yield();
}