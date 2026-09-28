#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  Valid bits لكل نوع رد
// ═══════════════════════════════════════════════════════════════
enum VoltronicValidBit : uint32_t
{
  VVB_QPIGS = 0x00000001,
  VVB_QPIGS2 = 0x00000002,
  VVB_QPIRI = 0x00000004,
  VVB_QMOD = 0x00000008,
  VVB_QPIWS = 0x00000010,
  VVB_QFLAG = 0x00000020,
  VVB_QID = 0x00000040,
  VVB_QDI = 0x00000080,
  VVB_QMCHGCR = 0x00000100,
  VVB_QMUCHGCR = 0x00000200,
  VVB_QOPPT = 0x00000400,
  VVB_QCHPT = 0x00000800,
  VVB_QT = 0x00001000,
  VVB_QBEQI = 0x00002000,
  VVB_QMN = 0x00004000,
  VVB_QGMN = 0x00008000,
  VVB_QBOOT = 0x00010000,
  VVB_QBATCD = 0x00020000,
  VVB_QLED = 0x00040000,
  VVB_QPGS = 0x00080000,
  VVB_QVFW = 0x00100000,
  VVB_QSID = 0x00200000,
  VVB_QVFW3 = 0x00400000,
  VVB_VERFW = 0x00800000,
};

// ═══════════════════════════════════════════════════════════════
//  QPIGS — Compact (x10 / x100)
// ═══════════════════════════════════════════════════════════════
struct QPIGSData
{
  uint16_t gridVoltage_x10 = 0;
  uint16_t gridFrequency_x10 = 0;
  uint16_t acOutputVoltage_x10 = 0;
  uint16_t acOutputFrequency_x10 = 0;
  uint16_t acOutputApparentPower = 0;  // VA
  uint16_t acOutputActivePower = 0;    // W
  uint8_t loadPercent = 0;             // %
  uint16_t busVoltage = 0;             // V
  uint16_t batteryVoltage_x100 = 0;
  uint16_t batteryChargingCurrent = 0;  // A
  uint8_t batteryCapacity = 0;          // %
  uint16_t inverterTemperature = 0;     // °C
  uint16_t pv1InputCurrent_x10 = 0;
  uint16_t pv1InputVoltage_x10 = 0;
  uint16_t sccBatteryVoltage_x100 = 0;
  uint16_t batteryDischargeCurrent = 0;  // A
  uint8_t deviceStatus = 0;              // b7..b0
  uint16_t batteryVoltageOffsetForFans = 0;
  uint8_t eepromVersion = 0;
  uint16_t pv1ChargingPower = 0;  // W
  uint8_t deviceStatus2 = 0;      // b10b9b8
  uint8_t solarFeedToGridStatus = 0;
  uint8_t countryRegulation = 0;
  uint16_t solarFeedToGridPower = 0;  // W
  // ═══════════════════════════════════════════════════════════
  //  Device Status Decoded
  //  يفك الـ 8 bits إلى بنية مفهومة
  // ═══════════════════════════════════════════════════════════
  struct DeviceStatusInfo
  {
    bool sbuPriorityVersion;
    bool configurationChanged;
    bool sccFirmwareUpdated;
    bool loadOn;
    bool batterySteady;
    uint8_t chargingCode;  // 0-7
  };

  DeviceStatusInfo decodeDeviceStatus() const
  {
    DeviceStatusInfo info;
    info.sbuPriorityVersion = deviceStatus & 0x80;
    info.configurationChanged = deviceStatus & 0x40;
    info.sccFirmwareUpdated = deviceStatus & 0x20;
    info.loadOn = deviceStatus & 0x10;
    info.batterySteady = deviceStatus & 0x08;
    info.chargingCode = deviceStatus & 0x07;
    return info;
  }
  float gridVoltage() const
  {
    return gridVoltage_x10 / 10.0f;
  }
  float gridFrequency() const
  {
    return gridFrequency_x10 / 10.0f;
  }
  float acOutputVoltage() const
  {
    return acOutputVoltage_x10 / 10.0f;
  }
  float acOutputFrequency() const
  {
    return acOutputFrequency_x10 / 10.0f;
  }
  float batteryVoltage() const
  {
    return batteryVoltage_x100 / 100.0f;
  }
  float pv1InputCurrent() const
  {
    return pv1InputCurrent_x10 / 10.0f;
  }
  float pv1InputVoltage() const
  {
    return pv1InputVoltage_x10 / 10.0f;
  }
  float sccBatteryVoltage() const
  {
    return sccBatteryVoltage_x100 / 100.0f;
  }

  // Status bits (b7..b0)
  bool sbAddSBUPriorityVersion() const
  {
    return deviceStatus & 0x80;
  }
  bool sbConfigurationChanged() const
  {
    return deviceStatus & 0x40;
  }
  bool sbSccFirmwareUpdated() const
  {
    return deviceStatus & 0x20;
  }
  bool sbLoadOn() const
  {
    return deviceStatus & 0x10;
  }
  bool sbBatterySteady() const
  {
    return deviceStatus & 0x08;
  }
  // يرجع 0..7 — charging status code (مو bit واحد)
  uint8_t sbChargingStatus() const
  {
    return deviceStatus & 0x07;
  }

  // Status bits2 (b10b9b8)
  bool sb2ChargingToFloat() const
  {
    return deviceStatus2 & 0x01;
  }
  bool sb2SwitchOn() const
  {
    return deviceStatus2 & 0x02;
  }
  bool sb2DustproofInstalled() const
  {
    return deviceStatus2 & 0x04;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QPIGS2
// ═══════════════════════════════════════════════════════════════
struct QPIGS2Data
{
  uint16_t pv2InputCurrent_x10 = 0;
  uint16_t pv2InputVoltage_x10 = 0;
  uint16_t pv2ChargingPower = 0;

  float pv2InputCurrent() const
  {
    return pv2InputCurrent_x10 / 10.0f;
  }
  float pv2InputVoltage() const
  {
    return pv2InputVoltage_x10 / 10.0f;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QPIRI
// ═══════════════════════════════════════════════════════════════
struct QPIRIData
{
  uint16_t gridRatingVoltage_x10 = 0;
  uint16_t gridRatingCurrent_x10 = 0;
  uint16_t acOutputRatingVoltage_x10 = 0;
  uint16_t acOutputRatingFrequency_x10 = 0;
  uint16_t acOutputRatingCurrent_x10 = 0;
  uint16_t acOutputRatingApparentPower = 0;
  uint16_t acOutputRatingActivePower = 0;
  uint16_t batteryRatingVoltage_x10 = 0;
  uint16_t batteryRechargeVoltage_x10 = 0;
  uint16_t batteryUnderVoltage_x10 = 0;
  uint16_t batteryBulkVoltage_x10 = 0;
  uint16_t batteryFloatVoltage_x10 = 0;
  uint8_t batteryType = 0;
  uint8_t maxACChargingCurrent = 0;
  uint8_t maxChargingCurrent = 0;
  uint8_t inputVoltageRange = 0;
  uint8_t outputSourcePriority = 0;
  uint8_t chargerSourcePriority = 0;
  uint8_t parallelMaxNum = 0;
  uint8_t machineType = 0;
  uint8_t outputMode = 0;
  uint16_t batteryRedischargeVoltage_x10 = 0;
  uint8_t pvOKConditionForParallel = 0;
  uint8_t pvPowerBalance = 0;

  float gridRatingVoltage() const
  {
    return gridRatingVoltage_x10 / 10.0f;
  }
  float gridRatingCurrent() const
  {
    return gridRatingCurrent_x10 / 10.0f;
  }
  float acOutputRatingVoltage() const
  {
    return acOutputRatingVoltage_x10 / 10.0f;
  }
  float acOutputRatingFrequency() const
  {
    return acOutputRatingFrequency_x10 / 10.0f;
  }
  float acOutputRatingCurrent() const
  {
    return acOutputRatingCurrent_x10 / 10.0f;
  }
  float batteryRatingVoltage() const
  {
    return batteryRatingVoltage_x10 / 10.0f;
  }
  float batteryRechargeVoltage() const
  {
    return batteryRechargeVoltage_x10 / 10.0f;
  }
  float batteryUnderVoltage() const
  {
    return batteryUnderVoltage_x10 / 10.0f;
  }
  float batteryBulkVoltage() const
  {
    return batteryBulkVoltage_x10 / 10.0f;
  }
  float batteryFloatVoltage() const
  {
    return batteryFloatVoltage_x10 / 10.0f;
  }
  float batteryRedischargeVoltage() const
  {
    return batteryRedischargeVoltage_x10 / 10.0f;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QFLAG
//  ملاحظة: raw يحتوي النص الأصلي كما جاء من الجهاز (مثال "EaxDby...")
//  parser يملؤه دائماً — استخدمه للـ debug أو لعرض الحالة الكاملة
// ═══════════════════════════════════════════════════════════════
struct QFLAGData
{
  static constexpr uint8_t RAW_MAX = 20;

  uint16_t flags = 0;
  char raw[RAW_MAX] = {0};

  bool buzzerEnabled() const
  {
    return flags & (1u << 0);
  }
  bool overloadBypass() const
  {
    return flags & (1u << 1);
  }
  bool solarFeedToGrid() const
  {
    return flags & (1u << 2);
  }
  bool powerSaving() const
  {
    return flags & (1u << 3);
  }
  bool lcdEscapeToDefault() const
  {
    return flags & (1u << 4);
  }
  bool overloadRestart() const
  {
    return flags & (1u << 5);
  }
  bool overTempRestart() const
  {
    return flags & (1u << 6);
  }
  bool backlightOn() const
  {
    return flags & (1u << 7);
  }
  bool alarmOnPrimaryInterrupt() const
  {
    return flags & (1u << 8);
  }  // ✅ FIXED
  bool faultCodeRecord() const
  {
    return flags & (1u << 9);
  }
};

// ═══════════════════════════════════════════════════════════════
//  Warning decode (QPIWS)
//  البروتوكول يرسل 36 بت (a0..a35) — ندعمها كلها الآن
//  ⚠️ هذا يغيّر حجم WarningDecoded من 4 إلى 8 بايت
// ═══════════════════════════════════════════════════════════════
struct WarningDecoded
{
  uint64_t raw = 0;

  bool pvLoss() const
  {
    return raw & (1ULL << 0);
  }
  bool inverterFault() const
  {
    return raw & (1ULL << 1);
  }
  bool busOver() const
  {
    return raw & (1ULL << 2);
  }
  bool busUnder() const
  {
    return raw & (1ULL << 3);
  }
  bool busSoftFail() const
  {
    return raw & (1ULL << 4);
  }
  bool lineFail() const
  {
    return raw & (1ULL << 5);
  }
  bool opvShort() const
  {
    return raw & (1ULL << 6);
  }
  bool inverterVoltageLow() const
  {
    return raw & (1ULL << 7);
  }
  bool inverterVoltageHigh() const
  {
    return raw & (1ULL << 8);
  }
  bool overTemperature() const
  {
    return raw & (1ULL << 9);
  }
  bool fanLocked() const
  {
    return raw & (1ULL << 10);
  }
  bool batteryVoltageHigh() const
  {
    return raw & (1ULL << 11);
  }
  bool batteryLowAlarm() const
  {
    return raw & (1ULL << 12);
  }
  bool batteryUnderShutdown() const
  {
    return raw & (1ULL << 14);
  }
  bool batteryDerating() const
  {
    return raw & (1ULL << 15);
  }
  bool overload() const
  {
    return raw & (1ULL << 16);
  }
  bool eepromFault() const
  {
    return raw & (1ULL << 17);
  }
  bool inverterOverCurrent() const
  {
    return raw & (1ULL << 18);
  }
  bool inverterSoftFail() const
  {
    return raw & (1ULL << 19);
  }
  bool selfTestFail() const
  {
    return raw & (1ULL << 20);
  }
  bool opDcVoltageOver() const
  {
    return raw & (1ULL << 21);
  }
  bool batOpen() const
  {
    return raw & (1ULL << 22);
  }
  bool currentSensorFail() const
  {
    return raw & (1ULL << 23);
  }
  bool batteryWeak() const
  {
    return raw & (1ULL << 31);
  }
  bool batteryEqualization() const
  {
    return raw & (1ULL << 35);
  }  // ✅ جديد

  bool any() const
  {
    return raw != 0;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QPGSn — Parallel info (compact)
// ═══════════════════════════════════════════════════════════════
struct ParallelInfo
{
  static constexpr uint8_t SERIAL_MAX = 16;

  uint8_t parallelNumber = 0;
  bool exists = false;
  char serialNumber[SERIAL_MAX] = {0};
  char workMode = ' ';
  uint8_t faultCode = 0;
  uint16_t gridVoltage_x10 = 0;
  uint16_t gridFrequency_x10 = 0;
  uint16_t acOutputVoltage_x10 = 0;
  uint16_t acOutputFrequency_x10 = 0;
  uint16_t acOutputApparentPower = 0;
  uint16_t acOutputActivePower = 0;
  uint8_t loadPercent = 0;
  uint16_t batteryVoltage_x10 = 0;
  uint16_t batteryChargingCurrent = 0;
  uint8_t batteryCapacity = 0;
  uint16_t pvInputVoltage_x10 = 0;
  uint16_t pvInputCurrent_x10 = 0;
  uint16_t totalChargingCurrent = 0;
  uint16_t totalACOutputApparentPower = 0;
  uint16_t totalOutputActivePower = 0;
  uint8_t totalACOutputPercentage = 0;
  uint8_t inverterStatus = 0;
  uint8_t outputMode = 0;
  uint8_t chargerSourcePriority = 0;
  uint8_t maxChargerCurrent = 0;
  uint8_t maxChargerRange = 0;
  uint8_t maxACChargerCurrent = 0;
  uint16_t pvInputCurrent2_x10 = 0;

  float gridVoltage() const
  {
    return gridVoltage_x10 / 10.0f;
  }
  float gridFrequency() const
  {
    return gridFrequency_x10 / 10.0f;
  }
  float acOutputVoltage() const
  {
    return acOutputVoltage_x10 / 10.0f;
  }
  float acOutputFrequency() const
  {
    return acOutputFrequency_x10 / 10.0f;
  }
  float batteryVoltage() const
  {
    return batteryVoltage_x10 / 10.0f;
  }
  float pvInputVoltage() const
  {
    return pvInputVoltage_x10 / 10.0f;
  }
  float pvInputCurrent() const
  {
    return pvInputCurrent_x10 / 10.0f;
  }
  float pvInputCurrent2() const
  {
    return pvInputCurrent2_x10 / 10.0f;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QBEQI — Battery equalization
// ═══════════════════════════════════════════════════════════════
struct BatteryEqualizationInfo
{
  bool enabled = false;
  uint16_t timeMinutes = 0;
  uint16_t periodDays = 0;
  uint16_t voltage_x10 = 0;
  uint16_t overTimeMinutes = 0;
  uint16_t maxTimeMinutes = 0;
  uint8_t activeNow = 0;

  float voltage() const
  {
    return voltage_x10 / 10.0f;
  }
};

// ═══════════════════════════════════════════════════════════════
//  QLED — LED status
// ═══════════════════════════════════════════════════════════════
struct LedInfo
{
  static constexpr uint8_t MAX_COLORS = 3;

  bool enabled = false;
  uint8_t speed = 0;       // 0=low, 1=medium, 2=fast
  uint8_t effect = 0;      // 0=breathing, 1=flashing, 2=solid, 3..6=scroll/spark
  uint8_t brightness = 0;  // 1..9
  uint8_t colorCount = 0;  // 1..3
  uint8_t red[MAX_COLORS] = {0};
  uint8_t green[MAX_COLORS] = {0};
  uint8_t blue[MAX_COLORS] = {0};
};

// ═══════════════════════════════════════════════════════════════
//  QDI — Default settings
// ═══════════════════════════════════════════════════════════════
struct DefaultsInfo
{
  uint16_t acOutputVoltage_x10 = 0;
  uint16_t acOutputFrequency_x10 = 0;
  uint8_t maxACChargingCurrent = 0;
  uint16_t batteryUnderVoltage_x10 = 0;
  uint16_t batteryFloatVoltage_x10 = 0;
  uint16_t batteryBulkVoltage_x10 = 0;
  uint16_t batteryRechargeVoltage_x10 = 0;
  uint8_t maxChargingCurrent = 0;
  uint8_t acInputVoltageRange = 0;
  uint8_t outputSourcePriority = 0;
  uint8_t chargerSourcePriority = 0;
  uint8_t batteryType = 0;
};

// ═══════════════════════════════════════════════════════════════
//  QBATCD — Battery control status
// ═══════════════════════════════════════════════════════════════
struct BatteryControlStatus
{
  uint8_t dischargeCompletely = 0;
  uint8_t dischargeAllowed = 0;
  uint8_t chargeCompletely = 0;
};

// ═══════════════════════════════════════════════════════════════
//  QMCHGCR / QMUCHGCR — قوائم قيم قابلة للاختيار
// ═══════════════════════════════════════════════════════════════
struct SelectableValues
{
  static constexpr uint8_t MAX_VALUES = 16;
  uint8_t values[MAX_VALUES] = {0};
  uint8_t count = 0;
};

// ═══════════════════════════════════════════════════════════════
//  QOPPT / QCHPT — جدول زمني 24 ساعة
// ═══════════════════════════════════════════════════════════════
struct TimeOrderInfo
{
  static constexpr uint8_t HOURS = 24;
  uint8_t hours[HOURS] = {0};
  uint8_t defaultVal = 0;
  uint8_t order = 0;
};