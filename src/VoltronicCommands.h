#pragma once

// ═══════════════════════════════════════════════════════════════
//  PI30 ASCII Commands — Voltronic Axpert MAX / PIP / MPP
//
//  جميع الأوامر تُرسل بدون CRC — المكتبة تضيف CRC-16/XMODEM + CR
//  البروتوكول: 2400 8N1 (بمعظم الأجهزة)
//
//  المراجع:
//    - Voltronic Power "PI30 Protocol" (rev 3.x)
//    - Axpert MAX HV7.2kW / LV5kW firmware notes
// ═══════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════
//  Inquiry Commands (25)
// ═══════════════════════════════════════════════════════════════
#define VC_QPI "QPI"           // Protocol ID
#define VC_QID "QID"           // Serial number (short)
#define VC_QSID "QSID"         // Serial number (long / 4-digit)
#define VC_QVFW "QVFW"         // Main CPU firmware version
#define VC_QVFW3 "QVFW3"       // Secondary CPU firmware version
#define VC_VERFW "VERFW:"      // Bluetooth FW (with ':' suffix)
#define VC_QPIRI "QPIRI"       // Rated information
#define VC_QFLAG "QFLAG"       // Device status flags
#define VC_QPIGS "QPIGS"       // General status (live data)
#define VC_QPIGS2 "QPIGS2"     // General status 2 (PV2)
#define VC_QPGS "QPGS"         // Parallel info (needs index: QPGS0..n)
#define VC_QMOD "QMOD"         // Working mode
#define VC_QPIWS "QPIWS"       // Warning status (36 bits)
#define VC_QDI "QDI"           // Default settings
#define VC_QMCHGCR "QMCHGCR"   // Max charging current selectable
#define VC_QMUCHGCR "QMUCHGCR" // Max utility charging current selectable
#define VC_QOPPT "QOPPT"       // Output source priority time order
#define VC_QCHPT "QCHPT"       // Charger source priority time order
#define VC_QT "QT"             // Current device time
#define VC_QBEQI "QBEQI"       // Battery equalization info
#define VC_QMN "QMN"           // Model name
#define VC_QGMN "QGMN"         // General model name
#define VC_QBOOT "QBOOT"       // DSP bootstrap presence
#define VC_QBATCD "QBATCD"     // Battery control status
#define VC_QLED "QLED"         // LED status/effect

// ═══════════════════════════════════════════════════════════════
//  Setting Commands (30)
// ═══════════════════════════════════════════════════════════════

// ─── Flags ───
#define VC_PE "PE" // Enable flag  (e.g. "PEa")
#define VC_PD "PD" // Disable flag (e.g. "PDa")
#define VC_PF "PF" // Factory reset / restore defaults

// ─── Charging / discharging ───
#define VC_MNCHGC "MNCHGC"           // Max charging current  (3 digits: 000..)
#define VC_MUCHGC "MUCHGC"           // Max utility charging  (3 digits)
#define VC_PBATMAXDISC "PBATMAXDISC" // Max discharging current (3 digits)
#define VC_PCVT "PCVT"               // Max CV charging time   (minutes)

// ─── Output ───
#define VC_F "F"       // Output frequency     (2 digits: 50/60)
#define VC_V "V"       // Output voltage       (3 digits: 220/230/240)
#define VC_POP "POP"   // Output source priority (2 digits: 00/01/02)
#define VC_POPM "POPM" // Output mode            (2 digits)

// ─── Battery ───
#define VC_PBCV "PBCV" // Battery recharge voltage  (V.d)
#define VC_PBDV "PBDV" // Battery redischarge voltage
#define VC_PSDV "PSDV" // Battery cutoff voltage
#define VC_PCVV "PCVV" // Battery CV (bulk) voltage
#define VC_PBFT "PBFT" // Battery float voltage
#define VC_PBT "PBT"   // Battery type (2 digits: 00..03)

// ─── AC input / charger ───
#define VC_PCP "PCP"   // Charger source priority (2 digits: 01..03)
#define VC_PGR "PGR"   // Grid working range (2 digits)
#define VC_PPCP "PPCP" // Parallel charger priority (M PP)

// ─── Energy / log ───
#define VC_RTEY "RTEY" // Reset PV energy / total energy
#define VC_RTDL "RTDL" // Erase data log

// ─── Battery equalization ───
#define VC_PBEQE "PBEQE"   // Equalization enable   (1 digit)
#define VC_PBEQT "PBEQT"   // Equalization time     (3 digits, minutes)
#define VC_PBEQP "PBEQP"   // Equalization period   (3 digits, days)
#define VC_PBEQV "PBEQV"   // Equalization voltage  (V.dd)
#define VC_PBEQOT "PBEQOT" // Equalization over time
#define VC_PBEQA "PBEQA"   // Activate equalization now (1 digit)

// ─── Date / time / battery control ───
#define VC_DAT "DAT"       // Set date/time (yymmddhhmmss)
#define VC_PBATCD "PBATCD" // Battery control (3 digits: a b c)

// ═══════════════════════════════════════════════════════════════
//  Mode characters (QMOD response)
// ═══════════════════════════════════════════════════════════════
#define VC_MODE_POWER_ON 'P'
#define VC_MODE_STANDBY 'S'
#define VC_MODE_LINE 'L'
#define VC_MODE_BATTERY 'B'
#define VC_MODE_FAULT 'F'
#define VC_MODE_POWER_SAVING 'H'
#define VC_MODE_SHUTDOWN 'D'

// ═══════════════════════════════════════════════════════════════
//  Priority values
// ═══════════════════════════════════════════════════════════════
#define VC_OUT_PRIO_UTILITY_FIRST 0
#define VC_OUT_PRIO_SOLAR_FIRST 1
#define VC_OUT_PRIO_SBU 2

#define VC_CHG_PRIO_SOLAR_FIRST 1
#define VC_CHG_PRIO_SOLAR_UTILITY 2
#define VC_CHG_PRIO_SOLAR_ONLY 3

// ═══════════════════════════════════════════════════════════════
//  Battery type values (PBT)
// ═══════════════════════════════════════════════════════════════
#define VC_BAT_TYPE_AGM 0
#define VC_BAT_TYPE_FLOODED 1
#define VC_BAT_TYPE_USER 2
#define VC_BAT_TYPE_PYLONTECH 3

// ═══════════════════════════════════════════════════════════════
//  Flag characters (PEx / PDx)
//
//  الاستعمال: setFlag('a') → "PEa"  |  clearFlag('a') → "PDa"
// ═══════════════════════════════════════════════════════════════
#define VC_FLAG_BUZZER 'a'           // Buzzer on/off
#define VC_FLAG_OVERLOAD_BYPASS 'b'  // Overload bypass
#define VC_FLAG_SOLAR_FEED_GRID 'd'  // Solar feed to grid
#define VC_FLAG_POWER_SAVING 'j'     // Power saving mode
#define VC_FLAG_LCD_DEFAULT 'k'      // LCD returns to default after 1min
#define VC_FLAG_OVERLOAD_RESTART 'u' // Overload auto-restart
#define VC_FLAG_OVERTEMP_RESTART 'v' // Over-temp auto-restart
#define VC_FLAG_BACKLIGHT 'x'        // LCD backlight on/off
#define VC_FLAG_ALARM_PRIMARY 'y'    // Alarm on primary source interrupt
#define VC_FLAG_FAULT_CODE_REC 'z'   // Fault code recording