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
#include "sysheader.h"

static unsigned int RESET_IDX_LIST[2] =
{
	RESETINDEX_OF_TIMER_MODULE_PRESETn,
	RESETINDEX_OF_PWM_MODULE_PRESETn
};

void __init ResetCon(U32 devicenum, CBOOL en)
{
	if (en)
		ClearIO32(&pReg_RstCon->REGRST[(devicenum >> 5) & 0x3],
			  (0x1 << (devicenum & 0x1F))); // reset
	else
		SetIO32(&pReg_RstCon->REGRST[(devicenum >> 5) & 0x3],
			(0x1 << (devicenum & 0x1F))); // reset negate
}

void device_reset(void)
{
	unsigned int i;
	for (i = 0; i < (sizeof(RESET_IDX_LIST)/sizeof(unsigned int)); i++) {
		ResetCon(RESET_IDX_LIST[i], CTRUE);	// reset on
		ResetCon(RESET_IDX_LIST[i], CFALSE);	// reset negate
	}
}


