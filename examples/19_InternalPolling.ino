#include <VoltronicMAX.h>
#include <SoftwareSerial.h>

SoftwareSerial invSerial(D1, D2);
VoltronicMAX inverter(invSerial);

void setup() {
  Serial.begin(115200);
  Serial.println(F("--- 19 Internal Polling ---"));
  inverter.begin(2400);

  VoltronicPollSchedule sched;
  sched.qpigsMs  = 2000;
  sched.qpigs2Ms = 2000;
  sched.qmodMs   = 5000;
  sched.qpiwsMs  = 30000;
  sched.qpiriMs  = 60000;
  sched.qbatcdMs = 120000;
  // qflagMs / qidMs / qbeqiMs = 0 → معطّلة

  inverter.startPolling(sched);
}

void loop() {
  inverter.poll();

  if (inverter.pollingCycleDone()) {
    const QPIGSData& d = inverter.qpigs();
    Serial.printf("[cycle %lu] Batt=%.2fV Load=%u%% Mode=%s\n",
                  inverter.pollingCycles(),
                  d.batteryVoltage(), d.loadPercent,
                  inverter.modeString());
  }
}