/*
 * (C) Copyright 2009
 * jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __S5P4418_ASV_H__
#define __S5P4418_ASV_H__

#define	VOLTAGE_STEP_UV		(12500)	/* 12.5 mV */
#define	ASV_DEFAULT_LEVEL	(0)		/* for 4330 */

/*
 *	=============================================================================
 * 	|	ASV Group	|	ASV0	|	ASV1	|	ASV2	|	ASV3	|	ASV4	|
 *	-----------------------------------------------------------------------------
 * 	|	ARM_IDS		|	<= 10mA	|	<= 15mA	|	<= 20mA	|	<= 50mA	|	> 50mA	|
 *	-----------------------------------------------------------------------------
 * 	|	ARM_RO		|	<= 110	|	<= 130	|	<= 140	|	<= 170 	|	170 >	|
 *	=============================================================================
 * 	|	EMA_CPU		|	EMA:001	|	EMA:001	|	EMA:001	|	EMA:011	|	EMA:011	|
 *	=============================================================================
 * 	|  0: 1400 MHZ	|	1,350 mV|	1,300 mV|	1,250 mV|	1,200 mV|	1,175 mV|
 *	-----------------------------------------------------------------------------
 * 	|  1: 1300 MHZ	|	1,300 mV|	1,250 mV|	1,200 mV|	1,150 mV|	1,100 mV|
 *	-----------------------------------------------------------------------------
 * 	|  2: 1200 MHZ	|	1,250 mV|	1,200 mV|	1,150 mV|	1,100 mV|	1,050 mV|
 *	-----------------------------------------------------------------------------
 * 	|  3: 1100 MHZ	|	1,200 mV|	1,150 mV|	1,100 mV|	1,050 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  4: 1000 MHZ	|	1,175 mV|	1,125 mV|	1,075 mV|	1,025 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  5: 900 MHZ	|	1,150 mV|	1,100 mV|	1,050 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  6: 800 MHZ	|	1,125 mV|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  7: 700 MHZ	|	1,100 mV|	1,050 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  8: 650 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	|  9: 500 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	-----------------------------------------------------------------------------
 * 	| 10: 400 MHZ	|	1,075 mV|	1,025 mV|	1,000 mV|	1,000 mV|	1,000 mV|
 *	=============================================================================
 */


struct asv_tb_info {
	int ids;
	int ro;
};


struct asv_tb_info asv_tables[] = {
	[0] = {	.ids = 10, .ro = 110,
	},
	[1] = {	.ids = 15, .ro = 130,
	},
	[2] = {	.ids = 20, .ro = 140,
	},
	[3] = {	.ids = 50, .ro = 170,
	},
	[4] = {	.ids = 50, .ro = 170,
	},
};
#define	ASV_ARRAY_SIZE	ARRAY_SIZE(asv_tables)

struct asv_param {
	int level;
	int ids, ro;
	int flag, group, shift;
};

struct asv_tb_info *pAsv_Table = CNULL;
struct asv_param	Asv_Param = { 0, };

unsigned int MtoL(unsigned int data, int bits)
{
	unsigned int result = 0;
	unsigned int mask = 1;
	int i = 0;
	for (i = 0; i<bits ; i++) {
		if (data&(1<<i))
			result |= mask<<(bits-i-1);
	}
	return result;
}

unsigned int s5p4418_asv_setup_table(void)
{
	int i, ids = 0, ro = 0;
	int idslv, rolv, asvlv;
	unsigned int ecid_1;

	ecid_1 = mmio_read_32(&pReg_ECID->ECID[1]);

	/* Check Sample date */
	if(((ecid_1 >> 10) & 0x1F) != 0x0) {
		asvlv = ASV_DEFAULT_LEVEL;
		goto asv_find;
	}

	/* Use IDS/Ro */
	ids = MtoL((ecid_1>>16) & 0xFF, 8);
	ro  = MtoL((ecid_1>>24) & 0xFF, 8);

	/* find IDS Level */
	for (i=0; (ASV_ARRAY_SIZE-1) > i; i++) {
		pAsv_Table = &asv_tables[i];
		if (pAsv_Table->ids >= ids)
			break;
	}
	idslv = ASV_ARRAY_SIZE != i ? i: (ASV_ARRAY_SIZE-1);

	/* find RO Level */
	for (i=0; (ASV_ARRAY_SIZE-1) > i; i++) {
		pAsv_Table = &asv_tables[i];
		if (pAsv_Table->ro >= ro)
			break;
	}
	rolv = ASV_ARRAY_SIZE != i ? i: (ASV_ARRAY_SIZE-1);

	/* find Lowest ASV Level */
	asvlv = idslv > rolv ? rolv: idslv;

asv_find:
	pAsv_Table = &asv_tables[asvlv];
	Asv_Param.level = asvlv;
	Asv_Param.ids = ids;
	Asv_Param.ro  = ro;
	//printf("ASV[%d] IDS(%dmA,%3d) Ro(%d,%3d)\r\n",
	//		Asv_Param.level, pAsv_Table->ids, ids, pAsv_Table->ro, ro);

	return asvlv;
}


#endif


