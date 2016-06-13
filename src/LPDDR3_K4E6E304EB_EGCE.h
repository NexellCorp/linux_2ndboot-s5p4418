/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *	FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : Memory Controller
 *      File            : LPDDR3_K4E6E304EB.c
 *      Description     :
 *      Author          : Hans
 *      History         : 
 */
#ifndef __LPDDR3_K4E6E304EB_EGCE_H__
#define __LPDDR3_K4E6E304EB_EGCE_H__

// Clock List
#define DDR3_533        0
#define DDR3_666        1
#define DDR3_800        2


// User Define
//#define MEM_CLK         DDR3_533
//#define MEM_CLK         DDR3_666
//#define MEM_CLK         DDR3_760
#define MEM_CLK         DDR3_800

#define _DDR_CS_NUM     2               // nCS Number : Tablet=1, VTK=2
#define _DDR_BUS_WIDTH  16              // 16bit, 8bit

#define _DDR_ROW_NUM    14
#define _DDR_COL_NUM    10
#define _DDR_BANK_NUM   8


// Refer to Memory Datasheet
//#define cs_size         (0x40000000>>24)
#define cs_size         (0x20000000>>24)

#define chip_row        (_DDR_ROW_NUM-12)       // ROW address bit : 15bit : 3(Tabletx2), 16bit : 4(Tabletx4, Elcomtech)
#define chip_col        (_DDR_COL_NUM-7)        // Column Address Bit. 2:9bit, 3:10bit, others:Reserved
#if (_DDR_BANK_NUM == 8)
#define chip_bank       (3)                     // Bank bit : 3:8bank, 2:4bank
#else
#define chip_bank       (2)                     // Bank bit : 3:8bank, 2:4bank
#endif
#define chip_base0      0x040
#define chip_base1      (chip_base0+cs_size)

#if 1
#define chip_mask       (0x800-cs_size)         // Capacity per nCS: 2G=0x780, 1G=0x7C0(Tabletx2, VTK)
#else
#define chip_mask       (0x800-(cs_size<<1))    // for Timing Calculation.
#endif

#define RDFETCH         0x1                     // CONCONTROL.rd_fetch[14:12]

#if 0
#define READDELAY       0x08080808              // ctrl_offsetr
#define WRITEDELAY      0x08080808              // ctrl_offsetw
#else
#define READDELAY       0x0A0A0A0A              //- ctrl_offsetr, 0x0C0C0C0C, 0x18181818
#define WRITEDELAY      0x05050505              //- ctrl_offsetw, 0x04040404, 0x02020202
#endif


// Option
#define CONFIG_ODTOFF_GATELEVELINGON    0        // Not support yet.



// AC Timing
#if (MEM_CLK == DDR3_800)

#define MR1_nWR         0xC
#define MR2_RLWL        0xC

#define tPZQ            0x008035
#define tREFI           0x30C   // 3.9us

#define tRFC            0x34
#define tRRD            0x4
#define tRP             0x9
#define tRCD            0x8
#define tRC             0x1A
#define tRAS            0x11

#define tWTR            0x3
#define tWR             0x6
#define tRTP            0x3
#define W2W_C2C         0x1
#define R2R_C2C         0x1
#define tDQSCK          0x5     // DDR3 : 0
#define tWL             0x6
#define tRL             0xC

#define tFAW            0x14
#define tXSR            0x38
#define tXP             0x3
#define tCKE            0x3
#define tMRD            0x6

//#define tADR            0x4     // Micron (20ns)
#define tADR            0x3     // Samsung (15ns)
#define tWLO            (tADR)

#endif  //#if (MEM_CLK == DDR3_800)


#if (MEM_CLK == DDR3_666)

#define MR1_nWR         0xA
#define MR2_RLWL        0xA

#define tPZQ            0x8014
#define tREFI           0x28A        // 3.9us

#define tRFC            0x2C
#define tRRD            0x4
#define tRP             0x7
#define tRCD            0x6
#define tRC             0x15
#define tRAS            0x0E

#define tWTR            0x3
#define tWR             0x5
#define tRTP            0x3
#define W2W_C2C         0x1
#define R2R_C2C         0x1
#define tDQSCK          0x4        // DDR3 : 0
#define tWL             0x6
#define tRL             0xA

#define tFAW            0x11
//#define tXSR            0x100
#define tXSR            0x02F
#define tXP             0x03
#define tCKE            0x3
#define tMRD            0x5

//#define tADR            0x4     // Micron (20ns)
#define tADR            0x3     // Samsung (15ns)
#define tWLO            (tADR)

#endif  //#if (MEM_CLK == DDR3_666)


#if (MEM_CLK == DDR3_533)

#define MR1_nWR         0x8
#define MR2_RLWL        0x9

#define tPZQ            0x8020
#define tREFI           0x208        // 3.9us

#define tRFC            0x23
#define tRRD            0x3
#define tRP             0x6
#define tRCD            0x5
#define tRC             0x12
#define tRAS            0x0C

#define tWTR            0x2
#define tWR             0x4
#define tRTP            0x2
#define W2W_C2C         0x1
#define R2R_C2C         0x1
#define tDQSCK          0x3   // DDR3 : 0
#define tWL             0x5
#define tRL             0x9

#define tFAW            0x0E
#define tXSR            0x026
#define tXP             0x02
#define tCKE            0x2
#define tMRD            0x5

//#define tADR            0x3     // Micron (20ns)
#define tADR            0x3     // Samsung (15ns)
#define tWLO            (tADR)

#endif  //#if (MEM_CLK == DDR3_533)

#define nWL             (tWL)
#define nRL             (tRL)

#endif  //#ifndef __LPDDR3_K4E6E304EB_EGCE_H__
