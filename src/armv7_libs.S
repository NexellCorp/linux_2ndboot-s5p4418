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
//  File            : armv7_libs.S
//  Description     :
//  Author          : Hans
//  History         :
//                          2015-07-16      Hans
////////////////////////////////////////////////////////////////////////////////
#include "nx_pyrope.h"
#include "cfgBootDefine.h"


//;==================================================================
//; PLL Change
//;==================================================================
        .align 5                                    //; below instruction number is 8, 32bytes

.global __pllchange
__pllchange:                                        //; r0:data r1:pll address r2:delay count
        mov     r3, #0x100                          //; for icache prefetch
pllchangedelayloop:                                 //; this code will be already within i-cache. no bus transaction will make
        subs    r3, r3, #1
        streq   r0, [r1]                            //; pll change start
        moveqs  r3, r2                              //; real delay time set
        cmpne   r3, #0x100
        bne     pllchangedelayloop
        bx      lr
        nop

        .align 5
//;==================================================================
//; Cache Invalidation code for Cortex-A9
//;==================================================================

//;==================================================================
//; Invalidate entire branch predictor
//;==================================================================
.global Invalidate_entire_branch_predictor
Invalidate_entire_branch_predictor:
        mov     r0, #0
        mcr     p15, 0, r0, c7, c5, 0               //; invalidate all instruction caches
        mcr     p15, 0, r0, c7, c5, 6               //; BPIALL - Invalidate entire branch predictor array
        bx      lr

.global InvalidateDataCache
InvalidateDataCache:
        //;----------------------------------------------------------------------
        //; Invalidate Data/Unified Caches
        //;----------------------------------------------------------------------
        mrc     p15, 1, r0, c0, c0, 1               //; Read CLIDR
        ands    r3, r0, #0x07000000
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

        bx      lr

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
        mrc     p15, 0, r0, c1, c0, 0
        bicne   r0, r0, #BIT1_I
        orreq   r0, r0, #BIT1_I
        mcr     p15, 0, r0, c1, c0, 0

        mov     pc, lr
        b       .


