#pragma once
#include <Arduino.h>

// CRC-16/XMODEM (poly 0x1021, init 0x0000, no reflect, no xorout)
inline uint16_t voltronicCRC(const uint8_t* data, size_t len) {
  uint16_t crc = 0x0000;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t b = 0; b < 8; b++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else              crc <<= 1;
    }
  }
  return crc;
}

// ─── Voltronic quirk ───
// الأجهزة ترفض بايتات CRC إذا كانت تساوي 0x0A (LF), 0x0D (CR), 0x28 '('
// الحل: نزيدها +1
inline uint8_t voltronicEscapeByte(uint8_t b) {
  if (b == 0x0A || b == 0x0D || b == 0x28) return (uint8_t)(b + 1);
  return b;
}

inline uint8_t voltronicUnescapeByte(uint8_t b) {
  if (b == 0x0B || b == 0x0E || b == 0x29) return (uint8_t)(b - 1);
  return b;
}

// ─── CRC bytes مع escape ───
inline void voltronicCRCBytes(uint16_t crc, uint8_t& hi, uint8_t& lo, bool escape) {
  hi = (uint8_t)((crc >> 8) & 0xFF);
  lo = (uint8_t)(crc & 0xFF);
  if (escape) {
    hi = voltronicEscapeByte(hi);
    lo = voltronicEscapeByte(lo);
  }
}