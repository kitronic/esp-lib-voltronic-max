# Changelog

All notable changes to this project will be documented in this file.



// CHANGELOG.md
## [1.1.1] - 2026-09-23
### Fixed
- Parser: single-digit fraction normalization in QPIGS/QPIGS2/QPIRI
  (e.g. "230.1" now parses as 230.1, not 230.0)
- Stream compatibility: removed `_serial.begin()` on abstract Stream
- Removed invalid bit-35 accessor (WarningDecoded)
- Removed unused readFloatFixed declaration


## [1.1.0] - 2026-09-22

### Added
- test/test_native: unit test suite (54 tests)
- test/test_native/Arduino.h: native mock for host builds
- Full command coverage: 25 inquiry + 30 setting commands
- Parsers: QPGSn, QBEQI, QLED, QDI, QBATCD, QMCHGCR, QMUCHGCR, QOPPT, QCHPT, QBOOT
- Data structs: ParallelInfo, BatteryEqualizationInfo, LedInfo, DefaultsInfo,
  BatteryControlStatus, SelectableValues, TimeOrderInfo
- Unit tests (CRC, parsers, frame building)
- 18 examples covering every command
- `docs/COMMANDS.md`, `docs/PROTOCOL.md`, `docs/CRC.md`, `docs/MEMORY.md`
- CI workflows for ESP32/ESP8266/AVR + native tests

### Changed
- Zero dynamic allocation — no `String` anywhere
- Compact numeric storage (x10 / x100)
- `< 1 KB RAM` for all data

## [1.0.0] - 2026-09-22

### Added
- Initial release by Kitronic
- Core: QPIGS, QPIGS2, QPIRI, QMOD, QPIWS, QFLAG
- Basic settings
- CRC-16/XMODEM
- Examples for basic usage