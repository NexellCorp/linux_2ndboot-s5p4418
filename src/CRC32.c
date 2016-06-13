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
 *      Module          :
 *      File            : crc32.c
 *      Description     :
 *      Author          : Hans
 *      History         : 2013.01.10 First implementation
 */
#include "sysheader.h"
#include <nx_pyrope.h>
#include <nx_type.h>

U32 get_fcs(U32 fcs, U8 data)
{
	int i;

	fcs ^= (U32)data;
	for (i = 0; i < 8; i++) {
		if (fcs & 0x01)
			fcs ^= POLY;
		fcs >>= 1;
	}

	return fcs;
}

U32 iget_fcs(U32 fcs, U32 data)
{
	int i;

	fcs ^= data;
	for (i = 0; i < 32; i++) {
		if (fcs & 0x01)
			fcs ^= POLY;
		fcs >>= 1;
	}

	return fcs;
}

/* CRC Calcurate Function
 * CHKSTRIDE is Data Stride.
 */
#define CHKSTRIDE 1
U32 __calc_crc(void *addr, int len)
{
	U32 *c = (U32 *)addr;
	U32 crc = 0, chkcnt = ((len + 3) / 4);
	U32 i;

	for (i = 0; chkcnt > i; i += CHKSTRIDE, c += CHKSTRIDE) {
		crc = iget_fcs(crc, *c);
	}

	return crc;
}

/*
 * Add CRC Check Function.
 * When there is a problem, except for boot device(SD/eMMC/USB/etc), you can check the error.
 */
int CRC_Check(void* buf, unsigned int size, unsigned int ref_crc)
{
	unsigned int crc;
#if 0	/* Debug Message */
	printf("CRC - Addr : %X, Size : %X \r\n", buf, size);
#endif
	crc = __calc_crc((void*)buf, (int)size);
	if (ref_crc != crc) {
		printf("CRC Check failed!! (%08X:%08X) \r\n"
				,ref_crc, crc);
		return 0;
	} else {
		printf("CRC Check Success!! (%08X:%08X) \r\n"
				,ref_crc, crc);
	}

	return 1;
}
