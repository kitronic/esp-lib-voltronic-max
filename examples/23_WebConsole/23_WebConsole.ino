// ═══════════════════════════════════════════════════════════════
//  VoltronicMAX — Web Console Example (ESP8266)
//
//  يوفر:
//    - واجهة ويب كاملة للتحكم بالإنفرتر
//    - إرسال الأوامر الجاهزة بضغطة زر
//    - إرسال أوامر خام (raw)
//    - عرض الردود مباشرة في المتصفح
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
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASS";

// ─── Web Auth (اتركها فارغة لتعطيل الحماية) ───
//  ⚠️ على شبكة عامة، ضع كلمة مرور قوية!
const char* WEB_USER = "admin";
const char* WEB_PASS = "changeme";

// ─── Web Prefix (المسار في المتصفح) ───
//  "/inv" → http://<IP>/inv/
//  ""     → http://<IP>/
const char* WEB_PREFIX = "/inv";

// ─── UART pins (ESP8266) ───
//  D2 (GPIO4) = RX من الإنفرتر
//  D1 (GPIO5) = TX إلى الإنفرتر
#define INV_RX_PIN  4
#define INV_TX_PIN  5

// ─── Baud ───
#define INV_BAUD  2400

// ═══════════════════════════════════════════════════════════════
//  Globals
// ═══════════════════════════════════════════════════════════════
ESP8266WebServer web(80);
VoltronicMAX     inverter(&Serial1);

// ═══════════════════════════════════════════════════════════════
//  Setup
// ═══════════════════════════════════════════════════════════════
void setup() {
  // ─── Serial للـ debug ───
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("╔════════════════════════════════════╗"));
  Serial.println(F("║  VoltronicMAX — Web Console        ║"));
  Serial.println(F("╚════════════════════════════════════╝"));

  // ─── UART للإنفرتر ───
  //  ⚠️ مهم: هيّئ Serial قبل inverter.begin()
  Serial1.begin(INV_BAUD, SERIAL_8N1, INV_RX_PIN, INV_TX_PIN);
  inverter.begin(INV_BAUD);
  Serial.printf_P(PSTR("[Boot] UART ready: %u 8N1 (RX=D%d TX=D%d)\n"),
                  INV_BAUD, INV_RX_PIN, INV_TX_PIN);

  // ─── فحص سريع أن الإنفرتر يستجيب ───
  Serial.print(F("[Boot] Testing inverter... "));
  char proto[16];
  if (inverter.queryProtocolID(proto, sizeof(proto))) {
    Serial.printf_P(PSTR("OK (%s)\n"), proto);
  } else {
    Serial.printf_P(PSTR("no response (%s)\n"), inverter.lastErrorName());
    Serial.println(F("[Boot] Check wiring / RX-TX swap"));
  }

  // ─── WiFi ───
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf_P(PSTR("[Boot] Connecting to \"%s\""), WIFI_SSID);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print('.');
    if (millis() - start > 20000) {
      Serial.println(F("\n[Boot] WiFi timeout — restarting"));
      ESP.restart();
    }
  }
  Serial.println();
  Serial.printf_P(PSTR("[Boot] WiFi connected: %s\n"), WiFi.localIP().toString().c_str());
  Serial.printf_P(PSTR("[Boot] Signal: %d dBm\n"), WiFi.RSSI());

  // ─── Web Server ───
  if (WEB_USER && WEB_USER[0] != '\0' && WEB_PASS && WEB_PASS[0] != '\0') {
    inverter.attachWebServer(&web, WEB_USER, WEB_PASS);
    Serial.printf_P(PSTR("[Boot] Web auth: user=\"%s\" pass=\"%s\"\n"),
                    WEB_USER, WEB_PASS);
  } else {
    inverter.attachWebServer(&web);
    Serial.println(F("[Boot] Web auth: DISABLED"));
  }

  if (WEB_PREFIX && WEB_PREFIX[0] != '\0') {
    inverter.setWebPrefix(WEB_PREFIX);
  }

  web.begin();

  // ─── روابط جاهزة ───
  Serial.println(F("────────────────────────────────────"));
  Serial.println(F("  Ready. Open in browser:"));
  Serial.printf_P(PSTR("    http://%s%s/\n"),
                  WiFi.localIP().toString().c_str(),
                  (WEB_PREFIX && WEB_PREFIX[0]) ? WEB_PREFIX : "");
  Serial.printf_P(PSTR("  API commands:  http://%s%s/api/commands\n"),
                  WiFi.localIP().toString().c_str(),
                  (WEB_PREFIX && WEB_PREFIX[0]) ? WEB_PREFIX : "");
  Serial.println(F("────────────────────────────────────"));
}

// ═══════════════════════════════════════════════════════════════
//  Loop
// ═══════════════════════════════════════════════════════════════
void loop() {
  // ─── معالجة طلبات الويب ───
  web.handleClient();

  // ─── مراقبة WiFi ───
  static uint32_t lastWifiCheck = 0;
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F("[WiFi] Disconnected — reconnecting"));
      WiFi.reconnect();
    }
  }

  // ─── مراقبة الذاكرة (كل دقيقة) ───
  static uint32_t lastHeapCheck = 0;
  if (millis() - lastHeapCheck > 60000) {
    lastHeapCheck = millis();
    Serial.printf_P(PSTR("[Heap] Free: %u bytes\n"), ESP.getFreeHeap());
  }

  // ⚠️ لا delay() طويل هنا — الويب يحتاج loop سريع
  yield();
}