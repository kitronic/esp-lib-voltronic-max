#pragma once
#include <Arduino.h>
#include "VoltronicConfig.h"
#include "VoltronicCommands.h"
#include "VoltronicCRC.h"
#include "VoltronicTypes.h"
#include "VoltronicParser.h"
#include "VoltronicTransport.h"

class VoltronicMAX {
public:
  enum Error : uint8_t {
    ERR_NONE     = 0,
    ERR_TIMEOUT  = 1,
    ERR_SHORT    = 2,
    ERR_CRC      = 3,
    ERR_NAK      = 4,
    ERR_BAD_RESP = 5,
    ERR_PARSE    = 6,
    ERR_BUSY     = 7,
    ERR_TOO_LONG = 8,
  };

  explicit VoltronicMAX(Stream& serial);

  // ═══════════════════════════════════════════════════════════
  //  Init
  // ═══════════════════════════════════════════════════════════
  bool begin(const VoltronicConfig& cfg);
  bool begin(uint32_t baud = VOLTRONIC_DEFAULT_BAUD);
  void setTimeout(uint16_t ms);
  void setRetries(uint8_t r);

  // ═══════════════════════════════════════════════════════════
  //  Raw
  // ═══════════════════════════════════════════════════════════
  bool sendRaw(const char* cmd);

  // ═══════════════════════════════════════════════════════════
  //  INQUIRY COMMANDS (25)
  // ═══════════════════════════════════════════════════════════
  bool queryProtocolID(char* out, size_t len);
  bool querySerialNumber(char* out, size_t len);
  bool querySerialNumberLong(char* out, size_t len);
  bool queryFirmware(char* out, size_t len);
  bool queryFirmware2(char* out, size_t len);
  bool queryBluetoothVersion(char* out, size_t len);
  bool queryModelName(char* out, size_t len);
  bool queryGeneralModelName(char* out, size_t len);
  bool queryBoot(bool& hasBootstrap);
  bool queryRating();
  bool queryGeneralStatus();
  bool queryGeneralStatus2();
  bool queryMode();
  bool queryWarnings();
  bool queryFlags();
  bool queryDefaults();
  bool queryParallel(uint8_t n);
  bool queryBatteryEqualization();
  bool queryLed();
  bool queryBatteryControl();
  bool queryTime(char* out, size_t len);
  bool queryMaxChargingCurrents();
  bool queryMaxUtilityChargingCurrents();
  bool queryOutputPriorityTimeOrder();
  bool queryChargerPriorityTimeOrder();

  // ═══════════════════════════════════════════════════════════
  //  SETTING COMMANDS (30)
  // ═══════════════════════════════════════════════════════════
  bool setFlag(char flag);
  bool clearFlag(char flag);
  bool resetDefaults();
  bool setMaxChargingCurrent(uint16_t amps);
  bool setMaxUtilityChargingCurrent(uint16_t amps);
  bool setMaxDischargingCurrent(uint16_t amps);
  bool setOutputVoltage(uint16_t volts);
  bool setOutputFrequency(uint8_t hz);
  bool setOutputSourcePriority(uint8_t p);
  bool setOutputMode(uint8_t mode);
  bool setBatteryRechargeVoltage(float v);
  bool setBatteryRedischargeVoltage(float v);
  bool setBatteryCutoffVoltage(float v);
  bool setBatteryCvVoltage(float v);
  bool setBatteryFloatVoltage(float v);
  bool setChargerSourcePriority(uint8_t p);
  bool setGridWorkingRange(uint8_t r);
  bool setBatteryType(uint8_t t);
  bool setParallelChargerPriority(uint8_t m, uint8_t p);
  bool resetEnergy();
  bool eraseLog();
  bool setBatteryEqualizationEnabled(bool en);
  bool setBatteryEqualizationTime(uint16_t min);
  bool setBatteryEqualizationPeriod(uint16_t days);
  bool setBatteryEqualizationVoltage(float v);
  bool setBatteryEqualizationOverTime(uint16_t min);
  bool activateBatteryEqualization(bool active);
  bool setMaxCvChargingTime(uint16_t min);
  bool setDateTime(const char* yymmddhhmmss);
  bool setBatteryControl(uint8_t a, uint8_t b, uint8_t c);

  // ═══════════════════════════════════════════════════════════
  //  DATA ACCESS
  // ═══════════════════════════════════════════════════════════
  const QPIGSData&               qpigs()        const { return _qpigs; }
  const QPIGS2Data&              qpigs2()       const { return _qpigs2; }
  const QPIRIData&               qpiri()        const { return _qpiri; }
  const QFLAGData&               qflag()        const { return _qflag; }
  const ParallelInfo&            parallel()     const { return _parallel; }
  const BatteryEqualizationInfo& equalization() const { return _beqi; }
  const LedInfo&                 led()          const { return _led; }
  const DefaultsInfo&            defaults()     const { return _defaults; }
  const BatteryControlStatus&    batteryCtrl()  const { return _batcd; }
  const SelectableValues&        maxChgOptions()const { return _maxChg; }
  const SelectableValues&        maxUtilChgOptions() const { return _maxUtilChg; }
  const TimeOrderInfo&           outputTimeOrder()  const { return _outputTO; }
  const TimeOrderInfo&           chargerTimeOrder() const { return _chargerTO; }

  WarningDecoded  warnings()  const { WarningDecoded w; w.raw = _warningsRaw; return w; }
  char            mode()      const { return _mode; }
  const char*     modeString()const { return VoltronicParser::modeToString(_mode); }
  uint32_t        validBits() const { return _validBits; }
  bool            hasBoot()   const { return _hasBoot; }

  // ═══════════════════════════════════════════════════════════
  //  NON-BLOCKING POLLING
  // ═══════════════════════════════════════════════════════════
  void startPolling(const VoltronicPollSchedule& schedule);
  void startPolling();
  void stopPolling();

  // ادعها من loop() — لا تحجب
  // ترجع true إذا خلصت دورة كاملة
  bool poll();

  bool     pollingCycleDone() const { return _pollCycleDone; }
  uint32_t pollingCycles()    const { return _pollCycles; }

  // ─── Status ───
  Error       lastError()    const { return _lastError; }
  const char* lastResponse() const { return _respBuf; }
  const char* lastCommand()  const { return _cmdBuf; }

private:
  VoltronicTransport _transport;
  VoltronicConfig    _localCfg;

  char     _respBuf[VOLTRONIC_RESP_BUF_SIZE];
  size_t   _respLen = 0;
  char     _cmdBuf[VOLTRONIC_CMD_BUF_SIZE];

  // Data structs
  QPIGSData               _qpigs;
  QPIGS2Data              _qpigs2;
  QPIRIData               _qpiri;
  QFLAGData               _qflag;
  ParallelInfo            _parallel;
  BatteryEqualizationInfo _beqi;
  LedInfo                 _led;
  DefaultsInfo            _defaults;
  BatteryControlStatus    _batcd;
  SelectableValues        _maxChg;
  SelectableValues        _maxUtilChg;
  TimeOrderInfo           _outputTO;
  TimeOrderInfo           _chargerTO;
  uint32_t                _warningsRaw = 0;
  char                    _mode = ' ';
  bool                    _hasBoot = false;
  uint32_t                _validBits = 0;

  Error _lastError = ERR_NONE;

  // ─── Poll state ───
  VoltronicPollSchedule _pollSchedule;
  bool     _pollEnabled   = false;
  uint8_t  _pollIndex     = 0;
  uint32_t _pollLastFire[9] = {0};
  uint32_t _pollCycles    = 0;
  bool     _pollCycleDone = false;

  // ─── Helpers ───
  bool transact(const char* cmd, bool expectAck);
  bool buildFrame(const char* cmd, uint8_t* out, size_t& outLen);
  bool checkCRC();
  bool isAck() const;
  bool isNak() const;
  const char* payloadStart() const;

  // ─── Poll helpers ───
  bool     execPollQuery(uint8_t idx);
  uint16_t getPollInterval(uint8_t idx) const;
};