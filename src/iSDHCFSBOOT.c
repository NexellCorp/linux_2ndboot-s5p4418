////////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2009 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	Nexell informs that this code and information is provided "as is" base
//	and without warranty of any kind, either expressed or implied, including
//	but not limited to the implied warranties of merchantability and/or fitness
//	for a particular puporse.
//
//
//	Module		:
//	File		:
//	Description	:
//	Author		: Hans
//	History		: 2013.02.06 First implementation
//
////////////////////////////////////////////////////////////////////////////////

#include "sysHeader.h"

#include <nx_sdmmc.h>
#include <iSDHCBOOT.h>
#include <fatfs.h>

#ifdef DEBUG
#define dprintf(x, ...)	printf(x, ...)
#else
#define dprintf(x, ...)
#endif

void ResetCon(U32 devicenum, CBOOL en);
void GPIOSetAltFunction(U32 AltFunc);

extern U32 const SDResetNum[3];

extern struct NX_CLKGEN_RegisterSet * const pgSDClkGenReg[3];
extern struct NX_SDMMC_RegisterSet * const pgSDXCReg[3];
#if 1
// Nexell System Information Header
//------------------------------------------------------------------------------
CBOOL ProcessNSIH( FIL *file, U8 *pOutData )
{
	S32 writesize = 0, skipline = 0, bytesize = 0, line = 0;
	U32 writeval = 0, FSize = file->fsize;

	dprintf( "file size: %d\r\n", file->fsize);

	while( FSize )
	{
		U32 RSize;
		S8 txtbuffer[512];

		if(f_read(file, (void*)txtbuffer, 512, &RSize) == FR_OK)
		{
			U32 chcnt = 0;
			FSize -= RSize;
			while(RSize > chcnt)
			{
				S8 ch = txtbuffer[chcnt++];

				if( skipline == 0 )
				{
					if( ch >= '0' && ch <= '9' )
					{
						writeval = writeval * 16 + ch - '0';
						writesize += 4;
					}
					else if( ch >= 'a' && ch <= 'f' )
					{
						writeval = writeval * 16 + ch - 'a' + 10;
						writesize += 4;
					}
					else if( ch >= 'A' && ch <= 'F' )
					{
						writeval = writeval * 16 + ch - 'A' + 10;
						writesize += 4;
					}
					else
					{
						if( writesize == 8 || writesize == 16 || writesize == 32 )
						{
							S32 i;
							for( i=0 ; i<writesize/8 ; i++ )
							{
								pOutData[bytesize++] = (U8)(writeval & 0xFF);
								writeval >>= 8;
							}
						}
						else
						{
							if( writesize != 0 )
							{
								printf("ProcessNSIH : Error at %d line.\r\n", line+1);
								return CFALSE;
							}
						}

						writesize = 0;
						skipline = 1;
					}
				}

				if( ch == '\n' )
				{
					line++;
					skipline = 0;
					writeval = 0;
				}
			}
		}else
			return CFALSE;
	}

	printf("ProcessNSIH : %d line, %d bytes generated.\r\n", line+1, bytesize);

	return bytesize;
}
#endif
static CBOOL FSBoot(SDXCBOOTSTATUS * pSDXCBootStatus, struct NX_SecondBootInfo * pTBI)
{
	FATFS FATFS;	/* Pointer to the file system objects (logical drives) */
	const char *diskname = "0:";
	FATFS.fs_type = 0;
	FATFS.drive = 0;
	FATFS.reserved = (U32)pSDXCBootStatus;
#if 1
	printf( "mount to disk 0\r\n" );
	if (FR_OK == f_mount(&diskname, &FATFS, 0))
	{
		const char *headerfilename = "NXDATA.TBH";
		FIL hfile;

		if(FR_OK == f_open(&hfile, headerfilename, FA_READ, &FATFS))
		{
			printf( "NXDATA.TBH file size: %d\r\n", hfile.fsize);

			if(CFALSE != ProcessNSIH(&hfile, (U8*)pTBI))
			{
				printf( "open NXDATA.TBL\r\n" );

				if(pTBI->SIGNATURE == HEADER_ID)
				{
					const char *loaderfilename = "NXDATA.TBL";
					FIL lfile;

					if(FR_OK == f_open(&lfile, loaderfilename, FA_READ, &FATFS))
					{
						U32 RSize;

						if(FR_OK == f_read(&lfile, (void*)pTBI->LOADADDR, lfile.fsize, &RSize))
						{
							if(RSize == lfile.fsize)
							{
								printf( "%d bytes load done\r\n", RSize );
								return CTRUE;
							}
							printf( "NXDATA.TBL image read error\r\n" );
						}
						f_close(&lfile);
					}
					else
					{
						printf( "Third Boot file open failure\r\n" );
					}
				}
				else
				{
					printf("3rd boot Sinature is wrong!\r\n");
				}
			}
			f_close(&hfile);
		}
		else
		{
			dprintf("Error: cannot open NXDATA.TBH\r\n");
		}
	}else
	{
		printf( "disk mount failure\r\n" );
	}
#else
	struct NX_SecondBootInfo *pSBI = (struct NX_SecondBootInfo*)BASEADDR_SRAM;

	pTBI->LOADADDR = pSBI->LOADADDR;
	pTBI->LAUNCHADDR = pSBI->LAUNCHADDR;
	dprintf( "read NXDATA.TBL to 0x%08X\r\n", pSBI->LOADADDR );

	if (FR_OK == f_mount(&diskname, &FATFS, 0))
	{
		const char *loaderfilename = "NXDATA.TBL";
		if(f_open(&file, loaderfilename, FA_READ, &FATFS) == FR_OK)
		{
			if(pSBI->SIGNATURE == HEADER_ID)
			{
				if(f_read(&file, (void*)pSBI->LOADADDR, file.fsize, &RSize) == FR_OK)
				{
					if(RSize == file.fsize)
					{
						return CTRUE;
					}
					else
					{
						dprintf( "NXDATA.TBL image read error size is different\r\n" );
					}
				}
			}else
			{
				dprintf( "boot signature invalid\r\n" );
			}
		}else
		{
			dprintf( "file open failure\r\n" );
		}
	}else
	{
		dprintf( "disk mount failure\r\n" );
	}
#endif
	return CFALSE;
}

//------------------------------------------------------------------------------
static	CBOOL	SDMMCFSBOOT( SDXCBOOTSTATUS * pSDXCBootStatus, struct NX_SecondBootInfo * pTBI )
{
	CBOOL	result = CFALSE;
	struct NX_SDMMC_RegisterSet * const pSDXCReg = pgSDXCReg[pSDXCBootStatus->SDPort];

	dprintf( "open SDMMC\r\n" );
	if( CTRUE == NX_SDMMC_Open(pSDXCBootStatus) )
	{
		if( 0 == (pSDXCReg->STATUS & NX_SDXC_STATUS_FIFOEMPTY) )
		{
			dprintf( "FIFO Reset!!!\r\n" );
			pSDXCReg->CTRL = NX_SDXC_CTRL_FIFORST;				// Reset the FIFO.
			while( pSDXCReg->CTRL & NX_SDXC_CTRL_FIFORST );		// Wait until the FIFO reset is completed.
		}
//		return CFALSE;
		result = FSBoot(pSDXCBootStatus, pTBI);
	}else
	{
		dprintf( "SDMMC open fail\r\n" );
	}

	NX_SDMMC_Close(pSDXCBootStatus);

	return result;
}

//------------------------------------------------------------------------------
U32	iSDXCFSBOOT( struct NX_SecondBootInfo * pTBI )
{
	SDXCBOOTSTATUS lSDXCBootStatus;
	SDXCBOOTSTATUS * pSDXCBootStatus;
	CBOOL	result = CFALSE;
	pSDXCBootStatus = &lSDXCBootStatus;

	pSDXCBootStatus->SDPort = pSBI->DBI.SDMMCBI.PortNumber;
	dprintf("Load from %d port\r\n", pSDXCBootStatus->SDPort);

	NX_SDPADSetALT(pSDXCBootStatus->SDPort);

	//--------------------------------------------------------------------------
	// Normal SD(eSD)/MMC ver 4.2 boot

	NX_SDMMC_Init(pSDXCBootStatus);
	result = SDMMCFSBOOT(pSDXCBootStatus, pTBI);
	NX_SDMMC_Terminate(pSDXCBootStatus);

	NX_SDPADSetGPIO(pSDXCBootStatus->SDPort);

	return result;
}
