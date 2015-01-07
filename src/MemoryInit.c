#include "sysHeader.h"

#include <nx_drex.h>
#include <nx_ddrphy.h>
#include "nx_reg_base.h"
#include "ddr3_sdram.h"

#define DDR_NEW_LEVELING_TRAINING       (1)

#define DDR_RW_CAL      0

#define DDR_WRITE_LEVELING_EN           (0)
#define DDR_CA_CALIB_EN                 (0)     // for LPDDR3
#define DDR_GATE_LEVELING_EN            (1)     // for DDR3, great then 800MHz
#define DDR_READ_DQ_CALIB_EN            (1)
#define DDR_WRITE_LEVELING_CALIB_EN     (0)     // for Fly-by
#define DDR_WRITE_DQ_CALIB_EN           (1)

#define DDR_RESET_GATE_LVL              (1)
#define DDR_RESET_READ_DQ               (1)
#define DDR_RESET_WRITE_DQ              (1)

#define DDR_RESET_QOS1                  (1)     // Release version is '1'


#if (CFG_NSIH_EN == 0)
#include "DDR3_K4B8G1646B_MCK0.h"
#endif

#define nop() __asm__ __volatile__("mov\tr0,r0\t@ nop\n\t");

static U32  g_DDRLock;
static U32  g_GateCycle;
static U32  g_RDvwmc;
static U32  g_WRvwmc;


inline void DMC_Delay(int milisecond)
{
    register volatile  int    count;

    for (count = 0; count < milisecond; count++)
    {
        nop();
    }
}

inline void SendDirectCommand(SDRAM_CMD cmd, U8 chipnum, SDRAM_MODE_REG mrx, U16 value)
{
    WriteIO32((U32*)&pReg_Drex->DIRECTCMD, cmd<<24 | chipnum<<20 | mrx<<16 | value);
}

void enterSelfRefresh(void)
{
    union SDRAM_MR MR;
    U32     nTemp;
    U32     nChips = 0;

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    nChips = 0x3;
#else
    nChips = 0x1;
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        nChips = 0x3;
    else
        nChips = 0x1;
#endif


    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & 0xF )
    {
        nop();
    }


#if 1
    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
    DMC_Delay(100);
#endif



#if 1
    // odt off
    MR.Reg          = 0;
    MR.MR2.RTT_WR   = 0;        // 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
//    MR.MR2.RTT_WR   = 2;        // 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
    MR.MR2.SRT      = 0;        // self refresh normal range, if (ASR == 1) SRT = 0;
    MR.MR2.ASR      = 1;        // auto self refresh enable
#if (CFG_NSIH_EN == 0)
    MR.MR2.CWL      = (nCWL - 5);
#else
    MR.MR2.CWL      = (pSBI->DII.CWL - 5);
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif
//    DMC_Delay(10);
#endif



#if 1
    MR.Reg          = 0;
    MR.MR1.DLL      = 1;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR.MR1.AL       = MR1_nAL;
#else
    MR.MR1.AL       = pSBI->DII.MR1_AL;
#endif
    MR.MR1.ODS1     = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR.MR1.ODS0     = 1;
    MR.MR1.QOff     = 0;
    MR.MR1.RTT_Nom2     = 0;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
    MR.MR1.RTT_Nom1     = 1;
    MR.MR1.RTT_Nom0     = 0;
    MR.MR1.WL       = 0;
#if (CFG_NSIH_EN == 0)
    MR.MR1.TDQS     = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR.MR1.TDQS     = (pSBI->DII.BusWidth>>3) & 1;
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif
//    DMC_Delay(10);
#endif



    /* Enter self-refresh command */
    SendDirectCommand(SDRAM_CMD_REFS, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_REFS, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
//    DMC_Delay(10);



#if 1
    do
    {
        nTemp = ( ReadIO32(&pReg_Drex->CHIPSTATUS) & nChips );
    } while( nTemp );

    do
    {
        nTemp = ( (ReadIO32(&pReg_Drex->CHIPSTATUS) >> 8) & nChips );
    } while( nTemp != nChips );
#else

    // for self-refresh check routine.
    while( 1 )
    {
        nTemp = ReadIO32(&pReg_Drex->CHIPSTATUS);
        if (nTemp)
            MEMMSG("ChipStatus = 0x%04x\r\n", nTemp);
    }
#endif


    // Step 52 Auto refresh counter disable
    ClearIO32( &pReg_Drex->CONCONTROL,  (0x1 << 5));        // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1

    // Step 10  ACK, ACKB off
    SetIO32( &pReg_Drex->MEMCONTROL,    (0x1 << 0));        // clk_stop_en[0] : Dynamic Clock Control       :: 1'b0  - Always running

    DMC_Delay(1000 * 3);
}

void exitSelfRefresh(void)
{
    union SDRAM_MR MR;

    // Step 10    ACK, ACKB on
    ClearIO32( &pReg_Drex->MEMCONTROL,  (0x1 << 0));        // clk_stop_en[0]   : Dynamic Clock Control                 :: 1'b0  - Always running
    DMC_Delay(10);

    // Step 52 Auto refresh counter enable
    SetIO32( &pReg_Drex->CONCONTROL,    (0x1 << 5));        // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
    DMC_Delay(10);


    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif


    MR.Reg          = 0;
    MR.MR1.DLL      = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR.MR1.AL       = MR1_nAL;
#else
    MR.MR1.AL       = pSBI->DII.MR1_AL;
#endif
    MR.MR1.ODS1     = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR.MR1.ODS0     = 1;
    MR.MR1.QOff     = 0;
    MR.MR1.RTT_Nom2     = 0;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
    MR.MR1.RTT_Nom1     = 1;
    MR.MR1.RTT_Nom0     = 0;
    MR.MR1.WL       = 0;
#if (CFG_NSIH_EN == 0)
    MR.MR1.TDQS     = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR.MR1.TDQS     = (pSBI->DII.BusWidth>>3) & 1;
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR.Reg);
#endif


    // odt on
    MR.Reg          = 0;
//    MR.MR2.RTT_WR   = 0;        // 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
    MR.MR2.RTT_WR   = 2;        // 0: disable, 1: RZQ/4 (60ohm), 2: RZQ/2 (120ohm)
    MR.MR2.SRT      = 0;        // self refresh normal range
    MR.MR2.ASR      = 0;        // auto self refresh disable
#if (CFG_NSIH_EN == 0)
    MR.MR2.CWL      = (nCWL - 5);
#else
    MR.MR2.CWL      = (pSBI->DII.CWL - 5);
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR.Reg);
#endif


    /* Exit self-refresh command */
    SendDirectCommand(SDRAM_CMD_REFSX, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_REFSX, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif


    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & (0xF << 8) )
    {
        nop();
    }

    DMC_Delay(1000 * 2);
}

void real_change_pll(U32 *clkpwr_reg, U32 *AXIBottomBase, struct NX_DREXSDRAM_RegisterSet *pReg_Drex, U32 pll_data)
{
#if 1
    U32 temp;

    temp = (U32)clkpwr_reg;
    temp = (U32)AXIBottomBase;
    temp = (U32)pReg_Drex;
    temp = pll_data;
    temp = temp;
#else

    register U32 i_slot;
    register U32 const pll_num  = (pll_data & 0x000000FF)>>  0;
    register U32 const s        = (pll_data & 0x0000FF00)>>  8;
    register U32 const m        = (pll_data & 0x00FF0000)>> 16;
    register U32 const p        = (pll_data & 0xFF000000)>> 24;
    register volatile U32 * const pllset_reg = (clkpwr_reg + 2 + pll_num);

    WriteIO32( &pReg_Drex->DIRECTCMD,     ((DIRCMD_PALL <<  DIRCMD_TYPE_SHIFT)| (DIRCMD_CHIP_0 <<  DIRCMD_CHIP_SHIFT)) );        // precharge all cmd
    ClearIO32( &pReg_Drex->CONCONTROL,    (0x1    <<     5));                // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
    for(i_slot = 0; i_slot<32; i_slot++)
    {
//        WriteIO32( NX_BASE_REG_PL301_BOTT| (0x408    <<  0), (i_slot <<  24)| (i_slot & 1));            // reserved only for cpu bus
        *(U32*)((U32)AXIBottomBase + 0x408) = (i_slot <<  24)| (i_slot & 1);
    }

    WriteIO32( &pReg_Drex->BRBRSVCONFIG,      0xFFF0FFF0 );        // Port 0(bottom): 0, Port 1(display): 0xF
    WriteIO32( &pReg_Drex->BRBRSVCONTROL,     0x00000022 );        // disable read/write brb reservation for AXI port 0
    WriteIO32( &pReg_Drex->BRBQOSCONFIG,      0x00000001 );        // QoS timer decrease below QOS value 0x00000FFF
    WriteIO32( &pReg_Drex->QOS[0],            0x00000FFF );        // above QoS counter decrease value counting value

    *pllset_reg &= ~(1<< 28);
    *clkpwr_reg = (1<< pll_num);
    while(*clkpwr_reg & (1<< 31));

    *pllset_reg = ((1UL <<  29)|
                    (0UL <<  28)|
                    (s <<  0)|
                    (m <<  8)|
                    (p <<  18));
    *clkpwr_reg = (1<< pll_num);
    while(*clkpwr_reg & (1<< 31));

    *pllset_reg &= ~((U32)(1UL<< 29));
    *clkpwr_reg = (1<< pll_num);
    while(*clkpwr_reg & (1<< 31));

    *pllset_reg|= (1<< 28);
    *clkpwr_reg = (1<< pll_num);
    while(*clkpwr_reg & (1<< 31));

    WriteIO32( &pReg_Drex->BRBRSVCONFIG,      0xFFF1FFF1 );
    WriteIO32( &pReg_Drex->BRBRSVCONTROL,     0x00000033 );
    WriteIO32( &pReg_Drex->BRBQOSCONFIG,      0x00000010 );        // QoS timer decrease below QOS value 0x00000FFF
    WriteIO32( &pReg_Drex->QOS[0],            0x00000200 );        // restore

    for(i_slot = 0; i_slot<32; i_slot++)
    {
//        WriteIO32( NX_BASE_REG_PL301_BOTT| (0x408    <<  0), (i_slot <<  24)| i_slot);        // each port reserved for themself
        *(U32*)((U32)AXIBottomBase + 0x408) = (i_slot <<  24)| i_slot;
    }
    WriteIO32( &pReg_Drex->DIRECTCMD,     ((DIRCMD_PALL <<  DIRCMD_TYPE_SHIFT)| (DIRCMD_CHIP_0 <<  DIRCMD_CHIP_SHIFT)) );        // precharge all cmd
    SetIO32( &pReg_Drex->CONCONTROL,      (0x1    <<     5));            // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1
#endif

    return;
}

void setAXIBus(void)
{
#if (DDR_RESET_QOS1 == 1)
    U32 DREX_QOS = (1<<12) | (1<<8) | (0<<4) | (0<<0);
    U32 index;
#endif

    WriteIO32( NX_BASE_REG_PL301_BOTT_QOS_TRDMARK,   (1 << 0) );
    ClearIO32( NX_BASE_REG_PL301_BOTT_QOS_CTRL,      (1 << 6) | (1 << 7) );

    WriteIO32( NX_BASE_REG_PL301_DISP_AR,   (0 << 24) | (0 << 0) ); // VIP1
    WriteIO32( NX_BASE_REG_PL301_DISP_AR,   (1 << 24) | (1 << 0) ); // VIP2
#if 1
    WriteIO32( NX_BASE_REG_PL301_DISP_AR,   (2 << 24) | (1 << 0) ); // GMAC -> VIP2
#else
    WriteIO32( NX_BASE_REG_PL301_DISP_AR,   (2 << 24) | (2 << 0) ); // GMAC
#endif

#if (DDR_RESET_QOS1 == 1)
    WriteIO32( &pReg_Drex->BRBRSVCONFIG,    0xFFF1FFF1 );
    WriteIO32( &pReg_Drex->BRBRSVCONTROL,   0x00000033 );

    WriteIO32( &pReg_Drex->QOSCONTROL[1],   0x00000000 );

    index = ((TIEOFFINDEX_OF_MALI4000_PP0_NX_NPOWERDOWN) & 0xffff)/32;
    WriteIO32(&pReg_Tieoff->TIEOFFREG[index], DREX_QOS);
#else

    WriteIO32( &pReg_Drex->BRBRSVCONFIG,    0xFFF1FFF1 );
    WriteIO32( &pReg_Drex->BRBRSVCONTROL,   0x00000033 );

    WriteIO32( &pReg_Drex->QOSCONTROL[0],   0x00000100 );
#endif

    return;
}

#if DDR_RW_CAL
void DDR3_RW_Delay_Calibration(void)
{
extern    void BurstZero(U32 *WriteAddr, U32 WData);
extern    void BurstWrite(U32 *WriteAddr, U32 WData);
extern    void BurstRead(U32 *ReadAddr, U32 *SaveAddr);
    unsigned int rnw, lane, adjusted_dqs_delay=0, bit, pmin, nmin;
    unsigned int *tptr = (unsigned int *)0x40100000;
    int toggle;

    for(rnw = 0; rnw<2; rnw++)
    {
        MEMMSG("\r\nserching %s delay value......\r\n", rnw?"read":"write");
        bit = 0;
        for(bit = 0; bit<32; bit++)
        {
            unsigned int readdata[8];
            unsigned int dqs_wdelay, repeatcount;
            unsigned char pwdelay, nwdelay;
            lane = bit>>3;

            if((bit & 7) == 0)
            {
                pmin = 0x7f;
                nmin = 0x7f;
            }
            MEMMSG("bit:%02d\t", bit);
            pwdelay = 0x80;
            if(rnw ==0)
                WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], 0x80<<(8*lane));
            else
                WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], 0x80<<(8*lane));
            SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            DMC_Delay(10000);
            for(dqs_wdelay = 0; dqs_wdelay<=0x7f && pwdelay==0x80; dqs_wdelay++)
            {
                repeatcount=0;
                if(rnw ==0)
                    WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], dqs_wdelay<<(8*lane));
                else
                    WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], dqs_wdelay<<(8*lane));
                SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                DMC_Delay(10000);
                while(repeatcount<100)
                {
                    for(toggle=1; toggle>=0; toggle--)
                    {
                        if(toggle)
                            BurstWrite(tptr, 1<<bit);
                        else
                            BurstWrite(tptr, ~(1<<bit));
                        BurstRead(tptr, readdata);
                        if( ((readdata[0]>>bit)&0x01) == !toggle &&
                            ((readdata[1]>>bit)&0x01) == !toggle &&
                            ((readdata[2]>>bit)&0x01) == !toggle &&
                            ((readdata[3]>>bit)&0x01) == !toggle &&
                            ((readdata[4]>>bit)&0x01) ==  toggle &&
                            ((readdata[5]>>bit)&0x01) == !toggle &&
                            ((readdata[6]>>bit)&0x01) == !toggle &&
                            ((readdata[7]>>bit)&0x01) == !toggle)
                        {
                            repeatcount++;
                        }else
                        {
                            pwdelay = dqs_wdelay;
                            if(pmin > pwdelay)
                                pmin = pwdelay;
                            MEMMSG("p%d:%02d ", toggle, pwdelay);
                            repeatcount = 100;
                            toggle = -1;
                            break;
                        }
                    }
                }
            }    // dqs_wdelay
            if(rnw==0)
                WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], 0<<(8*lane));
            else
                WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], 0<<(8*lane));
            SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));           // Force DLL Resyncronization
            DMC_Delay(10000);
            nwdelay = 0;
            for(dqs_wdelay = 0x80; dqs_wdelay<=0xFF && nwdelay==0; dqs_wdelay++)
            {
                repeatcount=0;
                if(rnw == 0)
                    WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], dqs_wdelay<<(8*lane));
                else
                    WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], dqs_wdelay<<(8*lane));
                SetIO32  ( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                ClearIO32( &pReg_Drex->PHYCONTROL,      (0x1    <<   3));       // Force DLL Resyncronization
                DMC_Delay(10000);
                while(repeatcount<100)
                {
                    for(toggle=1; toggle>=0; toggle--)
                    {
                        if(toggle)
                            BurstWrite(tptr, 1<<bit);
                        else
                            BurstWrite(tptr, ~(1<<bit));
                        BurstRead(tptr, readdata);
                        if( ((readdata[0]>>bit)&0x01) == !toggle &&
                            ((readdata[1]>>bit)&0x01) == !toggle &&
                            ((readdata[2]>>bit)&0x01) == !toggle &&
                            ((readdata[3]>>bit)&0x01) == !toggle &&
                            ((readdata[4]>>bit)&0x01) ==  toggle &&
                            ((readdata[5]>>bit)&0x01) == !toggle &&
                            ((readdata[6]>>bit)&0x01) == !toggle &&
                            ((readdata[7]>>bit)&0x01) == !toggle)
                        {
                            repeatcount++;
                        }else
                        {
                            nwdelay = dqs_wdelay & 0x7F;
                            if(nmin > nwdelay)
                                nmin = nwdelay;
                            MEMMSG("n%d:%02d  ", toggle, nwdelay);
                            repeatcount = 100;
                            toggle = -1;
                            break;
                        }
                    }
                }
            }    // dqs_wdelay

            if(pwdelay > nwdelay)    // biased to positive side
            {
                MEMMSG("margin: %2d  adj: %2d\r\n", (pwdelay - nwdelay), (pwdelay - nwdelay)>>1);
            }
            else    // biased to negative side
            {
                MEMMSG("margin: %2d  adj:-%2d\r\n", (nwdelay - pwdelay), (nwdelay - pwdelay)>>1);
            }
            if((bit & 7)==7)
            {
                MEMMSG("lane average positive min:%d negative min:%d ", pmin, nmin);
                if(pmin > nmin) // biased to positive side
                {
                    adjusted_dqs_delay |= ((pmin - nmin)>>1) << (8*lane);
                    MEMMSG("margin: %2d  adj: %2d\r\n", (pmin - nmin), (pmin - nmin)>>1);
                }
                else    // biased to negative side
                {
                    adjusted_dqs_delay |= (((nmin - pmin)>>1) | 0x80) << (8*lane);
                    MEMMSG("margin: %2d  adj:-%2d\r\n", (nmin - pmin), (nmin - pmin)>>1);
                }
            }
            if(((bit+1) & 0x7) == 0)
                MEMMSG("\n");
        }   // lane

        if(rnw == 0)
            WriteIO32(&pReg_DDRPHY->OFFSETW_CON[0], adjusted_dqs_delay);
        else
            WriteIO32(&pReg_DDRPHY->OFFSETR_CON[0], adjusted_dqs_delay);
        SetIO32  ( &pReg_Drex->PHYCONTROL,   (0x1   <<   3));           // Force DLL Resyncronization
        ClearIO32( &pReg_Drex->PHYCONTROL,   (0x1   <<   3));           // Force DLL Resyncronization
        MEMMSG("\r\n");
        MEMMSG("read  delay value is 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->OFFSETR_CON[0]));
        MEMMSG("write delay value is 0x%08X\r\n", ReadIO32(&pReg_DDRPHY->OFFSETW_CON[0]));
    }
}
#endif

#if (DDR_NEW_LEVELING_TRAINING == 1)
#if (DDR_WRITE_LEVELING_EN == 1)
void DDR_Write_Leveling(void)
{
#if 0
    MEMMSG("\r\n########## Write Leveling ##########\r\n");

#else
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR1;
#endif
    U32 temp;

    MEMMSG("\r\n########## Write Leveling - Start ##########\r\n");

    SetIO32( &pReg_DDRPHY->PHY_CON[26+1],       (0x3    <<   7) );          // cmd_default, ODT[8:7]=0x3
    SetIO32( &pReg_DDRPHY->PHY_CON[0],          (0x1    <<  16) );          // wrlvl_mode[16]=1

#if defined(MEM_TYPE_DDR3)
    /* Set MPR mode enable */
    MR1.Reg         = 0;
    MR1.MR1.DLL     = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR1.MR1.AL      = MR1_nAL;
#else
    MR1.MR1.AL      = pSBI->DII.MR1_AL;
#endif
    MR1.MR1.ODS1    = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR1.MR1.ODS0    = 1;
    MR1.MR1.QOff    = 0;
    MR1.MR1.RTT_Nom2    = 0;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
    MR1.MR1.RTT_Nom1    = 1;
    MR1.MR1.RTT_Nom0    = 0;
    MR1.MR1.WL      = 1;
#if (CFG_NSIH_EN == 0)
    MR1.MR1.TDQS    = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR1.MR1.TDQS    = (pSBI->DII.BusWidth>>3) & 1;
#endif

    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#endif

#if 0
    // Send NOP command.
    SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#endif

    temp = ( (0x8 << 24) | (0x8 << 17) | (0x8 << 8) | (0x8 << 0)  );        // PHY_CON30[30:24] = ctrl_wrlvl_code3, PHY_CON30[23:17] = ctrl_wrlvl_code2, PHY_CON30[14:8] = ctrl_wrlvl_code1, PHY_CON30[6:0] = ctrl_wrlvl_code0
    WriteIO32( &pReg_DDRPHY->PHY_CON[30+1],     temp );
    MEMMSG("ctrl_wrlvl_code = 0x%08X\r\n", temp);

    // SDLL update.
    SetIO32  ( &pReg_DDRPHY->PHY_CON[30+1],     (0x1    <<  16) );          // wrlvl_enable[16]=1, ctrl_wrlvl_resync
    ClearIO32( &pReg_DDRPHY->PHY_CON[30+1],     (0x1    <<  16) );          // wrlvl_enable[16]=0, ctrl_wrlvl_resync

    temp = ReadIO32( &pReg_DDRPHY->PHY_CON[30+1] );                         // PHY_CON30[30:24] = ctrl_wrlvl_code3, PHY_CON30[23:17] = ctrl_wrlvl_code2, PHY_CON30[14:8] = ctrl_wrlvl_code1, PHY_CON30[6:0] = ctrl_wrlvl_code0
    MEMMSG("ctrl_wrlvl_code = 0x%08X\r\n", temp);

    ClearIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1    <<  16) );          // wrlvl_mode[16]=0
    ClearIO32( &pReg_DDRPHY->PHY_CON[26+1],     (0x3    <<   7) );          // cmd_default, ODT[8:7]=0x0

    MEMMSG("\r\n########## Write Leveling - End ##########\r\n");
#endif
}
#endif

#if (DDR_CA_CALIB_EN == 1)
void DDR_CA_Calibration(void)
{
//LPDDR3
#if 1
    MEMMSG("\r\n########## CA Calibration - Start ##########\r\n");

#else

    MEMMSG("\r\n########## CA Calibration - Start ##########\r\n");
    MEMMSG("\r\n########## CA Calibration - End ##########\r\n");
#endif
}
#endif

#if (DDR_GATE_LEVELING_EN == 1)
CBOOL DDR_Gate_Leveling(U32 isResume)
{
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR;
#endif
    volatile U32 cal_count = 0;
#if (DDR_RESET_GATE_LVL == 1)
    U32     i;
    U8      gate_cycle[4];
#endif
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Gate Leveling - Start ##########\r\n");

    SetIO32  ( &pReg_DDRPHY->PHY_CON[14],   (0xF    <<  0) );               // ctrl_pulld_dqs[3:0] = 0
    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],    (0x1    << 13) );               // byte_rdlvl_en[13]=1

    if (isResume == 0)
    {
        SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

#if defined(MEM_TYPE_DDR3)
        /* Set MPR mode enable */
        MR.Reg          = 0;
        MR.MR3.MPR      = 1;
        MR.MR3.MPR_RF   = 0;

        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif
    }


    SetIO32( &pReg_DDRPHY->PHY_CON[2],      (0x1    <<  24) );              // rdlvl_gate_en[24] = 1
    SetIO32( &pReg_DDRPHY->PHY_CON[0],      (0x1    <<   8) );              // ctrl_shgate[8] = 1
#if defined(MEM_TYPE_DDR3)
    ClearIO32( &pReg_DDRPHY->PHY_CON[1],    (0xF    <<  20) );              // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9
#endif
#if defined(MEM_TYPE_LPDDR23)
    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1]) & ~(0xF <<  20);            // ctrl_gateduradj[23:20] = DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9
    temp |= (0xB    << 20);
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],    temp );
#endif

    /* Update reset DLL */
    WriteIO32( &pReg_Drex->RDLVL_CONFIG,    (0x1    <<   0) );              // ctrl_rdlvl_gate_en[0] = 1

    if (isResume)
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1 << 14) ) == 0 );      // rdlvl_complete[14] = 1
        WriteIO32( &pReg_Drex->RDLVL_CONFIG,    0 );                            // ctrl_rdlvl_gate_en[0] = 0
    }
    else
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1 << 14) ) == 0 )   // rdlvl_complete[14] : Wating until READ leveling is complete
        {
            if (cal_count > 100)                                            // Failure Case
            {
                WriteIO32( &pReg_Drex->RDLVL_CONFIG, 0x0 );                 // ctrl_rdlvl_data_en[0]=0 : Stopping it after completion of READ leveling.

                ret = CFALSE;
                goto gate_err_ret;
            }
            else
            {
                DMC_Delay(0x100);
            }

            cal_count++;
        }


        nop();
        WriteIO32( &pReg_Drex->RDLVL_CONFIG, 0x0 );                         // ctrl_rdlvl_data_en[0]=0 : Stopping it after completion of READ leveling.

        //- Checking the calibration failure status
        //- After setting PHY_CON5(0x14) to "0xC", check whether PHY_CON21 is zero or nor. If PHY_CON21(0x58) is zero, calibration is normally complete.

        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    VWM_FAIL_STATUS );          // readmodecon[7:0] = 0xC
        cal_count = 0;
        do {
            if (cal_count > 100)
            {
                MEMMSG("\r\n\nRD VWM_FAIL_STATUS Checking : fail...!!!\r\n");
                ret = CFALSE;                                               // Failure Case
                goto gate_err_ret;
            }
            else if (cal_count)
            {
                DMC_Delay(0x100);
            }

            cal_count++;
        } while( ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] ) != 0x0 );

        //------------------------------------------------------------------------------------------------------------------------
        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    GATE_CENTER_CYCLE);
        g_GateCycle = ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] );
        //------------------------------------------------------------------------------------------------------------------------
    }

#if (DDR_RESET_GATE_LVL == 1)
    for (i = 0; i < 4; i++)
    {
        gate_cycle[i] = ((g_GateCycle >> (8*i)) & 0xFF);
    }

    temp = ( ((U8)gate_cycle[3] << 15) | ((U8)gate_cycle[2] << 10) | ((U8)gate_cycle[1] << 5) | (U8)gate_cycle[0] );
    WriteIO32( &pReg_DDRPHY->PHY_CON[3], temp );    // ctrl_shiftc
#endif  // #if (DDR_RESET_GATE_LVL == 1)

gate_err_ret:

    WriteIO32( &pReg_DDRPHY->PHY_CON[5],    0x0 );                          // readmodecon[7:0] = 0x0

    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );              // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );              // Force DLL Resyncronization

    if (isResume == 0)
    {
#if defined(MEM_TYPE_DDR3)

        /* Set MPR mode disable */
        MR.Reg          = 0;
        MR.MR3.MPR      = 0;
        MR.MR3.MPR_RF   = 0;

        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif
    }

    ClearIO32( &pReg_DDRPHY->PHY_CON[14],   (0xF    <<   0) );              // ctrl_pulld_dqs[3:0]=0

    MEMMSG("\r\n########## Gate Leveling - End ##########\r\n");

    return ret;
}
#endif  // #if (DDR_GATE_LEVELING_EN == 1)

#if (DDR_READ_DQ_CALIB_EN == 1)
CBOOL DDR_Read_DQ_Calibration(U32 isResume)
{
#if defined(MEM_TYPE_DDR3)
    union SDRAM_MR MR;
#endif
    volatile U32 cal_count = 0;
#if (DDR_RESET_READ_DQ == 1)
    U32     lock_div4;
    U32     i;
    U8      rlvl_vwmc[4];
    int     offsetr[4];
#endif
    U32     temp;
    CBOOL   ret = CTRUE;


    MEMMSG("\r\n########## Read DQ Calibration - Start ##########\r\n");

    SetIO32  ( &pReg_DDRPHY->PHY_CON[14],   (0xF    <<  0) );               // ctrl_pulld_dqs[3:0] = 0xF, Need for MPR

    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & 0xF )         //- Until chip_busy_state
    {
        nop();
    }


    if (isResume == 0)
    {
        SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
    }

#if defined(MEM_TYPE_DDR3)
    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0xFFFF0000;
//    temp |= 0xFF00;                                                     // rdlvl_rddata_adj[15:0]
    temp |= 0x0100;                                                     // rdlvl_rddata_adj[15:0]
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],    temp );


    if (isResume == 0)
    {
        /* Set MPR mode enable */
        MR.Reg          = 0;
        MR.MR3.MPR      = 1;
        MR.MR3.MPR_RF   = 0;

        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
    }

    WriteIO32( &pReg_DDRPHY->PHY_CON[24+1],
        (0x0    << 16) |    // [31:16] ddr3_default
        (0x0    <<  1) |    // [15: 1] ddr3_address
        (0x0    <<  0) );   // [    0] ca_swap_mode
#endif
#if defined(MEM_TYPE_LPDDR23)
    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0xFFFF0000;
    temp |= 0x00FF;                                                     // rdlvl_rddata_adj[15:0]
//    temp |= 0x0001;                                                     // rdlvl_rddata_adj[15:0]
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],    temp );

//    WriteIO32( &pReg_DDRPHY->PHY_CON[22+1],     0x00000208 );           // lpddr2_addr[15:0]=0x208
    WriteIO32( &pReg_DDRPHY->PHY_CON[22+1],     0x00000041 );           // lpddr2_addr[15:0]=0x041
#endif

    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],    (0x1    << 25) );           // rdlvl_en[25]=1

    WriteIO32( &pReg_Drex->RDLVL_CONFIG,    (0x1    <<  1) );           // ctrl_rdlvl_data_en[1]=1 : Starting READ leveling

    if (isResume)
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1 << 14) ) == 0 );  // rdlvl_complete[14] : Wating until READ leveling is complete
        WriteIO32( &pReg_Drex->RDLVL_CONFIG, 0x0 );                         // ctrl_rdlvl_data_en[1]=0 : Stopping it after completion of READ leveling.
    }
    else
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1 << 14) ) == 0 )   // rdlvl_complete[14] : Wating until READ leveling is complete
        {
            if (cal_count > 100)                                            // Failure Case
            {
                WriteIO32( &pReg_Drex->RDLVL_CONFIG, 0x0 );                 // ctrl_rdlvl_data_en[1]=0 : Stopping it after completion of READ leveling.

                ret = CFALSE;
                goto rd_err_ret;
            }
            else
            {
                DMC_Delay(0x100);
            }

            cal_count++;
        }

        nop();
        WriteIO32( &pReg_Drex->RDLVL_CONFIG, 0x0 );                         // ctrl_rdlvl_data_en[1]=0 : Stopping it after completion of READ leveling.

        //- Checking the calibration failure status
        //- After setting PHY_CON5(0x14) to "0xC", check whether PHY_CON21 is zero or nor. If PHY_CON21(0x58) is zero, calibration is normally complete.

        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    VWM_FAIL_STATUS );          // readmodecon[7:0] = 0xC
        cal_count = 0;
        do {
            if (cal_count > 100)
            {
                MEMMSG("\r\n\nRD VWM_FAIL_STATUS Checking : fail...!!!\r\n");
                ret = CFALSE;                                               // Failure Case
                goto rd_err_ret;
            }
            else if (cal_count)
            {
                DMC_Delay(0x100);
            }

            cal_count++;
        } while( ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] ) != 0x0 );

        //*** Read DQ Calibration Valid Window Margin
        //------------------------------------------------------------------------------------------------------------------------
        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    RD_VWMC);
        g_RDvwmc = ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] );
        //------------------------------------------------------------------------------------------------------------------------
    }

#if (DDR_RESET_READ_DQ == 1)
    for (i = 0; i < 4; i++)
    {
        rlvl_vwmc[i]   = ((g_RDvwmc >> (8*i)) & 0xFF);
    }

//*** Forcing manually offsetr to stop the calibration mode after completion of READ DQ Cal

    lock_div4 = (g_DDRLock >> 2);
    for (i = 0; i < 4; i++)
    {
        offsetr[i] = (int)(rlvl_vwmc[i] - lock_div4);
        if (offsetr[i] < 0)
        {
            MEMMSG("offsetr%d=%d\r\n", i, offsetr[i]);
            offsetr[i] *= -1;
            offsetr[i] |= 0x80;
        }
    }

#if 1
{
    int max, min, val;
    int i, inx;

//    temp = ( ((U8)offsetr[3] << 24) | ((U8)offsetr[2] << 16) | ((U8)offsetr[1] << 8) | (U8)offsetr[0] );
//    printf("RD DQ : Org value = 0x%08X\r\n", temp);

    for (inx = 0; inx < 4; inx++)
    {
        for (i = 1; i < 4; i++)
        {
            if (offsetr[i-1] > offsetr[i])
            {
               max          = offsetr[i-1];
               offsetr[i-1] = offsetr[i];
               offsetr[i]   = max;
            }
        }
    }

#if 0
    for (inx = 0; inx < 4; inx++)
    {
        printf( "Sorted Value[%d] = %d\r\n", inx, offsetw[inx] );
    }
#endif

    if ( offsetr[1] > offsetr[2])
    {
        max = offsetr[1];
        min = offsetr[2];
    }
    else
    {
        max = offsetr[2];
        min = offsetr[1];
    }

    if ( (max - min) > 5)
    {
        val = min;
    }
    else
    {
        val = max;
    }

    for (inx = 0; inx < 4; inx++)
    {
        offsetr[inx] = val;
    }
}
#endif

    temp = ( ((U8)offsetr[3] << 24) | ((U8)offsetr[2] << 16) | ((U8)offsetr[1] << 8) | (U8)offsetr[0] );
    WriteIO32( &pReg_DDRPHY->PHY_CON[4], temp );

    //*** Resync
    //*** Update READ SDLL Code (ctrl_offsetr) : Make "ctrl_resync" HIGH and LOW
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );
    SetIO32  ( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );      // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );      // ctrl_resync[24]=0x0 (LOW)
#endif  // #if (DDR_RESET_READ_DQ == 1)


rd_err_ret:

    //*** Set PHY0.CON2.rdlvl_en : setting MUX as "0" to force manually the result value of READ leveling.
    ClearIO32( &pReg_DDRPHY->PHY_CON[2],    (0x1    <<  25) );      // rdlvl_en[25]=0
    WriteIO32( &pReg_DDRPHY->PHY_CON[5],    0x0 );                  // readmodecon[7:0] = 0x0

    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );      // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );      // Force DLL Resyncronization


    if (isResume == 0)
    {
#if defined(MEM_TYPE_DDR3)

        /* Set MPR mode disable */
        MR.Reg          = 0;
        MR.MR3.MPR      = 0;
        MR.MR3.MPR_RF   = 0;

        SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#else
        if(pSBI->DII.ChipNum > 1)
            SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR.Reg);
#endif
#endif
    }

    ClearIO32( &pReg_DDRPHY->PHY_CON[14],   (0xF    <<   0) );              // ctrl_pulld_dqs[3:0]=0

    MEMMSG("\r\n########## Read DQ Calibration - End ##########\r\n");

    return ret;
}
#endif  // #if (DDR_READ_DQ_CALIB_EN == 1)

#if (DDR_WRITE_LEVELING_CALIB_EN == 1)
void DDR_Write_Leveling_Calibration(void)
{
    MEMMSG("\r\n########## Write Leveling Calibration - Start ##########\r\n");

}
#endif

#if (DDR_WRITE_DQ_CALIB_EN == 1)
CBOOL DDR_Write_DQ_Calibration(U32 isResume)
{
    volatile U32 cal_count = 0;
    U32     lock_div4;
    U32     i;
    U8      wlvl_vwmc[4];
    int     offsetw[4];
    U32     temp;
    CBOOL   ret = CTRUE;

    MEMMSG("\r\n########## Write DQ Calibration - Start ##########\r\n");


#if 1
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif

    DMC_Delay(0x100);
#endif


    while( ReadIO32(&pReg_Drex->CHIPSTATUS) & 0xF )         //- Until chip_busy_state
    {
        nop();
    }

    WriteIO32( &pReg_Drex->WRTRA_CONFIG,
        (0x0    << 16) |    // [31:16] row_addr
        (0x0    <<  1) |    // [ 3: 1] bank_addr
        (0x1    <<  0) );   // [    0] write_training_en

#if defined(MEM_TYPE_DDR3)
    WriteIO32( &pReg_DDRPHY->PHY_CON[24+1],
        (0x0    << 16) |    // [31:16] ddr3_default
        (0x0    <<  1) |    // [15: 1] ddr3_address
        (0x0    <<  0) );   // [    0] ca_swap_mode

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0xFFFF0000;
    if (ReadIO32( &pReg_DDRPHY->PHY_CON[0] ) & (0x1 << 13) )
        temp |= 0x0100;
    else
        temp |= 0xFF00;
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],        temp );
#endif
#if defined(MEM_TYPE_LPDDR23)
    WriteIO32( &pReg_DDRPHY->PHY_CON[22+1],     0x00000204 );       // lpddr2_addr[19:0] : Set swap pattern for write training.

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1] ) & 0xFFFF0000;
    if (ReadIO32( &pReg_DDRPHY->PHY_CON[0] ) & (0x1 << 13) )
        temp |= 0x0001;
    else
        temp |= 0x00FF;
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],        temp );
#endif

    SetIO32  ( &pReg_DDRPHY->PHY_CON[0],        (0x1    << 14) );   // p0_cmd_en[14] = 1

    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    << 26) );   // wr_deskew_con[26] = 1
    SetIO32  ( &pReg_DDRPHY->PHY_CON[2],        (0x1    << 27) );   // wr_deskew_en[27] = 1

    if (isResume)
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1   << 14) ) == 0 );  //- wait, rdlvl_complete[14]
        ClearIO32( &pReg_Drex->WRTRA_CONFIG,    (0x1    <<  0) );   // write_training_en[0] = 0

        MEMMSG("\r\n########## Write DQ Calibration - End ##########\r\n");
    }
    else
    {
        while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1   << 14) ) == 0 ) //- wait, rdlvl_complete[14]
        {
            if (cal_count > 100) {                                          // Failure Case
                //cal_error = 1;
                ClearIO32( &pReg_Drex->WRTRA_CONFIG, (0x1    <<  0) );      // write_training_en[0]=0 : Stopping it after completion of WRITE leveling.
                MEMMSG("\r\n\nWD DQ CAL Status Checking : fail...!!!\r\n");

                ret = CFALSE;
                goto wr_err_ret;
            }
            else
            {
                DMC_Delay(0x100);
                //MEMMSG("r\n\nWD DQ CAL Status Checking : %d\n", cal_count);
            }

            cal_count++;
        }

        nop();
        ClearIO32( &pReg_Drex->WRTRA_CONFIG,    (0x1    <<  0) );       // write_training_en[0] = 0

        //- Checking the calibration failure status
        //- After setting PHY_CON5(0x14) to "0xC", check whether PHY_CON21 is zero or nor. If PHY_CON21(0x58) is zero, calibration is normally complete.

        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    VWM_FAIL_STATUS );      // readmodecon[7:0] = 0xC
        cal_count = 0;
        do {
            if (cal_count > 100)
            {
                MEMMSG("\r\n\nWR VWM_FAIL_STATUS Checking : fail...!!!\r\n");
                ret = CFALSE;                                           // Failure Case
                goto wr_err_ret;
            }
            else if (cal_count)
            {
                DMC_Delay(0x100);
            }


            cal_count++;
        } while( ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] ) != 0x0 );

        //*** Write DQ Calibration Valid Window Margin
        //------------------------------------------------------------------------------------------------------------------------
    //    WriteIO32( &pReg_DDRPHY->PHY_CON[5],    VWM_CENTER);
        WriteIO32( &pReg_DDRPHY->PHY_CON[5],    WR_VWMC);
        g_WRvwmc = ReadIO32( &pReg_DDRPHY->PHY_CON[19+1] );
        //------------------------------------------------------------------------------------------------------------------------
    }

    for (i = 0; i < 4; i++)
    {
        wlvl_vwmc[i]   = ((g_WRvwmc >> (8*i)) & 0xFF);
    }


    //------------------------------------------------------------------------------------------------------------------------

#if (DDR_RESET_WRITE_DQ == 1)
//*** Forcing manually offsetr to stop the calibration mode after completion of WRITE DQ Cal

    lock_div4 = (g_DDRLock >> 2);
    for (i = 0; i < 4; i++)
    {
        offsetw[i] = (int)(wlvl_vwmc[i] - lock_div4);
        if (offsetw[i] < 0)
        {
            offsetw[i] *= -1;
            offsetw[i] |= 0x80;
        }
    }

#if 1
{
    int max, min, val;
    int i, inx;

//    temp = ( ((U8)offsetw[3] << 24) | ((U8)offsetw[2] << 16) | ((U8)offsetw[1] << 8) | (U8)offsetw[0] );
//    printf("WR DQ : Org value = 0x%08X\r\n", temp);

    for (inx = 0; inx < 4; inx++)
    {
        for (i = 1; i < 4; i++)
        {
            if (offsetw[i-1] > offsetw[i])
            {
               max          = offsetw[i-1];
               offsetw[i-1] = offsetw[i];
               offsetw[i]   = max;
            }
        }
    }

#if 0
    for (inx = 0; inx < 4; inx++)
    {
        printf( "Sorted Value[%d] = %d\r\n", inx, offsetw[inx] );
    }
#endif

    if ( offsetw[1] > offsetw[2])
    {
        max = offsetw[1];
        min = offsetw[2];
    }
    else
    {
        max = offsetw[2];
        min = offsetw[1];
    }
    temp = (max - min);

    if ( (max - min) > 5)
    {
        val = min;
    }
    else
    {
        val = max;
    }

    for (inx = 0; inx < 4; inx++)
    {
        offsetw[inx] = val;
    }
}
#endif

    temp = ( ((U8)offsetw[3] << 24) | ((U8)offsetw[2] << 16) | ((U8)offsetw[1] << 8) | (U8)offsetw[0] );
    WriteIO32( &pReg_DDRPHY->PHY_CON[6], temp );

    //*** Resync
    //*** Update WRITE SDLL Code (ctrl_offsetr) : Make "ctrl_resync" HIGH and LOW
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );
    SetIO32  ( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );          // ctrl_resync[24]=0x1 (HIGH)
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24) );          // ctrl_resync[24]=0x0 (LOW)
#endif  // #if (DDR_RESET_WRITE_DQ == 1)

wr_err_ret:

    ClearIO32( &pReg_DDRPHY->PHY_CON[2],    (0x3    << 26) );               // wr_deskew_en[27] = 0, wr_deskew_con[26] = 0

    WriteIO32( &pReg_DDRPHY->PHY_CON[5],    0x0 );                          // readmodecon[7:0] = 0x0

    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );              // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );              // Force DLL Resyncronization

    MEMMSG("\r\n########## Write DQ Calibration - End ##########\r\n");

    return ret;
}
#endif  // #if (DDR_WRITE_DQ_CALIB_EN == 1)
#endif  // #if (DDR_NEW_LEVELING_TRAINING == 1)


void initDDR3(U32 isResume)
{
    union SDRAM_MR MR0, MR1, MR2, MR3;
    U32 DDR_AL, DDR_WL, DDR_RL;
    U32 lock_div4;
    U32 temp;


    MEMMSG("\r\nDDR3 POR Init Start\r\n");

    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     1);                 // open alive power gate
    if (isResume)
    {
        g_GateCycle = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE5);    // read - ctrl_shiftc
        g_RDvwmc    = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE6);    // read - ctrl_offsetr
        g_WRvwmc    = ReadIO32(&pReg_Alive->ALIVESCRATCHVALUE7);    // read - ctrl_offsetw
    }

    WriteIO32(&pReg_Alive->ALIVESCRATCHRST5,    0xFFFFFFFF);        // clear - ctrl_shiftc
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST6,    0xFFFFFFFF);        // clear - ctrl_offsetr
    WriteIO32(&pReg_Alive->ALIVESCRATCHRST7,    0xFFFFFFFF);        // clear - ctrl_offsetw
    WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     0);                 // close alive power gate

    if (!g_GateCycle || !g_RDvwmc || !g_WRvwmc)
        isResume = 0;


#if (CFG_NSIH_EN == 0)
    MEMMSG("READDELAY   = 0x%08X\r\n", READDELAY);
    MEMMSG("WRITEDELAY  = 0x%08X\r\n", WRITEDELAY);
#else
    MEMMSG("READDELAY   = 0x%08X\r\n", pSBI->DII.READDELAY);
    MEMMSG("WRITEDELAY  = 0x%08X\r\n", pSBI->DII.WRITEDELAY);
#endif

    DDR_AL = 0;
#if (CFG_NSIH_EN == 0)
    if (MR1_nAL > 0)
        DDR_AL = nCL - MR1_nAL;

    DDR_WL = (DDR_AL + nCWL);
    DDR_RL = (DDR_AL + nCL);
#else
    if (pSBI->DII.MR1_AL > 0)
        DDR_AL = pSBI->DII.CL - pSBI->DII.MR1_AL;

    DDR_WL = (DDR_AL + pSBI->DII.CWL);
    DDR_RL = (DDR_AL + pSBI->DII.CL);
#endif

    MR2.Reg         = 0;
    MR2.MR2.RTT_WR  = 2; // RTT_WR - 0: disable, 1: RZQ/4, 2: RZQ/2
    MR2.MR2.SRT     = 0; // self refresh normal range
    MR2.MR2.ASR     = 0; // auto self refresh disable
#if (CFG_NSIH_EN == 0)
    MR2.MR2.CWL     = (nCWL - 5);
#else
    MR2.MR2.CWL     = (pSBI->DII.CWL - 5);
#endif

    MR3.Reg         = 0;
    MR3.MR3.MPR     = 0;
    MR3.MR3.MPR_RF  = 0;

    MR1.Reg         = 0;
    MR1.MR1.DLL     = 0;    // 0: Enable, 1 : Disable
#if (CFG_NSIH_EN == 0)
    MR1.MR1.AL      = MR1_nAL;
#else
    MR1.MR1.AL      = pSBI->DII.MR1_AL;
#endif
    MR1.MR1.ODS1    = 0;    // 00: RZQ/6, 01 : RZQ/7
    MR1.MR1.ODS0    = 1;
    MR1.MR1.QOff    = 0;
    MR1.MR1.RTT_Nom2    = 0;    // RTT_Nom - 001: RZQ/4, 010: RZQ/2, 011: RZQ/6, 100: RZQ/12, 101: RZQ/8
    MR1.MR1.RTT_Nom1    = 1;
    MR1.MR1.RTT_Nom0    = 0;
    MR1.MR1.WL      = 0;
#if (CFG_NSIH_EN == 0)
    MR1.MR1.TDQS    = (_DDR_BUS_WIDTH>>3) & 1;
#else
    MR1.MR1.TDQS    = (pSBI->DII.BusWidth>>3) & 1;
#endif

#if (CFG_NSIH_EN == 0)
    if (nCL > 11)
        temp = ((nCL-12) << 1) + 1;
    else
        temp = ((nCL-4) << 1);
#else
    if (pSBI->DII.CL > 11)
        temp = ((pSBI->DII.CL-12) << 1) + 1;
    else
        temp = ((pSBI->DII.CL-4) << 1);
#endif

    MR0.Reg         = 0;
    MR0.MR0.BL      = 0;
    MR0.MR0.BT      = 1;
    MR0.MR0.CL0     = (temp & 0x1);
    MR0.MR0.CL1     = ((temp>>1) & 0x7);
    MR0.MR0.DLL     = 0;//1;
#if (CFG_NSIH_EN == 0)
    MR0.MR0.WR      = MR0_nWR;
#else
    MR0.MR0.WR      = pSBI->DII.MR0_WR;
#endif
    MR0.MR0.PD      = 0;//1;

    // Step 1. reset (Min : 10ns, Typ : 200us)
    ClearIO32( NX_BASE_REG_PA_RSTCON, (0x7 << 26) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( NX_BASE_REG_PA_RSTCON, (0x7 << 26) );
    DMC_Delay(0x1000);                              // wait 300ms
    ClearIO32( NX_BASE_REG_PA_RSTCON, (0x7 << 26) );
    DMC_Delay(0x1000);                              // wait 300ms
    SetIO32  ( NX_BASE_REG_PA_RSTCON, (0x7 << 26) );
    DMC_Delay(0x10000);                             // wait 300ms

//    MEMMSG("PHY Version: %X\r\n", ReadIO32(&pReg_DDRPHY->VERSION_INFO));

// Step 2. Select Memory type : DDR3
// Check DDR3 MPR data and match it to PHY_CON[1]??

#if defined(MEM_TYPE_DDR3)
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],
        (0x0    <<  28) |           // [31:28] ctrl_gateadj
        (0x9    <<  24) |           // [27:24] ctrl_readadj
        (0x0    <<  20) |           // [23:20] ctrl_gateduradj  :: DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9
        (0x1    <<  16) |           // [19:16] rdlvl_pass_adj
//        (0x0100 <<   0) );          // [15: 0] rdlvl_rddata_adj :: DDR3 : 0x0100 or 0xFF00
        (0xFF00 <<   0) );          // [15: 0] rdlvl_rddata_adj :: DDR3 : 0x0100 or 0xFF00
#endif
#if defined(MEM_TYPE_LPDDR23)
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],
        (0x0    <<  28) |           // [31:28] ctrl_gateadj
        (0x9    <<  24) |           // [27:24] ctrl_readadj
        (0xB    <<  20) |           // [23:20] ctrl_gateduradj  :: DDR3: 0x0, LPDDR3: 0xB, LPDDR2: 0x9
        (0x1    <<  16) |           // [19:16] rdlvl_pass_adj
        (0x0001 <<   0) );          // [15: 0] rdlvl_rddata_adj :: LPDDR3 : 0x0001 or 0x00FF
//        (0x00FF <<   0) );          // [15: 0] rdlvl_rddata_adj :: LPDDR3 : 0x0001 or 0x00FF
#endif

    WriteIO32( &pReg_DDRPHY->PHY_CON[2],
        (0x0    <<  28) |           // [31:28] ctrl_readduradj
        (0x0    <<  27) |           // [   27] wr_deskew_en
        (0x0    <<  26) |           // [   26] wr_deskew_con
        (0x0    <<  25) |           // [   25] rdlvl_en
        (0x0    <<  24) |           // [   24] rdlvl_gate_en
        (0x0    <<  23) |           // [   23] rdlvl_ca_en
        (0x1    <<  16) |           // [22:16] rdlvl_incr_adj
        (0x0    <<  14) |           // [   14] wrdeskew_clear
        (0x0    <<  13) |           // [   13] rddeskew_clear
        (0x0    <<  12) |           // [   12] dlldeskew_en
        (0x2    <<  10) |           // [11:10] rdlvl_start_adj - Right shift, valid value: 1 or 2
        (0x1    <<   8) |           // [ 9: 8] rdlvl_start_adj - Left shift,  valid value: 0 ~ 2
        (0x0    <<   6) |           // [    6] initdeskewen
        (0x0    <<   0) );          // [ 1: 0] rdlvl_gateadj

    temp = (
        (0x0    <<  29) |           // [31:29] Reserved - SBZ.
        (0x17   <<  24) |           // [28:24] T_WrWrCmd.
//        (0x0    <<  22) |           // [23:22] Reserved - SBZ.
        (0x0    <<  20) |           // [21:20] ctrl_upd_range.
#if (CFG_NSIH_EN == 0)
#if (tWTR == 3)     // 6 cycles
        (0x7    <<  17) |           // [19:17] T_RwRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#elif (tWTR == 2)   // 4 cycles
        (0x6    <<  17) |           // [19:17] T_RwRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#endif
#endif
        (0x0    <<  16) |           // [   16] ctrl_wrlvl_en[16]. Write Leveling Enable. 0:Disable, 1:Enable
//        (0x0    <<  15) |           // [   15] Reserved SBZ.
        (0x0    <<  14) |           // [   14] p0_cmd_en. 0:Issue Phase1 Read command during Read Leveling. 1:Issue Phase0
        (0x0    <<  13) |           // [   13] byte_rdlvl_en. Read Leveling 0:Disable, 1:Enable
#if defined(MEM_TYPE_DDR3)
        (0x1    <<  11) |           // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
#endif
#if defined(MEM_TYPE_LPDDR23)
        (0x3    <<  11) |           // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
#endif
//        (0x0    <<  10) |           // [   10] Reserved - SBZ.
        (0x1    <<   9) |           // [    9] ctrl_dfdqs. 0:Single-ended DQS, 1:Differential DQS
//        (0x0    <<   8) |           // [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
        (0x1    <<   8) |           // [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
        (0x0    <<   7) |           // [    7] ctrl_ckdis. 0:Clock output Enable, 1:Disable
        (0x1    <<   6) |           // [    6] ctrl_atgate.
        (0x1    <<   5) |           // [    5] ctrl_read_disable. Read ODT disable signal. Variable. Set to '1', when you need Read Leveling test.
        (0x0    <<   4) |           // [    4] ctrl_cmosrcv.
        (0x0    <<   3) |           // [    3] ctrl_read_width.
        (0x0    <<   0));           // [ 2: 0] ctrl_fnc_fb. 000:Normal operation.

#if (CFG_NSIH_EN == 1)
    if ((pSBI->DII.TIMINGDATA >> 28) == 3)      // 6 cycles
        temp |= (0x7    <<  17);
    else if ((pSBI->DII.TIMINGDATA >> 28) == 2) // 4 cycles
        temp |= (0x6    <<  17);
#endif

    WriteIO32( &pReg_DDRPHY->PHY_CON[0],    temp );

    MEMMSG("phy init\r\n");

/*  Set ZQ clock div    */
    WriteIO32( &pReg_DDRPHY->PHY_CON[40+1], 0x07);

/*  Set ZQ Timer    */
//    WriteIO32( &pReg_DDRPHY->PHY_CON[41+1], 0xF0);

/* Set WL, RL, BL */
    WriteIO32( &pReg_DDRPHY->PHY_CON[42+1],
        (0x8    <<   8) |       // Burst Length(BL)
        (DDR_RL <<   0));       // Read Latency(RL), 800MHz:0xB, 533MHz:0x5

/* Set WL  */
#if defined(MEM_TYPE_DDR3)
    temp = ((0x105E <<  16)| 0x107E);                               // cmd_active= DDR3:0x105E, DDR3:0x107E
    WriteIO32( &pReg_DDRPHY->PHY_CON[25+1], temp);

#if (CFG_NSIH_EN == 0)
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            (DDR_WL <<  16) |       // T_wrdata_en, In DDR3
            (0x1    <<  12) |       // [  :12] RESET
            (0x0    <<   9) |       // [11: 9] BANK
            (0x0    <<   7) |       // [ 8: 7] ODT
            (0x1    <<   6) |       // [  : 6] RAS
            (0x1    <<   5) |       // [  : 5] CAS
            (0x1    <<   4) |       // [  : 4] WEN
#if (_DDR_CS_NUM > 1)
            (0x3    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x3    <<   0) );      // [ 1: 0] CS[1:0]
#else
            (0x1    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x1    <<   0) );      // [ 1: 0] CS[1:0]
#endif
#else
    if(pSBI->DII.ChipNum > 1)
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            (DDR_WL <<  16) |       // T_wrdata_en, In DDR3
            (0x1    <<  12) |       // [  :12] RESET
            (0x0    <<   9) |       // [11: 9] BANK
            (0x0    <<   7) |       // [ 8: 7] ODT
            (0x1    <<   6) |       // [  : 6] RAS
            (0x1    <<   5) |       // [  : 5] CAS
            (0x1    <<   4) |       // [  : 4] WEN
            (0x3    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x3    <<   0) );      // [ 1: 0] CS[1:0]
    }
    else
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            (DDR_WL <<  16) |       // T_wrdata_en, In DDR3
            (0x1    <<  12) |       // [  :12] RESET
            (0x0    <<   9) |       // [11: 9] BANK
            (0x0    <<   7) |       // [ 8: 7] ODT
            (0x1    <<   6) |       // [  : 6] RAS
            (0x1    <<   5) |       // [  : 5] CAS
            (0x1    <<   4) |       // [  : 4] WEN
            (0x1    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x1    <<   0) );      // [ 1: 0] CS[1:0]
    }
#endif  // #if (CFG_NSIH_EN == 0)
#endif  // #if defined(MEM_TYPE_DDR3)
#if defined(MEM_TYPE_LPDDR23)
    temp = ((0x105E <<  16)| 0x000E);                               // cmd_active= DDR3:0x105E, LPDDDR2 or LPDDDR3:0x000E
    WriteIO32( &pReg_DDRPHY->PHY_CON[25+1], temp);

#if (CFG_NSIH_EN == 0)
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            ((DDR_WL+1) <<  16) |   // T_wrdata_en, In LPDDR3 (WL+1)
#if (_DDR_CS_NUM > 1)
            (0x3    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x3    <<   0) );      // [ 1: 0] CS[1:0]
#else
            (0x1    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x1    <<   0) );      // [ 1: 0] CS[1:0]
#endif
#else
    if(pSBI->DII.ChipNum > 1)
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            ((DDR_WL+1) <<  16) |
            (0x3    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x3    <<   0) );      // [ 1: 0] CS[1:0]
    }
    else
    {
        WriteIO32( &pReg_DDRPHY->PHY_CON[26+1],
            ((DDR_WL+1) <<  16) |
            (0x1    <<   2) |       // [ 3: 2] CKE[1:0]
            (0x1    <<   0) );      // [ 1: 0] CS[1:0]
    }
#endif  // #if (CFG_NSIH_EN == 0)
#endif  // #if defined(MEM_TYPE_LPDDR23)

    /* ZQ Calibration */
    WriteIO32( &pReg_DDRPHY->PHY_CON[39+1],         // 100: 48ohm, 101: 40ohm, 110: 34ohm, 111: 30ohm
        (PHY_DRV_STRENGTH_34OHM <<  25) |           // Data Slice 3
        (PHY_DRV_STRENGTH_34OHM <<  22) |           // Data Slice 2
        (PHY_DRV_STRENGTH_34OHM <<  19) |           // Data Slice 1
        (PHY_DRV_STRENGTH_34OHM <<  16) |           // Data Slice 0
        (PHY_DRV_STRENGTH_34OHM <<   9) |           // CK
        (PHY_DRV_STRENGTH_34OHM <<   6) |           // CKE
        (PHY_DRV_STRENGTH_34OHM <<   3) |           // CS
        (PHY_DRV_STRENGTH_48OHM <<   0));           // CA[9:0], RAS, CAS, WEN, ODT[1:0], RESET, BANK[2:0]

    // Driver Strength(zq_mode_dds), zq_clk_div_en[18]=Enable
    WriteIO32( &pReg_DDRPHY->PHY_CON[16],
        (0x0    <<  28) |           // Reserved[31:28]. Should be '0'
        (0x1    <<  27) |           // zq_clk_en[27]. ZQ I/O clock enable.
        (PHY_DRV_STRENGTH_34OHM <<  24) |   // zq_mode_dds[26:24], Driver strength selection. 100 : 48ohm, 101 : 40ohm, 110 : 34ohm, 111 : 30ohm
        (PHY_ODT_120OHM <<  21) |   // ODT resistor value[23:21]. 001 : 120ohm, 010 : 60ohm, 011 : 40ohm, 100 : 30ohm
        (0x0    <<  20) |           // zq_rgddr3[20]. GDDR3 mode. 0:Enable, 1:Disable
        (0x0    <<  19) |           // zq_mode_noterm[19]. Termination. 0:Enable, 1:Disable
        (0x1    <<  18) |           // zq_clk_div_en[18]. Clock Dividing Enable : 0, Disable : 1
        (0x0    <<  15) |           // zq_force-impn[17:15]
        (0x0    <<  12) |           // zq_force-impp[14:12]
        (0x30   <<   4) |           // zq_udt_dly[11:4]
        (0x1    <<   2) |           // zq_manual_mode[3:2]. 0:Force Calibration, 1:Long cali, 2:Short cali
        (0x0    <<   1) |           // zq_manual_str[1]. Manual Calibration Stop : 0, Start : 1
        (0x0    <<   0));           // zq_auto_en[0]. Auto Calibration enable

    SetIO32( &pReg_DDRPHY->PHY_CON[16],     (0x1    <<   1) );          // zq_manual_str[1]. Manual Calibration Start=1
    while( ( ReadIO32( &pReg_DDRPHY->PHY_CON[17+1] ) & 0x1 ) == 0 );    //- PHY0: wait for zq_done
    ClearIO32( &pReg_DDRPHY->PHY_CON[16],   (0x1    <<   1) );          // zq_manual_str[1]. Manual Calibration Stop : 0, Start : 1

    ClearIO32( &pReg_DDRPHY->PHY_CON[16],   (0x1    <<  18) );          // zq_clk_div_en[18]. Clock Dividing Enable : 1, Disable : 0


    // Step 3. Set the PHY for dqs pull down mode
    WriteIO32( &pReg_DDRPHY->PHY_CON[14],
        (0x0    <<   8) |       // ctrl_pulld_dq[11:8]
        (0xF    <<   0));       // ctrl_pulld_dqs[7:0].  No Gate leveling : 0xF, Use Gate leveling : 0x0(X)
    // Step 4. ODT
    WriteIO32( &pReg_Drex->PHYCONTROL[0],
        (0x1    <<  31) |           // [   31] mem_term_en. Termination Enable for memory. Disable : 0, Enable : 1
        (0x1    <<  30) |           // [   30] phy_term_en. Termination Enable for PHY. Disable : 0, Enable : 1
        (0x1    <<  29) |           // [   29] ctrl_shgate. Duration of DQS Gating Signal. gate signal length <= 200MHz : 0, >200MHz : 1
        (0x0    <<  24) |           // [28:24] ctrl_pd. Input Gate for Power Down.
//        (0x0    <<   7) |           // [23: 7] Reserved - SBZ
        (0x0    <<   4) |           // [ 6: 4] dqs_delay. Delay cycles for DQS cleaning. refer to DREX datasheet
//        (0x1    <<   3) |           // [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
        (0x0    <<   3) |           // [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
        (0x0    <<   2) |           // [    2] Reserved - SBZ
        (0x0    <<   1) |           // [    1] sl_dll_dyn_con. Turn On PHY slave DLL dynamically. Disable : 0, Enable : 1
        (0x0    <<   0));           // [    0] mem_term_chips. Memory Termination between chips(2CS). Disable : 0, Enable : 1

#if 1
    WriteIO32( &pReg_Drex->CONCONTROL,
        (0x0    <<  28) |           // [   28] dfi_init_start
        (0xFFF  <<  16) |           // [27:16] timeout_level0
        (0x3    <<  12) |           // [14:12] rd_fetch
        (0x1    <<   8) |           // [    8] empty
        (0x1    <<   5) |           // [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
        (0x0    <<   3) |           // [    3] io_pd_con - I/O Powerdown Control in Low Power Mode(through LPI)
        (0x0    <<   1));           // [ 2: 1] clk_ratio. Clock ratio of Bus clock to Memory clock. 0x0 = 1:1, 0x1~0x3 = Reserved
#else

    temp = (U32)(
        (0x0    <<  28) |           // [   28] dfi_init_start
        (0xFFF  <<  16) |           // [27:16] timeout_level0
        (0x3    <<  12) |           // [14:12] rd_fetch
        (0x1    <<   8) |           // [    8] empty
        (0x1    <<   5) |           // [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
        (0x0    <<   3) |           // [    3] io_pd_con - I/O Powerdown Control in Low Power Mode(through LPI)
        (0x0    <<   1));           // [ 2: 1] clk_ratio. Clock ratio of Bus clock to Memory clock. 0x0 = 1:1, 0x1~0x3 = Reserved

    if (isResume)
        temp &= ~(0x1    <<   5);

    WriteIO32( &pReg_Drex->CONCONTROL,  temp );
#endif

    // Step 5. dfi_init_start : High
    SetIO32( &pReg_Drex->CONCONTROL,    (0x1    <<  28) );          // DFI PHY initialization start
    while( (ReadIO32( &pReg_Drex->PHYSTATUS ) & (0x1<<3) ) == 0);   // wait for DFI PHY initialization complete
    ClearIO32( &pReg_Drex->CONCONTROL,  (0x1    <<  28) );          // DFI PHY initialization clear

    WriteIO32( &pReg_DDRPHY->PHY_CON[12],
        (0x10   <<  24) |           // [30:24] ctrl_start_point
        (0x10   <<  16) |           // [22:16] ctrl_inc
        (0x0    <<   8) |           // [14: 8] ctrl_force
        (0x1    <<   6) |           // [    6] ctrl_start
        (0x1    <<   5) |           // [    5] ctrl_dll_on
        (0xF    <<   1));           // [ 4: 1] ctrl_ref
    DMC_Delay(1);


    // Step 8 : Update DLL information
    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );      // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );      // Force DLL Resyncronization


    // Step 11. MemBaseConfig
    WriteIO32( &pReg_Drex->MEMBASECONFIG[0],
        (0x0        <<  16) |                   // chip_base[26:16]. AXI Base Address. if 0x20 ==> AXI base addr of memory : 0x2000_0000
#if (CFG_NSIH_EN == 0)
        (chip_mask  <<   0));                   // 256MB:0x7F0, 512MB: 0x7E0, 1GB:0x7C0, 2GB: 0x780, 4GB:0x700
#else
        (pSBI->DII.ChipMask <<   0));           // chip_mask[10:0]. 1GB:0x7C0, 2GB:0x780
#endif

#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    {
        WriteIO32( &pReg_Drex->MEMBASECONFIG[1],
//            (chip_base1 <<  16) |               // chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory : 0x4000_0000, 16MB unit
            (0x040      <<  16) |               // chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory : 0x4000_0000, 16MB unit
            (chip_mask  <<   0));               // chip_mask[10:0]. 2048 - chip size
    }
#endif
#else
    if(pSBI->DII.ChipNum > 1)
    {
        WriteIO32( &pReg_Drex->MEMBASECONFIG[1],
            (pSBI->DII.ChipBase <<  16) |       // chip_base[26:16]. AXI Base Address. if 0x40 ==> AXI base addr of memory : 0x4000_0000, 16MB unit
            (pSBI->DII.ChipMask <<   0));       // chip_mask[10:0]. 2048 - chip size
    }
#endif

    // Step 12. MemConfig
    WriteIO32( &pReg_Drex->MEMCONFIG[0],
//        (0x0    <<  16) |           // [31:16] Reserved - SBZ
        (0x1    <<  12) |           // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0:Linear(Bank, Row, Column, Width), 1:Interleaved(Row, bank, column, width), other : reserved
#if (CFG_NSIH_EN == 0)
        (chip_col   <<   8) |       // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
        (chip_row   <<   4) |       // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
#else
        (pSBI->DII.ChipCol  <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
        (pSBI->DII.ChipRow  <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
#endif
        (0x3    <<   0));           // [ 3: 0] chip_bank. Number of  Bank Address Bit. others:Reserved, 2:4bank, 3:8banks


#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
        WriteIO32( &pReg_Drex->MEMCONFIG[1],
//            (0x0    <<  16) |       // [31:16] Reserved - SBZ
            (0x1    <<  12) |       // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0 : Linear(Bank, Row, Column, Width), 1 : Interleaved(Row, bank, column, width), other : reserved
            (chip_col   <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
            (chip_row   <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
            (0x3    <<   0));       // [ 3: 0] chip_bank. Number of  Row Address Bit. others:Reserved, 2:4bank, 3:8banks
#endif
#else
    if(pSBI->DII.ChipNum > 1) {
        WriteIO32( &pReg_Drex->MEMCONFIG[1],
//            (0x0    <<  16) |       // [31:16] Reserved - SBZ
            (0x1    <<  12) |       // [15:12] chip_map. Address Mapping Method (AXI to Memory). 0 : Linear(Bank, Row, Column, Width), 1 : Interleaved(Row, bank, column, width), other : reserved
            (pSBI->DII.ChipCol  <<   8) |   // [11: 8] chip_col. Number of Column Address Bit. others:Reserved, 2:9bit, 3:10bit,
            (pSBI->DII.ChipRow  <<   4) |   // [ 7: 4] chip_row. Number of  Row Address Bit. others:Reserved, 0:12bit, 1:13bit, 2:14bit, 3:15bit, 4:16bit
            (0x3    <<   0));       // [ 3: 0] chip_bank. Number of  Row Address Bit. others:Reserved, 2:4bank, 3:8banks
    }
#endif

    // Step 13. Precharge Configuration
//    WriteIO32( &pReg_Drex->PRECHCONFIG,     0xFF000000 );           //- precharge policy counter
//    WriteIO32( &pReg_Drex->PWRDNCONFIG,     0xFFFF00FF );           //- low power counter



    // Step 14.  AC Timing
#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_Drex->TIMINGAREF,
        (tREFI      <<   0));       //- refresh counter, 800MHz : 0x618

    WriteIO32( &pReg_Drex->TIMINGROW,
        (tRFC       <<  24) |
        (tRRD       <<  20) |
        (tRP        <<  16) |
        (tRCD       <<  12) |
        (tRC        <<   6) |
        (tRAS       <<   0)) ;
    WriteIO32( &pReg_Drex->TIMINGDATA,
        (tWTR       <<  28) |
        (tWR        <<  24) |
        (tRTP       <<  20) |
        (W2W_C2C    <<  16) |
        (R2R_C2C    <<  15) |
        (tDQSCK     <<  12) |
        (nWL        <<   8) |
        (nRL        <<   0));

    WriteIO32( &pReg_Drex->TIMINGPOWER,
        (tFAW       <<  26) |
        (tXSR       <<  16) |
        (tXP        <<   8) |
        (tCKE       <<   4) |
        (tMRD       <<   0));

//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );     //- average periodic ZQ interval. Max:0x4084
    WriteIO32( &pReg_Drex->TIMINGPZQ,   tPZQ );           //- average periodic ZQ interval. Max:0x4084
#else

    WriteIO32( &pReg_Drex->TIMINGAREF,      pSBI->DII.TIMINGAREF );         //- refresh counter, 800MHz : 0x618
    WriteIO32( &pReg_Drex->TIMINGROW,       pSBI->DII.TIMINGROW) ;
    WriteIO32( &pReg_Drex->TIMINGDATA,      pSBI->DII.TIMINGDATA );
    WriteIO32( &pReg_Drex->TIMINGPOWER,     pSBI->DII.TIMINGPOWER );

//    WriteIO32( &pReg_Drex->TIMINGPZQ,   0x00004084 );               //- average periodic ZQ interval. Max:0x4084
    WriteIO32( &pReg_Drex->TIMINGPZQ,   pSBI->DII.TIMINGPZQ );      //- average periodic ZQ interval. Max:0x4084
#endif

#if 1
#if defined(ARCH_NXP4330) || defined(ARCH_S5P4418)
#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_DDRPHY->PHY_CON[4],    READDELAY);
    WriteIO32( &pReg_DDRPHY->PHY_CON[6],    WRITEDELAY);
#else
    WriteIO32( &pReg_DDRPHY->PHY_CON[4],    pSBI->DII.READDELAY);
    WriteIO32( &pReg_DDRPHY->PHY_CON[6],    pSBI->DII.WRITEDELAY);
#endif
#endif
#if defined(ARCH_NXP5430)
#if (CFG_NSIH_EN == 0)
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0], READDELAY);
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0], WRITEDELAY);
#else
    WriteIO32( &pReg_DDRPHY->OFFSETR_CON[0], pSBI->DII.READDELAY);
    WriteIO32( &pReg_DDRPHY->OFFSETW_CON[0], pSBI->DII.WRITEDELAY);
#endif
#endif
#endif



    // Step 52  auto refresh start.
//    SetIO32( &pReg_Drex->CONCONTROL,        (0x1    <<   5));            // afre_en[5]. Auto Refresh Counter. Disable:0, Enable:1

    // Step 16. Confirm that after RESET# is de-asserted, 500 us have passed before CKE becomes active.
    // Step 17. Confirm that clocks(CK, CK#) need to be started and
    //     stabilized for at least 10 ns or 5 tCK (which is larger) before CKE goes active.


    // Step 18, 19 :  Send NOP command.
    SendDirectCommand(SDRAM_CMD_NOP, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_NOP, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif


    // Step 20 :  Send MR2 command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR2, MR2.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR2.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR2, MR2.Reg);
#endif

#if (REG_MSG)
    DebugPutString("\r\n########## pReg_Drex->DIRECTCMD[MR2] \t:");
    DebugPutHex(pReg_Drex->DIRECTCMD);
#endif

    // Step 21 :  Send MR3 command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR3, MR3.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR3.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR3, MR3.Reg);
#endif



    // Step 22 :  Send MR1 command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR1, MR1.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR1, MR1.Reg);
#endif

#if (REG_MSG)
    DebugPutString("\r\n########## pReg_Drex->DIRECTCMD[MR1] \t:");
    DebugPutHex(pReg_Drex->DIRECTCMD);
#endif


    // Step 23 :  Send MR0 command.
    SendDirectCommand(SDRAM_CMD_MRS, 0, SDRAM_MODE_REG_MR0, MR0.Reg);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR0, MR0.Reg);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_MRS, 1, SDRAM_MODE_REG_MR0, MR0.Reg);
#endif
//    DMC_Delay(100);
#if (REG_MSG)
    DebugPutString("\r\n########## pReg_Drex->DIRECTCMD[MR0] \t:");
    DebugPutHex(pReg_Drex->DIRECTCMD);
#endif


// Step 24
//        ClearIO32( &pReg_Drex->DIRECTCMD,           (0x1    <<   8));                   // DLL Reset[8]. 0:No, 1:Reset



// Step 51
/*
    WriteIO32( &pReg_Drex->MEMCONTROL,
        (0x0    <<  25) |           // mrr_byte[26:25]  : Mode Register Read Byte lane location
        (0x0    <<  24) |           // pzq_en[24]       : DDR3 periodic ZQ(ZQCS) enable
        (0x0    <<  23) |           // SBZ
        (0x3    <<  20) |           // bl[22:20]        : Memory Burst Length                       :: 3'h3  - 8
        (0x0    <<  16) |           // num_chip[19:16]  : Number of Memory Chips                    :: 4'h0  - 1chips
        (0x2    <<  12) |           // mem_width[15:12] : Width of Memory Data Bus                  :: 4'h2  - 32bits
        (0x6    <<   8) |           // mem_type[11:8]   : Type of Memory                            :: 4'h6  - ddr3
        (0x0    <<   6) |           // add_lat_pall[7:6]: Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
        (0x0    <<   5) |           // dsref_en[5]      : Dynamic Self Refresh                      :: 1'b0  - Disable
        (0x0    <<   4) |           // tp_en[4]         : Timeout Precharge                         :: 1'b0  - Disable
        (0x0    <<   2) |           // dpwrdn_type[3:2] : Type of Dynamic Power Down                :: 2'b00 - Active/precharge power down
        (0x0    <<   1) |           // dpwrdn_en[1]     : Dynamic Power Down                        :: 1'b0  - Disable
        (0x0    <<   0));           // clk_stop_en[0]   : Dynamic Clock Control                     :: 1'b0  - Always running
*/


    // Step 25 :  Send ZQ Init command
    SendDirectCommand(SDRAM_CMD_ZQINIT, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_ZQINIT, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_ZQINIT, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
    DMC_Delay(100);



#if 1   //(CONFIG_ODTOFF_GATELEVELINGON)
    MEMMSG("\r\n########## READ/GATE Level ##########\r\n");

#if (DDR_NEW_LEVELING_TRAINING == 0)
    SetIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1  <<  6) );       // ctrl_atgate=1
    SetIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1  << 14) );       // p0_cmd_en=1
    SetIO32( &pReg_DDRPHY->PHY_CON[2],        (0x1  <<  6) );       // InitDeskewEn=1
    SetIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1  << 13) );       // byte_rdlvl_en=1
#else
    SetIO32( &pReg_DDRPHY->PHY_CON[0],        (0x1  <<  6) );       // ctrl_atgate=1
    SetIO32( &pReg_DDRPHY->PHY_CON[2],        (0x1  <<  6) );       // InitDeskewEn=1
#endif

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[1]) & ~(0xF <<  16);    // rdlvl_pass_adj=4
    temp |= (0x4 <<  16);
    WriteIO32( &pReg_DDRPHY->PHY_CON[1],    temp);

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[2]) & ~(0x7F << 16);    // rdlvl_incr_adj=1
    temp |= (0x1 <<  16);
    WriteIO32( &pReg_DDRPHY->PHY_CON[2],    temp);


    g_DDRLock = (ReadIO32( &pReg_DDRPHY->PHY_CON[13]) >> 8) & 0x1FF;
    lock_div4 = (g_DDRLock >> 2);

    temp  = ReadIO32( &pReg_DDRPHY->PHY_CON[12] );
    temp &= ~((0x7F <<  8) | (0x1 <<  5));
    temp |= (lock_div4 <<  8);                                          // ctrl_force[14:8], ctrl_dll_on[5]=0
    WriteIO32( &pReg_DDRPHY->PHY_CON[12],   temp );

#if (DDR_NEW_LEVELING_TRAINING == 0)
    SetIO32( &pReg_DDRPHY->PHY_CON[2],      (0x1    <<  24) );          // rdlvl_gate_en=1

    SetIO32( &pReg_DDRPHY->PHY_CON[0],      (0x1    <<   8) );          // ctrl_shgate=1
    ClearIO32( &pReg_DDRPHY->PHY_CON[1],    (0xF    <<  20) );          // ctrl_gateduradj=0

    WriteIO32( &pReg_Drex->RDLVL_CONFIG,    0x00000001 );               // ctrl_rdlvl_data_en[1]=1, Gate Traning : Enable
    while( ( ReadIO32( &pReg_Drex->PHYSTATUS ) & 0x4000 ) != 0x4000 );  // Rdlvl_complete_ch0[14]=1

//    WriteIO32( &pReg_Drex->RDLVL_CONFIG,    0x00000000 );               //- ctrl_rdlvl_data_en[1]=0, Gate Traning : Disable
    WriteIO32( &pReg_Drex->RDLVL_CONFIG,    0x00000001 );               // LINARO

    WriteIO32( &pReg_DDRPHY->PHY_CON[14],   0x00000000 );               // ctrl_pulld_dq[11:8]=0x0, ctrl_pulld_dqs[3:0]=0x0

    SetIO32  ( &pReg_DDRPHY->PHY_CON[12],   (0x1    <<   6) );          // ctrl_start[6]=1
#else   // #if (DDR_NEW_LEVELING_TRAINING == 1)

    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );          // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3) );          // Force DLL Resyncronization

#if (DDR_WRITE_LEVELING_EN == 1)
    DDR_Write_Leveling();
#endif
#if (DDR_GATE_LEVELING_EN == 1)
    DDR_Gate_Leveling(isResume);
#endif
#if (DDR_READ_DQ_CALIB_EN == 1)
    DDR_Read_DQ_Calibration(isResume);
#endif
#if (DDR_WRITE_DQ_CALIB_EN == 1)
    DDR_Write_DQ_Calibration(isResume);
#endif

//    if (isResume == 0)
    {
        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     1);                 // open alive power gate
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET5,    g_GateCycle);       // store - ctrl_shiftc
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET6,    g_RDvwmc);          // store - ctrl_offsetr
        WriteIO32(&pReg_Alive->ALIVESCRATCHSET7,    g_WRvwmc);          // store - ctrl_offsetw
        WriteIO32(&pReg_Alive->ALIVEPWRGATEREG,     0);                 // close alive power gate
    }

    WriteIO32( &pReg_DDRPHY->PHY_CON[14],   0x00000000 );               // ctrl_pulld_dq[11:8]=0x0, ctrl_pulld_dqs[3:0]=0x0
    ClearIO32( &pReg_DDRPHY->PHY_CON[0],    (0x3    << 13) );           // p0_cmd_en[14]=0, byte_rdlvl_en[13]=0
#endif  // #if (DDR_NEW_LEVELING_TRAINING == 1)

    SetIO32( &pReg_DDRPHY->PHY_CON[12],     (0x1    <<   5));           // ctrl_dll_on[5]=1
    SetIO32( &pReg_DDRPHY->PHY_CON[2],      (0x1    <<  12));           // DLLDeskewEn[2]=1

#if defined(ARCH_NXP4330) || defined(ARCH_S5P4418)
    SetIO32  ( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24));           // ctrl_resync=1
    ClearIO32( &pReg_DDRPHY->PHY_CON[10],   (0x1    <<  24));           // ctrl_resync=0
#endif
#if defined(ARCH_NXP5430)
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,   (0x1    <<  28));           // upd_mode=0

    SetIO32  ( &pReg_DDRPHY->OFFSETD_CON,   (0x1    <<  24));           // ctrl_resync=1
    ClearIO32( &pReg_DDRPHY->OFFSETD_CON,   (0x1    <<  24));           // ctrl_resync=0
#endif

#endif  // gate leveling



    SetIO32  ( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3));           // Force DLL Resyncronization
    ClearIO32( &pReg_Drex->PHYCONTROL[0],   (0x1    <<   3));           // Force DLL Resyncronization

    temp = (U32)(
        (0x0    <<  29) |           // [31:29] Reserved - SBZ.
        (0x17   <<  24) |           // [28:24] T_WrWrCmd.
//        (0x0    <<  22) |           // [23:22] Reserved - SBZ.
        (0x0    <<  20) |           // [21:20] ctrl_upd_range.
#if (CFG_NSIH_EN == 0)
#if (tWTR == 3)     // 6 cycles
        (0x7    <<  17) |           // [19:17] T_RwRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#elif (tWTR == 2)   // 4 cycles
        (0x6    <<  17) |           // [19:17] T_RwRdCmd. 6:tWTR=4cycle, 7:tWTR=6cycle
#endif
#endif
        (0x0    <<  16) |           // [   16] ctrl_wrlvl_en[16]. Write Leveling Enable. 0:Disable, 1:Enable
//        (0x0    <<  15) |           // [   15] Reserved SBZ.
        (0x0    <<  14) |           // [   14] p0_cmd_en. 0:Issue Phase1 Read command during Read Leveling. 1:Issue Phase0
        (0x0    <<  13) |           // [   13] byte_rdlvl_en. Read Leveling 0:Disable, 1:Enable
        (0x1    <<  11) |           // [12:11] ctrl_ddr_mode. 0:DDR2&LPDDR1, 1:DDR3, 2:LPDDR2, 3:LPDDR3
//        (0x0    <<  10) |           // [   10] Reserved - SBZ.
        (0x1    <<   9) |           // [    9] ctrl_dfdqs. 0:Single-ended DQS, 1:Differential DQS
//        (0x0    <<   8) |           // [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
        (0x1    <<   8) |           // [    8] ctrl_shgate. 0:Gate signal length=burst length/2+N, 1:Gate signal length=burst length/2-1
        (0x0    <<   7) |           // [    7] ctrl_ckdis. 0:Clock output Enable, 1:Disable
        (0x1    <<   6) |           // [    6] ctrl_atgate.
        (0x1    <<   5) |           // [    5] ctrl_read_disable. Read ODT disable signal. Variable. Set to '1', when you need Read Leveling test.
        (0x0    <<   4) |           // [    4] ctrl_cmosrcv.
        (0x0    <<   3) |           // [    3] ctrl_read_width.
        (0x0    <<   0));           // [ 2: 0] ctrl_fnc_fb. 000:Normal operation.

#if (CFG_NSIH_EN == 1)
    if ((pSBI->DII.TIMINGDATA >> 28) == 3)      // 6 cycles
        temp |= (0x7    <<  17);
    else if ((pSBI->DII.TIMINGDATA >> 28) == 2) // 4 cycles
        temp |= (0x6    <<  17);
#endif

    WriteIO32( &pReg_DDRPHY->PHY_CON[0],    temp );


    /* Send PALL command */
    SendDirectCommand(SDRAM_CMD_PALL, 0, (SDRAM_MODE_REG)CNULL, CNULL);
#if (CFG_NSIH_EN == 0)
#if (_DDR_CS_NUM > 1)
    SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif
#else
    if(pSBI->DII.ChipNum > 1)
        SendDirectCommand(SDRAM_CMD_PALL, 1, (SDRAM_MODE_REG)CNULL, CNULL);
#endif


    WriteIO32( &pReg_Drex->MEMCONTROL,
        (0x0    <<  25) |           // [26:25] mrr_byte     : Mode Register Read Byte lane location
        (0x0    <<  24) |           // [   24] pzq_en       : DDR3 periodic ZQ(ZQCS) enable
//        (0x0    <<  23) |           // [   23] reserved     : SBZ
        (0x3    <<  20) |           // [22:20] bl           : Memory Burst Length                       :: 3'h3  - 8
#if (CFG_NSIH_EN == 0)
        ((_DDR_CS_NUM-1)        <<  16) |   // [19:16] num_chip : Number of Memory Chips                :: 4'h0  - 1chips
#else
        ((pSBI->DII.ChipNum-1)  <<  16) |   // [19:16] num_chip : Number of Memory Chips                :: 4'h0  - 1chips
#endif
        (0x2    <<  12) |           // [15:12] mem_width    : Width of Memory Data Bus                  :: 4'h2  - 32bits
        (0x6    <<   8) |           // [11: 8] mem_type     : Type of Memory                            :: 4'h6  - ddr3
        (0x0    <<   6) |           // [ 7: 6] add_lat_pall : Additional Latency for PALL in cclk cycle :: 2'b00 - 0 cycle
        (0x0    <<   5) |           // [    5] dsref_en     : Dynamic Self Refresh                      :: 1'b0  - Disable
        (0x0    <<   4) |           // [    4] tp_en        : Timeout Precharge                         :: 1'b0  - Disable
        (0x0    <<   2) |           // [ 3: 2] dpwrdn_type  : Type of Dynamic Power Down                :: 2'b00 - Active/precharge power down
        (0x0    <<   1) |           // [    1] dpwrdn_en    : Dynamic Power Down                        :: 1'b0  - Disable
        (0x0    <<   0));           // [    0] clk_stop_en  : Dynamic Clock Control                     :: 1'b0  - Always running

    WriteIO32( &pReg_Drex->PHYCONTROL[0],
        (0x1    <<  31) |           // [   31] mem_term_en. Termination Enable for memory. Disable : 0, Enable : 1
        (0x1    <<  30) |           // [   30] phy_term_en. Termination Enable for PHY. Disable : 0, Enable : 1
        (0x1    <<  29) |           // [   29] ctrl_shgate. Duration of DQS Gating Signal. gate signal length <= 200MHz : 0, >200MHz : 1
        (0x0    <<  24) |           // [28:24] ctrl_pd. Input Gate for Power Down.
//        (0x0    <<   7) |           // [23: 7] reserved - SBZ
        (0x0    <<   4) |           // [ 6: 4] dqs_delay. Delay cycles for DQS cleaning. refer to DREX datasheet
        (0x0    <<   3) |           // [    3] fp_resync. Force DLL Resyncronization : 1. Test : 0x0
        (0x0    <<   1) |           // [    1] sl_dll_dyn_con. Turn On PHY slave DLL dynamically. Disable : 0, Enable : 1
        (0x0    <<   0));           // [    0] mem_term_chips. Memory Termination between chips(2CS). Disable : 0, Enable : 1

    WriteIO32( &pReg_Drex->CONCONTROL,
        (0x0    <<  28) |           // [   28] dfi_init_start
        (0xFFF  <<  16) |           // [27:16] timeout_level0
        (0x1    <<  12) |           // [14:12] rd_fetch
        (0x1    <<   8) |           // [    8] empty
        (0x1    <<   5) |           // [    5] aref_en - Auto Refresh Counter. Disable:0, Enable:1
        (0x0    <<   3) |           // [    3] io_pd_con - I/O Powerdown Control in Low Power Mode(through LPI)
        (0x0    <<   1));           // [ 2: 1] clk_ratio. Clock ratio of Bus clock to Memory clock. 0x0 = 1:1, 0x1~0x3 = Reserved

#if DDR_RW_CAL
    DDR3_RW_Delay_Calibration();
#endif

    printf("Read  DQ = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->PHY_CON[4] ) );
    printf("Write DQ = 0x%08X\r\n", ReadIO32( &pReg_DDRPHY->PHY_CON[6] ) );
}

