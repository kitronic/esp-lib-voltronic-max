#include "VoltronicStorage.h"
#include <EEPROM.h>
#include <string.h>
#include "VoltronicLog.h"
#include "VoltronicMAX.h"

// ═══════════════════════════════════════════════════════════════
//  Platform-specific EEPROM
// ═══════════════════════════════════════════════════════════════
#if defined(ESP8266) || defined(ESP32)
#  define VSTORAGE_EEPROM_BEGIN(size) EEPROM.begin(size)
#  define VSTORAGE_EEPROM_COMMIT() EEPROM.commit()
#else
#  define VSTORAGE_EEPROM_BEGIN(size) EEPROM.begin()
#  define VSTORAGE_EEPROM_COMMIT() (true)
#endif

// ═══════════════════════════════════════════════════════════════
VoltronicStorage::VoltronicStorage()
{
  memset(&_data, 0, sizeof(_data));
}

// ─── CRC16 (XMODEM) ───
uint16_t VoltronicStorage::_calcCRC(const Data& d)
{
  const uint8_t* p = (const uint8_t*)&d;
  size_t len = sizeof(Data) - sizeof(d.crc) - sizeof(d._padEnd);
  uint16_t crc = 0;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= (uint16_t)p[i] << 8;
    for (uint8_t b = 0; b < 8; b++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
  }
  return crc;
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::begin(uint16_t eepromSize)
{
  _size = eepromSize;
  VSTORAGE_EEPROM_BEGIN(_size);
  VLOG("[Storage] EEPROM ready (v%u)", (unsigned)VERSION);
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::load(VoltronicMAX& inv)
{
  EEPROM.get(0, _data);

  if (_data.magic != MAGIC || _data.version != VERSION)
  {
    VLOG("[Storage] No valid data (magic=0x%08X ver=%u) - using defaults",
         (unsigned)_data.magic, (unsigned)_data.version);
    reset();
    return;
  }

  // تحقق CRC
  uint16_t expected = _data.crc;
  _data.crc = 0;
  uint16_t actual = _calcCRC(_data);
  _data.crc = expected;

  if (expected != actual)
  {
    VLOG("[Storage] CRC error (expected=0x%04X actual=0x%04X) - using defaults",
         expected, actual);
    reset();
    return;
  }

  _applyTo(inv);
  _loaded = true;
  VLOG("[Storage] Loaded OK");
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::save(const VoltronicMAX& inv)
{
  _captureFrom(inv);
  _data.magic = MAGIC;
  _data.version = VERSION;
  _data.crc = 0;
  _data.crc = _calcCRC(_data);

  EEPROM.put(0, _data);
  bool ok = VSTORAGE_EEPROM_COMMIT();

  if (ok)
  {
    _saves++;
    _dirty = false;
    VLOG("[Storage] Saved OK (#%u)", (unsigned)_saves);
  }
  else
  {
    VLOG("[Storage] Save FAILED");
  }
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::reset()
{
  memset(&_data, 0, sizeof(_data));
  _data.magic = MAGIC;
  _data.version = VERSION;

  // Battery defaults
  _data.batteryType = 0;  // User
  _data.batteryCapacityAh = 100.0f;
  _data.batteryVoltageEmpty = 42.0f;
  _data.batteryVoltageFull = 54.0f;
  _data.batterySoh = 100.0f;
  _data.batteryCycleCount = 0;

  // Smart Charger defaults
  _data.scMode = 1;  // STANDARD
  _data.scTargetAC = 40;
  _data.scTargetTotal = 80;
  _data.scFloatAC = 2;
  _data.scFloatTotal = 10;
  _data.scTempProtectC = 70;

  // ⚡ VERSION 2 defaults
  _data.sysResistance_mOhm = 15.0f;   // 0.015 Ohm (نفس قيمتك القديمة)
  _data.voltageEmaAlpha = 25.0f;      // 0.25 EMA

  // Power Mode defaults
  _data.pmLocked = 0;
  _data._pad1 = 0;  // ENGLISH
  _data.pmSocEmergency = 10;
  _data.pmSocPowerSaving = 25;
  _data.pmSocRecover = 30;
  _data.pmSocSurplus = 88;
  _data.pmGridMinVoltage = 150;

  _loaded = false;
  _dirty = false;
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::_applyTo(VoltronicMAX& inv)
{
  // ═══ Battery ═══
  inv.battery.setType((VoltronicBatteryType)_data.batteryType);
  inv.battery.setCapacity(_data.batteryCapacityAh);
  inv.battery.setVoltageEmpty(_data.batteryVoltageEmpty);
  inv.battery.setVoltageFull(_data.batteryVoltageFull);
  inv.battery.setInitialSoh(_data.batterySoh);
  inv.battery.setCycleCount(_data.batteryCycleCount);

  // ⚡ VERSION 2: IR compensation + EMA
  inv.battery.setSystemResistance(_data.sysResistance_mOhm / 1000.0f);
  inv.battery.setVoltageSmoothing(_data.voltageEmaAlpha / 100.0f);

  // ═══ Smart Charger ═══
  inv.smartCharger.setMode((VoltronicSmartCharger::Mode)_data.scMode);
  inv.smartCharger.setTargetAC(_data.scTargetAC);
  inv.smartCharger.setTargetTotal(_data.scTargetTotal);
  inv.smartCharger.setFloatAC(_data.scFloatAC);
  inv.smartCharger.setFloatTotal(_data.scFloatTotal);
  inv.smartCharger.setTempProtectC(_data.scTempProtectC);

  // ═══ Power Mode ═══
  inv.powerMode.setEmergencyLocked(_data.pmLocked != 0);
  inv.lang.setLanguage((VoltronicLang::Language)(_data._pad1 > 1 ? 0 : _data._pad1));

  if (_data.pmSocEmergency >= 5 && _data.pmSocEmergency <= 40)
    inv.powerMode.setSocEmergency((float)_data.pmSocEmergency);
  if (_data.pmSocPowerSaving >= 10 && _data.pmSocPowerSaving <= 50)
    inv.powerMode.setSocPowerSaving((float)_data.pmSocPowerSaving);
  if (_data.pmSocRecover >= 20 && _data.pmSocRecover <= 60)
    inv.powerMode.setSocRecover((float)_data.pmSocRecover);
  if (_data.pmSocSurplus >= 70 && _data.pmSocSurplus <= 99)
    inv.powerMode.setSocSurplus((float)_data.pmSocSurplus);
  if (_data.pmGridMinVoltage >= 100 && _data.pmGridMinVoltage <= 220)
    inv.powerMode.setGridMinVoltage((float)_data.pmGridMinVoltage);

  VLOG("[Storage] Battery: type=%u cap=%.0fAh vE=%.1f vF=%.1f R=%.1fmOhm EMA=%.0f%%",
       _data.batteryType,
       _data.batteryCapacityAh,
       _data.batteryVoltageEmpty,
       _data.batteryVoltageFull,
       _data.sysResistance_mOhm,
       _data.voltageEmaAlpha);
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::_captureFrom(const VoltronicMAX& inv)
{
  // ═══ Battery ═══
  _data.batteryType = (uint8_t)inv.battery.type();
  _data.batteryCapacityAh = inv.battery.capacityAh();
  _data.batteryVoltageEmpty = inv.battery.voltageEmpty();
  _data.batteryVoltageFull = inv.battery.voltageFull();
  _data.batterySoh = inv.battery.soh();
  _data.batteryCycleCount = inv.battery.cycleCount();

  // ⚡ VERSION 2
  _data.sysResistance_mOhm = inv.battery.systemResistance() * 1000.0f;
  _data.voltageEmaAlpha = inv.battery.voltageSmoothing() * 100.0f;

  // ═══ Smart Charger ═══
  _data.scMode = (uint8_t)inv.smartCharger.mode();
  _data.scTargetAC = inv.smartCharger.targetAC();
  _data.scTargetTotal = inv.smartCharger.targetTotal();
  _data.scFloatAC = inv.smartCharger.floatAC();
  _data.scFloatTotal = inv.smartCharger.floatTotal();
  _data.scTempProtectC = inv.smartCharger.tempProtectC();

  // ═══ Power Mode ═══
  _data.pmLocked = inv.powerMode.emergencyLocked() ? 1 : 0;
  _data._pad1 = (uint8_t)inv.lang.language();
  _data.pmSocEmergency = (uint8_t)inv.powerMode.socEmergency();
  _data.pmSocPowerSaving = (uint8_t)inv.powerMode.socPowerSaving();
  _data.pmSocRecover = (uint8_t)inv.powerMode.socRecover();
  _data.pmSocSurplus = (uint8_t)inv.powerMode.socSurplus();
  _data.pmGridMinVoltage = (uint16_t)inv.powerMode.gridMinVoltage();
}

// ═══════════════════════════════════════════════════════════════
void VoltronicStorage::tick(VoltronicMAX& inv)
{
  if (!_dirty) return;
  if (millis() - _lastChange < 5000) return;
  save(inv);
}