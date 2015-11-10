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
//  File            : SecondBoot.c
//  Description     :
//  Author          : Hans
//  History         :
//          2013-01-10  Hans
////////////////////////////////////////////////////////////////////////////////

#define __SET_GLOBAL_VARIABLES

#include "sysHeader.h"


#define CPU_BRINGUP_CHECK   (1)
#if defined(CHIPID_NXP4330)
#define AUTO_DETECT_EMA     (0)
#else
#define AUTO_DETECT_EMA     (1)
#endif
#define EMA_VALUE           (1)     // Manual setting - 1(001): 1.1V, 3(011): 1.0V

extern U32      __calc_crc(void *addr, int len);
extern void     DMC_Delay(int milisecond);

extern void     flushICache(void);
extern void     enableICache(CBOOL enable);

#if (CONFIG_SUSPEND_RESUME == 1)
extern void     enterSelfRefresh(void);
extern void     exitSelfRefresh(void);
#endif

#if (CONFIG_BUS_RECONFIG == 1)
extern void     setBusConfig(void);
#endif

extern CBOOL    iUSBBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iUARTBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSPIBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSDXCBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iNANDBOOTEC(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSDXCFSBOOT(struct NX_SecondBootInfo * pTBI);
extern void     initClock(void);
#ifdef MEM_TYPE_DDR3
extern void     init_DDR3(U32);
#endif
#ifdef MEM_TYPE_LPDDR23
extern void     init_LPDDR3(U32);
#endif

#if defined( INITPMIC_YES )
extern void     initPMIC(void);
#endif
extern void     buildinfo(void);

extern void     printClkInfo(void);

extern void     ResetCon(U32 devicenum, CBOOL en);

extern U32  g_GateCycle;
extern U32  g_GateCode;
extern U32  g_RDvwmc;
extern U32  g_WRvwmc;

//------------------------------------------------------------------------------
#if (AUTO_DETECT_EMA == 1)
struct asv_tb_info {
    int ids;
    int ro;
};

static struct asv_tb_info asv_tables[] = {
    [0] = { .ids = 10, .ro = 110, },
    [1] = { .ids = 15, .ro = 130, },
    [2] = { .ids = 20, .ro = 140, },
    [3] = { .ids = 50, .ro = 170, },
    [4] = { .ids = 50, .ro = 170, },
};

#define ASV_ARRAY_SIZE  (int)(sizeof(asv_tables)/sizeof(asv_tables[0]))

static inline U32 MtoL(U32 data, int bits)
{
    U32 result = 0, mask = 1;
    int i = 0;

    for (i = 0; i < bits ; i++) {
        if (data & (1 << i))
            result |= mask << (bits-i-1);
    }

    return result;
}

CBOOL isEMA3(U32 ecid_1, U32 ecid_2)
{
    int field = 0;

    if (ecid_2 & 0x1)
    {
        int gs = MtoL((ecid_2>>1) & 0x07, 3);
        int ag = MtoL((ecid_2>>4) & 0x0F, 4);

        field = (ag - gs);
    }
    else
    {
        struct asv_tb_info *tb = &asv_tables[0];
        int ids;
        int ro;
        int ids_L, ro_L;
        int i = 0;

        ids = MtoL((ecid_1>>16) & 0xFF, 8);
        ro  = MtoL((ecid_1>>24) & 0xFF, 8);

        /* find ids Level */
        for (i = 0; i < ASV_ARRAY_SIZE; i++) {
            if (tb[i].ids >= ids)
                break;
        }
        ids_L = i < ASV_ARRAY_SIZE ? i : (ASV_ARRAY_SIZE-1);

        /* find ro Level */
        for (i = 0; i < ASV_ARRAY_SIZE; i++) {
            if (tb[i].ro >= ro)
                break;
        }
        ro_L = i < ASV_ARRAY_SIZE ? i : (ASV_ARRAY_SIZE-1);

        /* find Lowest ASV Level */
        field = ids_L > ro_L ? ro_L : ids_L;
    }

    return (field > 2 ? CTRUE : CFALSE);
}
#endif  // #if (AUTO_DETECT_EMA == 1)

void setEMA(void)
{
#if (AUTO_DETECT_EMA == 1)
    U32 ecid_1, ecid_2;
#endif
    U32 ema, temp;

#if (AUTO_DETECT_EMA == 1)
    ecid_1 = ReadIO32(&pReg_ECID->ECID[1]);
    ecid_2 = ReadIO32(&pReg_ECID->ECID[2]);

    if ( isEMA3(ecid_1, ecid_2) )
        ema = 3;
    else
        ema = 1;
#else
    ema = EMA_VALUE;                            //; cortex-A9 L1 Cache EMA value (1: 1.1V, 3: 1.0V)
#endif

    enableICache(CFALSE);
    flushICache();

    // L2 Cache
    temp = ReadIO32( &pReg_Tieoff->TIEOFFREG[0] ) & ~(7<<22);
    temp |= (ema<<22);
    WriteIO32( &pReg_Tieoff->TIEOFFREG[0],  temp );

    // L1 Cache
    temp = ReadIO32( &pReg_Tieoff->TIEOFFREG[1] ) & ~(7<<2);
    temp |= (ema<<2);
    WriteIO32( &pReg_Tieoff->TIEOFFREG[1],  temp );

    enableICache(CTRUE);

    printf("EMA VALUE : %s\r\n", (ema == 3 ? "011" : "001") );

    return;
}

//------------------------------------------------------------------------------
#if (CONFIG_SUSPEND_RESUME == 1)
static void dowakeup(void)
{
    U32 fn, sign, phy, crc, len;
    void (*jumpkernel)(void) = 0;

    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);             // open alive power gate
    sign = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
    fn   = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE1);
    phy  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE3);
    crc  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE2);
    len  = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE4);
    jumpkernel = (void (*)(void))fn;

    WriteIO32(&pReg_Alive->ALIVESCRATCHRSTREG,  0xFFFFFFFF);
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST1,    0xFFFFFFFF);
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST2,    0xFFFFFFFF);
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST3,    0xFFFFFFFF);
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST4,    0xFFFFFFFF);

    if (SUSPEND_SIGNATURE == sign)
    {
        U32 ret = __calc_crc((void*)phy, len);

//        SYSMSG("CRC: 0x%08X FN: 0x%08X phy: 0x%08X len: 0x%08X ret: 0x%08X\r\n", crc, fn, phy, len, ret);
        printf("CRC: 0x%08X FN: 0x%08X phy: 0x%08X len: 0x%08X ret: 0x%08X\r\n", crc, fn, phy, len, ret);
        if (fn && (crc == ret))
        {
//            U32 temp = 0x100000;
            printf("It's WARM BOOT\r\nJump to Kernel!\r\n");
//            while(DebugIsBusy() && temp--);
            while(!DebugIsUartTxDone());
            jumpkernel();
        }
    }
    else
    {
        printf("Suspend Signature is different\r\nRead Signature :0x%08X\r\n", sign);
    }

    printf("It's COLD BOOT\r\n");
}
#endif  // #if (CONFIG_SUSPEND_RESUME == 1)

CBOOL TurnOnCPUnonedelay( U32 CPUID )
{
    if( (CPUID > 3) || (CPUID == 0) )
        return CFALSE;

#if (CPU_BRINGUP_CHECK == 1)
    // high vector;
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[0],  ((1<<CPUID)<<18) );
#else
    // low vector;
    ClearIO32( &pReg_Tieoff->TIEOFFREG[0],  ((1<<CPUID)<<18) );
#endif

    // reset assert
    ResetCon(CPUID, CTRUE);

    // CPUCLKOFF Set to 1 except CPU0
    SetIO32  ( &pReg_Tieoff->TIEOFFREG[1],  ((1<<CPUID)<<(37-32)) );

    // reset negate
    ResetCon(CPUID, CFALSE);

    // CPUCLKOFF Set to 0 except CPU0
    // supply clock to CPUCLK real startup cpu
    ClearIO32( &pReg_Tieoff->TIEOFFREG[1],  ((1<<CPUID)<<(37-32)) );

    return CTRUE;
}

void __WFI(void);
#if (CPU_BRINGUP_CHECK == 1)
#define CPU_ALIVE_FLAG_ADDR    0xC0010238
void SubCPUBoot( U32 CPUID )
{
    volatile U32 *aliveflag = (U32*)CPU_ALIVE_FLAG_ADDR;
    DebugPutch('0'+CPUID);
    *aliveflag = 1;

    __WFI();
}
#endif


#if (CONFIG_SUSPEND_RESUME == 1)
void vddPowerOff( void )
{
    ClearIO32( &pReg_ClkPwr->PWRCONT,           (0xFF   <<   8) );  //; Clear USE_WFI & USE_WFE bits for STOP mode.

    WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x00000001 );       //; alive power gate open

    //----------------------------------
    // Save leveling & training values.
#if 1
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST5,    0xFFFFFFFF);        // clear - ctrl_shiftc
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST6,    0xFFFFFFFF);        // clear - ctrl_offsetC
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST7,    0xFFFFFFFF);        // clear - ctrl_offsetr
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST8,    0xFFFFFFFF);        // clear - ctrl_offsetw

    WriteIO32(&pReg_Alive->ALIVESCRATCHSET5,    g_GateCycle);       // store - ctrl_shiftc
    WriteIO32(&pReg_Alive->ALIVESCRATCHSET6,    g_GateCode);        // store - ctrl_offsetc
    WriteIO32(&pReg_Alive->ALIVESCRATCHSET7,    g_RDvwmc);          // store - ctrl_offsetr
    WriteIO32(&pReg_Alive->ALIVESCRATCHSET8,    g_WRvwmc);          // store - ctrl_offsetw
#endif

    WriteIO32( &pReg_Alive->VDDOFFCNTVALUERST,  0xFFFFFFFF );       //; clear delay counter, refrence rtc clock
    WriteIO32( &pReg_Alive->VDDOFFCNTVALUESET,  0x00000001 );       //; set minimum delay time for VDDPWRON pin. 1 cycle per 32.768Kh (about 30us)

    __asm__ __volatile__ ("cpsid i");                               //; core interrupt off.
    WriteIO32( &pReg_Alive->VDDCTRLSETREG,      0x000003FC );       //; Retention off (Pad hold off)
    WriteIO32( &pReg_Alive->VDDCTRLRSTREG,      0x00000001 );       //; vddpoweron off, start counting down.

    DMC_Delay(600);     // 600 : 110us, Delay for Pending Clear. When CPU clock is 400MHz, this value is minimum delay value.

    WriteIO32( &pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF );         //; all alive pend pending clear until power down.
//    WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x00000000 );       //; alive power gate close

    while(1)
    {
//        SetIO32  ( &pReg_ClkPwr->PWRMODE,       (0x1    <<   1) );  //; enter STOP mode.
        WriteIO32( &pReg_ClkPwr->PWRMODE,       (0x1    <<   1) );  //; enter STOP mode.
        __asm__ __volatile__ ("wfi");                               //; now real entering point to stop mode.
    }                                                               //; this time, core power will off and so cpu will die.
}
#endif  // #if (CONFIG_SUSPEND_RESUME == 1)

void sleepMain( void )
{
#if 0
//    WriteIO32( &clkpwr->PLLSETREG[1],   0x100CC801 );       //; set PLL1 - 800Mhz
    WriteIO32( &pReg_ClkPwr->PLLSETREG[1],   0x100CC802 );       //; set PLL1 - 400Mhz

    __pllchange(pReg_ClkPwr->PWRMODE | 0x1<<15, &pReg_ClkPwr->PWRMODE, 0x20000); //533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

    {
        volatile U32 delay = 0x100000;
        while((pReg_ClkPwr->PWRMODE & 0x1<<15) && delay--);    // it's never checked here, just for insure
        if( pReg_ClkPwr->PWRMODE & 0x1<<15 )
        {
//            printf("pll does not locked\r\nsystem halt!\r\r\n");    // in this point, it's not initialized uart debug port yet
            while(1);        // system reset code need.
        }
    }
#endif

//    DebugInit();

#if (CONFIG_SUSPEND_RESUME == 1)
    enterSelfRefresh();
    vddPowerOff();
#endif
}


//------------------------------------------------------------------------------
void BootMain( U32 CPUID )
{
    struct NX_SecondBootInfo    TBI;
    struct NX_SecondBootInfo * pTBI = &TBI;    // third boot info
    CBOOL Result = CFALSE;
#if (CONFIG_SUSPEND_RESUME == 1)
    U32 sign;
#endif
    U32 isResume = 0;

    CPUID = CPUID;

//    DebugInit();

#if (CONFIG_SUSPEND_RESUME == 1)
    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);
    sign = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
    if ((SUSPEND_SIGNATURE == sign) && ReadIO32(&pReg_Alive->WAKEUPSTATUS))
    {
        isResume = 1;
    }

    //--------------------------------------------------------------------------
    // Initialize PMIC device.
    //--------------------------------------------------------------------------
#if defined( INITPMIC_YES )
    if (isResume == 0)
        initPMIC();
#endif
#else

    //--------------------------------------------------------------------------
    // Initialize PMIC device.
    //--------------------------------------------------------------------------
#if defined( INITPMIC_YES )
    initPMIC();
#endif
#endif  //#if (CONFIG_SUSPEND_RESUME == 1)

#if 0
    regvalue  = *(volatile U32 *)PHY_BASEADDR_RSTCON_MODULE;
    regvalue |= 1UL<<RESETINDEX_OF_DISPLAYTOP_MODULE_i_Top_nRST;
    regvalue |= 1UL<<RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST;
    *(volatile U32 *)PHY_BASEADDR_RSTCON_MODULE = regvalue;

    *(volatile U32 *)(0xc01023c0)       = (U32)0;       // mlc0
    *(volatile U32 *)(0xc01027c0)       = (U32)0;       // mlc1

    *(volatile U32 *)(0xc0102Bc0+4)     = (U32)0x1C;    // dpc0
    *(volatile U32 *)(0xc0102Fc0+4)     = (U32)0x1C;    // dpc1

    *(volatile U32 *)(0xc0108000+4)     = (U32)0x1c;    // lvds
    *(volatile U32 *)(0xc0109000+4)     = (U32)0x1c;    // hdmi
    *(volatile U32 *)(0xc0105000+4)     = (U32)0x1c;    // mipi-dsi

    *(volatile U32 *)(0xc0109000)       = 1UL<<3;       // hdmi link clkgen enable

    *(volatile U32 *)(0xc0109400+0x7C)  = 0<<7;         // hdmi phy off
    *(volatile U32 *)(0xc0109400+0x7C)  = 0<<7;         // twice every write
    *(volatile U32 *)(0xc0109400+0x74)  = 0xFFFFFFFF;   // hdmi phy off
    *(volatile U32 *)(0xc0109400+0x74)  = 0xFFFFFFFF;   // twice every write

    *(volatile U32 *)(0xc0109000)       = 0;            // hdmi link clkgen disable

    regvalue &= ~(1UL<<RESETINDEX_OF_DISPLAYTOP_MODULE_i_Top_nRST);
    regvalue &= ~(1UL<<RESETINDEX_OF_DISPLAYTOP_MODULE_i_DualDisplay_nRST);

    *(volatile U32 *)(PHY_BASEADDR_CLKGEN0_MODULE+4) = (U32)0x1C;       // timer01
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN1_MODULE+4) = (U32)0x1C;       // timer02
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN2_MODULE+4) = (U32)0x1C;       // timer03
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN3_MODULE+4) = (U32)0x1C;       // pwm1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN4_MODULE+4) = (U32)0x1C;       // pwm2
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN5_MODULE+4) = (U32)0x1C;       // pwm3
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN6_MODULE)   = (U32)0x0;        // i2c0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN7_MODULE)   = (U32)0x0;        // i2c1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN8_MODULE)   = (U32)0x0;        // i2c2

    *(volatile U32 *)(PHY_BASEADDR_CLKGEN9_MODULE+4)  = (U32)0x1C;      // mipi-csi

    *(volatile U32 *)(PHY_BASEADDR_CLKGEN10_MODULE+4) = (U32)0x1C;      // gmac
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN11_MODULE+4) = (U32)0x1C;      // spdif tx
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN12_MODULE)   = (U32)0x0;       // mpeg ts
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN13_MODULE+4) = (U32)0x1C;      // pwm0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN14_MODULE+4) = (U32)0x1C;      // timer0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN15_MODULE+4) = (U32)0x1C;      // i2s0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN16_MODULE+4) = (U32)0x1C;      // i2s1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN17_MODULE+4) = (U32)0x1C;      // i2s2
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN18_MODULE+4) = (U32)0x1C;      // sdmmc0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN19_MODULE+4) = (U32)0x1C;      // sdmmc1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN20_MODULE+4) = (U32)0x1C;      // sdmmc2
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN21_MODULE)   = (U32)0x0;       // gpu
//    *(volatile U32 *)(PHY_BASEADDR_CLKGEN22_MODULE+4) = (U32)0x1C;      // uart0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN23_MODULE+4) = (U32)0x1C;      // uart1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN24_MODULE+4) = (U32)0x1C;      // uart2
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN25_MODULE+4) = (U32)0x1C;      // uart3
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN26_MODULE+4) = (U32)0x1C;      // uart4
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN27_MODULE+4) = (U32)0x1C;      // uart5

    *(volatile U32 *)(PHY_BASEADDR_CLKGEN28_MODULE)   = (U32)0x0;       // deinterlace
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN29_MODULE+4) = (U32)0x1C;      // ppm
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN30_MODULE+4) = (U32)0x1C;      // vip0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN31_MODULE+4) = (U32)0x1C;      // vip1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN32_MODULE+4) = (U32)0x1C;      // ehci
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN33_MODULE)   = (U32)0x0;       // mpeg
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN34_MODULE)   = (U32)0x0;       // crypto
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN35_MODULE)   = (U32)0x0;       // scaler
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN36_MODULE)   = (U32)0x0;       // pdm
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN37_MODULE+4) = (U32)0x1C;      // spi0
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN38_MODULE+4) = (U32)0x1C;      // spi1
    *(volatile U32 *)(PHY_BASEADDR_CLKGEN39_MODULE+4) = (U32)0x1C;      // spi2

//    WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x1 );  // open alive power gate
//    WriteIO32( &pReg_Alive->PMUNPWRUP,          0x3 );  // mpeg & 3d power down
//    WriteIO32( &pReg_Alive->PMUNPWRUPPRE,       0x3 );  // mpeg & 3d pre power down
//    WriteIO32( &pReg_Alive->PMUNISOLATE,        0x0 );  // mpeg & 3d power isolating
#endif

    initClock();

    //--------------------------------------------------------------------------
    // Debug Console
    //--------------------------------------------------------------------------
    DebugInit();


    //--------------------------------------------------------------------------
    // build information. version, build time and date
    //--------------------------------------------------------------------------
    buildinfo();

    //--------------------------------------------------------------------------
    // set EMA value
    //--------------------------------------------------------------------------
    setEMA();

    //--------------------------------------------------------------------------
    // print clock information
    //--------------------------------------------------------------------------
//    printClkInfo();



#if (CPU_BRINGUP_CHECK == 1)
{
    volatile U32 *aliveflag = (U32*)CPU_ALIVE_FLAG_ADDR;
    int CPUNumber, retry = 0;

    for(CPUNumber = 1; CPUNumber < 4; )
    {
        register volatile U32 delay;
        *aliveflag = 0;
        delay = 0x10000;
        TurnOnCPUnonedelay(CPUNumber);
        while((*aliveflag == 0) && (--delay));
        if(delay == 0)
        {
            if(retry > 3)
            {
                printf("maybe cpu %d is dead. -_-;\r\n", CPUNumber);
                retry = 0;
                CPUNumber++;
            }
            else
            {
                printf("cpu %d is not bringup, retry\r\n", CPUNumber);
                retry++;
            }
        }
        else
        {
            retry = 0;
            CPUNumber++;
        }
    }
}
#else
    TurnOnCPUnonedelay(1);
    TurnOnCPUnonedelay(2);
    TurnOnCPUnonedelay(3);
#endif

#if (CONFIG_SUSPEND_RESUME == 1)
#ifdef MEM_TYPE_DDR3
    init_DDR3(isResume);
#endif
#ifdef MEM_TYPE_LPDDR23
    init_LPDDR3(isResume);
#endif

    if (isResume)
    {
        exitSelfRefresh();
    }

    SYSMSG( "DDR3 Init Done!\r\n" );

#if (CONFIG_BUS_RECONFIG == 1)
    setBusConfig();
#endif

    if (isResume)
    {
        printf( " DDR3 SelfRefresh exit Done!\r\n0x%08X\r\n", ReadIO32(&pReg_Alive->WAKEUPSTATUS) );
        dowakeup();
    }
    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0);
#else   // #if (CONFIG_SUSPEND_RESUME == 1)

#ifdef MEM_TYPE_DDR3
    init_DDR3(isResume);
#endif
#ifdef MEM_TYPE_LPDDR23
    init_LPDDR3(isResume);
#endif

    SYSMSG( "DDR3 Init Done!\r\n" );

#if (CONFIG_BUS_RECONFIG == 1)
    setBusConfig();
#endif
#endif  // #if (CONFIG_SUSPEND_RESUME == 1)

#if 0
    if(USBREBOOT_SIGNATURE == ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE5))
    {
        // TODO
    }
#endif

    if (pSBI->SIGNATURE != HEADER_ID)
        printf( "2nd Boot Header is invalid, Please check it out!\r\n" );

#if defined( LOAD_FROM_USB )
    printf( "Loading from usb...\r\n" );
    Result = iUSBBOOT(pTBI);            // for USB boot
#endif
#if defined( LOAD_FROM_SPI )
    printf( "Loading from spi...\r\n" );
    Result = iSPIBOOT(pTBI);            // for SPI boot
#endif
#if defined( LOAD_FROM_SDMMC )
    printf( "Loading from sdmmc...\r\n" );
    Result = iSDXCBOOT(pTBI);           // for SD boot
#endif
#if defined( LOAD_FROM_SDFS )
    printf( "Loading from sd FATFS...\r\n" );
    Result = iSDXCFSBOOT(pTBI);         // for SDFS boot
#endif
#if defined( LOAD_FROM_NAND )
    printf( "Loading from nand...\r\n" );
    Result = iNANDBOOTEC(pTBI);         // for NAND boot
#endif
#if defined( LOAD_FROM_UART )
    printf( "Loading from uart...\r\n" );
    Result = iUARTBOOT(pTBI);           // for UART boot
#endif

#if defined( LOAD_FROM_ALL ) && defined( CHIPID_S5P4418 )
    switch(pSBI->DBI.SPIBI.LoadDevice)
    {
    case BOOT_FROM_USB:
        printf( "Loading from usb...\r\n" );
        Result = iUSBBOOT(pTBI);        // for USB boot
        break;
    case BOOT_FROM_SPI:
        printf( "Loading from spi...\r\n" );
        Result = iSPIBOOT(pTBI);        // for SPI boot
        break;
#if 0
    case BOOT_FROM_NAND:
        printf( "Loading from nand...\r\n" );
        Result = iNANDBOOTEC(pTBI);     // for NAND boot
        break;
#endif
    case BOOT_FROM_SDMMC:
        printf( "Loading from sdmmc...\r\n" );
        Result = iSDXCBOOT(pTBI);       // for SD boot
        break;
    case BOOT_FROM_SDFS:
        printf( "Loading from sd FATFS...\r\n" );
        Result = iSDXCFSBOOT(pTBI);     // for SDFS boot
        break;
#if 0
    case BOOT_FROM_UART:
        printf( "Loading from uart...\r\n" );
        Result = iUARTBOOT(pTBI);       // for UART boot
        break;
#endif
    }
#endif

#if defined( LOAD_FROM_ALL ) && defined( CHIPID_NXP4330 )
    #error "Does not support compile option!!!"
#endif

#if 0   // for memory test
    {
        U32 *pSrc = (U32 *)pTBI->LAUNCHADDR;
        U32 *pDst = (U32 *)(0x50000000);
        int i;

        for (i = 0; i < (int)(pTBI->LOADSIZE >> 2); i++)
        {
            pDst[i] = pSrc[i];
        }

        for (i = 0; i < (int)(pTBI->LOADSIZE >> 2); i++)
        {
            if (pDst[i] != pSrc[i])
            {
                printf( "Copy check faile...\r\n" );
                break;
            }
        }
    }
#endif


    if(Result)
    {
        void (*pLaunch)(U32,U32) = (void(*)(U32,U32))pTBI->LAUNCHADDR;
        printf( " Image Loading Done!\r\n" );
        printf( "Launch to 0x%08X\r\n", (U32)pLaunch );
//        while(!DebugIsTXEmpty());
//        while(DebugIsBusy());
        while(!DebugIsUartTxDone());
        pLaunch(0, 4330);
    }

    printf( " Image Loading Failure Try to USB boot\r\n" );
    while(!DebugIsUartTxDone());
//    while(!DebugIsTXEmpty());
//    while(DebugIsBusy());
}
