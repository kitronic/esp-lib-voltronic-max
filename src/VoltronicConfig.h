#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  Compile-time configuration
//  كل شي قابل للتعديل قبل الـ include
// ═══════════════════════════════════════════════════════════════

#ifndef VOLTRONIC_RESP_BUF_SIZE
  #define VOLTRONIC_RESP_BUF_SIZE  160   // أقصى طول رد (بايت)
#endif

#ifndef VOLTRONIC_CMD_BUF_SIZE
  #define VOLTRONIC_CMD_BUF_SIZE   16    // أقصى طول أمر
#endif

#ifndef VOLTRONIC_MAX_RETRIES
  #define VOLTRONIC_MAX_RETRIES    2
#endif

#ifndef VOLTRONIC_DEFAULT_BAUD
  #define VOLTRONIC_DEFAULT_BAUD   2400
#endif

#ifndef VOLTRONIC_DEFAULT_TIMEOUT_MS
  #define VOLTRONIC_DEFAULT_TIMEOUT_MS 800
#endif

// فعّل escape للـ CRC (ضروري لكل أجهزة Voltronic PI30)
#ifndef VOLTRONIC_CRC_ESCAPE
  #define VOLTRONIC_CRC_ESCAPE 1
#endif

// اطبع فقط الأحرف المطبوعة (32..126) أثناء القراءة
#ifndef VOLTRONIC_FILTER_PRINTABLE
  #define VOLTRONIC_FILTER_PRINTABLE 1
#endif

// استدعِ yield() أثناء القراءة (ضروري ESP8266)
#ifndef VOLTRONIC_YIELD_IN_READ
  #define VOLTRONIC_YIELD_IN_READ 1
#endif

// ─── Runtime configuration ───
struct VoltronicConfig {
  uint32_t baud              = VOLTRONIC_DEFAULT_BAUD;
  uint16_t responseTimeoutMs = VOLTRONIC_DEFAULT_TIMEOUT_MS;
  uint8_t  retries           = VOLTRONIC_MAX_RETRIES;

  // Serial pins (اختياري — للـ ESP32/ESP8266 HardwareSerial)
  int8_t   rxPin             = -1;
  int8_t   txPin             = -1;

  // خيارات السلوك
  bool     applyCrcEscape    = (VOLTRONIC_CRC_ESCAPE != 0);
  bool     filterPrintable   = (VOLTRONIC_FILTER_PRINTABLE != 0);
  bool     yieldDuringRead   = (VOLTRONIC_YIELD_IN_READ != 0);
  bool     clearBufferBeforeSend = true;
};