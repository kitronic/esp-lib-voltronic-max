#pragma once
#include <Arduino.h>

// ===== QPIGS =====
struct InverterData {
  // Grid
  float    gridVoltage       = 0;    // V
  float    gridFrequency     = 0;    // Hz

  // AC Output
  float    acOutputVoltage   = 0;    // V
  float    acOutputFrequency = 0;    // Hz
  uint32_t acOutputApparentPower = 0; // VA
  uint32_t acOutputActivePower   = 0; // W
  uint8_t  loadPercent       = 0;    // %

  // DC Bus
  uint16_t busVoltage        = 0;    // V

  // Battery
  float    batteryVoltage    = 0;    // V
  uint16_t batteryChargingCurrent = 0; // A
  uint8_t  batteryCapacity   = 0;    // %
  uint16_t batteryDischargeCurrent = 0; // A
  uint16_t batteryVoltageOffsetForFans = 0; // 10mV
  uint8_t  eepromVersion     = 0;

  // Thermal
  uint16_t inverterHeatSinkTemperature = 0; // °C

  // PV
  float    pv1InputCurrent   = 0;    // A
  float    pv1InputVoltage   = 0;    // V
  uint16_t pv1ChargingPower  = 0;    // W
  float    pv2InputCurrent   = 0;    // A (QPIGS2)
  float    pv2InputVoltage   = 0;    // V (QPIGS2)
  uint16_t pv2ChargingPower  = 0;    // W (QPIGS2)

  // SCC
  float    sccBatteryVoltage = 0;    // V

  // Device status (8 bits from QPIGS)
  uint8_t  deviceStatusByte  = 0;
  uint8_t  deviceStatusByte2 = 0;    // 3 bits from QPIGS2

  // Grid feed
  uint8_t  solarFeedToGridStatus = 0;
  uint16_t solarFeedToGridPower  = 0; // W
  uint8_t  countryRegulation     = 0;

  // Mode
  char     mode              = ' ';

  // Warnings
  uint32_t warnings          = 0;

  bool     qpigsValid  = false;
  bool     qpigs2Valid = false;
  bool     qmodValid   = false;
  bool     qpiwsValid  = false;

  bool anyValid() const {
    return qpigsValid || qpigs2Valid || qmodValid || qpiwsValid;
  }
};

// ===== QPIRI =====
struct RatingInfo {
  float    gridRatingVoltage      = 0; // V
  float    gridRatingCurrent      = 0; // A
  float    acOutputRatingVoltage  = 0; // V
  float    acOutputRatingFrequency= 0; // Hz
  float    acOutputRatingCurrent  = 0; // A
  uint32_t acOutputRatingApparentPower = 0; // VA
  uint32_t acOutputRatingActivePower   = 0; // W
  float    batteryRatingVoltage   = 0; // V
  float    batteryRechargeVoltage = 0; // V
  float    batteryUnderVoltage    = 0; // V
  float    batteryBulkVoltage     = 0; // V
  float    batteryFloatVoltage    = 0; // V
  uint8_t  batteryType            = 0;
  uint8_t  maxACChargingCurrent   = 0; // A
  uint8_t  maxChargingCurrent     = 0; // A
  uint8_t  inputVoltageRange      = 0;
  uint8_t  outputSourcePriority   = 0;
  uint8_t  chargerSourcePriority  = 0;
  uint8_t  parallelMaxNum         = 0;
  uint8_t  machineType            = 0;
  uint8_t  outputMode             = 0;
  uint8_t  batteryRedischargeVoltage = 0;
  uint8_t  pvOKConditionForParallel = 0;
  uint8_t  pvPowerBalance         = 0;
  uint8_t  gridFeedEnabled        = 0;
  bool     valid = false;
};

// ===== QPGSn =====
struct ParallelInfo {
  uint8_t  parallelNumber   = 0;
  bool     exists           = false;
  char     serialNumber[16] = {0};
  char     workMode         = ' ';
  uint8_t  faultCode        = 0;
  float    gridVoltage      = 0;
  float    gridFrequency    = 0;
  float    acOutputVoltage  = 0;
  float    acOutputFrequency= 0;
  uint32_t acOutputApparentPower = 0;
  uint32_t acOutputActivePower   = 0;
  uint8_t  loadPercent      = 0;
  float    batteryVoltage   = 0;
  uint16_t batteryChargingCurrent = 0;
  uint8_t  batteryCapacity  = 0;
  float    pvInputVoltage   = 0;
  float    pvInputCurrent   = 0;
  uint16_t totalChargingCurrent = 0;
  uint32_t totalACOutputApparentPower = 0;
  uint32_t totalOutputActivePower = 0;
  uint8_t  totalACOutputPercentage = 0;
  uint8_t  inverterStatus   = 0;
  uint8_t  outputMode       = 0;
  uint8_t  chargerSourcePriority = 0;
  uint8_t  maxChargerCurrent = 0;
  uint8_t  maxChargerRange  = 0;
  uint8_t  maxACChargerCurrent = 0;
  float    pvInputCurrent2  = 0;
  bool     valid = false;
};

// ===== QBEQI =====
struct BatteryEqualizationInfo {
  bool     enabled          = false;
  uint16_t timeMinutes      = 0;
  uint16_t periodDays       = 0;
  float    voltage          = 0;
  uint16_t overTimeMinutes  = 0;
  uint16_t maxTimeMinutes   = 0;
  uint8_t  activeNow        = 0;
  bool     valid = false;
};

// ===== QLED =====
struct LedInfo {
  bool     enabled          = false;
  uint8_t  speed            = 0;
  uint8_t  effect           = 0;
  uint8_t  brightness       = 0;
  uint8_t  colorCount       = 0;
  uint8_t  red[3]           = {0};
  uint8_t  green[3]         = {0};
  uint8_t  blue[3]          = {0};
  bool     valid = false;
};

// ===== QDI / QFLAG helpers =====
struct DeviceFlags {
  bool     silenceBuzzer    = false;
  bool     overloadBypass   = false;
  bool     solarFeedToGrid  = false;
  bool     powerSaving      = false;
  bool     lcdEscapeToDefault = false;
  bool     overloadRestart  = false;
  bool     overTempRestart  = false;
  bool     backlightOn      = false;
  bool     alarmOnPrimaryInterrupt = false;
  bool     faultCodeRecord  = false;
  bool     valid = false;
};

// ===== Warning decoded =====
struct WarningDecoded {
  bool pvLoss              = false;
  bool inverterFault       = false;
  bool busOver             = false;
  bool busUnder            = false;
  bool busSoftFail         = false;
  bool lineFail            = false;
  bool opvShort            = false;
  bool inverterVoltageLow  = false;
  bool inverterVoltageHigh = false;
  bool overTemperature     = false;
  bool fanLocked           = false;
  bool batteryVoltageHigh  = false;
  bool batteryLowAlarm     = false;
  bool batteryUnderShutdown= false;
  bool batteryDerating     = false;
  bool overload            = false;
  bool eepromFault         = false;
  bool inverterOverCurrent = false;
  bool inverterSoftFail    = false;
  bool selfTestFail        = false;
  bool opDcVoltageOver     = false;
  bool batOpen             = false;
  bool currentSensorFail   = false;
  bool batteryWeak         = false;
  bool batteryEqualization = false;
  bool any() const {
    return pvLoss || inverterFault || busOver || busUnder || busSoftFail ||
           lineFail || opvShort || inverterVoltageLow || inverterVoltageHigh ||
           overTemperature || fanLocked || batteryVoltageHigh || batteryLowAlarm ||
           batteryUnderShutdown || batteryDerating || overload || eepromFault ||
           inverterOverCurrent || inverterSoftFail || selfTestFail ||
           opDcVoltageOver || batOpen || currentSensorFail || batteryWeak ||
           batteryEqualization;
  }
};