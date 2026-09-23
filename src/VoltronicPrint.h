#pragma once
#include <Arduino.h>
#include "VoltronicTypes.h"

// ═══════════════════════════════════════════════════════════════
//  Print helpers — تعمل على كل المنصات
//  بديل لـ Serial.printf() على AVR
// ═══════════════════════════════════════════════════════════════
namespace VoltronicPrint {

inline void pad2(Stream& s, uint8_t v) {
  if (v < 10) s.print('0');
  s.print(v);
}

inline void printFixed(Stream& s, float v, uint8_t decimals) {
  s.print(v, decimals);
}

inline void printStatus(Stream& s, const QPIGSData& d) {
  s.print(F("Grid: "));
  s.print(d.gridVoltage(), 1);   s.print(F("V "));
  s.print(d.gridFrequency(), 1); s.println(F("Hz"));

  s.print(F("Out:  "));
  s.print(d.acOutputVoltage(), 1);   s.print(F("V "));
  s.print(d.acOutputFrequency(), 1); s.print(F("Hz | Load "));
  s.print(d.loadPercent);            s.println(F("%"));

  s.print(F("Batt: "));
  s.print(d.batteryVoltage(), 2); s.print(F("V "));
  s.print(d.batteryChargingCurrent); s.print(F("A "));
  s.print(d.batteryCapacity);     s.print(F("% | Temp "));
  s.print(d.inverterTemperature); s.println(F("C"));

  s.print(F("PV1:  "));
  s.print(d.pv1InputVoltage(), 1); s.print(F("V "));
  s.print(d.pv1InputCurrent(), 1); s.print(F("A "));
  s.print(d.pv1ChargingPower);     s.println(F("W"));
}

} // namespace