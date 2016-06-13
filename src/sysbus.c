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
 #include "sysheader.h"

#include <nx_drex.h>
#include <nx_ddrphy.h>
#include "sysbus.h"

/*------------------------------------------------------------------------------
 *      BUS config
 */
#define CFG_DREX_PORT0_QOS_ENB              0
#define CFG_DREX_PORT1_QOS_ENB              1
#define CFG_BUS_RECONFIG_ENB                1       /* if want bus reconfig, select this first */

#define CFG_BUS_RECONFIG_DREXQOS            1
#define CFG_BUS_RECONFIG_TOPBUSSI           0
#define CFG_BUS_RECONFIG_TOPBUSQOS          0
#define CFG_BUS_RECONFIG_BOTTOMBUSSI        0
#define CFG_BUS_RECONFIG_BOTTOMBUSQOS       0
#define CFG_BUS_RECONFIG_DISPBUSSI          1

#if (CONFIG_BUS_RECONFIG == 1)
#if 1
void setBusConfig(void)
{
#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
    U32 DREX_QOS = 0;
#endif
    U8  miNum;

#if (CFG_DREX_PORT0_QOS_ENB == 1)
    DREX_QOS   |= (1<<4) | (1<<0);
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
    DREX_QOS   |= (1<<12) | (1<<8);
#endif

#if 1
    miNum = 0;
    WriteIO32( NX_BASE_ADDR_BOTT_QOS_TRDMARK    + (0x20 * miNum),   (1 << 0) );    // wait count for MI0 QoS adjust.
//    ClearIO32( NX_BASE_ADDR_BOTT_QOS_CTRL       + (0x20 * miNum),   (1 << 6) | (1 << 7) ); // org - coda960
    ClearIO32( NX_BASE_ADDR_BOTT_QOS_CTRL       + (0x20 * miNum),   (1 << 6) | (1 << 5) ); // patch - coda960
#else

    miNum = 1;  // MI1 of bottom bus.
    WriteIO32( NX_BASE_ADDR_BOTT_QOS_TRDMARK    + (0x20 * miNum),   (1 << 0) );    // wait count for MI1 QoS adjust.
    WriteIO32( NX_BASE_ADDR_BOTT_QOS_CTRL       + (0x20 * miNum),   (1 << 3) );    // SI3 - TOP bus

    miNum = 0;  // MI0 of bottom bus.
    WriteIO32( NX_BASE_ADDR_TOP_QOS_TRDMARK     + (0x20 * miNum),   (1 << 0) );    // wait count for MI0 QoS adjust.
    WriteIO32( NX_BASE_ADDR_TOP_QOS_CTRL        + (0x20 * miNum),   (1 << 5) );    // SI5 - OTG
#endif

    WriteIO32( NX_BASE_ADDR_DISP_AR,        (0 << 24) | (0 << 0) ); // VIP1
    WriteIO32( NX_BASE_ADDR_DISP_AR,        (1 << 24) | (1 << 0) ); // VIP2
#if 1
    WriteIO32( NX_BASE_ADDR_DISP_AR,        (2 << 24) | (1 << 0) ); // GMAC -> VIP2
#else
    WriteIO32( NX_BASE_ADDR_DISP_AR,        (2 << 24) | (2 << 0) ); // GMAC
#endif

    WriteIO32( &pReg_Drex->BRBRSVCONFIG,    0xFFF1FFF1 );
    WriteIO32( &pReg_Drex->BRBRSVCONTROL,   0x00000033 );

#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
#if (CFG_DREX_PORT0_QOS_ENB == 1)
    WriteIO32( &pReg_Drex->QOSCONTROL[0],   0x00000000 );
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
    WriteIO32( &pReg_Drex->QOSCONTROL[1],   0x00000000 );
#endif
    WriteIO32(&pReg_Tieoff->TIEOFFREG[24],  DREX_QOS);
#else

    WriteIO32( &pReg_Drex->QOSCONTROL[0],   0x00000100 );
#endif

    return;
}
#else

/*------------------------------------------------------------------------------
 * BUS Configure
 */
#if (CFG_BUS_RECONFIG_ENB == 1)
#include <mach/s5p4418_bus.h>

const u8 g_DrexBRB_RD[2] = {
        0x1,            // Port0
        0xF             // Port1
};

const u8 g_DrexBRB_WR[2] = {
        0x1,            // Port0
        0xF             // Port1
};

const u16 g_DrexQoS[2] = {
        0x100,          // S0
        0xFFF           // S1, Default value
};

#if (CFG_BUS_RECONFIG_TOPBUSSI == 1)
const u8 g_TopBusSI[8] = {
        TOPBUS_SI_SLOT_DMAC0,
        TOPBUS_SI_SLOT_USBOTG,
        TOPBUS_SI_SLOT_USBHOST0,
        TOPBUS_SI_SLOT_DMAC1,
        TOPBUS_SI_SLOT_SDMMC,
        TOPBUS_SI_SLOT_USBOTG,
        TOPBUS_SI_SLOT_USBHOST1,
        TOPBUS_SI_SLOT_USBOTG
};
#endif

#if (CFG_BUS_RECONFIG_TOPBUSQOS == 1)
const u8 g_TopQoSSI[2] = {
        1,      // Tidemark
        (1<<TOPBUS_SI_SLOT_DMAC0) |     // Control
        (1<<TOPBUS_SI_SLOT_MP2TS) |
        (1<<TOPBUS_SI_SLOT_DMAC1) |
        (1<<TOPBUS_SI_SLOT_SDMMC) |
        (1<<TOPBUS_SI_SLOT_USBOTG) |
        (1<<TOPBUS_SI_SLOT_USBHOST0) |
        (1<<TOPBUS_SI_SLOT_USBHOST1)
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1)
const u8 g_BottomBusSI[8] = {
        BOTBUS_SI_SLOT_1ST_ARM,
        BOTBUS_SI_SLOT_MALI,
        BOTBUS_SI_SLOT_DEINTERLACE,
        BOTBUS_SI_SLOT_1ST_CODA,
        BOTBUS_SI_SLOT_2ND_ARM,
        BOTBUS_SI_SLOT_SCALER,
        BOTBUS_SI_SLOT_TOP,
        BOTBUS_SI_SLOT_2ND_CODA
};
#endif

#if (CFG_BUS_RECONFIG_BOTTOMBUSQOS == 1)
const u8 g_BottomQoSSI[2] = {
        1,      // Tidemark
        (1<<BOTBUS_SI_SLOT_1ST_ARM) |   // Control
        (1<<BOTBUS_SI_SLOT_2ND_ARM) |
        (1<<BOTBUS_SI_SLOT_MALI) |
        (1<<BOTBUS_SI_SLOT_TOP) |
        (1<<BOTBUS_SI_SLOT_DEINTERLACE) |
        (1<<BOTBUS_SI_SLOT_1ST_CODA)
};
#endif

#if (CFG_BUS_RECONFIG_DISPBUSSI == 1)
const u8 g_DispBusSI[3] = {
        DISBUS_SI_SLOT_1ST_DISPLAY,
        DISBUS_SI_SLOT_2ND_DISPLAY,
        DISBUS_SI_SLOT_2ND_DISPLAY  //DISBUS_SI_SLOT_GMAC
};
#endif
#endif /* #if (CFG_BUS_RECONFIG_ENB == 1) */

void setBusConfig(void)
{
	u32 val;
	u32 num_si, num_mi;
	u32 i_slot, temp;
#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
	u32 drex_qos_bits = 0;
#endif

#if (CFG_DREX_PORT0_QOS_ENB == 1)
	drex_qos_bits |= (1 << 4) | (1 << 0);
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
	drex_qos_bits |= (1 << 12) | (1 << 8);
#endif

#if 0
    writel( 0xFFF1FFF1,     &pReg_Drex->BRBRSVCONFIG );
#else

    temp    = ( 0xFF00FF00
            | ((g_DrexBRB_WR[1] & 0xF) <<   20)
            | ((g_DrexBRB_WR[0] & 0xF) <<   16)
            | ((g_DrexBRB_RD[1] & 0xF) <<    4)
            | ((g_DrexBRB_RD[0] & 0xF) <<    0));
    writel( temp,           &pReg_Drex->BRBRSVCONFIG );
#endif
	writel(0x00000033, &pReg_Drex->BRBRSVCONTROL);

#if ((CFG_DREX_PORT0_QOS_ENB == 1) || (CFG_DREX_PORT1_QOS_ENB == 1))
#if (CFG_DREX_PORT0_QOS_ENB == 1)
	g_DrexQoS[0] = (u16)0x0000;
#endif
#if (CFG_DREX_PORT1_QOS_ENB == 1)
	g_DrexQoS[1] = (u16)0x0000;
#endif
	writel(drex_qos_bits, &pReg_Tieoff->TIEOFFREG[24]);
#else
#endif

	/* ------------- DREX QoS -------------- */
	#if 1   //(CFG_BUS_RECONFIG_DREXQOS == 1)
	for (i_slot = 0; i_slot < 2; i_slot++) {
		val = readl(NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET + (i_slot<<3));
		if (val != g_DrexQoS[i_slot])
			writel( g_DrexQoS[i_slot], (NX_VA_BASE_REG_DREX + NX_DREX_QOS_OFFSET + (i_slot<<3)) );
	}
	#endif /* (CFG_BUS_RECONFIG_DREXQOS == 1) */

	/* ------------- Bottom BUS ------------ */
	/* MI1 - Set SI QoS */
    #if (CFG_BUS_RECONFIG_BOTTOMBUSQOS == 1)
	val = readl(NX_BASE_REG_PL301_BOTT_QOS_TRDMARK + 0x20);
	if (val != g_BottomQoSSI[0])
		writel(g_BottomQoSSI[0], (NX_BASE_REG_PL301_BOTT_QOS_TRDMARK + 0x20) );

	val = readl(NX_BASE_REG_PL301_BOTT_QOS_CTRL + 0x20);
	if (val != g_BottomQoSSI[1])
		writel(g_BottomQoSSI[1], (NX_BASE_REG_PL301_BOTT_QOS_CTRL + 0x20) );
    #endif

    #if (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_BOTT + 0xFC4);

	/* Set progamming for AR */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++) {
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_BOTT_AR );
		val = readl(NX_BASE_REG_PL301_BOTT_AR);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  NX_BASE_REG_PL301_BOTT_AR );
	}

	// MI1 - Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AR + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AR + 0x20);
		if (val != g_BottomBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_BOTT_AR + 0x20) );
	}

	/* Set progamming for AW */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++) {
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_BOTT_AW );
		val = readl(NX_BASE_REG_PL301_BOTT_AW);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  NX_BASE_REG_PL301_BOTT_AW );
	}

	// MI1 - Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_BOTT_AW + 0x20) );
		val = readl(NX_BASE_REG_PL301_BOTT_AW + 0x20);
		if (val != g_BottomBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_BottomBusSI[i_slot] << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_BOTT_AW + 0x20) );
	}
	#endif /* (CFG_BUS_RECONFIG_BOTTOMBUSSI == 1) */

	/* ------------- Top BUS ------------ */
    #if (CFG_BUS_RECONFIG_TOPBUSQOS == 1)
	/* MI0 - Set SI QoS */
	val = readl(NX_BASE_REG_PL301_TOP_QOS_TRDMARK);
	if (val != g_TopQoSSI[0])
		writel(g_TopQoSSI[0], NX_BASE_REG_PL301_TOP_QOS_TRDMARK);

	val = readl(NX_BASE_REG_PL301_TOP_QOS_CTRL);
	if (val != g_TopQoSSI[1])
		writel(g_TopQoSSI[1], NX_BASE_REG_PL301_TOP_QOS_CTRL);
    #endif

	#if (CFG_BUS_RECONFIG_TOPBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_TOP + 0xFC4);

	/* Set progamming for AR */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++) {
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AR );
		val = readl(NX_BASE_REG_PL301_TOP_AR);
		if (val != g_TopBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_TopBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_TOP_AR );
	}

	// MI1 - Slave Interface
#if 0
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_TOP_AR + 0x20) );
		val = readl(NX_BASE_REG_PL301_TOP_AR + 0x20);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_TOP_AR + 0x20) );
	}
#endif

	/* Set progamming for AW */
	// MI0 - Slave Interface
	for (i_slot = 0; i_slot < num_mi; i_slot++) {
		writel( (0xFF000000 | i_slot),  NX_BASE_REG_PL301_TOP_AW );
		val = readl(NX_BASE_REG_PL301_TOP_AW);
		if (val != g_TopBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_TopBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_TOP_AW );
	}

	// MI1 - Slave Interface
#if 0
	for (i_slot = 0; i_slot < num_si; i_slot++)
	{
		writel( (0xFF000000 | i_slot),  (NX_BASE_REG_PL301_TOP_AW + 0x20) );
		val = readl(NX_BASE_REG_PL301_TOP_AW + 0x20);
		if (val != i_slot)
			writel( (i_slot << SLOT_NUM_POS) | (i_slot << SI_IF_NUM_POS),  (NX_BASE_REG_PL301_TOP_AW + 0x20) );
	}
#endif
#endif /* (CFG_BUS_RECONFIG_TOPBUSSI == 1) */

/* ------------- Display BUS ----------- */
#if (CFG_BUS_RECONFIG_DISPBUSSI == 1)
	num_si = readl(NX_VA_BASE_REG_PL301_DISP + 0xFC0);
	num_mi = readl(NX_VA_BASE_REG_PL301_DISP + 0xFC4);

	/* Set progamming for AR */
	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), NX_BASE_REG_PL301_DISP_AR);
		val = readl(NX_BASE_REG_PL301_DISP_AR);
		if (val != g_DispBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_DispBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_DISP_AR );
	}

	/* Set progamming for AW */
	// Slave Interface
	for (i_slot = 0; i_slot < num_si; i_slot++) {
		writel((0xFF000000 | i_slot), NX_BASE_REG_PL301_DISP_AW);
		val = readl(NX_BASE_REG_PL301_DISP_AW);
		if (val != g_DispBusSI[i_slot])
			writel( (i_slot << SLOT_NUM_POS) | (g_DispBusSI[i_slot] << SI_IF_NUM_POS),  NX_BASE_REG_PL301_DISP_AW );
	}
#endif /* (CFG_BUS_RECONFIG_DISPBUSSI == 1) */

	return;
}
#endif

#endif // #if (CONFIG_BUS_RECONFIG == 1)