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
 *      Module          : System Bus
 *      File            : sysbus.h
 *      Description     : 
 *      Author          : Hans
 *      History         : 
 */
#ifndef __SYS_BUS_H__
#define __SYS_BUS_H__

#include "sysheader.h"
#if 0
#include "nx_type.h"
#include "printf.h"
#include "debug.h"
#include <nx_debug2.h>
#include "nx_peridot.h"
#include <nx_tieoff.h>

#include <nx_drex.h>
#include "secondboot.h"
#endif

//------------------------------------------------------------------------------
// MISC
//
#define IO_ADDRESS(x)           (x)
#if 0
#define readb(addr)             ({U8  _v = ReadIO8(addr);  _v;})
#define readw(addr)             ({U16 _v = ReadIO16(addr); _v;})
#define readl(addr)             ({U32 _v = ReadIO32(addr); _v;})
#else
#define readb(addr)             ReadIO8(addr)
#define readw(addr)             ReadIO16(addr)
#define readl(addr)             ReadIO32(addr)
#endif
#if 0
#define writeb(data, addr)      ({U8  *_v = (U8 *)addr;  WriteIO8(_v, data);})
#define writew(data, addr)      ({U16 *_v = (U16 *)addr; WriteIO16(_v, data);})
#define writel(data, addr)      ({U32 *_v = (U32 *)addr; WriteIO32(_v, data);})
#else
#define writeb(data, addr)      WriteIO8(addr, data)
#define writew(data, addr)      WriteIO16(addr, data)
#define writel(data, addr)      WriteIO32(addr, data)
#endif
#define u8                      U8
#define u16                     U16
#define u32                     U32
#define u64                     U64

//------------------------------------------------------------------------------
// DREX BRB
//
#define NX_DREX_PORT_3_W_BRB_EN_SHFT    (7)
#define NX_DREX_PORT_2_W_BRB_EN_SHFT    (6)
#define NX_DREX_PORT_1_W_BRB_EN_SHFT    (5)
#define NX_DREX_PORT_0_W_BRB_EN_SHFT    (4)
#define NX_DREX_PORT_3_R_BRB_EN_SHFT    (3)
#define NX_DREX_PORT_2_R_BRB_EN_SHFT    (2)
#define NX_DREX_PORT_1_R_BRB_EN_SHFT    (1)
#define NX_DREX_PORT_0_R_BRB_EN_SHFT    (0)

#define NX_DREX_PORT_3_W_BRB_TH_SHFT    (28)
#define NX_DREX_PORT_2_W_BRB_TH_SHFT    (24)
#define NX_DREX_PORT_1_W_BRB_TH_SHFT    (20)
#define NX_DREX_PORT_0_W_BRB_TH_SHFT    (16)
#define NX_DREX_PORT_3_R_BRB_TH_SHFT    (12)
#define NX_DREX_PORT_2_R_BRB_TH_SHFT    (8)
#define NX_DREX_PORT_1_R_BRB_TH_SHFT    (4)
#define NX_DREX_PORT_0_R_BRB_TH_SHFT    (0)

//------------------------------------------------------------------------------
//  PL301
//
#define PERIBUS_SI_SLOT_TOP             (3)

#define NX_DREX_QOS_NUMBER              (16)

#define NX_DREX_QOS_OFFSET              (0x60)
#define NX_DREX_BRBRSV_CTRL_OFFSET      (0x100)
#define NX_DREX_BRBRSV_CFG_OFFSET       (0x104)

#define NX_PA_BASE_REG_TIEOFF           (0xC0011000)
#define NX_PA_BASE_REG_DREX             (0xC00E0000)
#define NX_PA_BASE_REG_DDRPHY           (0xC00E1000)

#define NX_VA_BASE_REG_TIEOFF           IO_ADDRESS(NX_PA_BASE_REG_TIEOFF)
#define NX_VA_BASE_REG_DREX             IO_ADDRESS(NX_PA_BASE_REG_DREX)
#define NX_VA_BASE_REG_DDRPHY           IO_ADDRESS(NX_PA_BASE_REG_DDRPHY)

#define NX_PL301_QOS_TRDMARK_OFFSET     (0x400)
#define NX_PL301_QOS_CTRL_OFFSET        (0x404)
#define NX_PL301_AR_OFFSET              (0x408)
#define NX_PL301_AW_OFFSET              (0x40C)

#define NX_PA_BASE_REG_PL301_BOTT       (0xC0050000)
#define NX_PA_BASE_REG_PL301_TOP        (0xC0052000)
#define NX_PA_BASE_REG_PL301_DISP       (0xC005E000)

#define NX_VA_BASE_REG_PL301_BOTT       IO_ADDRESS(NX_PA_BASE_REG_PL301_BOTT)
#define NX_VA_BASE_REG_PL301_TOP        IO_ADDRESS(NX_PA_BASE_REG_PL301_TOP)
#define NX_VA_BASE_REG_PL301_DISP       IO_ADDRESS(NX_PA_BASE_REG_PL301_DISP)

#define NX_BASE_ADDR_BOTT_QOS_TRDMARK       (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_BOTT_QOS_CTRL          (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_BOTT_AR                (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_BOTT_AW                (NX_VA_BASE_REG_PL301_BOTT + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_TOP_QOS_TRDMARK        (NX_VA_BASE_REG_PL301_TOP + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_TOP_QOS_CTRL           (NX_VA_BASE_REG_PL301_TOP + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_TOP_AR                 (NX_VA_BASE_REG_PL301_TOP + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_TOP_AW                 (NX_VA_BASE_REG_PL301_TOP + NX_PL301_AW_OFFSET)

#define NX_BASE_ADDR_DISP_QOS_TRDMARK       (NX_VA_BASE_REG_PL301_DISP + NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_ADDR_DISP_QOS_CTRL          (NX_VA_BASE_REG_PL301_DISP + NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_ADDR_DISP_AR                (NX_VA_BASE_REG_PL301_DISP + NX_PL301_AR_OFFSET)
#define NX_BASE_ADDR_DISP_AW                (NX_VA_BASE_REG_PL301_DISP + NX_PL301_AW_OFFSET)

#define SI_IF_NUM_POS                       0
#define SLOT_NUM_POS                        24

//------------------------------------------------------------------------------

// Interface slot number of  TOP_BUS.
#define TOPBUS_SI_SLOT_DMAC0                0
#define TOPBUS_SI_SLOT_MP2TS                1
#define TOPBUS_SI_SLOT_DMAC1                2
#define TOPBUS_SI_SLOT_REMAP                3
#define TOPBUS_SI_SLOT_SDMMC                4
#define TOPBUS_SI_SLOT_USBOTG               5
#define TOPBUS_SI_SLOT_USBHOST0             6
#define TOPBUS_SI_SLOT_USBHOST1             7

// Interface slot number of  BOTTOM_BUS.
#define BOTBUS_SI_SLOT_1ST_ARM              0
#define BOTBUS_SI_SLOT_2ND_ARM              1
#define BOTBUS_SI_SLOT_MALI                 2
#define BOTBUS_SI_SLOT_TOP                  3
#define BOTBUS_SI_SLOT_DEINTERLACE          4
#define BOTBUS_SI_SLOT_1ST_CODA             5
#define BOTBUS_SI_SLOT_2ND_CODA             6
#define BOTBUS_SI_SLOT_SCALER               7

// Interface slot number of  DISPLAY_BUS.
#define DISBUS_SI_SLOT_1ST_DISPLAY          0
#define DISBUS_SI_SLOT_2ND_DISPLAY          1
#define DISBUS_SI_SLOT_GMAC                 2

#endif  // #ifndef __SYS_BUS_H__
