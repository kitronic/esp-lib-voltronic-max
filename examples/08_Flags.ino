#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 08 QFLAG ---"));
  inverter.begin(2400);
}

void loop() {
  if (inverter.queryFlags()) {
    const QFLAGData& f = inverter.qflag();
    Serial.println(F("═══ QFLAG ═══"));
    Serial.printf("Buzzer Enabled:         %d\n", f.buzzerEnabled());
    Serial.printf("Overload Bypass:        %d\n", f.overloadBypass());
    Serial.printf("Solar Feed to Grid:     %d\n", f.solarFeedToGrid());
    Serial.printf("Power Saving:           %d\n", f.powerSaving());
    Serial.printf("LCD Escape to Default:  %d\n", f.lcdEscapeToDefault());
    Serial.printf("Overload Restart:       %d\n", f.overloadRestart());
    Serial.printf("Over-Temp Restart:      %d\n", f.overTempRestart());
    Serial.printf("Backlight On:           %d\n", f.backlightOn());
    Serial.printf("Alarm on Primary Int:   %d\n", f.alarmOnPrimaryInterrupt());
    Serial.printf("Fault Code Record:      %d\n", f.faultCodeRecord());
    Serial.printf("Raw: %s\n", f.raw);
  }
  delay(5000);
}