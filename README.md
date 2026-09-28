# VoltronicMAX Arduino Library

**Author:** Kitronic  
**Contact:** info@kitronic.tech  
**Repository:** https://github.com/kitronic/esp-lib-voltronic-max.git  
**License:** MIT  
**Version:** 1.3.0

مكتبة Arduino كاملة للتواصل مع إنفرترات **Voltronic Power Axpert MAX** (موديلات HV7.2kW و LV5kW) عبر بروتوكول **PI30 ASCII**.

> ⚠️ **ملاحظة مهمة**: هذه المكتبة **ليست Modbus**. البروتوكول ASCII على RS232/UART بسرعة 2400 8N1.

---

## ✨ المميزات

### الأساسيات
- ✅ **تغطية كاملة** لكل أوامر الملف الرسمي (25 inquiry + 30 setting)
- ✅ **Zero dynamic allocation** — بدون `malloc` / `new` / `String`
- ✅ **ذاكرة منخفضة** — أقل من 1 KB RAM لكل البيانات
- ✅ **CRC-16/XMODEM** مع دعم escape quirk للأجهزة
- ✅ **Non-blocking friendly** — بدون `delay()` داخلي
- ✅ **دعم SoftwareSerial و HardwareSerial**
- ✅ **ESP32 / ESP8266 / AVR / SAMD / STM32**
- ✅ **قابل للتخصيص** — baud، timeout، retries، behavior flags

### إضافات v1.3.0 — حساب البطارية والتحكم الذكي
- ✅ **`VoltronicBattery`** — SOC/SOH/Energy/C-Rate من الجهد (منحنيات LiFePO4/AGM)
- ✅ **`VoltronicSmartCharger`** — شحن ذكي تلقائي (MCHGC/MUCHGC)
- ✅ **`VoltronicPowerMode`** — وضع الطاقة (عادي/طوارئ/فائض) بعتبات قابلة للتخصيص
- ✅ **`VoltronicStorage`** — حفظ دائم في EEPROM مع CRC16
- ✅ **`VoltronicLang`** — عربي/إنجليزي تلقائي (50+ ترجمة)
- ✅ **Device Status Decoder** — فك bits QPIGS.deviceStatus
- ✅ **Web Console** — واجهة تحكم كاملة (ESP8266/ESP32)

### الاختبارات والأمثلة
- ✅ **20 مثال** جاهز
- ✅ **Unit tests** لـ CRC والـ parsers

---

## 📥 التثبيت

### Arduino Library Manager

1. `Sketch → Include Library → Manage Libraries`
2. ابحث عن **VoltronicMAX**
3. Install

### يدوي

```bash
cd ~/Arduino/libraries
git clone https://github.com/kitronic/esp-lib-voltronic-max.git
```

### PlatformIO

```ini
lib_deps =
    https://github.com/kitronic/esp-lib-voltronic-max.git
```

---

## 🚀 الاستخدام السريع

```cpp
#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);       // ESP8266
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  invSerial.begin(2400);                // ← مهم: هيّئ Serial بنفسك
  inverter.begin(2400);                 // ← المكتبة تخزن الإعدادات فقط
}

void loop() {
  if (inverter.queryGeneralStatus()) {
    const QPIGSData& d = inverter.qpigs();
    Serial.printf("Grid=%.1fV Batt=%.2fV Load=%u%%\n",
                  d.gridVoltage(), d.batteryVoltage(), d.loadPercent);
  }
  delay(2000);
}
```

> ⚠️ **مهم:** المكتبة لا تهيّئ `Serial` تلقائياً. لازم تنادي `invSerial.begin(2400)`  
> (أو `Serial2.begin(2400, SERIAL_8N1, rx, tx)` للـ ESP32) قبل `inverter.begin()`.

---

## ⚙️ الإعداد الكامل (VoltronicConfig)

```cpp
VoltronicConfig cfg;
cfg.baud                     = 2400;    // معدل الباود
cfg.responseTimeoutMs        = 800;     // مهلة الاستجابة
cfg.retries                  = 2;       // عدد محاولات إعادة الإرسال
cfg.applyCrcEscape           = true;    // تفعيل escape للـ CRC
cfg.filterPrintable          = true;    // استبدال الأحرف غير المطبوعة
cfg.yieldDuringRead          = true;    // yield() أثناء القراءة (ESP8266)
cfg.clearBufferBeforeSend    = true;    // تنظيف البافر قبل الإرسال

inverter.begin(cfg);
```

### إعدادات compile-time (في VoltronicConfig.h)

| التعريف | الافتراضي | الوصف |
|---|---|---|
| `VOLTRONIC_RESP_BUF_SIZE` | 160 | حجم buffer الرد |
| `VOLTRONIC_CMD_BUF_SIZE` | 32 | حجم buffer الأمر |
| `VOLTRONIC_MAX_RETRIES` | 2 | عدد محاولات إعادة الإرسال |
| `VOLTRONIC_DEFAULT_BAUD` | 2400 | الباود الافتراضي |
| `VOLTRONIC_DEFAULT_TIMEOUT_MS` | 800 | مهلة الاستجابة |
| `VOLTRONIC_CRC_ESCAPE` | 1 | تفعيل CRC escape |
| `VOLTRONIC_FILTER_PRINTABLE` | 1 | filter الأحرف |
| `VOLTRONIC_YIELD_IN_READ` | 1 | yield أثناء القراءة |

---

## 📋 الأوامر المدعومة (كامل)

### 🔍 Inquiry Commands (25)

| # | الأمر | الدالة | الوصف |
|---|---|---|---|
| 1 | `QPI` | `queryProtocolID()` | Protocol ID |
| 2 | `QID` | `querySerialNumber()` | Serial number |
| 3 | `QSID` | `querySerialNumberLong()` | Serial (طويل) |
| 4 | `QVFW` | `queryFirmware()` | Main CPU FW |
| 5 | `QVFW3` | `queryFirmware2()` | Secondary CPU FW |
| 6 | `VERFW:` | `queryBluetoothVersion()` | Bluetooth FW |
| 7 | `QPIRI` | `queryRating()` | Rating info |
| 8 | `QFLAG` | `queryFlags()` | Device flags |
| 9 | `QPIGS` | `queryGeneralStatus()` | Live status |
| 10 | `QPIGS2` | `queryGeneralStatus2()` | Live status (PV2) |
| 11 | `QPGSn` | `queryParallel(n)` | Parallel info |
| 12 | `QMOD` | `queryMode()` | Working mode |
| 13 | `QPIWS` | `queryWarnings()` | Warning status |
| 14 | `QDI` | `queryDefaults()` | Default settings |
| 15 | `QMCHGCR` | `queryMaxChargingCurrents()` | Max charge options |
| 16 | `QMUCHGCR` | `queryMaxUtilityChargingCurrents()` | Max utility options |
| 17 | `QOPPT` | `queryOutputPriorityTimeOrder()` | Output priority table |
| 18 | `QCHPT` | `queryChargerPriorityTimeOrder()` | Charger priority table |
| 19 | `QT` | `queryTime()` | Device time |
| 20 | `QBEQI` | `queryBatteryEqualization()` | Battery equalization |
| 21 | `QMN` | `queryModelName()` | Model name |
| 22 | `QGMN` | `queryGeneralModelName()` | General model name |
| 23 | `QBOOT` | `queryBoot()` | DSP bootstrap |
| 24 | `QBATCD` | `queryBatteryControl()` | Battery control status |
| 25 | `QLED` | `queryLed()` | LED status |

### ⚙️ Setting Commands (30)

| # | الأمر | الدالة | الوصف |
|---|---|---|---|
| 1 | `PEx` | `setFlag(x)` | Enable flag |
| 2 | `PDx` | `clearFlag(x)` | Disable flag |
| 3 | `PF` | `resetDefaults()` | Factory reset |
| 4 | `MNCHGC` | `setMaxChargingCurrent(amps)` | Max charging current |
| 5 | `MUCHGC` | `setMaxUtilityChargingCurrent(amps)` | Max utility current |
| 6 | `F` | `setOutputFrequency(hz)` | Output frequency |
| 7 | `V` | `setOutputVoltage(v)` | Output voltage |
| 8 | `POP` | `setOutputSourcePriority(p)` | Output priority |
| 9 | `PBCV` | `setBatteryRechargeVoltage(v)` | Battery recharge |
| 10 | `PBDV` | `setBatteryRedischargeVoltage(v)` | Battery redischarge |
| 11 | `PCP` | `setChargerSourcePriority(p)` | Charger priority |
| 12 | `PGR` | `setGridWorkingRange(r)` | Grid range |
| 13 | `PBT` | `setBatteryType(t)` | Battery type |
| 14 | `POPM` | `setOutputMode(m)` | Output mode |
| 15 | `PPCP` | `setParallelChargerPriority(m,p)` | Parallel charger |
| 16 | `PSDV` | `setBatteryCutoffVoltage(v)` | Battery cutoff |
| 17 | `PCVV` | `setBatteryCvVoltage(v)` | CV charging |
| 18 | `PBFT` | `setBatteryFloatVoltage(v)` | Float charging |
| 19 | `RTEY` | `resetEnergy()` | Reset PV/load energy |
| 20 | `RTDL` | `eraseLog()` | Erase data log |
| 21 | `PBEQE` | `setBatteryEqualizationEnabled(en)` | Enable equalization |
| 22 | `PBEQT` | `setBatteryEqualizationTime(min)` | Equalization time |
| 23 | `PBEQP` | `setBatteryEqualizationPeriod(days)` | Equalization period |
| 24 | `PBEQV` | `setBatteryEqualizationVoltage(v)` | Equalization voltage |
| 25 | `PBEQOT` | `setBatteryEqualizationOverTime(min)` | Equalization over time |
| 26 | `PBEQA` | `activateBatteryEqualization(a)` | Activate now |
| 27 | `PCVT` | `setMaxCvChargingTime(min)` | Max CV time |
| 28 | `DAT` | `setDateTime(s)` | Set date/time |
| 29 | `PBATCD` | `setBatteryControl(a,b,c)` | Battery C/D control |
| 30 | `PBATMAXDISC` | `setMaxDischargingCurrent(a)` | Max discharge |

---

## 🌐 Web Console (جديد في v1.2.0)

واجهة تحكم كاملة عبر المتصفح لـ **ESP8266** و **ESP32**. تعرض كل الأوامر كأزرار وتقبل أوامر خام مباشرة.

### التمثيل البصري

```
┌─────────────────────────────────────┐
│  Voltronic MAX                      │
│                                     │
│  ┌─ Commands ─────────────────────┐ │
│  │ Protocol ID      QPI    [Send] │ │
│  │ General status   QPIGS  [Send] │ │
│  │ Battery CV       PBCV [56.4]   │ │
│  │ ...                            │ │
│  └────────────────────────────────┘ │
│                                     │
│  ┌─ Raw command ──────────────────┐ │
│  │ [QPIGS___________]  [Send]     │ │
│  └────────────────────────────────┘ │
│                                     │
│  ┌─ Output ───────────────────────┐ │
│  │ [14:32:05] > QPIGS             │ │
│  │ (230.1 50.0 230.1 50.0 ...     │ │
│  └────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### التفعيل (ESP8266)

```cpp
#define VOLTRONIC_USE_WEB       // ← قبل include
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "VoltronicMAX.h"

ESP8266WebServer web(80);
VoltronicMAX     inverter(&Serial1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(2400, SERIAL_8N1, 4, 5);   // RX=D2, TX=D1
  inverter.begin(2400);

  WiFi.begin("SSID", "PASS");
  while (WiFi.status() != WL_CONNECTED) delay(200);

  inverter.attachWebServer(&web, "admin", "changeme");
  inverter.setWebPrefix("/inv");           // → http://<ip>/inv/
  web.begin();
}

void loop() {
  web.handleClient();
}
```

### التفعيل (ESP32)

```cpp
#define VOLTRONIC_USE_WEB
#include <WiFi.h>
#include <WebServer.h>          // ← WebServer بدل ESP8266WebServer
#include "VoltronicMAX.h"

WebServer    web(80);
VoltronicMAX inverter(&Serial2);

void setup() {
  Serial.begin(115200);
  Serial2.begin(2400, SERIAL_8N1, 16, 17);   // RX, TX
  inverter.begin(2400);

  WiFi.begin("SSID", "PASS");
  while (WiFi.status() != WL_CONNECTED) delay(200);

  inverter.attachWebServer(&web);   // بدون auth
  inverter.setWebPrefix("");
  web.begin();
}

void loop() {
  web.handleClient();
}
```

### API الـ REST

| Method | Endpoint | الوصف |
|---|---|---|
| `GET` | `{prefix}/` | صفحة HTML |
| `GET` | `{prefix}/api/commands` | JSON قائمة الأوامر |
| `POST` | `{prefix}/api/cmd` | تنفيذ أمر |

**مثال POST:**

```bash
curl -X POST http://192.168.1.100/inv/api/cmd \
     -d "cmd=QPIGS&param=&type=0"
```

**الرد:**

```json
{
  "ok": true,
  "cmd": "QPIGS",
  "response": "(230.1 50.0 230.1 50.0 ...",
  "error": ""
}
```

### استهلاك الذاكرة

| المكوّن | الحجم |
|---|---|
| HTML/CSS/JS (PROGMEM) | ~3 KB Flash |
| جدول الأوامر (PROGMEM) | ~700 B Flash |
| Static buffers (RAM) | ~5.4 KB |
| **المجموع** | **~3.7 KB Flash + 5.4 KB RAM** |

على ESP8266 (80 KB heap) → **6.75% فقط**.

### ملاحظات أمنية

- ⚠️ **افتراضياً بدون auth** — للاستخدام المحلي فقط
- ✅ مع auth: Basic Auth بسيط (HTTPS مو مفعّل افتراضياً)
- ✅ فلترة صارمة للأوامر: `A-Z 0-9 :` فقط
- ✅ فلترة للمعاملات: `0-9 . - ,` فقط
- ✅ حد أقصى للطول: 20 حرف للأمر، 32 للكامل
- ❌ **لا تفتح المنفذ 80 على الإنترنت بدون reverse proxy + HTTPS**

---

## 📊 هيكل البيانات

### `QPIGSData` — حالة الإنفرتر

```cpp
struct QPIGSData {
  uint16_t gridVoltage_x10;       // 2301 = 230.1 V
  uint16_t gridFrequency_x10;     // 500  = 50.0 Hz
  uint16_t acOutputVoltage_x10;
  uint16_t acOutputFrequency_x10;
  uint16_t acOutputApparentPower; // VA
  uint16_t acOutputActivePower;   // W
  uint8_t  loadPercent;           // %
  uint16_t busVoltage;
  uint16_t batteryVoltage_x100;   // 5200 = 52.00 V
  uint16_t batteryChargingCurrent;
  uint8_t  batteryCapacity;
  uint16_t inverterTemperature;
  uint16_t pv1InputCurrent_x10;
  uint16_t pv1InputVoltage_x10;
  uint16_t sccBatteryVoltage_x100;
  uint16_t batteryDischargeCurrent;
  uint8_t  deviceStatus;          // b7..b0
  uint16_t batteryVoltageOffsetForFans;
  uint8_t  eepromVersion;
  uint16_t pv1ChargingPower;
  uint8_t  deviceStatus2;         // b10b9b8
  uint8_t  solarFeedToGridStatus;
  uint8_t  countryRegulation;
  uint16_t solarFeedToGridPower;

  // Getter للقيم الفعلية
  float gridVoltage() const;
  float batteryVoltage() const;
  // ...
};
```

- `QPIRIData` — معلومات التقييم
- `QFLAGData` — الأعلام (مع `raw` نصي)
- `WarningDecoded` — التحذيرات (36-bit، `uint64_t`)
- `ParallelInfo` — معلومات التوازي
- `BatteryEqualizationInfo` — معادلة البطارية
- `LedInfo` — حالة LED
- `DefaultsInfo` — الإعدادات الافتراضية
- `BatteryControlStatus` — حالة التحكم
- `SelectableValues` — القيم القابلة للاختيار
- `TimeOrderInfo` — الجدول الزمني

---

## 🧠 استهلاك الذاكرة

| البند | الحجم (بايت) |
|---|---|
| `QPIGSData` | ~50 |
| `QPIGS2Data` | ~8 |
| `QPIRIData` | ~40 |
| `QFLAGData` | ~22 |
| `ParallelInfo` | ~50 |
| `BatteryEqualizationInfo` | ~14 |
| `LedInfo` | ~13 |
| `DefaultsInfo` | ~22 |
| `BatteryControlStatus` | 3 |
| `SelectableValues` × 2 | ~34 |
| `TimeOrderInfo` × 2 | ~52 |
| Buffers (`_respBuf` + `_cmdBuf`) | ~192 |
| **المجموع الأساسي** | **~500 بايت** |
| + Web Console (اختياري) | +56 بايت |

على ESP8266/ESP32 يترك مساحة كبيرة للتطبيق.

---

## 🧪 Unit Tests

المكتبة فيها اختبارات لـ:

- **CRC-16/XMODEM** — حساب + escape
- **Parsers** — كل parser مع عينات حقيقية
- **Frame building** — بناء الأمر + CRC
- **Response validation** — تحقق من الردود

### تشغيل الاختبارات

**مع PlatformIO**

```bash
pio test -e native
```

**يدويًا**

```bash
cd test/test_native
g++ -std=c++11 -I../../src test_native.cpp ../../src/VoltronicParser.cpp -o test
./test
```

راجع `test/README.md` لتفاصيل أكثر.

---

## 📁 الأمثلة (20 مثال)

| # | المثال | الوصف |
|---|---|---|
| 01 | `BasicRead` | قراءة عامة |
| 02 | `DeviceInfo` | S/N، FW، Model |
| 03 | `GeneralStatus` | QPIGS كامل |
| 04 | `GeneralStatus2` | QPIGS2 (PV2) |
| 05 | `Rating` | QPIRI |
| 06 | `Mode` | QMOD |
| 07 | `Warnings` | QPIWS |
| 08 | `Flags` | QFLAG |
| 09 | `Defaults` | QDI |
| 10 | `Parallel` | QPGSn |
| 11 | `BatteryEqualization` | QBEQI |
| 12 | `Led` | QLED |
| 13 | `TimeOrder` | QOPPT/QCHPT |
| 14 | `BatteryControl` | QBATCD |
| 15 | `SelectableValues` | QMCHGCR/QMUCHGCR |
| 16 | `AllSettings` | كل أوامر الضبط |
| 17 | `NonBlocking` | state machine |
| 18 | `AsyncPoller` | جدولة دورية |
| 19 | `WebConsole` | واجهة ويب (ESP8266) |
| 20 | `WebConsoleESP32` | واجهة ويب (ESP32) |

---

## 🔧 استكشاف الأخطاء

### الرد لا يبدأ بـ `(` أو `ACK` / `NAK`

- تحقق من إعدادات المنفذ: 2400 8N1
- تحقق من توصيلات RX/TX
- جرب تعكس RX و TX

### `lastError() == ERR_CRC`

- تحقق من الباود
- جرب `cfg.applyCrcEscape = false`
- تحقق من عدم وجود noise على الخط

### `lastError() == ERR_TIMEOUT`

- زد `cfg.responseTimeoutMs`
- تحقق من أن الإنفرتر مستجيب (استعمل `QPI` أولاً)

### `lastError() == ERR_NAK`

- الأمر غير مدعوم في هذه النسخة من الفيرموير

### ESP8266 — رسائل Exception

- تأكد من `cfg.yieldDuringRead = true`
- قلل `responseTimeoutMs`

### ESP32 — Serial2 pins

```cpp
Serial2.begin(2400, SERIAL_8N1, 16, 17);
inverter.begin(2400);
```

### Web Console لا تُحمّل

- تأكد `#define VOLTRONIC_USE_WEB` **قبل** `#include`
- تحقق من Serial Monitor: يجب أن ترى `[Voltronic] Web routes at "/inv"`
- افتح `http://<ip>/inv/` (مع `/` في النهاية)
- لو auth مفعّل: اسم المستخدم/كلمة المرور المحددة

### Web Console: `Load error`

- افتح Console في المتصفح (F12)
- تحقق من `api/commands` endpoint مباشرة في المتصفح
- تأكد من `<base href>` — مطلوب للـ fetch النسبي

---

## 📄 الترخيص

MIT © Kitronic

---

## 🔗 مراجع

- [PI30 Protocol Reference (Voltronic)](docs/PROTOCOL.md)
- [Commands Reference](docs/COMMANDS.md)
- [CRC Details](docs/CRC.md)
- [Memory Analysis](docs/MEMORY.md)

---

## 📝 Changelog

راجع [CHANGELOG.md](CHANGELOG.md) لكل الإصدارات.