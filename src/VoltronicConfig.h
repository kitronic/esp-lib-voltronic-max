#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  Compile-time configuration
//  كل القيم قابلة للتجاوز من السكتش قبل الـ include
// ═══════════════════════════════════════════════════════════════

// ─── Buffer sizes ───
#ifndef VOLTRONIC_RESP_BUF_SIZE
#define VOLTRONIC_RESP_BUF_SIZE 160 // أطول رد PI30 ~ 120 بايت
#endif

#ifndef VOLTRONIC_CMD_BUF_SIZE
#define VOLTRONIC_CMD_BUF_SIZE 32 // DAT يحتاج 17، مع هامش آمن
#endif

// ─── Retries ───
#ifndef VOLTRONIC_MAX_RETRIES
#define VOLTRONIC_MAX_RETRIES 2
#endif

// ─── Serial defaults ───
#ifndef VOLTRONIC_DEFAULT_BAUD
#define VOLTRONIC_DEFAULT_BAUD 2400
#endif

#ifndef VOLTRONIC_DEFAULT_TIMEOUT_MS
#define VOLTRONIC_DEFAULT_TIMEOUT_MS 800
#endif

// ─── Behavior flags (compile-time defaults) ───
#ifndef VOLTRONIC_CRC_ESCAPE
#define VOLTRONIC_CRC_ESCAPE 1
#endif

#ifndef VOLTRONIC_FILTER_PRINTABLE
#define VOLTRONIC_FILTER_PRINTABLE 1
#endif

#ifndef VOLTRONIC_YIELD_IN_READ
#define VOLTRONIC_YIELD_IN_READ 1
#endif

// ═══════════════════════════════════════════════════════════════
//  Runtime configuration
//
//  ⚠️ ملاحظة مهمة: المكتبة لا تهيّئ Serial تلقائياً لأن
//  Stream مجردة (abstract) وما فيها begin().
//  المستخدم لازم:
//    SoftwareSerial invSerial(rx, tx);
//    invSerial.begin(cfg.baud);
//    inverter.begin(cfg);
//  أو على ESP32:
//    Serial2.begin(cfg.baud, SERIAL_8N1, rx, tx);
//    inverter.begin(cfg);
// ═══════════════════════════════════════════════════════════════
struct VoltronicConfig
{
  // ─── UART ───
  uint32_t baud = VOLTRONIC_DEFAULT_BAUD;

  // ─── Transaction ───
  uint16_t responseTimeoutMs = VOLTRONIC_DEFAULT_TIMEOUT_MS;
  uint8_t retries = VOLTRONIC_MAX_RETRIES;

  // ─── Pins (informational only — لا تُستخدم للتهيئة) ───
  int8_t rxPin = -1;
  int8_t txPin = -1;

  // ─── Behavior ───
  bool applyCrcEscape = (VOLTRONIC_CRC_ESCAPE != 0);
  bool filterPrintable = (VOLTRONIC_FILTER_PRINTABLE != 0);
  bool yieldDuringRead = (VOLTRONIC_YIELD_IN_READ != 0);
  bool clearBufferBeforeSend = true;

  // ─── منشئ للقيم المخصصة ───
  VoltronicConfig() = default;

  VoltronicConfig(uint32_t baudRate, uint16_t timeoutMs, uint8_t retryCount)
      : baud(baudRate), responseTimeoutMs(timeoutMs), retries(retryCount)
  {
  }
};

// ═══════════════════════════════════════════════════════════════
//  Poll schedule — non-blocking polling
//  القيم بالميلي ثانية. 0 = معطّل
// ═══════════════════════════════════════════════════════════════
struct VoltronicPollSchedule
{
  uint16_t qpigsMs = 2000;  // QPIGS
  uint16_t qpigs2Ms = 2000; // QPIGS2
  uint16_t qpiriMs = 60000; // QPIRI
  uint16_t qmodMs = 5000;   // QMOD
  uint16_t qpiwsMs = 30000; // QPIWS
  uint16_t qflagMs = 0;     // QFLAG  (معطّل افتراضياً)
  uint16_t qidMs = 0;       // QID    (معطّل افتراضياً)
  uint16_t qbeqiMs = 0;     // QBEQI  (معطّل افتراضياً)
  uint16_t qbatcdMs = 0;    // QBATCD (معطّل افتراضياً)
};