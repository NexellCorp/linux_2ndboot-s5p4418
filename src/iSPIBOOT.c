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
//	Module		: iSPIBOOT.c
//	File		:
//	Description	:
//	Author		: Hans
//	History		: 2013.01.10 First implementation
//
////////////////////////////////////////////////////////////////////////////////

#include "sysHeader.h"
#include <nx_ssp.h>

//------------------------------------------------------------------------------

#define SER_WREN		0x06		// Set Write Enable Latch
#define SER_WRDI		0x04		// Reset Write Enable Latch
#define SER_RDSR		0x05		// Read Status Register
#define SER_WRSR		0x01		// Write Status Register
#define SER_READ		0x03		// Read Data from Memory Array
#define SER_WRITE		0x02		// Write Data to Memory Array

#define SER_SECERASE	0x20		// Sector Erase

#define SER_SR_BUSY	1<<0			// Busy bit
#define SER_SR_WEN		1<<1		// Write Enable indicate 0:not write enabled 1:write enabled
#define SER_SR_BPx		3<<2		// Block Write Protect bits
#define SER_SR_WPEN		1<<7		// Write Protect Enable bit

#define EEPROM_MINADDR	2

#define EEPROM_PAGE		256			// winbond W25Q128FV
#define EEPROM_SECTOR	4096		// winbond W25Q128FV
#define	EEPROM_ADDR		3			// winbond W25Q128FV

#define SPI_SOURCE_CLOCK	0
#define SPI_SOURCE_DIVID	(20UL)				// 800000000/20 = 40MHz

#define LOAD_FLASH_ADDR		0x10000		// 64KB

#define SSPFIFOSTATUS_BSY	4	// tx/rx frame is busy
#define SSPFIFOSTATUS_RFF	3	// receive fifo full
#define SSPFIFOSTATUS_RNE	2	// receive fifo not empty
#define SSPFIFOSTATUS_TNF	1	// transmit fifo not full
#define SSPFIFOSTATUS_TFE	0	// transmit fifo empty

extern U32  getquotient(U32 dividend, U32 divisor);
extern U32  get_fcs(U32 fcs, U8 data);
extern void DMC_Delay(int milisecond);
extern void ResetCon(U32 devicenum, CBOOL en);
extern void GPIOSetAltFunction(U32 AltFunc);

#if 1
typedef struct {
    U32 nPllNum;
    U32 nFreqHz;
    U32 nClkDiv;
    U32 nClkGenDiv;
} NX_CLKINFO_SPI;

CBOOL   NX_SPI_GetClkParam( NX_CLKINFO_SPI *pClkInfo )
{
    U32 srcFreq;
    U32 nRetry = 1, nTemp = 0;
    CBOOL   fRet = CFALSE;

    srcFreq = NX_CLKPWR_GetPLLFreq(pClkInfo->nPllNum);

retry_getparam:
    for (pClkInfo->nClkDiv = 2; ; pClkInfo->nClkDiv += 2)
    {
        nTemp   = (pClkInfo->nFreqHz * pClkInfo->nClkDiv);
        pClkInfo->nClkGenDiv  = getquotient(srcFreq, nTemp);      // (srcFreq / nTemp)

        if (srcFreq > (pClkInfo->nFreqHz * pClkInfo->nClkDiv))
            pClkInfo->nClkGenDiv+=2;

        if (pClkInfo->nClkGenDiv < 255)
            break;
    }

    nTemp = getquotient(srcFreq, (pClkInfo->nClkGenDiv * pClkInfo->nClkDiv));
    if (nTemp <= pClkInfo->nFreqHz)
    {
        fRet = CTRUE;
        goto exit_getparam;
    }

    if (nRetry)
    {
        nRetry--;
        goto retry_getparam;
    }

exit_getparam:
#if 0
    if (nRetry)
        printf("getClk = %d\r\n", nTemp);
#endif

    return fRet;
}
#endif


//------------------------------------------------------------------------------
static struct NX_CLKGEN_RegisterSet * const pSSPClkGenReg = (struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN37_MODULE;
static struct NX_SSP_RegisterSet * const pSSPSPIReg = (struct NX_SSP_RegisterSet *)PHY_BASEADDR_SSP0_MODULE;

void SPI_Init(void)
{
	register U32 *pGPIOxRegC = (U32 *)&pReg_GPIO[2]->GPIOxALTFN[1];
	register U32 *pGPIOxRegD = (U32 *)&pReg_GPIO[3]->GPIOxALTFN[0];
	*pGPIOxRegC = (*pGPIOxRegC & ~0xFCC00000) | 0x54400000;
	*pGPIOxRegD = (*pGPIOxRegD & ~0x00000003) | 0x00000001;

	ResetCon(RESETINDEX_OF_SSP0_MODULE_PRESETn, CFALSE);	// reset negate
	ResetCon(RESETINDEX_OF_SSP0_MODULE_nSSPRST, CFALSE);	// reset negate

#if 1
		NX_CLKINFO_SPI clkInfo;
		CBOOL ret;

		clkInfo.nPllNum = NX_CLKSRC_SPI;
		clkInfo.nFreqHz = 40000000;

		ret = NX_SPI_GetClkParam( &clkInfo );
		if (ret == CFALSE)
			printf("get clock param faile.\r\n");
#endif

#if 0
	pSSPClkGenReg->CLKENB = 0x1<<3; // pclk mode on but not supply operation clock
	pSSPClkGenReg->CLKGEN[0] = (SPI_SOURCE_DIVID-1)<<5 | 0x0<<2;	// select clock source is pll0, 800MHz and supply clock is 800/20 = 40.000MHz
	pSSPClkGenReg->CLKENB = 0x1<<3 | 0x1<<2;	// supply operation clock
#else
	pSSPClkGenReg->CLKENB = 0x1<<3; // pclk mode on but not supply operation clock
	pSSPClkGenReg->CLKGEN[0] = ((clkInfo.nClkGenDiv-1)<<5) | (clkInfo.nPllNum<<2);	// select clock source is pll0, 800MHz and supply clock is 800/20 = 40.000MHz
	pSSPClkGenReg->CLKENB = 0x1<<3 | 0x1<<2;	// supply operation clock

#endif
	pSSPSPIReg->SSPCR0 =
		(2-1)<<8|		// clock divider(0~255) 40.000MHz/(CPSDVR x (SCR + 1)) 	40.000/(2 * ((2-1) + 1)) = 10.0MHz
		0x1<<7	|		// clk out phase (SPH)
		0x1<<6	|		// clk out polarity (SPO)
		0x0<<4	|		// frame format 0:motorola spi 1:ti synchronous serial 2:national microwire
		(8-1)<< 0;		// data size - 1 =	DSS

	pSSPSPIReg->SSPCR1 =
		0x1<<3	|		// slave mode will not use
		0x0<<2	|		// master mode only used	0:master	1:slave
		0x1<<1	|		// sspspi enable 0:disabled 	1:enabled
		0x0<<0; 		// Loop back mode	0: normal mode		1:loop back mode

	pSSPSPIReg->SSPCPSR = (2 & 0xFE);	// clock prescale divisor (2~254 even) 40MHz / 2 = 20MHz

	pSSPSPIReg->SSPIMSC = 0;		// all interrupt mask 0: masked 1:unmasked

	pSSPSPIReg->SSPICR	= 0x3;		// all interrupt pending clear

	pSSPSPIReg->SSPDMACR = 0;		// dma TX/RX disabled


	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_TFE) );	// wait for TX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;			// RX data read

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);		// stop spi
}

void SPI_Deinit(void)
{
	register U32 *pGPIOxRegC = (U32 *)&pReg_GPIO[(PADINDEX_OF_SSP0_SSPCLK_IO>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_SSP0_SSPCLK_IO>>6)&0x1];
	register U32 *pGPIOxRegD = (U32 *)&pReg_GPIO[(PADINDEX_OF_SSP0_SSPRXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_SSP0_SSPRXD>>6)&0x1];

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);			// SPI Stop

	pSSPClkGenReg->CLKENB	= 0;				// PCLKMODE : disable, Clock Gen Disable

	//--------------------------------------------------------------------------
	ResetCon(RESETINDEX_OF_SSP0_MODULE_PRESETn, CTRUE); // reset on
	ResetCon(RESETINDEX_OF_SSP0_MODULE_nSSPRST, CTRUE); // reset on

	pReg_GPIO[2]->GPIOxOUT &= ~(0x1<<27);
	pReg_GPIO[2]->GPIOxOUTENB &= ~(0x1<<27);

	*pGPIOxRegC &= ~0xFC400000;
	*pGPIOxRegD &= ~0x00000003;
}

U32	SPI_EEPROMRead( U32 FlashBase, U32 *DDRBase, U32 Size, U32 FlashAddrCount, U32 fcs )
{
	register U8 *pdwBuffer = (U8*)DDRBase;
	register U32	iRxSize=0;
	register struct NX_GPIO_RegisterSet * pGPIOxReg = (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_C];

//	printf("SPI Flash Address: 0x%08X\r\n", FlashBase );

	pGPIOxReg->GPIOxOUT |= 0x40000000;	// gpio c 30 frm is gpio mode and output high first;
	pGPIOxReg->GPIOxOUTENB |= 0x40000000;	// gpio c 30 frm is gpio mode and out mode;
	pGPIOxReg->GPIOxALTFN[1] &=  ~(0x30000000);	// spi 0 gpio c 29 alt 1, 30 alt 0, 31 alt 1 // frm will use gpio mode

	pGPIOxReg->GPIOxOUT &= ~0x40000000;	// gpio c 30 frm is gpio mode and output low;

	pSSPSPIReg->SSPCR1 |= 0x1<<1;		// spi start (cs alreay low)

	while(!(pSSPSPIReg->SSPSR & 0x1<<0) );	// wait for TX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<4);		// wait for TRX shift reg is empty
	while(pSSPSPIReg->SSPSR & 0x1<<2)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// discard RX data cmd & address

	pSSPSPIReg->SSPDR = SER_READ;		// read command, Read Data from Memory Array

	while(FlashAddrCount)
	{
		U32 tmpdr = (FlashBase>>(8*(FlashAddrCount-1))) & 0xFF;		// start memory array address
		pSSPSPIReg->SSPDR = tmpdr;
		FlashAddrCount--;
	}

	// maximum fifo data is 6 (cmd:1, addr:5)
	while(!(pSSPSPIReg->SSPSR & 0x1<<0) );	// wait for TX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<4);		// wait for TRX shift reg is empty
	while(pSSPSPIReg->SSPSR & 0x1<<2)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// discard RX data cmd & address

	pSSPSPIReg->SSPDR = 0xAA;				// send dummy data for receive read data.
	pSSPSPIReg->SSPDR = 0xAA;				// send dummy data for receive read data.
	pSSPSPIReg->SSPDR = 0xAA;				// send dummy data for receive read data.
	pSSPSPIReg->SSPDR = 0xAA;				// send dummy data for receive read data.
	while(!(pSSPSPIReg->SSPSR & 0x1<<2));	// wait RX fifo is not empty

	while( Size > iRxSize ) // 4 is for crc32 fcs
	{
		register U8 tmpdata;
		pSSPSPIReg->SSPDR = 0xAA;			// send dummy data for receive read data.
		while(!(pSSPSPIReg->SSPSR & 0x1<<2));	// wait RX fifo is not empty
		tmpdata = (U8)pSSPSPIReg->SSPDR;

		pdwBuffer[iRxSize] = tmpdata;
		fcs = get_fcs(fcs, tmpdata);
		iRxSize++;
	}

	while(!(pSSPSPIReg->SSPSR & 0x1<<0) );	// wait for TX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<4);		// wait for TRX shift reg is empty
	while(pSSPSPIReg->SSPSR & 0x1<<2)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// RX data read

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);			// SPI Stop

	pGPIOxReg->GPIOxOUT |= 0x40000000;			// gpio c 30 frm is gpio mode and output high;

	printf("SPI Read completed!\r\n" );

	return fcs;
}
#if 0
void SPI_EEPROM_WriteEnable(void)
{
	pSSPSPIReg->SSPDR = SER_WREN;			// write enable command

	pSSPSPIReg->SSPCR1 |= 0x1<<1;			// spi start (cs will be low)

	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE));	// wait while RX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// throw out dummy data

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);		// stop spi
}
void SPI_EEPROM_WriteDisable(void)
{
	pSSPSPIReg->SSPDR = SER_WRDI;			// write enable command

	pSSPSPIReg->SSPCR1 |= 0x1<<1;			// spi start (cs will be low)

	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE));	// wait while RX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// throw out dummy data

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);		// stop spi
}
U32 SPI_EEPROM_GetStatus(void)
{
	U32 ret;

	pSSPSPIReg->SSPDR = SER_RDSR;			// Read status command
	pSSPSPIReg->SSPDR = 0xAA;				// dummy

	pSSPSPIReg->SSPCR1 |= 0x1<<1;			// spi start (cs will be low)

	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE));	// wait while RX fifo is empty
	pSSPSPIReg->SSPDR;						// throw out dummy data
	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE));	// wait while RX fifo is empty
	ret = pSSPSPIReg->SSPDR;

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);		// stop spi

	return ret;
}
void SPI_EEPROM_SectorErase(U32 FlashAddr, U32 FlashAddrCount)
{
	pSSPSPIReg->SSPDR = SER_SECERASE;			// Sector Erase command

	while(FlashAddrCount)
	{
		U32 tmpdr = (FlashAddr>>(8*(FlashAddrCount-1))) & 0xFF;
		pSSPSPIReg->SSPDR = tmpdr;
		FlashAddrCount--;
	}

	pSSPSPIReg->SSPCR1 |= 0x1<<1;			// spi start (cs will be low)

	while(!(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE));	// wait while RX fifo is empty
	while(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE)		// while RX fifo is not empty
		pSSPSPIReg->SSPDR;					// throw out dummy data

	pSSPSPIReg->SSPCR1 &= ~(0x1<<1);		// stop spi
}


U32 SPI_EEPROM_PageWrite(U32 FlashAddr, U8 *pDataAddr, U32 FlashAddrCount, U32 FlashPageSize, U32 fcs)
{
	U32 dummycount = 8-FlashAddrCount-1;		// fifo size is 8, command data, address count

	pSSPSPIReg->SSPDR = SER_WRITE;			// write command, write Data to Memory Array

	while(FlashAddrCount)
	{
		U32 tmpdr = (FlashAddr>>(8*(FlashAddrCount-1))) & 0xFF;
		pSSPSPIReg->SSPDR = tmpdr;
		FlashAddrCount--;
	}

	while(dummycount>0)
	{
		register U8 tmpdata = *pDataAddr++;
		pSSPSPIReg->SSPDR = tmpdata;
		fcs = get_fcs(fcs, tmpdata);
		FlashPageSize--;
		dummycount--;
	}

	pSSPSPIReg->SSPCR1 |= 0x1<<1;			// spi start (cs will be low)
	while(FlashPageSize >0)
	{
//		if(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_TNF)
		if(pSSPSPIReg->SSPSR & 0x1<<SSPFIFOSTATUS_RNE)	// if RX fifo is not empty
		{
			register U8 tmpdata = *pDataAddr++;
			pSSPSPIReg->SSPDR;				// throw out dummy data
			pSSPSPIReg->SSPDR = tmpdata;

			fcs = get_fcs(fcs, tmpdata);
			FlashPageSize--;
		}
	}
	return fcs;
}

CBOOL SPI_EEPROMWrite(U32 FlashAddr, U32 *DataAddr, S32 Size)
{
	U32 fcs=0;
	U8 *pDataAddr = (U8*)DataAddr;

	while(Size >0)
	{
		U32 count;
		if(Size < EEPROM_SECTOR)
			count = Size;
		else
			count = EEPROM_SECTOR;

		if((FlashAddr & 0xFFF) == 0)
		{
			do{
				SPI_EEPROM_WriteEnable();
			}while(!(SPI_EEPROM_GetStatus() & SER_SR_WEN));

			SPI_EEPROM_SectorErase(FlashAddr, EEPROM_ADDR);
			while(SPI_EEPROM_GetStatus() & SER_SR_BUSY);
		}

		Size -= count;
		while(count > 0)
		{
			U32 PageCount;
			if(count < EEPROM_PAGE)
				PageCount = count;
			else
				PageCount = EEPROM_PAGE;
			do{
				SPI_EEPROM_WriteEnable();
			}while(!(SPI_EEPROM_GetStatus() & SER_SR_WEN));
			fcs = SPI_EEPROM_PageWrite(FlashAddr, pDataAddr, EEPROM_ADDR, PageCount, fcs);
			while(SPI_EEPROM_GetStatus() & SER_SR_BUSY);
			count -= PageCount;
			pDataAddr += PageCount;
			FlashAddr += PageCount;
		}
	}

	SPI_EEPROM_WriteDisable();

	printf("SPI Write completed!\r\n" );


	return fcs;
}
#endif

CBOOL iSPIBOOT(struct NX_SecondBootInfo * pTBI)
{
	U32 FlashAddr, *fcsdata;
	U8 *DataAddr;
	S32 DataSize;
	U32 fcs = 0;

	printf("SPI Device Read Address: 0x%08X\r\nSPI address step   : 0x%08X\r\n", pSBI->DEVICEADDR, pSBI->DBI.SPIBI.AddrStep);


	FlashAddr = pSBI->DEVICEADDR;

	SPI_Init();

//	fcs = SPI_EEPROMWrite(FlashAddr, (U32*)pSBI->LOADADDR, pSBI->LOADSIZE);

	SPI_EEPROMRead(FlashAddr, (U32*)pTBI, sizeof(struct NX_SecondBootInfo), pSBI->DBI.SPIBI.AddrStep, 0);	// get NSIH


	if(pTBI->SIGNATURE != HEADER_ID)
	{
		printf("3rd boot Signature is wrong! SPI boot failure\r\n");
		fcs = 1;
		goto spifailure;
	}

	DataAddr = (U8*)pTBI->LOADADDR;
	DataSize = (U32)pTBI->LOADSIZE;

	printf("SPI 3rd boot Load Address: 0x%08X\r\n", (U32)DataAddr );
	printf("SPI Load Size   : 0x%08X\r\n", DataSize );
	printf("CRC   : 0x%08X\r\n", pTBI->DBI.SPIBI.CRC32 );


	FlashAddr += sizeof(struct NX_SecondBootInfo);

	fcs = SPI_EEPROMRead(FlashAddr, (U32*)DataAddr, DataSize, pSBI->DBI.SPIBI.AddrStep, 0);

	// if all data are zero, then crc result is zero.
	fcsdata = (U32*)DataAddr;

	while(DataSize > 0)
	{
		if(*fcsdata++ != 0)
			break;
		DataSize -= 4;
	}


	if(fcs != pTBI->DBI.SPIBI.CRC32)
	{
		printf("fcs check failure. generated crc is 0x%08X\r\n", fcs);
		fcs = 1;
	}else
	if(DataSize)
	{
		printf("SPI 3rd boot image load success!\r\n");
		fcs = 0;
	}

spifailure:
	SPI_Deinit();

	return !fcs;
}
