# VoltronicMAX — Project Structure Reference

> مرجع بنية المشروع v1.3.0 — Kitronic
> آخر تحديث: 2026-09-28

---

## 📁 شجرة المشروع الكاملة
esp-lib-voltronic-max/
│
├── library.properties ← Arduino Library Manager
├── library.json ← PlatformIO Registry
├── keywords.txt ← Arduino IDE syntax highlighting
├── README.md ← الشرح الرئيسي
├── CHANGELOG.md ← سجل الإصدارات
├── CONTRIBUTING.md ← دليل المساهمة
├── LICENSE ← MIT
├── .gitignore
├── .gitattributes
│
├── .github/
│ └── workflows/
│ └── arduino-ci.yml ← CI/CD (ESP32 + ESP8266 + AVR + native tests)
│
├── docs/
│ ├── COMMANDS.md ← مرجع الأوامر
│ ├── PROTOCOL.md ← بروتوكول PI30
│ ├── CRC.md ← شرح CRC-16/XMODEM
│ ├── MEMORY.md ← تحليل الذاكرة
│ └── STRUCTURE.md ← هذا الملف
│
├── src/ ← الكود المصدري (20 ملف)
│ │
│ ├── VoltronicMAX.h ← الكلاس الرئيسي (API)
│ ├── VoltronicMAX.cpp ← التنفيذ
│ ├── VoltronicConfig.h ← إعدادات compile-time + runtime
│ ├── VoltronicCommands.h ← ثوابت الأوامر (VC_*)
│ ├── VoltronicCRC.h ← CRC-16/XMODEM + escape
│ ├── VoltronicTypes.h ← structs (QPIGSData, QPIRIData, ...)
│ ├── VoltronicParser.h ← Parsers (static class)
│ ├── VoltronicParser.cpp ← التنفيذ
│ ├── VoltronicTransport.h ← Stream wrapper (UART)
│ │
│ ├── VoltronicWeb.h ← Web Console (opt-in VOLTRONIC_USE_WEB)
│ │
│ ├── VoltronicBattery.h ← v1.3.0: SOC/SOH/Energy
│ ├── VoltronicBattery.cpp ← v1.3.0: منحنيات الجهد→SOC
│ ├── VoltronicSmartCharger.h ← v1.3.0: شحن ذكي (MCHGC/MUCHGC)
│ ├── VoltronicSmartCharger.cpp ← v1.3.0: منطق الشحن
│ ├── VoltronicPowerMode.h ← v1.3.0: وضع الطاقة
│ ├── VoltronicPowerMode.cpp ← v1.3.0: Emergency/Surplus
│ ├── VoltronicStorage.h ← v1.3.0: EEPROM persistence
│ ├── VoltronicStorage.cpp ← v1.3.0: CRC16 + auto-save
│ ├── VoltronicLang.h ← v1.3.0: عربي/إنجليزي
│ └── VoltronicLang.cpp ← v1.3.0: جداول الترجمة
│
├── examples/ ← 20 مثال
│ ├── 01_BasicRead/
│ │ └── 01_BasicRead.ino
│ ├── 02_DeviceInfo/
│ │ └── 02_DeviceInfo.ino
│ ├── 03_GeneralStatus/
│ │ └── 03_GeneralStatus.ino
│ ├── 04_GeneralStatus2/
│ │ └── 04_GeneralStatus2.ino
│ ├── 05_Rating/
│ │ └── 05_Rating.ino
│ ├── 06_Mode/
│ │ └── 06_Mode.ino
│ ├── 07_Warnings/
│ │ └── 07_Warnings.ino
│ ├── 08_Flags/
│ │ └── 08_Flags.ino
│ ├── 09_Defaults/
│ │ └── 09_Defaults.ino
│ ├── 10_Parallel/
│ │ └── 10_Parallel.ino
│ ├── 11_BatteryEqualization/
│ │ └── 11_BatteryEqualization.ino
│ ├── 12_Led/
│ │ └── 12_Led.ino
│ ├── 13_TimeOrder/
│ │ └── 13_TimeOrder.ino
│ ├── 14_BatteryControl/
│ │ └── 14_BatteryControl.ino
│ ├── 15_SelectableValues/
│ │ └── 15_SelectableValues.ino
│ ├── 16_AllSettings/
│ │ └── 16_AllSettings.ino
│ ├── 17_NonBlocking/
│ │ └── 17_NonBlocking.ino
│ ├── 18_AsyncPoller/
│ │ └── 18_AsyncPoller.ino
│ ├── 19_WebConsole/
│ │ └── 19_WebConsole.ino ← ESP8266
│ └── 20_WebConsoleESP32/
│ └── 20_WebConsoleESP32.ino ← ESP32
│
└── test/
├── README.md
└── test_native/
├── test_native.cpp ← 54+ اختبار
├── Arduino.h ← mock للـ host
└── Arduino.cpp ← تنفيذ mock


---

## 🧩 الملفات الأساسية (Core)

### 1) `VoltronicMAX.h/cpp` — الكلاس الرئيسي

**يحتوي على:**
- `begin(cfg)` / `begin(baud)` — تهيئة
- `sendRaw(cmd)` — أمر استعلام خام
- `sendRawSetting(cmd)` — أمر ضبط خام (مع ACK)
- 25 دالة `queryXxx()` — استعلامات
- 30 دالة `setXxx()` — إعدادات
- Getters: `qpigs()`, `qpiri()`, `qflag()`, ...
- Polling: `startPolling()`, `stopPolling()`, `poll()`
- Status: `lastError()`, `lastResponse()`, `lastErrorName()`
- 5 كائنات (v1.3.0): `battery`, `smartCharger`, `powerMode`, `storage`, `lang`
- Web (opt-in): `attachWebServer()`, `setWebPrefix()`

### 2) `VoltronicConfig.h`

**يحتوي على:**
- `#define VOLTRONIC_RESP_BUF_SIZE` (افتراضي 160)
- `#define VOLTRONIC_CMD_BUF_SIZE` (افتراضي 32)
- `struct VoltronicConfig` — baud/timeout/retries/flags
- `struct VoltronicPollSchedule` — جدولة الاستعلام

### 3) `VoltronicTypes.h`

**يحتوي على:**
- `enum VoltronicValidBit` (VVB_*)
- `struct QPIGSData` + `DeviceStatusInfo`
- `struct QPIGS2Data`
- `struct QPIRIData`
- `struct QFLAGData`
- `struct WarningDecoded` (uint64)
- `struct ParallelInfo`
- `struct BatteryEqualizationInfo`
- `struct LedInfo`
- `struct DefaultsInfo`
- `struct BatteryControlStatus`
- `struct SelectableValues`
- `struct TimeOrderInfo`

### 4) `VoltronicCommands.h`

**يحتوي على:**
- 25 ثابت `VC_Q*` (استعلامات)
- 30 ثابت `VC_P*` (إعدادات)
- `VC_MODE_*` — أوضاع الإنفرتر
- `VC_OUT_PRIO_*` / `VC_CHG_PRIO_*` — أولويات
- `VC_BAT_TYPE_*` — أنواع البطاريات
- `VC_FLAG_*` — أعلام PEx/PDx

### 5) `VoltronicCRC.h`

**يحتوي على:**
- `voltronicCRC()` — حساب CRC-16/XMODEM
- `voltronicCRCUpdate()` — تزايدي
- `voltronicEscapeByte()` / `voltronicUnescapeByte()`
- `voltronicCRCBytes()` — التحويل لـ hi/lo

### 6) `VoltronicParser.h/cpp`

**يحتوي على:**
- `parseQPIGS()` / `parseQPIGS2()`
- `parseQPIRI()` / `parseQMOD()` / `parseQPIWS()`
- `parseQFLAG()` / `parseQPGSn()` / `parseQBEQI()`
- `parseQLED()` / `parseQDI()` / `parseQBATCD()`
- `parseQMCHGCR()` / `parseQMUCHGCR()`
- `parseQOPPT()` / `parseQCHPT()` / `parseQBOOT()`
- Helpers: `strToU16x10()`, `strToU16x100()`, `modeToString()`

### 7) `VoltronicTransport.h`

**يحتوي على:**
- `class VoltronicTransport`
- `readUntilCR()` — قراءة حتى `\r`
- `writeRaw()` — إرسال فريم
- `clear()` — تفريغ البافر

---

## 🆕 ملفات v1.3.0 (الجديدة)

### 8) `VoltronicBattery.h/cpp`

**الكلاس:** `VoltronicBattery`

**الوظائف:**
- `setType()` / `setCapacity()` / `setVoltageEmpty()` / `setVoltageFull()`
- `update(g)` — يُستدعى من `queryGeneralStatus()`
- Getter: `soc()`, `soh()`, `netPower()`, `remainingKWh()`, `timeToFull()`, `cRate()`, `status()`
- منحنيات: `CURVE_LIFEPO4_15S`, `CURVE_LIFEPO4_16S`, `CURVE_LEADACID_48V`

### 9) `VoltronicSmartCharger.h/cpp`

**الكلاس:** `VoltronicSmartCharger`

**الوظائف:**
- `setMode(DISABLED|STANDARD|FAST)`
- `setTargetAC()` / `setTargetTotal()` / `setFloatAC()` / `setFloatTotal()`
- `setTempProtectC()` — حماية الحرارة
- `update(inv, bat)` — يُستدعى دورياً
- يقرأ الخيارات من QMCHGCR/QMUCHGCR (لا hardcoded)
- إحصاءات: `changes()`, `floatEntries()`, `tempProtects()`

### 10) `VoltronicPowerMode.h/cpp`

**الكلاس:** `VoltronicPowerMode`

**الوظائف:**
- `setSocEmergency()` / `setSocPowerSaving()`
- `setSocRecover()` / `setSocSurplus()`
- `setGridMinVoltage()`
- `update(inv, bat)` — يقيّم الوضع
- الأوضاع: `NORMAL`, `POWER_SAVING`, `EMERGENCY_MAX`, `SOLAR_SURPLUS`, `GRID_SURPLUS`, `COMBINED_SURPLUS`

### 11) `VoltronicStorage.h/cpp`

**الكلاس:** `VoltronicStorage`

**الوظائف:**
- `begin(eepromSize=512)` — يُستدعى في `inverter.begin()`
- `load(inv)` — تحميل من EEPROM
- `save(inv)` — حفظ إلى EEPROM
- `reset()` — إرجاع للافتراضي
- `tick(inv)` — auto-save بعد 5s
- `isLoaded()`, `isDirty()`, `saves()`
- CRC16 للتحقق

### 12) `VoltronicLang.h/cpp`

**الكلاس:** `VoltronicLang`

**الوظائف:**
- `setLanguage(ARABIC|ENGLISH)`
- `tr(Key)` — ترجمة
- `isArabic()`
- Helpers: `batteryTypeStr()`, `inverterModeStr()`, `outputPrioStr()`, `chargerPrioStr()`, `inputRangeStr()`, `powerModeStr()`, `chargingCodeStr()`
- 60+ مفتاح ترجمة

### 13) `VoltronicWeb.h`

**بدون كلاس منفصل** — امتداد لـ `VoltronicMAX`.

**الوظائف:**
- `attachWebServer(server)` / `attachWebServer(server, user, pass)`
- `setWebPrefix()`
- Routes: `/`, `/api/commands`, `/api/cmd`, `/api/battery`, `/api/smartcharger`, `/api/powermode`, `/api/status`
- HTML/CSS/JS في PROGMEM
- Chunked Transfer-Encoding

---

## 📦 ملفات الجذر

### `library.properties` — Arduino Library Manager
name=VoltronicMAX
version=1.3.0
author=Kitronic
maintainer=Kitronic info@kitronic.tech
sentence=...
paragraph=...
category=Communication
url=https://github.com/kitronic/esp-lib-voltronic-max.git
repository=https://github.com/kitronic/esp-lib-voltronic-max.git
architectures=esp32,esp8266,avr,samd,stm32
includes=VoltronicMAX.h

### `library.json` — PlatformIO Registry

```json
{
  "name": "VoltronicMAX",
  "version": "1.3.0",
  "frameworks": "arduino",
  "platforms": "*",
  "headers": "VoltronicMAX.h",
  ...
}