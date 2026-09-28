#include "VoltronicSmartCharger.h"
#include "VoltronicMAX.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
// ═══════════════════════════════════════════════════════════════
VoltronicSmartCharger::VoltronicSmartCharger()
    : _mode(STANDARD), _targetAC(40), _targetTotal(80), _floatAC(2), _floatTotal(10), _tempProtectC(70), _currentAC(0), _currentTotal(0), _changes(0), _floatEntries(0), _tempProtects(0), _lastChange(0), _inTempProtect(false), _lastWasFloat(false), _optionsLoaded(false)
{
    _stage[0] = 'I';
    _stage[1] = 'd';
    _stage[2] = 'l';
    _stage[3] = 'e';
    _stage[4] = '\0';
    const char *init = "Starting...";
    strncpy(_status, init, sizeof(_status) - 1);
    _status[sizeof(_status) - 1] = '\0';
}

// ═══════════════════════════════════════════════════════════════
//  تحميل الخيارات من الإنفرتر (مرة واحدة + تحديث دوري)
// ═══════════════════════════════════════════════════════════════
void VoltronicSmartCharger::_loadOptionsIfNeeded(VoltronicMAX &inv)
{
    // كل 1 ساعة نعيد تحميل الخيارات (نادراً ما تتغير)
    static uint32_t lastLoad = 0;
    if (_optionsLoaded && (millis() - lastLoad < 3600000UL))
        return;

    lastLoad = millis();

    // جلب QMCHGCR (التيارات الإجمالية)
    if (inv.queryMaxChargingCurrents())
    {
        _totalOptions = inv.maxChgOptions();
    }
    // جلب QMUCHGCR (تيارات AC)
    if (inv.queryMaxUtilityChargingCurrents())
    {
        _acOptions = inv.maxUtilChgOptions();
    }

    _optionsLoaded = (_totalOptions.count > 0) && (_acOptions.count > 0);

    // إذا لم ينجح الجلب → fallback آمن
    if (!_optionsLoaded)
    {
        _acOptions.count = 9;
        const uint8_t fallbackAC[9] = {2, 10, 20, 30, 40, 50, 60, 70, 80};
        memcpy(_acOptions.values, fallbackAC, 9);

        _totalOptions.count = 8;
        const uint8_t fallbackTotal[8] = {10, 20, 30, 40, 50, 60, 70, 80};
        memcpy(_totalOptions.values, fallbackTotal, 8);
    }
}

// ═══════════════════════════════════════════════════════════════
//  اختيار أقرب قيمة متاحة (تعميم بدل hardcoded arrays)
// ═══════════════════════════════════════════════════════════════
uint8_t VoltronicSmartCharger::_pickClosest(bool ac, uint8_t desired) const
{
    const SelectableValues &s = ac ? _acOptions : _totalOptions;
    if (s.count == 0)
        return desired;

    uint8_t best = s.values[0];
    uint8_t bestDiff = 255;
    for (uint8_t i = 0; i < s.count; i++)
    {
        uint8_t diff = (s.values[i] > desired) ? (s.values[i] - desired) : (desired - s.values[i]);
        if (diff < bestDiff)
        {
            bestDiff = diff;
            best = s.values[i];
        }
    }
    return best;
}

// ═══════════════════════════════════════════════════════════════
//  _stepDown — يختار أكبر قيمة <= cur و >= target
// ═══════════════════════════════════════════════════════════════
uint8_t VoltronicSmartCharger::_stepDown(uint8_t cur, uint8_t target, bool ac) const
{
    const SelectableValues &s = ac ? _acOptions : _totalOptions;
    uint8_t best = cur;
    for (uint8_t i = 0; i < s.count; i++)
    {
        uint8_t v = s.values[i];
        if (v < cur && v >= target && v > best)
            best = v;
        else if (best == cur && v < cur && v >= target)
            best = v;
    }
    // نجد أقرب قيمة أقل من cur
    uint8_t result = cur;
    for (uint8_t i = 0; i < s.count; i++)
    {
        uint8_t v = s.values[i];
        if (v < cur && v >= target)
        {
            if (result == cur || v > result)
                result = v;
        }
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
//  _stepUp — يختار أصغر قيمة > cur و <= target
// ═══════════════════════════════════════════════════════════════
uint8_t VoltronicSmartCharger::_stepUp(uint8_t cur, uint8_t target, bool ac) const
{
    const SelectableValues &s = ac ? _acOptions : _totalOptions;
    uint8_t result = cur;
    for (uint8_t i = 0; i < s.count; i++)
    {
        uint8_t v = s.values[i];
        if (v > cur && v <= target)
        {
            if (result == cur || v < result)
                result = v;
        }
    }
    return result;
}

// ═══════════════════════════════════════════════════════════════
void VoltronicSmartCharger::_applyAC(VoltronicMAX &inv, uint8_t amps)
{
    if (amps == _currentAC)
        return;
    if (inv.setMaxUtilityChargingCurrent(amps))
    {
        _currentAC = amps;
        _lastChange = millis();
        _changes++;
    }
}

void VoltronicSmartCharger::_applyTotal(VoltronicMAX &inv, uint8_t amps)
{
    if (amps == _currentTotal)
        return;
    if (inv.setMaxChargingCurrent(amps))
    {
        _currentTotal = amps;
        _lastChange = millis();
        _changes++;
    }
}

void VoltronicSmartCharger::_setStatus(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(_status, sizeof(_status), fmt, ap);
    va_end(ap);
}

void VoltronicSmartCharger::_setStage(const char *s)
{
    strncpy(_stage, s, sizeof(_stage) - 1);
    _stage[sizeof(_stage) - 1] = '\0';
}

// ═══════════════════════════════════════════════════════════════
//  التحديث الرئيسي
// ═══════════════════════════════════════════════════════════════
void VoltronicSmartCharger::update(VoltronicMAX &inv, const VoltronicBattery &bat)
{
    _loadOptionsIfNeeded(inv);

    const QPIGSData &g = inv.qpigs();
    _currentAC = inv.qpiri().maxACChargingCurrent;
    _currentTotal = inv.qpiri().maxChargingCurrent;
    _currentAC = _pickClosest(true, _currentAC);
    _currentTotal = _pickClosest(false, _currentTotal);

    float soc = bat.soc();
    float temp = g.inverterTemperature;
    bool isFloat = g.sb2ChargingToFloat();
    bool gridOk = g.gridVoltage() > 100.0f;

    // Edge detection
    if (isFloat && !_lastWasFloat)
    {
        _floatEntries++;
        _lastWasFloat = true;
    }
    else if (!isFloat)
    {
        _lastWasFloat = false;
    }

    // Stage
    if (_mode == DISABLED)
        _setStage(inv.lang.tr(VoltronicLang::SC_STAGE_OFF));
    else if (temp >= _tempProtectC)
        _setStage(inv.lang.tr(VoltronicLang::SC_STAGE_TEMP));
    else if (isFloat)
        _setStage(inv.lang.tr(VoltronicLang::SC_STAGE_FLOAT));
    else if (soc >= 90.0f)
        _setStage(inv.lang.tr(VoltronicLang::SC_STAGE_NEAR));
    else
        _setStage(inv.lang.tr(VoltronicLang::SC_STAGE_CHARGE));

    // Disabled
    if (_mode == DISABLED)
    {
        _setStatus(inv.lang.tr(VoltronicLang::SC_STATUS_OFF));
        return;
    }

    // Temp Protect
    bool wasProtect = _inTempProtect;
    _inTempProtect = (temp >= _tempProtectC);
    if (_inTempProtect && !wasProtect)
        _tempProtects++;

    if (_inTempProtect)
    {
        uint8_t lowAC = (_acOptions.count > 0) ? _acOptions.values[0] : 10;
        if (_currentAC > lowAC)
        {
            uint8_t newAC = _stepDown(_currentAC, lowAC, true);
            if (newAC != _currentAC)
            {
                _applyAC(inv, newAC);
                snprintf(_status, sizeof(_status),
                         inv.lang.tr(VoltronicLang::SC_STATUS_TEMP_AC), newAC);
                return;
            }
        }
        uint8_t lowT = (_totalOptions.count > 0) ? _totalOptions.values[0] : 20;
        if (_currentTotal > lowT)
        {
            uint8_t newT = _stepDown(_currentTotal, lowT, false);
            if (newT != _currentTotal)
            {
                _applyTotal(inv, newT);
                snprintf(_status, sizeof(_status),
                         inv.lang.tr(VoltronicLang::SC_STATUS_TEMP_TOTAL), newT);
                return;
            }
        }
        _setStatus(inv.lang.tr(VoltronicLang::SC_STATUS_TEMP_ACTIVE));
        return;
    }

    // FAST mode
    if (_mode == FAST)
    {
        uint8_t targetAC = isFloat ? _floatAC : _targetAC;
        uint8_t targetT = isFloat ? _floatTotal : _targetTotal;

        if (_currentAC > targetAC && gridOk)
        {
            _applyAC(inv, _stepDown(_currentAC, targetAC, true));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_FAST_AC), _currentAC);
            return;
        }
        if (_currentAC < targetAC && gridOk)
        {
            _applyAC(inv, _stepUp(_currentAC, targetAC, true));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_FAST_AC), _currentAC);
            return;
        }
        if (_currentTotal > targetT)
        {
            _applyTotal(inv, _stepDown(_currentTotal, targetT, false));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_FAST_TOTAL), _currentTotal);
            return;
        }
        if (_currentTotal < targetT)
        {
            _applyTotal(inv, _stepUp(_currentTotal, targetT, false));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_FAST_TOTAL), _currentTotal);
            return;
        }
        snprintf(_status, sizeof(_status),
                 inv.lang.tr(VoltronicLang::SC_STATUS_FAST_STEADY),
                 _currentAC, _currentTotal);
        return;
    }

    // STANDARD mode
    if (_mode == STANDARD)
    {
        uint8_t targetAC = _targetAC;
        uint8_t targetT = _targetTotal;

        if (isFloat)
        {
            targetAC = _floatAC;
            targetT = _floatTotal;
        }
        else if (soc >= 90.0f)
        {
            if (targetAC > 20)
                targetAC = 20;
            if (targetT > 20)
                targetT = 20;
        }

        if (_currentAC > targetAC && gridOk)
        {
            _applyAC(inv, _stepDown(_currentAC, targetAC, true));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_STD_AC_DN),
                     _currentAC, soc);
            return;
        }
        if (_currentAC < targetAC && gridOk)
        {
            _applyAC(inv, _stepUp(_currentAC, targetAC, true));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_STD_AC_UP), _currentAC);
            return;
        }
        if (_currentTotal > targetT)
        {
            _applyTotal(inv, _stepDown(_currentTotal, targetT, false));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_STD_T_DN),
                     _currentTotal, soc);
            return;
        }
        if (_currentTotal < targetT)
        {
            _applyTotal(inv, _stepUp(_currentTotal, targetT, false));
            snprintf(_status, sizeof(_status),
                     inv.lang.tr(VoltronicLang::SC_STATUS_STD_T_UP), _currentTotal);
            return;
        }
        snprintf(_status, sizeof(_status),
                 inv.lang.tr(VoltronicLang::SC_STATUS_STD_STEADY),
                 _currentAC, _currentTotal, soc);
    }
}

const char *VoltronicSmartCharger::modeToString(Mode m) const
{
    switch (m)
    {
    case DISABLED:
        return "Disabled";
    case STANDARD:
        return "Standard";
    case FAST:
        return "Fast";
    default:
        return "?";
    }
}