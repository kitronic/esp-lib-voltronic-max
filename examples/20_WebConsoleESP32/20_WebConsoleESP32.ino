// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Web Console Example (ESP32) — v1.3.0
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
//    2. تأكد من توصيلات UART2 (RX/TX)
//    3. ارفع السكتش
//    4. افتح Serial Monitor لعرض IP
// ═══════════════════════════════════════════════════════════════

// ⚠️ يجب أن يكون قبل include
#define VOLTRONIC_USE_WEB

#include <WiFi.h>
#include <WebServer.h>
#include "VoltronicMAX.h"

// ═══════════════════════════════════════════════════════════════
//  إعدادات المستخدم
// ═══════════════════════════════════════════════════════════════

// ─── WiFi ───
const char *WIFI_SSID = "YOUR_SSID";
const char *WIFI_PASS = "YOUR_PASS";

// ─── Web Auth ───
const char *WEB_USER = "admin";
const char *WEB_PASS = "changeme";

// ─── Web Prefix ───
const char *WEB_PREFIX = "/inv";

// ─── UART2 pins (ESP32) ───
#define INV_RX_PIN 16
#define INV_TX_PIN 17

// ─── Baud ───
#define INV_BAUD 2400

// ═══════════════════════════════════════════════════════════════
//  Battery Configuration (v1.3.0)
// ═══════════════════════════════════════════════════════════════
#define BAT_TYPE VoltronicBatteryType::LiFePO4_16S
#define BAT_CAPACITY_AH 300.0f
#define BAT_VOLT_EMPTY 44.0f
#define BAT_VOLT_FULL 58.4f

// ─── Smart Charger ───
#define SC_TARGET_AC 40
#define SC_TARGET_TOTAL 80
#define SC_FLOAT_AC 2
#define SC_FLOAT_TOTAL 10
#define SC_TEMP_PROTECT 70

// ─── Power Mode ───
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
WebServer web(80);
VoltronicMAX inverter(&Serial2);

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
    Serial.println(F("║  VoltronicMAX — Web Console (ESP32)║"));
    Serial.println(F("║  v1.3.0                            ║"));
    Serial.println(F("╚════════════════════════════════════╝"));

    // ─── معلومات النظام ───
    Serial.printf_P(PSTR("[Boot] Chip:      %s rev %d\n"),
                    ESP.getChipModel(), ESP.getChipRevision());
    Serial.printf_P(PSTR("[Boot] Cores:     %d @ %u MHz\n"),
                    ESP.getChipCores(), (unsigned)getCpuFrequencyMhz());
    Serial.printf_P(PSTR("[Boot] Flash:     %u bytes\n"),
                    (unsigned)ESP.getFlashChipSize());
    Serial.printf_P(PSTR("[Boot] Free heap: %u bytes\n"),
                    (unsigned)ESP.getFreeHeap());

    // ─── UART2 للإنفرتر ───
    Serial2.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
    inverter.begin(INV_BAUD);
    Serial.printf_P(PSTR("[Boot] UART2 ready: %u 8N1 (RX=GPIO%d TX=GPIO%d)\n"),
                    INV_BAUD, INV_RX_PIN, INV_TX_PIN);

    // ─── Battery Configuration ───
    //  storage.load() استُدعي في inverter.begin()
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

    // ─── فحص سريع ───
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

    // ─── جلب QPIRI مبدئياً ───
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
    Serial.printf_P(PSTR("[Boot] MAC: %s\n"), WiFi.macAddress().c_str());

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

    // ─── روابط ───
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

        inverter.queryGeneralStatus();
        inverter.queryGeneralStatus2();
        inverter.queryWarnings();
        inverter.queryMode();
        inverter.queryRating();
    }

    // ═══════════════════════════════════════════════════════════
    //  Smart Charger + Power Mode (كل 30s)
    // ═══════════════════════════════════════════════════════════
    static uint32_t lastSmart = 0;
    if (millis() - lastSmart > 30000)
    {
        lastSmart = millis();

        inverter.smartCharger.update(inverter, inverter.battery);
        inverter.powerMode.update(inverter, inverter.battery);
    }

    // ═══════════════════════════════════════════════════════════
    //  Storage auto-save
    // ═══════════════════════════════════════════════════════════
    inverter.storage.tick(inverter);

    // ─── Heap monitor (كل 60s) ───
    static uint32_t lastHeapCheck = 0;
    if (millis() - lastHeapCheck > 60000)
    {
        lastHeapCheck = millis();
        Serial.printf_P(PSTR("[Heap] Free: %u bytes (min: %u)\n"),
                        (unsigned)ESP.getFreeHeap(),
                        (unsigned)ESP.getMinFreeHeap());
    }

    // ⚠️ ESP32 dual-core — لا حاجة لـ yield()
    delay(2);
}