#pragma once
// ═══════════════════════════════════════════════════════════════
//  Arduino.h mock — للـ native tests فقط
//  يوفر الحد الأدنى اللي تحتاجه VoltronicParser
// ═══════════════════════════════════════════════════════════════

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ─── الأنواع الأساسية ───
typedef uint8_t  byte;
typedef bool     boolean;

// ─── Time & yield ───
#ifndef millis
inline uint32_t millis() { return 0; }
#endif
inline void yield() {}

// ─── PROGMEM / F() ───
#ifndef PROGMEM
  #define PROGMEM
#endif
#ifndef F
  #define F(x) x
#endif

// ─── Serial constants (لأي include عابر) ───
#ifndef SERIAL_8N1
  #define SERIAL_8N1 0x800001c
#endif

// ─── Stream (base class — لا نستعملها في الاختبارات) ───
class Stream {
public:
  virtual int available() { return 0; }
  virtual int read() { return -1; }
  virtual size_t write(uint8_t) { return 0; }
  virtual size_t write(const uint8_t*, size_t) { return 0; }
  virtual void flush() {}
};

// ─── Serial stub ───
class SerialStub : public Stream {
public:
  void begin(uint32_t) {}
  void print(const char*) {}
  void println(const char*) {}
  int printf(const char*, ...) { return 0; }
};

extern SerialStub Serial;