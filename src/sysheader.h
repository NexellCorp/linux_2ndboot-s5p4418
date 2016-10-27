/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sangjong, Han <hans@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __SYS_HEADER_H__
#define __SYS_HEADER_H__

#include "cfgBootDefine.h"
#include "cfgFreqDefine.h"

#include <nx_pyrope.h>
#include <nx_type.h>
#include <nx_debug2.h>
#include <nx_chip.h>
#include <nx_rstcon.h>

#include <nx_clkpwr.h>
#include <nx_ecid.h>
#include <nx_gpio.h>
#include <nx_alive.h>
#include <nx_rtc.h>
#include <nx_tieoff.h>
#include <nx_intc.h>
#include <nx_clkgen.h>
#include <nx_ssp.h>
#include <nx_uart.h>
#include <nx_wdt.h>

#include "secondboot.h"
#include "printf.h"
#include "debug.h"
#include "type.h"
#include <common.h>

#define BL1_SDRAMBOOT_LOADADDR			(0xFFFF0000)
#define BL1_SDMMCBOOT_DEVADDR			(0x200)
#define BL1_SDMMCBOOT_LOADSIZE			(16*1024)
#define SRAM_MAXSIZE				(32*1024)
#define BL1_STACKSIZE				(3072)

#define __section(S)	__attribute__ ((__section__(#S)))
#define __init		__section(.init.text)
#define __initdata	__section(.init.data)
#define __initcode	__section(.init.code)

//------------------------------------------------------------------------------
//  Set DEBUG Macro
//------------------------------------------------------------------------------

#if defined(SYSLOG_ON)
#define SYSMSG printf
#else
#define SYSMSG(x, ...)
#endif

// Memory debug message
#if defined(SYSLOG_ON)
#define MEMMSG  printf
#else
#define MEMMSG(x, ...)
#endif

// UserDebug Message
#if 0
#define DBGOUT  printf
#else
#define DBGOUT(x, ...)
#endif

//------------------------------------------------------------------------------
//  Set global variables
//------------------------------------------------------------------------------

#if defined(__SET_GLOBAL_VARIABLES)

struct NX_SecondBootInfo *const __initdata pSBI =
    (struct NX_SecondBootInfo * const)BASEADDR_SRAM;
struct NX_SecondBootInfo *const __initdata pTBI =
    (struct NX_SecondBootInfo * const)BASEADDR_SRAM;
struct NX_GPIO_RegisterSet (*const __initdata pReg_GPIO)[1] =
    (struct NX_GPIO_RegisterSet (*const)[])PHY_BASEADDR_GPIOA_MODULE;
struct NX_ALIVE_RegisterSet *const pReg_Alive =
    (struct NX_ALIVE_RegisterSet * const)PHY_BASEADDR_ALIVE_MODULE;
struct NX_TIEOFF_RegisterSet *const __initdata pReg_Tieoff =
    (struct NX_TIEOFF_RegisterSet * const)PHY_BASEADDR_TIEOFF_MODULE;
struct NX_ECID_RegisterSet *const pReg_ECID =
    (struct NX_ECID_RegisterSet * const)PHY_BASEADDR_ECID_MODULE;
struct NX_CLKPWR_RegisterSet *const __initdata pReg_ClkPwr =
    (struct NX_CLKPWR_RegisterSet * const)PHY_BASEADDR_CLKPWR_MODULE;
struct NX_RSTCON_RegisterSet *const __initdata pReg_RstCon =
    (struct NX_RSTCON_RegisterSet * const)PHY_BASEADDR_RSTCON_MODULE;
struct NX_DREXSDRAM_RegisterSet *const pReg_Drex =
    (struct NX_DREXSDRAM_RegisterSet * const)PHY_BASEADDR_DREX_MODULE_CH0_APB;
struct NX_DDRPHY_RegisterSet *const pReg_DDRPHY =
    (struct NX_DDRPHY_RegisterSet * const)PHY_BASEADDR_DREX_MODULE_CH1_APB;
struct NX_RTC_RegisterSet *const pReg_RTC =
    (struct NX_RTC_RegisterSet * const)PHY_BASEADDR_RTC_MODULE;

struct NX_WDT_RegisterSet *const pReg_WDT =
    (struct NX_WDT_RegisterSet * const)PHY_BASEADDR_WDT_MODULE;

#else

extern struct NX_SecondBootInfo *const __initdata pSBI; // second boot info
extern struct NX_SecondBootInfo *const __initdata pTBI; // third boot info
extern struct NX_GPIO_RegisterSet (*const pReg_GPIO)[1];
extern struct NX_ALIVE_RegisterSet *const pReg_Alive;
extern struct NX_TIEOFF_RegisterSet *const __initdata pReg_Tieoff;
extern struct NX_ECID_RegisterSet *const pReg_ECID;
extern struct NX_CLKPWR_RegisterSet *const __initdata pReg_ClkPwr;
extern struct NX_RSTCON_RegisterSet *const __initdata pReg_RstCon;
extern struct NX_DREXSDRAM_RegisterSet *const pReg_Drex;
extern struct NX_DDRPHY_RegisterSet *const pReg_DDRPHY;
extern struct NX_RTC_RegisterSet *const pReg_RTC;
#endif

#endif //	__SYS_HEADER_H__
