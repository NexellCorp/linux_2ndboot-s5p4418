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


        .align 8
/*
 * start and end of BSS
 */

.globl __bss_start
.globl __bss_end__

/*
 * entry point of main function
 */
.global BootMain
.global SubCPUBoot

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
        b       SystemSleep
//        b       DynamicPllChange    //; for Samsung VD

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
        mov     r1, #0x3c0
        str     r1, [r0, #0x9c]                     //; disable pad holding

//;==================================================================
//; Set SRAM size
//;==================================================================
        ldr     r0, =0xC0067000                     //; ECID Base address
        ldr     r1, [r0, #0x0C]                     //; Read ECID_03 register.

        mov     r2, #0
        ldr     r0, =g_SRAM_SIZE
        cmp     r1, r2                              //; if (r1 == 0) r1 = nxp4330 else r1 = s5p4418
        ldreq   r1, =(16*1024)                      //; Set 16KB (nxp4330)
        ldrne   r1, =(32*1024)                      //; Set 32KB (s5p4418)
        str     r1, [r0]

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
        ldr     sp, =BASEADDR_SRAM
        add     r1, r1, #(INTERNAL_SRAM_SIZE/2)
        add     r2, r1, #(INTERNAL_SRAM_SIZE/2)     // half of sram
        mov     r3, #0
clearsram:
        str     r3, [r1], #4
        str     r3, [r1], #4
        str     r3, [r1], #4
        str     r3, [r1], #4
        cmp     r1, r2
        bne     clearsram
#else
        ldr     r1, =__bss_start                    // this is auto-relocated!
        ldr     r2, =__bss_end__                    // this is auto-relocated!

        mov     r3, #0x00000000                     // prepare zero to clear BSS

clbss_l:
        cmp     r1, r2                              // while not at end of BSS
        strlo   r3, [r1]                            // clear 32-bit BSS word
        addlo   r1, r1, #4                          // move to next
        blo     clbss_l
#endif

//;==================================================================
//; Setup stacks
//;==================================================================
Mainroot:

        msr     CPSR_c, #(Mode_SVC|I_Bit|F_Bit)

        //; Get value of internal sram size.
        ldr     r0, =g_SRAM_SIZE
        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
        add     sp, sp, r4                          //; Add to internal sram size.
        sub     sp, sp, #0x60
        mov     r0, r12

        bl      BootMain                            //; save this in register for possible long jump
        b       ExceptionV

//;==================================================================
//; PLL Change
//;==================================================================
        .align 4                                    //; below instruction number is 4, 16bytes

.global __pllchange
__pllchange:                                        //; r0:data r1:pll address r2:delay count
        str     r0, [r1]                            //; pll change start
pllchangedelayloop:                                 //; this code will be already within i-cache. no bus transaction will make
        subs    r2, r2, #1                          //; wait for pll change done
        bne     pllchangedelayloop
        bx      lr

//;==================================================================
//; Self-Refresh Service
//;==================================================================
.global enterSelfRefresh                            //; this code is call linux kernel, so here is virtual memory space.
.global sleepMain
.global vddPowerOff
SystemSleep:        //; r0:alive r1:drex

    //;-----------------------
    //;   Disable MMU
    //;-----------------------

DisableMMU:
        mrc     p15, 0, r1, c1, c0, 0   //; Read control register
        bic     r1, r1, #0x1            //; Disable MMU.
        bic     r1, r1, #0x1000         //; Disable IC.
        bic     r1, r1, #0x4            //; Dsiable DC.

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

        .align 4
PhysicalStart:

        //; Disable IRQ & FIQ.
        mrs     r0, CPSR
        orr     r0, r0, #(I_Bit|F_Bit)
        msr     CPSR_cxsf, r0

        //; Get value of internal sram size.
        ldr     r0, =g_SRAM_SIZE
        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
        add     sp, sp, r4                          //; Add to internal sram size.

        //; Goto sleepMain function.
        bl      sleepMain
        bl      vddPowerOff
        b       .                                   //; this time, core power will off and so cpu will die.


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


//;==================================================================
//; Invalidate entire branch predictor
//;==================================================================
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0               //; invalidate all instruction caches
        mcr     p15, 0, r0, c7, c5, 6               //; BPIALL - Invalidate entire branch predictor array

//;==================================================================
//; Cache Invalidation code for Cortex-A9
//;==================================================================

        //;----------------------------------------------------------------------
        //; Invalidate Data/Unified Caches
        //;----------------------------------------------------------------------
        mrc     p15, 1, r0, c0, c0, 1               //; Read CLIDR
        ands    r3, r0, #0x7000000
        mov     r3, r3, lsr #23                     //; Total cache levels << 1
        beq     finished
        mov     r10, #0                             //; r10 holds current cache level << 1

loop1:
        add     r2, r10, r10, lsr #1                //; r2 holds cache "Set" position
        mov     r1, r0, lsr r2                      //; Bottom 3 bits are the Cache-type for this level
        and     r1, r1, #7                          //; Get those 3 bits alone
        cmp     r1, #2
        blt     skip                                //; No cache or only instruction cache at this level

        mcr     p15, 2, r10, c0, c0, 0              //; Write the Cache Size selection register
        mov     r1, #0
        mcr     p15, 0, r1, c7, c5, 4               //; PrefetchFlush to sync the change to the CacheSizeID reg
        mrc     p15, 1, r1, c0, c0, 0               //; Reads current Cache Size ID register
        and     r2, r1, #0x7                        //; Extract the line length field
        add     r2, r2, #4                          //; Add 4 for the line length offset (log2 16 bytes)
        ldr     r6, =0x3FF
        ands    r6, r6, r1, lsr #3                  //; r6 is the max number on the way size (right aligned)
        clz     r5, r6                              //; r5 is the bit position of the way size increment
        ldr     r7, =0x00007FFF
        ands    r7, r7, r1, lsr #13                 //; r7 is the max number of the index size (right aligned)

loop2:
        mov     r9, r6                              //; r9 working copy of the max way size (right aligned)

loop3:
        orr     r11, r10, r9, lsl r5                //; Factor in the Way number and cache number into r11
        orr     r11, r11, r7, lsl r2                //; Factor in the Set number
        mcr     p15, 0, r11, c7, C6, 2              //; Clean and Invalidate by set/way
        subs    r9, r9, #1                          //; Decrement the Way number
        bge     loop3
        subs    r7, r7, #1                          //; Decrement the Set number
        bge     loop2

skip:
        add     r10, r10, #2                        //; increment the cache number
        cmp     r3, r10
        bgt     loop1

finished:
        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4              //; drain write buffer

        mcr     p15, 0, r0, c8, c7, 0               //; invalidate I + D TLBs
        mcr     p15, 0, r0, C2, c0, 2               //; TTB control register


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

        mov     r0, #0xFF
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
        ldr     r0, =g_SRAM_SIZE
        ldr     r4, [r0]

        //; Set stack pointer
        ldr     sp, =BASEADDR_SRAM
        add     sp, sp, r4                          //; Add to internal sram size.

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


#if 0
.global BurstWrite
BurstWrite:
        push    {r2-r9, lr}
        mvn     r2, r1
        mvn     r3, r1
        mvn     r4, r1
        mvn     r5, r1
        mov     r6, r1
        mvn     r7, r1
        mvn     r8, r1
        mvn     r9, r1
        stmia   r0!, {r2-r9}
        pop     {r2-r9, pc}

.global BurstRead
BurstRead:
        push    {r2-r9, lr}
        ldmia   r0!, {r2-r9}
        stmia   r1!, {r2-r9}
        pop     {r2-r9, pc}
#endif

//;==================================================================
//; Cache control
//;==================================================================

.global flushICache
flushICache:
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0

        mov     pc, lr
        b       .


.global enableICache
enableICache:
        cmp     r0, #1
        mrc     p15, 0, r1, c1, c0, 0
        bicne   r1, r1, #0x1000
        orreq   r1, r1, #0x1000
        mcr     p15, 0, r1, c1, c0, 0

        mov     pc, lr
        b       .


//;==================================================================
//; Global data
//;==================================================================
.global g_SRAM_SIZE
g_SRAM_SIZE:
        .long   (16*1024)                           //;4330 : 16KB, 4418 : 32KB

//;==================================================================
//; End of startup.s
//;==================================================================
