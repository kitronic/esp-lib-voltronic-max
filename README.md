# VoltronicMAX Arduino Library

**Author:** Kitronic
**Contact:** info@kitronic.tech
**Repository:** https://github.com/kitronic/esp-lib-voltronic-max.git
**License:** MIT

مكتبة Arduino كاملة للتواصل مع إنفرترات **Voltronic Power Axpert MAX** (موديلات HV7.2kW و LV5kW) عبر بروتوكول **PI30 ASCII**.

> ⚠️ **ملاحظة مهمة**: هذه المكتبة **ليست Modbus**. البروتوكول ASCII على RS232/UART بسرعة 2400 8N1.

---

## ✨ المميزات

- ✅ **تغطية كاملة** لكل أوامر الملف الرسمي (25 inquiry + 30 setting)
- ✅ **Zero dynamic allocation** — بدون `malloc` / `new` / `String`
- ✅ **ذاكرة منخفضة** — أقل من 1 KB RAM لكل البيانات
- ✅ **CRC-16/XMODEM** مع دعم escape quirk للأجهزة
- ✅ **Non-blocking friendly** — بدون `delay()` داخلي
- ✅ **دعم SoftwareSerial و HardwareSerial**
- ✅ **ESP32 / ESP8266 / AVR / SAMD / STM32**
- ✅ **قابل للتخصيص** — baud، timeout، retries، behavior flags
- ✅ **18 مثال** جاهز
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

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  inverter.begin(2400);
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

---

## ⚙️ الإعداد الكامل (VoltronicConfig)

```cpp
VoltronicConfig cfg;
cfg.baud                     = 2400;    // معدل الباود
cfg.responseTimeoutMs        = 800;     // مهلة الاستجابة
cfg.retries                  = 2;       // عدد محاولات إعادة الإرسال
cfg.applyCrcEscape           = true;    // تفعيل escape للـ CRC
cfg.filterPrintable          = true;    // تجاهل الأحرف غير المطبوعة
cfg.yieldDuringRead          = true;    // yield() أثناء القراءة (ESP8266)
cfg.clearBufferBeforeSend    = true;    // تنظيف البافر قبل الإرسال

inverter.begin(cfg);
```

### إعدادات compile-time (في VoltronicConfig.h)

| التعريف | الافتراضي | الوصف |
|---|---|---|
| `VOLTRONIC_RESP_BUF_SIZE` | 160 | حجم buffer الرد |
| `VOLTRONIC_CMD_BUF_SIZE` | 16 | حجم buffer الأمر |
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
- `QFLAGData` — الأعلام
- `WarningDecoded` — التحذيرات
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
| Buffers | ~180 |
| **المجموع** | **~500 بايت** |

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

## 📁 الأمثلة

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

---

## 🔧 استكشاف الأخطاء

**الرد لا يبدأ بـ `(` أو `ACK` / `NAK`**
- تحقق من إعدادات المنفذ: 2400 8N1
- تحقق من توصيلات RX/TX
- جرب تعكس RX و TX

**`lastError() == ERR_CRC`**
- تحقق من الباود
- جرب `cfg.applyCrcEscape = false`
- تحقق من عدم وجود noise على الخط

**`lastError() == ERR_TIMEOUT`**
- زد `cfg.responseTimeoutMs`
- تحقق من أن الإنفرتر مستجيب (استعمل `QPI` أولاً)

**`lastError() == ERR_NAK`**
- الأمر غير مدعوم في هذه النسخة من الفيرموير

**ESP8266 — رسائل Exception**
- تأكد من `cfg.yieldDuringRead = true`
- قلل `responseTimeoutMs`

**ESP32 — Serial2 pins**
```cpp
Serial2.begin(2400, SERIAL_8N1, 16, 17);
inverter.begin(2400);
```

---

## 📄 الترخيص

MIT © Kitronic

---

# 📦 الخطوة 4: الأمثلة (كل الأوامر)

## `examples/01_BasicRead/01_BasicRead.ino`

```cpp
#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);  // ESP8266
// ESP32: use Serial2 with pins
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- BasicRead ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryGeneralStatus()) {
    const QPIGSData& d = inverter.qpigs();
    Serial.printf("Grid: %.1fV %.1fHz\n", d.gridVoltage(), d.gridFrequency());
    Serial.printf("Out:  %.1fV %.1fHz | Load %u%%\n",
                  d.acOutputVoltage(), d.acOutputFrequency(), d.loadPercent);
    Serial.printf("Batt: %.2fV %uA %u%% | Temp %uC\n",
                  d.batteryVoltage(), d.batteryChargingCurrent(),
                  d.batteryCapacity, d.inverterTemperature);
    Serial.printf("PV1:  %.1fV %.1fA %uW\n",
                  d.pv1InputVoltage(), d.pv1InputCurrent(), d.pv1ChargingPower);
  } else {
    Serial.printf("Failed, err=%u\n", inverter.lastError());
  }
  delay(3000);
}
