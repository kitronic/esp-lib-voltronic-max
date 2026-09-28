#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  VoltronicLog — Platform-aware logging
//  ═══════════════════════════════════════════════════════════════
//
//  ESP8266 / ESP32:
//      Serial.printf_P(PSTR(fmt), ...)   → formatting كامل
//      EEPROM.begin(size) + EEPROM.commit()
//
//  AVR / SAMD / STM32:
//      لا logs (توفير ~500 بايت Flash)
//      EEPROM.begin() بدون حجم
//
//  الاستخدام:
//      #include "VoltronicLog.h"
//      VLOG("[Storage] EEPROM %u bytes\n", size);
// ═══════════════════════════════════════════════════════════════

// ─── Platform Detection ───
#if defined(ESP8266) || defined(ESP32)
#define VOLTRONIC_HAS_PRINTF_P 1
#define VOLTRONIC_EEPROM_API 1 // begin(size) + commit()
#else
#define VOLTRONIC_HAS_PRINTF_P 0
#define VOLTRONIC_EEPROM_API 0 // begin() فقط
#endif

// ─── Logging macro ───
#if VOLTRONIC_HAS_PRINTF_P
#define VLOG(fmt, ...) Serial.printf_P(PSTR(fmt), ##__VA_ARGS__)
#else
#define VLOG(fmt, ...) \
    do                 \
    {                  \
    } while (0)
#endif

// ─── EEPROM macros ───
#if VOLTRONIC_EEPROM_API
#define VOLTRONIC_EEPROM_BEGIN(size) EEPROM.begin(size)
#define VOLTRONIC_EEPROM_COMMIT() EEPROM.commit()
#else
#define VOLTRONIC_EEPROM_BEGIN(size) EEPROM.begin()
#define VOLTRONIC_EEPROM_COMMIT() (true)
#endif