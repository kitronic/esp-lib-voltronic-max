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
  //  INQUIRY COMMANDS (25 commands)
  // ═══════════════════════════════════════════════════════════

  // ─── Device info ───
  bool queryProtocolID(char* out, size_t len);         // QPI
  bool querySerialNumber(char* out, size_t len);       // QID
  bool querySerialNumberLong(char* out, size_t len);   // QSID
  bool queryFirmware(char* out, size_t len);           // QVFW
  bool queryFirmware2(char* out, size_t len);          // QVFW3
  bool queryBluetoothVersion(char* out, size_t len);   // VERFW:
  bool queryModelName(char* out, size_t len);          // QMN
  bool queryGeneralModelName(char* out, size_t len);   // QGMN
  bool queryBoot(bool& hasBootstrap);                  // QBOOT

  // ─── Data queries ───
  bool queryRating();                    // QPIRI
  bool queryGeneralStatus();             // QPIGS
  bool queryGeneralStatus2();            // QPIGS2
  bool queryMode();                      // QMOD
  bool queryWarnings();                  // QPIWS
  bool queryFlags();                     // QFLAG
  bool queryDefaults();                  // QDI
  bool queryParallel(uint8_t n);         // QPGSn
  bool queryBatteryEqualization();       // QBEQI
  bool queryLed();                       // QLED
  bool queryBatteryControl();            // QBATCD
  bool queryTime(char* out, size_t len); // QT

  // ─── Selectable values ───
  bool queryMaxChargingCurrents();       // QMCHGCR
  bool queryMaxUtilityChargingCurrents();// QMUCHGCR

  // ─── Time order tables ───
  bool queryOutputPriorityTimeOrder();   // QOPPT
  bool queryChargerPriorityTimeOrder();  // QCHPT

  // ═══════════════════════════════════════════════════════════
  //  SETTING COMMANDS (30 commands)
  // ═══════════════════════════════════════════════════════════

  // ─── Flag enable/disable ───
  bool setFlag(char flag);               // PEx
  bool clearFlag(char flag);             // PDx
  bool resetDefaults();                  // PF

  // ─── Currents ───
  bool setMaxChargingCurrent(uint16_t amps);        // MNCHGC
  bool setMaxUtilityChargingCurrent(uint16_t amps); // MUCHGC
  bool setMaxDischargingCurrent(uint16_t amps);     // PBATMAXDISC

  // ─── Output ───
  bool setOutputVoltage(uint16_t volts);            // V
  bool setOutputFrequency(uint8_t hz);              // F
  bool setOutputSourcePriority(uint8_t p);          // POP
  bool setOutputMode(uint8_t mode);                 // POPM

  // ─── Battery voltages ───
  bool setBatteryRechargeVoltage(float v);          // PBCV
  bool setBatteryRedischargeVoltage(float v);       // PBDV
  bool setBatteryCutoffVoltage(float v);            // PSDV
  bool setBatteryCvVoltage(float v);                // PCVV
  bool setBatteryFloatVoltage(float v);             // PBFT

  // ─── Charger / grid ───
  bool setChargerSourcePriority(uint8_t p);         // PCP
  bool setGridWorkingRange(uint8_t r);              // PGR
  bool setBatteryType(uint8_t t);                   // PBT
  bool setParallelChargerPriority(uint8_t m, uint8_t p); // PPCP

  // ─── Energy / log ───
  bool resetEnergy();                               // RTEY
  bool eraseLog();                                  // RTDL

  // ─── Battery equalization ───
  bool setBatteryEqualizationEnabled(bool en);      // PBEQE
  bool setBatteryEqualizationTime(uint16_t min);    // PBEQT
  bool setBatteryEqualizationPeriod(uint16_t days); // PBEQP
  bool setBatteryEqualizationVoltage(float v);      // PBEQV
  bool setBatteryEqualizationOverTime(uint16_t min);// PBEQOT
  bool activateBatteryEqualization(bool active);    // PBEQA

  // ─── Misc ───
  bool setMaxCvChargingTime(uint16_t min);          // PCVT
  bool setDateTime(const char* yymmddhhmmss);       // DAT
  bool setBatteryControl(uint8_t a, uint8_t b, uint8_t c); // PBATCD

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

  // Helpers
  bool transact(const char* cmd, bool expectAck);
  bool buildFrame(const char* cmd, uint8_t* out, size_t& outLen);
  bool checkCRC();
  bool isAck() const;
  bool isNak() const;
  const char* payloadStart() const;
};