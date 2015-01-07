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
#define AUTO_DETECT_EMA     (1)
#define EMA_VALUE           (1)     // Manual setting - 1(001): 1.1V, 3(011): 1.0V

extern U32      iget_fcs(U32 fcs, U32 data);
extern U32      __calc_crc(void *addr, int len);
extern void     DMC_Delay(int milisecond);

extern void     flushICache(void);
extern void     enableICache(CBOOL enable);

extern void     enterSelfRefresh(void);
extern void     exitSelfRefresh(void);
extern void     setAXIBus(void);

extern CBOOL    iUSBBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iUARTBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSPIBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSDXCBOOT(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iNANDBOOTEC(struct NX_SecondBootInfo * pTBI);
extern CBOOL    iSDXCFSBOOT(struct NX_SecondBootInfo * pTBI);
extern void     initClock(void);
extern void     initDDR3(U32);

extern void     printClkInfo(void);

extern void     ResetCon(U32 devicenum, CBOOL en);


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

#define	ASV_ARRAY_SIZE  (int)(sizeof(asv_tables)/sizeof(asv_tables[0]))

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
            tb = &asv_tables[i];
            if (tb->ids >= ids)
                break;
        }
        ids_L = i < ASV_ARRAY_SIZE ? i : (ASV_ARRAY_SIZE-1);

        /* find ro Level */
        for (i = 0; i < ASV_ARRAY_SIZE; i++) {
            tb = &asv_tables[i];
            if (tb->ro >= ro)
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

    temp = ReadIO32( &pReg_Tieoff->TIEOFFREG[1] ) & ~(7<<2);
#if (AUTO_DETECT_EMA == 1)
    ecid_1 = ReadIO32(PHY_BASEADDR_ECID_MODULE + (1<<2));
    ecid_2 = ReadIO32(PHY_BASEADDR_ECID_MODULE + (2<<2));

    if ( isEMA3(ecid_1, ecid_2) )
        ema = 3;
    else
        ema = 1;
#else
    ema = EMA_VALUE;                            //; cortex-A9 L1 Cache EMA value (1: 1.1V, 3: 1.0V)
#endif
    temp |= (ema<<2);
    flushICache();
    enableICache(CFALSE);
    WriteIO32( &pReg_Tieoff->TIEOFFREG[1],  temp );
    enableICache(CTRUE);

    printf("EMA VALUE : %s\r\n", (ema == 3 ? "011" : "001") );

    return;
}

//------------------------------------------------------------------------------
static void dowakeup(void)
{
    U32 fn, sign, phy, crc, len;
    void (*jumpkernel)(void) = 0;

    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);             // open alive power gate
    fn   = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE1);
    sign = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
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

        printf("CRC: 0x%08X FN: 0x%08X phy: 0x%08X len: 0x%08X ret: 0x%08X\r\n", crc, fn, phy, len, ret);
        if (fn && (crc == ret))
        {
            U32 temp = 0x100000;
            printf("It's WARM BOOT\r\nJump to Kernel!\r\n");
            while(DebugIsBusy() && temp--);
            jumpkernel();
        }
    }
    else
    {
        printf("Suspend Signature is different\r\nRead Signature :0x%08X\r\n", sign);
    }

    printf("It's COLD BOOT\r\n");
}

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


void sleepMain( void )
{
    DebugInit();
    enterSelfRefresh();
}


void vddPowerOff( void )
{
    WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x00000001 );   //; alive power gate open

    WriteIO32( &pReg_Alive->VDDOFFCNTVALUERST,  0xFFFFFFFF );   //; clear delay counter, refrence rtc clock
    WriteIO32( &pReg_Alive->VDDOFFCNTVALUESET,  0x00000001 );   //; no need delay time

    WriteIO32( &pReg_Alive->VDDCTRLRSTREG,      0x00000001 );   //; vddpoweron off, start delay counter

    while(1)
    {
        __asm__ __volatile__ ("wfi");                           //; now real entering point to stop mode.
        WriteIO32( &pReg_Alive->ALIVEGPIODETECTPENDREG, 0xFF ); //; all alive pend pending clear until power down.
    }                                                           //; this time, core power will off and so cpu will die.
}


//------------------------------------------------------------------------------
void BootMain( U32 CPUID )
{
    struct NX_SecondBootInfo    TBI;
    struct NX_SecondBootInfo * pTBI = &TBI;    // third boot info
    CBOOL Result = CFALSE;
    U32 sign, isResume = 0;
    U32 temp;

    CPUID = CPUID;

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
#endif

//    WriteIO32( &pReg_Alive->ALIVEPWRGATEREG,    0x1 );  // open alive power gate
//    WriteIO32( &pReg_Alive->PMUNPWRUP,          0x3 );  // mpeg & 3d power down
//    WriteIO32( &pReg_Alive->PMUNPWRUPPRE,       0x3 );  // mpeg & 3d  pre power down
//    WriteIO32( &pReg_Alive->PMUNISOLATE,        0x0 );  // mpeg & 3d power isolating

    initClock();

    //--------------------------------------------------------------------------
    // Debug Console
    //--------------------------------------------------------------------------
    DebugInit();


    printf( "\r\n" );
    printf( "--------------------------------------------------------------------------------\r\n" );
    printf( " Second Boot by Nexell Co. : Ver0.5 - Built on %s %s\r\n", __DATE__, __TIME__ );
    printf( "--------------------------------------------------------------------------------\r\n" );


    //--------------------------------------------------------------------------
    // set EMA value
    //--------------------------------------------------------------------------
    setEMA();

    //--------------------------------------------------------------------------
    // print clock information
    //--------------------------------------------------------------------------
    printClkInfo();

    //--------------------------------------------------------------------------
    // get VID & PID for USBD
    //--------------------------------------------------------------------------
    temp = ReadIO32(PHY_BASEADDR_ECID_MODULE + (3<<2));
    g_USBD_VID = (temp >> 16) & 0xFFFF;
    g_USBD_PID = (temp & 0xFFFF);

    if ((g_USBD_VID == 0) || (g_USBD_PID == 0))
    {
        g_USBD_VID = USBD_VID;
        g_USBD_PID = USBD_PID;
    }
    else
    {
        g_USBD_VID = 0x04E8;
        g_USBD_PID = 0x1234;
    }
    SYSMSG("USBD VID = %04X, PID = %04X\r\n", g_USBD_VID, g_USBD_PID);


#if (CPU_BRINGUP_CHECK == 1)
{
    volatile U32 *aliveflag = (U32*)CPU_ALIVE_FLAG_ADDR;
    int i, delay, retry;
    for(i = 1; i < 4; )
    {
        *aliveflag = 0;
        delay = 0x10000;
        TurnOnCPUnonedelay(i);
        while((*aliveflag == 0) && --delay);
        if(delay == 0)
        {
            if(retry > 3)
            {
                printf("maybe cpu %d is dead. -_-;\r\n", i);
                i++;
            }
            else
                printf("cpu %d is not bringup, retry\r\n", i);
        }
        else
        {
            retry = 0;
            i++;
        }
    }
}
#else
    TurnOnCPUnonedelay(1);
    TurnOnCPUnonedelay(2);
    TurnOnCPUnonedelay(3);
#endif

    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 1);
    sign = ReadIO32(&pReg_Alive->ALIVESCRATCHREADREG);
    if ((SUSPEND_SIGNATURE == sign) && ReadIO32(&pReg_Alive->WAKEUPSTATUS))
    {
        isResume = 1;
    }

    initDDR3(isResume);

    if (isResume)
    {
        exitSelfRefresh();
    }

    SYSMSG( "DDR3 Init Done!\r\n" );

    setAXIBus();

    if (isResume)
    {
        printf( " DDR3 SelfRefresh exit Done!\r\n0x%08X\r\n", ReadIO32(&pReg_Alive->WAKEUPSTATUS) );
        dowakeup();
    }
    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG, 0);

    if (pSBI->SIGNATURE != HEADER_ID)
        printf( "2nd Boot Header is invalid, Please check it out!\r\n" );

#if defined(CHIPID_LF3000)
#ifdef usbload
    printf( "Loading from usb...\r\n" );
    Result = iUSBBOOT(pTBI);            // for USB boot
#elif   defined spiload
    printf( "Loading from spi...\r\n" );
    Result = iSPIBOOT(pTBI);            // for SPI boot
#elif   defined nandload
    printf( "Loading from nand...\r\n" );
    Result = iNANDBOOTEC(pTBI);         // for NAND boot
#elif   defined sdmmcload
    printf( "Loading from sdmmc...\r\n" );
    Result = iSDXCBOOT(pTBI);           // for SD boot
#elif   defined sdfsload
    printf( "Loading from sd FATFS...\r\n" );
    Result = iSDXCFSBOOT(pTBI);         // for SDFS boot
#elif   defined uartload
    printf( "Loading from uart...\r\n" );
    Result = iUARTBOOT(pTBI);           // for UART boot
#endif
#elif defined(CHIPID_NXP4330)

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
    case BOOT_FROM_NAND:
        printf( "Loading from nand...\r\n" );
        Result = iNANDBOOTEC(pTBI);     // for NAND boot
        break;
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
        temp = 0x10000000;
        while(DebugIsBusy() && temp--);
        pLaunch(0, 4330);
    }

    printf( " Image Loading Failure Try to USB boot\r\n" );
    temp = 0x10000000;
    while(DebugIsBusy() && temp--);
}
