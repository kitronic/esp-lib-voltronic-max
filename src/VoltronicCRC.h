#pragma once

// ═══════════════════════════════════════════════════════════════
//  CRC-16/XMODEM + Voltronic escape quirk
//
//  الخصائص:
//    - Polynomial : 0x1021 (CCITT)
//    - Init       : 0x0000
//    - Reflect in : No
//    - Reflect out: No
//    - XorOut     : 0x0000
//
//  ⚠️ ملاحظة: هذا الملف لا يعتمد على Arduino.h —
//  يشتغل في native tests مباشرة بدون mock.
// ═══════════════════════════════════════════════════════════════

#include <stddef.h>
#include <stdint.h>

// ─── ثوابت CRC ───
namespace VoltronicCRCDetail
{
constexpr uint16_t POLY = 0x1021;
constexpr uint16_t INIT = 0x0000;

// بايتات تُهربها بعض الأجهزة (تُزاد +1)
constexpr uint8_t ESC_0A = 0x0A;  // LF
constexpr uint8_t ESC_0D = 0x0D;  // CR
constexpr uint8_t ESC_28 = 0x28;  // '('

// القيم بعد التهريب
constexpr uint8_t UNESC_0B = 0x0B;
constexpr uint8_t UNESC_0E = 0x0E;
constexpr uint8_t UNESC_29 = 0x29;
}  // namespace VoltronicCRCDetail

// ═══════════════════════════════════════════════════════════════
//  CRC-16/XMODEM — حساب لكل الـ buffer
// ═══════════════════════════════════════════════════════════════
inline uint16_t voltronicCRC(const uint8_t *data, size_t len)
{
  uint16_t crc = VoltronicCRCDetail::INIT;
  for (size_t i = 0; i < len; i++)
  {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t b = 0; b < 8; b++)
    {
      if (crc & 0x8000)
        crc = (uint16_t)((crc << 1) ^ VoltronicCRCDetail::POLY);
      else
        crc = (uint16_t)(crc << 1);
    }
  }
  return crc;
}

// ═══════════════════════════════════════════════════════════════
//  CRC تزايدي — لمستخدمين يبنون الفريم بايت بايت
//  الاستعمال:
//    uint16_t c = 0;
//    c = voltronicCRCUpdate(c, byte);
//    c = voltronicCRCUpdate(c, byte2);
// ═══════════════════════════════════════════════════════════════
inline uint16_t voltronicCRCUpdate(uint16_t crc, uint8_t byte)
{
  crc ^= (uint16_t)byte << 8;
  for (uint8_t b = 0; b < 8; b++)
  {
    if (crc & 0x8000)
      crc = (uint16_t)((crc << 1) ^ VoltronicCRCDetail::POLY);
    else
      crc = (uint16_t)(crc << 1);
  }
  return crc;
}

// ═══════════════════════════════════════════════════════════════
//  Voltronic quirk — escape / unescape
//
//  بعض أجهزة Voltronic ترفض بايتات CRC إذا كانت تساوي:
//    0x0A (LF) — لأنها تشبه نهاية سطر
//    0x0D (CR) — لأنها نهاية فريم PI30
//    0x28 ( '(') — لأنها بداية payload
//
//  الحل: نزيد البايت +1
// ═══════════════════════════════════════════════════════════════
inline uint8_t voltronicEscapeByte(uint8_t b)
{
  if (b == VoltronicCRCDetail::ESC_0A || b == VoltronicCRCDetail::ESC_0D ||
      b == VoltronicCRCDetail::ESC_28)
    return (uint8_t)(b + 1);
  return b;
}

inline uint8_t voltronicUnescapeByte(uint8_t b)
{
  if (b == VoltronicCRCDetail::UNESC_0B || b == VoltronicCRCDetail::UNESC_0E ||
      b == VoltronicCRCDetail::UNESC_29)
    return (uint8_t)(b - 1);
  return b;
}

// ═══════════════════════════════════════════════════════════════
//  تحويل CRC إلى بايتين (Hi, Lo) مع خيار التهريب
// ═══════════════════════════════════════════════════════════════
inline void voltronicCRCBytes(uint16_t crc, uint8_t &hi, uint8_t &lo, bool applyEscape)
{
  hi = (uint8_t)((crc >> 8) & 0xFF);
  lo = (uint8_t)(crc & 0xFF);
  if (applyEscape)
  {
    hi = voltronicEscapeByte(hi);
    lo = voltronicEscapeByte(lo);
  }
}