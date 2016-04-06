////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  Nexell informs that this code and information is provided "as is" base
//  and without warranty of any kind, either expressed or implied, including
//  but not limited to the implied warranties of merchantability and/or fitness
//  for a particular puporse.
//
//
//  Module          :
//  File            : Startup.S
//  Description     :
//  Author          : Hans
//  History         :
//                          2013-01-10      Hans
////////////////////////////////////////////////////////////////////////////////
#include "nx_pyrope.h"
#include "cfgBootDefine.h"


        .align
/*
 * start and end of BSS
 */

.globl __bss_start__
.globl __bss_end__

/*
 * entry point of main function
 */
.global BootMain
.global SubCPUBoot
.global ExceptionV
.global SystemSleep

        .text
//;==================================================================
//; Vectors
//;==================================================================
.global Vectors
Vectors:
        b       Reset_Handler       //; 00 - Reset
        b       ExceptionV          //; 04 - Undefined instructions
        b       .                   //; 08 - SWI instructions
        b       ExceptionV          //; 0C - Instruction fetch aborts
        b       ExceptionV          //; 10 - Data access aborts
        b       .                   //; 14 - Reserved (was address exception)
        b       .                   //; 18 - IRQ interrupts
        b       .                   //; 1C - FIQ interrupts
.global Sleep
Sleep:
        b       SystemSleep         //; 20

BuildInfo:
        .word   0x44180602          //; 24, , Chip name - 4418, Build num - v0.6.01

//        b       DynamicPllChange    //; for Samsung VD

//.global ExceptionV
ExceptionV:
        msr     CPSR_c, #(Mode_SVC|I_Bit|F_Bit)

        mrc     p15, 0, r0, c1, c0, 0               //; Read CP15 System Control register
        bic     r0, r0, #(BIT1_V|BIT1_I|BIT1_Z)     //; Clear V:13, I:12, Z:11 bit to disable High Vector, I-Cache, Branch Prediction
        bic     r0, r0, #(BIT1_C|BIT1_M)            //; Clear C:2, M:0 bit to disable D Cache, MMU
        mcr     p15, 0, r0, c1, c0, 0               //; Write value back to CP15 System Control register

        mov     r1, #0xC0000000
        orr     r1, r1, #0x00010000
        ldr     r0, [r1, #0x27C]                    //; Get Boot Config

        bic     r0, r0, #0x7                        //; clear boot mode
        orr     r0, r0, #0x6                        //; set boot mode to USB

        mov     r1, #0x1C4                          //; rom boot jump address
        bx      r1


Reset_Handler:
//;        mcr        p15, 0 r0, c8, c9, 0            //; set debug break;

        mrc     p15, 0, r12, c0, c0, 5              //; Get our cpu id
        ands    r12, r12, #0xF                      //; Save CPU id
        bne     CPUBRINGUP

//;==================================================================
//; Release pad holding
//;==================================================================
        ldr     r0, =0xc0010800                     //; alive base address
        mov     r1, #0x01
        str     r1, [r0, #0x00]                     //; open write gate
//        mov     r1, #0x3C0
        mov     r1, #0x3FC
        str     r1, [r0, #0x9c]                     //; disable pad holding

//;==================================================================
//; Set SRAM size
//;==================================================================
#if 0
        ldr     r0, =0xC0067000                     //; ECID Base address
        ldr     r1, [r0, #0x0C]                     //; Read ECID_03 register.

        mov     r2, #0
        ldr     r0, =g_SRAM_SIZE
        cmp     r1, r2                              //; if (r1 == 0) r1 = nxp4330 else r1 = s5p4418
        ldreq   r1, =(16*1024)                      //; Set 16KB (nxp4330)
        ldrne   r1, =(32*1024)                      //; Set 32KB (s5p4418)
        str     r1, [r0]
#endif

//;==================================================================
//; Set SMP Start Address to Invalid
//;==================================================================
        mov     r0, #0xC0000000                     //; peri base
        orr     r0, r0, #0x00010800                 //; Alive offset
        mov     r1, #1
        str     r1, [r0]                            //; power gate open
        mov     r1, #0xFFFFFFFF                     //; mark to invalid address
        str     r1, [r0, #0x41C]                    //; set invalid jump address

//;==================================================================
//; Clear SRAM
//;==================================================================
#if 0
        ldr     r1, =__bss_start__                  // this is auto-relocated!

        ldr     r0, =g_SRAM_SIZE
        ldr     r3, [r0]                            // get SRAM size.
        mov     r4, #0xFF000000
        orr     r4, r4, #0x00FF0000
        sub     r2, r3, #(4096)                     // sram_size - stack_size
        orr     r2, r2, r4
#else

        ldr     r1, =__bss_start__                  // this is auto-relocated!
        ldr     r2, =__bss_end__                    // this is auto-relocated!
#endif

        mov     r3, #0x00000000                     // prepare zero to clear BSS

clbss_l:
        cmp     r1, r2                              // while not at end of BSS
        strlo   r3, [r1]                            // clear 32-bit BSS word
        addlo   r1, r1, #4                          // move to next
        blo     clbss_l

//;==================================================================
//; Setup stacks
//;==================================================================
Mainroot:

        msr     CPSR_c, #(Mode_SVC|I_Bit|F_Bit)

        //; Get value of internal sram size.
//        ldr     r0, =g_SRAM_SIZE
//        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
//        add     sp, sp, r4                          //; Add to internal sram size.
        add     sp, sp, #INTERNAL_SRAM_SIZE         //; Add to internal sram size.
        sub     sp, sp, #0x60
        mov     r0, r12

        bl      BootMain                            //; save this in register for possible long jump
        b       ExceptionV




#if 0   //; for Samsung VD
//; void real_change_pll(u32 *clkpwr_reg, u32 *sram_base, void *drex_data, u32 pll_data)
.global real_change_pll
DynamicPllChange:
        push    {r12}
        mov     r12, sp                             //; save org stack value
        mov     sp, pc
        bic     sp, sp, #0x0000FF00
        bic     sp, sp, #0x000000FF
        orr     sp, sp, #0x00008000
        push    {r12, lr}                           //; save register
        bl      real_change_pll
        pop     {r12, lr}                           //; restore register
        mov     sp, r12                             //; restore stack value
        pop     {r12}
        mov     pc, lr                              //; return

        b       .
#endif

//;===================================================================
//; CPU 1, 2, 3 initialize
//;===================================================================
CPUBRINGUP:
        mrc     p15, 0, r0, c1, c0, 0               //; Read CP15 System Control register
        bic     r0, r0, #(BIT1_V|BIT1_I|BIT1_Z)     //; Clear V:13, I:12, Z:11 bit to disable High Vector, I-Cache, Branch Prediction
        bic     r0, r0, #(BIT1_C|BIT1_M)            //; Clear C:2, M:0 bit to disable D Cache, MMU
        mcr     p15, 0, r0, c1, c0, 0               //; Write value back to CP15 System Control register

        msr     CPSR_c, #(Mode_SVC|I_Bit|F_Bit)     //; Set CPU to SVC32 Mode

//;===================================================================
//; Set Vector Base Address Register (VBAR) to point to this application's vector table
//;===================================================================
        ldr     r0, =Vectors
        mcr     p15, 0, r0, c12, c0, 0              //; Write Secure or Non-secure Vector Base Address
        mrc     p15, 0, r0, c1, c0, 0               //; Read SCTLR
        bic     r0, #(1<<13)                        //; Clear V-bit
        mcr     p15, 0, r0, c1, c0, 0               //; Write SCTLR


        bl      Invalidate_entire_branch_predictor
        bl      InvalidateDataCache

//;==================================================================
//; Set SMP Mode for CPU 1, 2, 3
//;==================================================================
        //; Ensure interrupts can get through the priority mask
        mov     r1, #0xF0000000
        mov     r0, #0xFF
        str     r0, [r1, #0x104]

        mov     r0, #1
        orr     r2, r1, #0x1000
        str     r0, [r2]                            //; distributor enable

        str     r0, [r1, #0x100]                    //; CPU IF enable

        mov     r0, #0x00FF
        orr     r0, #0xFF00
        str     r0, [r2, #100]                      //; distributor set enable

        //;  Configure CPU
        mrc     p15, 0, r0, c1, c0, 1
        //;  Only try to set SMP mode if not yet set - cannot write AuxCtrl if in normal world
        tst     r0, #(1 << 6)                       //;  SMP bit
        orreq   r0, r0, #(1 << 6)                   //;  Set SMP bit in aux control register
        orreq   r0, r0, #0x7                        //;  L2C control
        mcreq   p15, 0, r0, c1, c0, 1               //;  write Aux Control Register

        //; Get value of internal sram size.
//        ldr     r0, =g_SRAM_SIZE
//        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
//        add     sp, sp, r4                          //; Add to internal sram size.
        add     sp, sp, #INTERNAL_SRAM_SIZE         //; Add to internal sram size.

        mov     r1, #0x20
        sub     r2, r12, #1
        and     r2, r2, #3
        mul     r1, r1, r2
        sub     sp, sp, r1

        mov     r0, r12
        b       SubCPUBoot
//;==================================================================
//; CPU 1, 2, 3 wait
//;==================================================================

.global __WFI
__WFI:
        WFI

        mov     r0, #0xC0000000                     //; peri base
        orr     r0, r0, #0x00010800                 //; Alive offset
        ldr     r2, [r0]                            //; save power gating status
        mov     r1, #1
        str     r1, [r0]                            //; power gate open
        ldr     r3, [r0, #0x41C]                    //; get jump address
        cmp     r3, #0xFFFFFFFF                     //; check if invalid address
        bxne    r3                                  //; if valid address, get jump.
//        streq   r2, [r0]                            //; restore power gating status
        beq     __WFI


//;==================================================================
//; Self-Refresh Service
//;==================================================================
#if (CONFIG_SUSPEND_RESUME == 1)
.global enterSelfRefresh                            //; this code is call linux kernel, so here is virtual memory space.
.global sleepMain
.global vddPowerOff
.global SystemSleep
SystemSleep:        //; r0:alive r1:drex

    //;-----------------------
    //;   Disable MMU
    //;-----------------------

DisableMMU:
        mrc     p15, 0, r1, c1, c0, 0   //; Read control register
        bic     r1, r1, #0x5            //; Disable MMU & DC.
        bic     r1, r1, #0x1000         //; Disable IC.

        ldr     r0, =PhysicalStart
        cmp     r0, #0                  //; make sure no stall on "mov pc,r0" below

        //; Disable the MMU.
        //;
        mcr     p15, 0, r1, c1, c0, 0

        //; Jump to the physical address of the 'PhysicalStart' label.
        //;
        mov     pc, r0                  //;  & jump to new physical address
        nop
        nop
        nop

        //; MMU & caches now disabled.
        //;

        .align 5
PhysicalStart:

        //; Disable IRQ & FIQ.
        mrs     r0, CPSR
        orr     r0, r0, #(I_Bit|F_Bit)
        msr     CPSR_cxsf, r0

        //; Get value of internal sram size.
//        ldr     r0, =g_SRAM_SIZE
//        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
//        add     sp, sp, r4                          //; Add to internal sram size.
        add     sp, sp, #INTERNAL_SRAM_SIZE         //; Add to internal sram size.

        //; Goto sleepMain function.
        bl      sleepMain
#else

SystemSleep:        //; r0:alive r1:drex
#endif
//        b       .                                   //; this time, core power will off and so cpu will die.


//;==================================================================
//; Global data
//;==================================================================
#if 0
.global g_SRAM_SIZE
g_SRAM_SIZE:
        .word   (32*1024)                           //;4330 : 16KB, 4418 : 32KB
#endif
        .ltorg
//;==================================================================
//; End of startup.s
//;==================================================================
