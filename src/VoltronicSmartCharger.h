#pragma once
#include <Arduino.h>
#include "VoltronicBattery.h"
#include "VoltronicTypes.h"

class VoltronicMAX;

// ═══════════════════════════════════════════════════════════════
//  VoltronicSmartCharger — شحن ذكي متعدد الأنماط
//  ⚠️ ملاحظة: أسماء enum بادئة MODE_ لتجنب تعارض
//     مع ESP32 hal macro DISABLED
// ═══════════════════════════════════════════════════════════════
class VoltronicSmartCharger
{
public:
    enum Mode : uint8_t
    {
        MODE_OFF = 0,  // معطّل
        MODE_STD = 1,  // قياسي (تخفيض تدريجي)
        MODE_FAST = 2, // سريع (Float فقط)
    };

    VoltronicSmartCharger();

    // ─── الإعدادات ───
    void setMode(Mode m) { _mode = m; }
    void setTargetAC(uint8_t a) { _targetAC = a; }
    void setTargetTotal(uint8_t a) { _targetTotal = a; }
    void setFloatAC(uint8_t a) { _floatAC = a; }
    void setFloatTotal(uint8_t a) { _floatTotal = a; }
    void setTempProtectC(uint8_t c) { _tempProtectC = c; }

    // ─── التحديث ───
    void update(VoltronicMAX &inv, const VoltronicBattery &bat);

    // ─── القراءات ───
    Mode mode() const { return _mode; }
    bool enabled() const { return _mode != MODE_OFF; }
    uint8_t targetAC() const { return _targetAC; }
    uint8_t targetTotal() const { return _targetTotal; }
    uint8_t floatAC() const { return _floatAC; }
    uint8_t floatTotal() const { return _floatTotal; }
    uint8_t tempProtectC() const { return _tempProtectC; }
    uint8_t currentAC() const { return _currentAC; }
    uint8_t currentTotal() const { return _currentTotal; }
    const char *stage() const { return _stage; }
    const char *status() const { return _status; }
    uint32_t changes() const { return _changes; }
    uint32_t floatEntries() const { return _floatEntries; }
    uint32_t tempProtects() const { return _tempProtects; }
    uint32_t lastChangeMs() const { return _lastChange; }
    bool tempProtection() const { return _inTempProtect; }
    bool configError() const { return _floatAC > _targetAC || _floatTotal > _targetTotal; }
    bool optionsLoaded() const { return _optionsLoaded; }

    const SelectableValues &acOptions() const { return _acOptions; }
    const SelectableValues &totalOptions() const { return _totalOptions; }

    const char *modeToString(Mode m) const;

private:
    Mode _mode;
    uint8_t _targetAC;
    uint8_t _targetTotal;
    uint8_t _floatAC;
    uint8_t _floatTotal;
    uint8_t _tempProtectC;

    uint8_t _currentAC;
    uint8_t _currentTotal;

    char _stage[24];
    char _status[96];

    uint32_t _changes;
    uint32_t _floatEntries;
    uint32_t _tempProtects;
    uint32_t _lastChange;

    bool _inTempProtect;
    bool _lastWasFloat;
    bool _optionsLoaded;
    bool _currentsInitialized;

    SelectableValues _acOptions;
    SelectableValues _totalOptions;

    void _loadOptionsIfNeeded(VoltronicMAX &inv);
    uint8_t _stepDown(uint8_t cur, uint8_t target, bool ac) const;
    uint8_t _stepUp(uint8_t cur, uint8_t target, bool ac) const;
    void _applyAC(VoltronicMAX &inv, uint8_t amps);
    void _applyTotal(VoltronicMAX &inv, uint8_t amps);
    void _setStatus(const char *fmt, ...);
    void _setStage(const char *s);
    uint8_t _pickClosest(bool ac, uint8_t desired) const;
};