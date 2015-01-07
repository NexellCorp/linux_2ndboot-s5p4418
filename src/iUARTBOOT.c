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
//	Module		: iUARTBOOT.c
//	File		:
//	Description	:
//	Author		: Hans
//	History		: 2013.01.10 First implementation
//
////////////////////////////////////////////////////////////////////////////////

#include "sysHeader.h"

//------------------------------------------------------------------------------
#define SOURCE_CLOCK	NX_CLKSRCPLL1_FREQ
#define SOURCE_DIVID	(10UL)				// 147456000/10 = 14745600
#define BAUD_RATE		(115200)

void ResetCon(U32 devicenum, CBOOL en);
void GPIOSetAltFunction(U32 AltFunc);
U32 get_fcs(U32 fcs, U8 data);


//------------------------------------------------------------------------------
CBOOL	iUARTBOOT( U32 option )
{
	U8 *pdwBuffer =(U8*)BASEADDR_SRAM;
	S32	iRxSize = 0;
	U32 fcs = 0;

#if DIRECT_IO
	register U32 *pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_UART0_UARTRXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_UART0_UARTRXD>>6)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x30000000) | 0x10000000;
#else
	 GPIOSetAltFunction(PADINDEX_OF_UART0_UARTRXD);
#endif
	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CFALSE);	// reset negate


	pReg_UartClkGen->CLKENB	= (1<<3);								// PCLKMODE : always, Clock Gen Disable
	pReg_UartClkGen->CLKGEN[0]	= ((SOURCE_DIVID-1)<<5) | (1<<2);	// UARTCLK = PLL1 / 10 = 147,456,000 / 10 = 14,745,600 Hz

	//--------------------------------------------------------------------------
	pReg_Uart->LCR_H		= 0x0070;	// 8 bit, none parity, stop 1, normal mode
	pReg_Uart->CR		= 0x0200;	// rx enable

	if(option & 1<<UARTBAUD)
	{
		pReg_Uart->IBRD		= (U16)(SOURCE_CLOCK/SOURCE_DIVID / ((BAUD_RATE/1)*16));	// IBRD = 8, 115200bps
		pReg_Uart->FBRD		= (U16)(SOURCE_CLOCK/SOURCE_DIVID % ((BAUD_RATE/1)*16));	// FBRD = 0,
	}
	else
	{
		pReg_Uart->IBRD		= (U16)(SOURCE_CLOCK/SOURCE_DIVID / ((BAUD_RATE/6)*16));	// IBRD = 48, 19200bps
		pReg_Uart->FBRD		= (U16)(SOURCE_CLOCK/SOURCE_DIVID % ((BAUD_RATE/6)*16));	// FBRD = 0,
	}

	pReg_UartClkGen->CLKENB	= (1<<3) | (1<<2);		// PCLKMODE : always, Clock Gen Enable
	pReg_Uart->CR		= 0x0201;	// rx enable, uart enable

	while( 1 )
	{
		register U32 RXDATA;
		while( pReg_Uart->FR & NX_UART_FLAG_RXFE )	{ ; }	// wait while RX fifo is empty

		RXDATA = pReg_Uart->DR;

		if( 0x00000F00 & RXDATA )
			break;

		pdwBuffer[iRxSize++] = (U8)(RXDATA & 0xFF);

		fcs = get_fcs(fcs, (U8)(RXDATA & 0xFF));

		if( SECONDBOOT_SIZE+4 <= iRxSize )			// 4 is for crc32 fcs
		{
			NX_DEBUG_MSG("Download completed!\n" );

			break;
		}
	}

	pReg_Uart->CR			= 0x0;					// all disable
	pReg_UartClkGen->CLKENB	= (1<<3);				// PCLKMODE : always, Clock Gen Disable


	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CTRUE);	// reset on
#if DIRECT_IO
	*pGPIOxReg &= ~0x30000000;
#else
	GPIOSetAltFunction(PADINDEX_OF_GPIOD_GPIO_14_);
#endif
	//--------------------------------------------------------------------------
	return !fcs;
}

