#pragma once

// ═══════════════════════════════════════════════════════════════
//  PI30 ASCII Commands (Voltronic Axpert MAX / PIP / MPP)
//  كل الأوامر بدون CRC — المكتبة تضيفه
// ═══════════════════════════════════════════════════════════════

// ─── Inquiry ───
#define VC_QPI      "QPI"
#define VC_QID      "QID"
#define VC_QSID     "QSID"
#define VC_QVFW     "QVFW"
#define VC_QVFW3    "QVFW3"
#define VC_VERFW    "VERFW:"
#define VC_QPIRI    "QPIRI"
#define VC_QFLAG    "QFLAG"
#define VC_QPIGS    "QPIGS"
#define VC_QPIGS2   "QPIGS2"
#define VC_QPGS     "QPGS"
#define VC_QMOD     "QMOD"
#define VC_QPIWS    "QPIWS"
#define VC_QDI      "QDI"
#define VC_QMCHGCR  "QMCHGCR"
#define VC_QMUCHGCR "QMUCHGCR"
#define VC_QOPPT    "QOPPT"
#define VC_QCHPT    "QCHPT"
#define VC_QT       "QT"
#define VC_QBEQI    "QBEQI"
#define VC_QMN      "QMN"
#define VC_QGMN     "QGMN"
#define VC_QBOOT    "QBOOT"
#define VC_QBATCD   "QBATCD"
#define VC_QLED     "QLED"

// ─── Setting ───
#define VC_PE       "PE"
#define VC_PD       "PD"
#define VC_PF       "PF"
#define VC_MNCHGC   "MNCHGC"
#define VC_MUCHGC   "MUCHGC"
#define VC_F        "F"
#define VC_V        "V"
#define VC_POP      "POP"
#define VC_PBCV     "PBCV"
#define VC_PBDV     "PBDV"
#define VC_PCP      "PCP"
#define VC_PGR      "PGR"
#define VC_PBT      "PBT"
#define VC_POPM     "POPM"
#define VC_PPCP     "PPCP"
#define VC_PSDV     "PSDV"
#define VC_PCVV     "PCVV"
#define VC_PBFT     "PBFT"
#define VC_RTEY     "RTEY"
#define VC_RTDL     "RTDL"
#define VC_PBEQE    "PBEQE"
#define VC_PBEQT    "PBEQT"
#define VC_PBEQP    "PBEQP"
#define VC_PBEQV    "PBEQV"
#define VC_PBEQOT   "PBEQOT"
#define VC_PBEQA    "PBEQA"
#define VC_PCVT     "PCVT"
#define VC_DAT      "DAT"
#define VC_PBATCD   "PBATCD"
#define VC_PBATMAXDISC "PBATMAXDISC"

// ─── Mode chars ───
#define VC_MODE_POWER_ON      'P'
#define VC_MODE_STANDBY       'S'
#define VC_MODE_LINE          'L'
#define VC_MODE_BATTERY       'B'
#define VC_MODE_FAULT         'F'
#define VC_MODE_POWER_SAVING  'H'
#define VC_MODE_SHUTDOWN      'D'

// ─── Priorities ───
#define VC_OUT_PRIO_UTILITY_FIRST  0
#define VC_OUT_PRIO_SOLAR_FIRST    1
#define VC_OUT_PRIO_SBU            2

#define VC_CHG_PRIO_SOLAR_FIRST    1
#define VC_CHG_PRIO_SOLAR_UTILITY  2
#define VC_CHG_PRIO_SOLAR_ONLY     3

#define VC_BAT_TYPE_AGM        0
#define VC_BAT_TYPE_FLOODED    1
#define VC_BAT_TYPE_USER       2
#define VC_BAT_TYPE_PYLONTECH  3