# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.3.0] - 2026-09-28

### Added

**Battery Calculation Library**
- New class: `VoltronicBattery`
  - SOC from voltage (piecewise-linear curves)
  - Supported types: User, AGM, Flooded, Pylontech, LiFePO4 15S, LiFePO4 16S
  - Net current / net power
  - Remaining energy (Wh / kWh)
  - Remaining hours (during discharge)
  - Time to full (during charge)
  - C-Rate
  - Status: Charging / Discharging / Idle
  - User config: capacity (Ah), voltage empty/full, initial SOH, cycle count
- New enum: `VoltronicBatteryType`

**Smart Charger**
- New class: `VoltronicSmartCharger`
  - Automatic MCHGC/MUCHGC control based on SOC + temperature + float stage
  - Modes: Disabled, Standard (Step Down), Fast (Float Only)
  - Temperature protection with configurable threshold
  - Reads available current options from `QMCHGCR` / `QMUCHGCR` (no hardcoded arrays)
  - Fallback values if inverter does not respond
  - Statistics: total changes, float entries, temp protects
  - One step per update (no flooding)
  - Bilingual status messages

**Power Mode**
- New class: `VoltronicPowerMode`
  - Modes: Normal, Power Saving, Emergency Max, Solar Surplus, Grid Surplus, Combined Surplus
  - User-configurable SOC thresholds (Emergency, Power Saving, Recover, Surplus)
  - Configurable grid minimum voltage
  - Emergency hysteresis with lock/unlock
  - PV detection via PV1 + PV2 power
  - Sustained discharge tracking (60s)

**Storage (EEPROM)**
- New class: `VoltronicStorage`
  - Persists all settings in EEPROM (512 bytes)
  - CRC16 validation
  - Auto-save with 5s debounce
  - Stores: Battery, Smart Charger, Power Mode, Language
  - `reset()` to factory defaults
  - `saves()` counter

**Language Support**
- New class: `VoltronicLang`
  - Arabic / English
  - 50+ translation keys
  - Enum-based: `lang.tr(Key)`
  - Helpers: `batteryTypeStr()`, `inverterModeStr()`, `outputPrioStr()`,
    `chargerPrioStr()`, `inputRangeStr()`, `powerModeStr()`, `chargingCodeStr()`
  - Persists in EEPROM
  - Runtime switchable via MQTT

**Device Status Decoder**
- New struct: `QPIGSData::DeviceStatusInfo`
- New method: `QPIGSData::decodeDeviceStatus()`
- Decodes: SBU priority, config changed, SCC FW updated, load ON, battery steady, charging code
- 8 charging codes (No Charge, Solar, AC, Solar+AC, Float, Equalization, Reserved, Float/Timer)

**Web UI Extensions**
- Chunked Transfer-Encoding (no 4KB page buffer limit)
- Battery settings card (Type, Capacity, Voltage Empty/Full)
- Smart Charger settings card (Mode, Targets, Float, Temp protect)
- Power Mode Thresholds card (5 configurable values)
- Live Status card (updates every 5s)
- New endpoints:
  - `POST /api/battery` → save battery settings
  - `POST /api/smartcharger` → save smart charger settings
  - `POST /api/powermode` → save power mode thresholds
  - `GET  /api/status` → JSON of all current values
- `showMsg()` floating banner (replaces alert)
- `loadInputs()` — loads form values once (no reset)
- `updateStatusBox()` — updates status only
- Device status parser in JavaScript

**MQTT Extensions**
- Battery entities from `VoltronicBattery` (13 sensors)
- Smart Charger entities (5 selects + 9 sensors)
- Language select entity (`solar/inverter/language/set`)
- Bilingual state values (all follow `inverter.lang`)
- TODOs (commented): Power Mode thresholds, Device Status sensors, Restart button

### Fixed

- `VoltronicPowerMode::update()`:
  - `hasSolar` now uses `pv1ChargingPower + pv2ChargingPower`
    (was incorrectly `deviceStatus & 0x20` = "SCC Firmware Updated")
  - Grid threshold now configurable (default 150V, was 90V)
- `chargingSourceStr()`:
  - Uses PV1 + PV2 power + charging current (was `deviceStatus` bits)
- `VoltronicBattery::LiFePO4_16S` curve:
  - Accurate: 53.9V → 82%, 54.0V → 83% (was 52%)
  - Range: 44.0V (0%) → 58.4V (100%)
- `VoltronicSmartCharger::update()`:
  - Reads QPIRI only once via `_currentsInitialized`
  - `modeToString()` changed from `static` to member
- `mqtt_manager.cpp`:
  - `snprintf` used instead of `String` for HAMQTT

### Changed

- `VoltronicWeb.h`:
  - `_webHandlePage()` uses chunked transfer
  - Page restructured with 7 cards
- `VoltronicSmartCharger`:
  - Options from `QMCHGCR`/`QMUCHGCR` (removed hardcoded arrays)
- `VoltronicPowerMode`:
  - Thresholds user-configurable
- `VoltronicStorage`:
  - Stores: Battery, Smart Charger, Power Mode, Language

### Documentation

- All new classes documented with inline comments
- Arabic/English labels throughout Web UI

---

## [1.2.0] - 2026-09-27

### Added
- **Web Console** for ESP8266 and ESP32 (opt-in via `#define VOLTRONIC_USE_WEB`)
  - `GET  {prefix}/`              → HTML page
  - `GET  {prefix}/api/commands`  → JSON command list
  - `POST {prefix}/api/cmd`       → Execute command
  - Optional Basic Auth (`attachWebServer(srv, user, pass)`)
  - Configurable path prefix (`setWebPrefix("/inv")`)
  - Zero heap — all HTML/CSS/JS in PROGMEM
  - Strict input validation (cmd: `A-Z0-9:`, param: `0-9.-,`)
- New public method: `sendRawSetting(cmd)` — send raw setting command with ACK
- New public method: `lastErrorName()` — error code as string
- New public method: `lastResponseLen()` — length of last response
- New public method: `hasBoot()` — check DSP bootstrap presence
- New getter: `WarningDecoded::batteryEqualization()` (bit 35)
- New compile-time constants: `VC_FLAG_*`
- New runtime config constructor: `VoltronicConfig(baud, timeout, retries)`
- New CRC helper: `voltronicCRCUpdate()`
- New examples: `19_WebConsole`, `20_WebConsoleESP32`
- `QFLAGData::raw` now filled by `parseQFLAG`
- `WarningDecoded` now supports full 36 bits
- `parseQPIWS` signature: `uint32_t&` → `uint64_t&`
- `validBits()` now updated by all string query methods

### Fixed
- **CRITICAL**: `isAck()` and `isNak()` now use `payloadStart()` to skip leading `(`
- **CRITICAL**: `buildFrame()` buffer check now uses `VOLTRONIC_CMD_BUF_SIZE + 4`
- **HIGH**: `_lastError` now reset to `ERR_NONE` on every successful return
- **HIGH**: `QFLAGData::alarmOnPrimaryInterrupt()` now reads bit 8 (was bit 7)
- **MEDIUM**: `parseQFLAG()` now correctly maps `'y'` → bit 8
- **MEDIUM**: `setMaxDischargingCurrent()` buffer enlarged from 16 → 20 bytes

### Changed
- `VOLTRONIC_CMD_BUF_SIZE` default raised from 16 → 32
- `VoltronicTransport::readUntilCR()` uses `cfg.yieldDuringRead` at runtime
- `VoltronicCRC.h` no longer includes `<Arduino.h>`
- `filterPrintable` now applied after CRC check
- `VoltronicConfig` — explicit default constructor + convenience constructor
- `VoltronicCommands.h` reorganized with detailed comments
- `keywords.txt` reorganized
- `README.md` rewritten with Web Console section
- `library.properties` version bumped to 1.2.0

### Removed
- `parseQPIWS` no longer truncates at 32 bits
- `WarningDecoded::raw` type: `uint32_t` → `uint64_t`

### Security
- Web Console input validation prevents command injection
- Param and cmd character whitelists
- Length limits: cmd ≤ 20, param ≤ 20, full ≤ 32

---

## [1.1.2] - 2026-09-25

### Fixed
- Minor documentation fixes
- `keywords.txt` cleanup

---

## [1.1.1] - 2026-09-23

### Fixed
- Parser: single-digit fraction normalization in QPIGS/QPIGS2/QPIRI
- Stream compatibility: removed `_serial.begin()` on abstract Stream
- Removed invalid bit-35 accessor (WarningDecoded)
- Removed unused `readFloatFixed` declaration

---

## [1.1.0] - 2026-09-22

### Added
- `test/test_native`: unit test suite (54 tests)
- `test/test_native/Arduino.h`: native mock
- Full command coverage: 25 inquiry + 30 setting commands
- Parsers: `QPGSn`, `QBEQI`, `QLED`, `QDI`, `QBATCD`, `QMCHGCR`, `QMUCHGCR`, `QOPPT`, `QCHPT`, `QBOOT`
- Data structs: `ParallelInfo`, `BatteryEqualizationInfo`, `LedInfo`, `DefaultsInfo`, `BatteryControlStatus`, `SelectableValues`, `TimeOrderInfo`
- Unit tests (CRC, parsers, frame building)
- 18 examples covering every command
- `docs/COMMANDS.md`, `docs/PROTOCOL.md`, `docs/CRC.md`, `docs/MEMORY.md`
- CI workflows for ESP32/ESP8266/AVR + native tests

### Changed
- Zero dynamic allocation — no `String` anywhere
- Compact numeric storage (x10 / x100)
- `< 1 KB RAM` for all data

---

## [1.0.0] - 2026-09-22

### Added
- Initial release by Kitronic
- Core queries: QPIGS, QPIGS2, QPIRI, QMOD, QPIWS, QFLAG
- Basic settings: PBCV, PBDV, PCP, POP, PGR, PBT, PBFT, PSDV, PCVV
- CRC-16/XMODEM with escape quirk
- Examples for basic usage
- `VoltronicTransport` wrapper for `Stream`
- `VoltronicParser` for response parsing
- `VoltronicCommands.h` command constants
- `VoltronicConfig` runtime configuration

---

## Migration Notes

### From 1.2.0 to 1.3.0

**No breaking changes.** All existing public APIs remain compatible.

**New optional classes:**
- `VoltronicBattery`    → `inverter.battery`
- `VoltronicSmartCharger` → `inverter.smartCharger`
- `VoltronicPowerMode`  → `inverter.powerMode`
- `VoltronicStorage`    → `inverter.storage`
- `VoltronicLang`       → `inverter.lang`

If you include `VoltronicMAX.h`, all are automatically available.
No additional `#include` needed.

**Behavior changes:**
- `VoltronicPowerMode::update()` — now uses user-configurable thresholds
  - Defaults: Emergency 10%, Power Saving 25%, Recover 30%, Surplus 88%, GridMin 150V
- `VoltronicSmartCharger::update()` — reads QPIRI only once (was every update)
- Web UI now has 7 cards instead of 4

**EEPROM compatibility:**
- Existing v1.2.0 sketches work without change (no EEPROM used in 1.2.0)
- New v1.3.0 sketches will use EEPROM via `VoltronicStorage`
- If you don't call `storage.begin()` manually, it's done in `inverter.begin()`

**MQTT:**
- New entities added (opt-in via commented TODO blocks in `mqtt_manager.cpp`)
- All MQTT entity names remain the same

### From 1.1.x to 1.2.0

**Binary compatibility**: `WarningDecoded` size changed from 4 to 8 bytes.
Recompile your sketch after update.

**Source compatibility**: All public APIs remain unchanged.
Only `parseQPIWS` signature changed (internal use):

```cpp
// Old (1.1.x)
static bool parseQPIWS(const char* payload, uint32_t& warningsOut);

// New (1.2.0)
static bool parseQPIWS(const char* payload, uint64_t& warningsOut);
```

**New features are opt-in**: Web Console requires `#define VOLTRONIC_USE_WEB` before
`#include <VoltronicMAX.h>`. Existing code works without any changes.

**Behavior fix**: `isAck()` and `isNak()` now work correctly. If you had code that
bypassed them due to the bug, you can now rely on them directly.

---

## Semantic Versioning Guide

- **MAJOR** — incompatible API changes
- **MINOR** — backward-compatible feature additions
- **PATCH** — backward-compatible bug fixes

---

## Links

- [GitHub Repository](https://github.com/kitronic/esp-lib-voltronic-max.git)
- [Issue Tracker](https://github.com/kitronic/esp-lib-voltronic-max/issues)
- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)


---

## 📊 ملخص الإضافات

### القسم الجديد `[1.3.0]`:

| القسم | العناصر |
|---|---|
| **Added** | 6 أقسام فرعية (Battery, SmartCharger, PowerMode, Storage, Lang, DeviceStatus) + Web + MQTT |
| **Fixed** | 4 إصلاحات رئيسية |
| **Changed** | 4 تغييرات |
| **Documentation** | محدّث |

### Migration Notes المحدّث:

| القسم | التغيير |
|---|---|
| **From 1.2.0 to 1.3.0** | ✅ جديد |
| **From 1.1.x to 1.2.0** | ✅ كما هو |

---

## ✅ التحقق

- [ ] قسم `[1.3.0]` في الأعلى
- [ ] التاريخ `2026-09-28`
- [ ] Migration Notes يشمل 1.2.0 → 1.3.0
- [ ] لا يوجد تكرار في الإدخالات

**بعد ما تأكدت، قل لي ونروح للملف التالي `library.properties`.** 🎯