#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
//  VoltronicLog — Platform-aware logging
//  ⚠️ نستخدم ARDUINO_ARCH_* (معيار Arduino الرسمي)
// ═══════════════════════════════════════════════════════════════

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#define VOLTRONIC_HAS_PRINTF_P 1
#define VOLTRONIC_EEPROM_API 1
#else
#define VOLTRONIC_HAS_PRINTF_P 0
#define VOLTRONIC_EEPROM_API 0
#endif

// ─── VLOG ───
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