//------------------------------------------------------------------------------
//
//	Copyright (C) 2009 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//	AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//	BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//	FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------
#include "cfgFreqDefine.h"

#include <nx_pyrope.h>
#include <nx_type.h>
#include <debug.h>

#include <nx_chip.h>

#include <nx_gpio.h>
#include <nx_clkgen.h>
#include <nx_uart.h>
#include <nx_tieoff.h>

#define UARTSRC	0
#define SOURCE_DIVID	(50UL)
#define BAUD_RATE		(115200)

extern U32 getquotient(int dividend, int divisor);
extern U32 getremainder(int dividend, int divisor);
void ResetCon(U32 devicenum, CBOOL en);
void GPIOSetAltFunction(U32 AltFunc);
U32 NX_CLKPWR_GetPLLFreq(U32 PllNumber);

U32 UARTBASEADDR[6] =
{
	PHY_BASEADDR_UART0_MODULE,
	PHY_BASEADDR_UART1_MODULE,
	PHY_BASEADDR_pl01115_Uart_modem_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma0_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma1_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma2_MODULE
};
U32 UARTCLKGENADDR[6] =
{
	PHY_BASEADDR_CLKGEN22_MODULE,
	PHY_BASEADDR_CLKGEN23_MODULE,
	PHY_BASEADDR_CLKGEN24_MODULE,
	PHY_BASEADDR_CLKGEN25_MODULE,
	PHY_BASEADDR_CLKGEN26_MODULE,
	PHY_BASEADDR_CLKGEN27_MODULE
};
U8 RESETNUM[6] =
{
	RESETINDEX_OF_UART0_MODULE_nUARTRST,
	RESETINDEX_OF_UART1_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma1_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma2_MODULE_nUARTRST,
};
U32 GPIOALTNUM[12] =
{
	PADINDEX_OF_UART0_UARTRXD,
	PADINDEX_OF_UART0_UARTTXD,
	PADINDEX_OF_UART1_UARTRXD,
	PADINDEX_OF_UART1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTRXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma2_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma2_UARTTXD,
};
#if DIRECT_IO
extern struct NX_GPIO_RegisterSet (* pReg_GPIO)[1];
#endif
extern struct NX_CLKGEN_RegisterSet * pReg_UartClkGen;
extern struct NX_UART_RegisterSet   * pReg_Uart;
extern struct NX_TIEOFF_RegisterSet * pReg_Tieoff;
//------------------------------------------------------------------------------

CBOOL DebugInit( void )
{
	U32 SOURCE_CLOCK = NX_CLKPWR_GetPLLFreq(NX_CLKSRC_UART);

#if DIRECT_IO
	register U32 *pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_UART0_UARTRXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_UART0_UARTRXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x30000000) | 0x10000000;
	pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_UART0_UARTTXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_UART0_UARTTXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x00000030) | 0x00000010;
#else
	 GPIOSetAltFunction(PADINDEX_OF_UART0_UARTRXD);
	 GPIOSetAltFunction(PADINDEX_OF_UART0_UARTTXD);
#endif
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_USESMC)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_USESMC)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_SMCTXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_SMCTXENB)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_SMCRXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_SMCRXENB)&0x1F)));

	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CFALSE);	// reset negate

	pReg_UartClkGen->CLKENB	= (1<<3);								// PCLKMODE : always, Clock Gen Disable
	pReg_UartClkGen->CLKGEN[0]	= ((SOURCE_DIVID-1)<<5) | (NX_CLKSRC_UART<<2);

	//--------------------------------------------------------------------------
	pReg_Uart->LCR_H		= 0x0070;	// 8 bit, none parity, stop 1, normal mode
	pReg_Uart->CR		= 0x0300;	// rx, tx enable

	pReg_Uart->IBRD		= (U16)getquotient(getquotient(SOURCE_CLOCK,SOURCE_DIVID) , ((BAUD_RATE/1)*16));	// IBRD = 8, 115200bps
	pReg_Uart->FBRD		= (U16)(getquotient(((getremainder(getquotient(SOURCE_CLOCK,SOURCE_DIVID) , ((BAUD_RATE/1)*16))+32)*64),((BAUD_RATE/1)*16)));	// FBRD = 0,

	pReg_UartClkGen->CLKENB	= (1<<3) | (1<<2);		// PCLKMODE : always, Clock Gen Enable

	pReg_Uart->CR		= 0x0301;	// rx, tx, uart enable

	return CTRUE;
}

void TestUartInit(U32 port)
{

	if(port == 0)
	{
	}else
	if(port == 1)
	{
	}else
	if(port == 2)
	{
	}else
	if(port == 3)
	{
	}

}

void	DebugPutch( S8 ch )
{
	const U16 TX_FIFO_FULL	= 1<<5;
	while( pReg_Uart->FR & TX_FIFO_FULL )	{ ; }
	pReg_Uart->DR = (U32)ch;
}

CBOOL	DebugIsTXEmpty(void)
{
	const U16 TX_FIFO_EMPTY	= 1<<7;
	return (CBOOL)( pReg_Uart->FR & TX_FIFO_EMPTY );
}

CBOOL	DebugIsBusy(void)
{
	const U16 UART_TX_BUSY	= 1<<3;
	return (CBOOL)( pReg_Uart->FR & UART_TX_BUSY );
}

S8	DebugGetch( void )
{
	const U16 RX_FIFO_EMPTY	= 1<<4;
	while( pReg_Uart->FR & RX_FIFO_EMPTY )	{ ; }
	return (S8)pReg_Uart->DR;
}

//------------------------------------------------------------------------------
#if 1
void	DebugPutString( const S8* const String )
{
	const S8 *pString;

	pString = (const S8 *)String;
	while( CNULL != *pString )
		DebugPutch(*pString++);
}

S32		DebugGetString( S8* const pStringBuffer )
{
	S8	*pString = pStringBuffer;
	S8	buf;
	S32		iSize	= 0;

	while(1)
	{
		/* get character */
		buf = DebugGetch();

		/* backspace */
		if( buf == 0x08 )
		{
			if( iSize > 0 )
			{
				DebugPutch(buf);
				DebugPutch(' ');
				DebugPutch(buf);

				pString--;
				iSize--;
			}

			continue;
		}

		/* print character */
		DebugPutch(buf);

		if(buf == '\r')	break;

		/* increase string index */
		*pString++ = buf;
		iSize++;
	}

	*pString++ = '\0';

	return iSize;
}

#if 0
void	DebugPrint( const S8* const FormatString, ... )
{
	static S8 String[256];

	va_list argptr;
	va_start(argptr, FormatString);
	vsprintf((S8*)String, FormatString, argptr);
	va_end(argptr);

	DebugPutString(String);
}
#endif

//------------------------------------------------------------------------------
void	DebugPutDec( S32 value )
{
	S8 ch[16];
	U32 data;
	S32	i, chsize;

	data = (value < 0) ? (U32)(-value) : (U32)value;

	chsize = 0;
	do
	{
		ch[chsize++] = getremainder(data , 10) + '0';
		data = getquotient(data,10);
	} while( data != 0 );

	if( value < 0 )		DebugPutch( '-' );

	for( i=0 ; i<chsize ; i++ )
	{
		DebugPutch( ch[chsize - i - 1] );
	}
}

//------------------------------------------------------------------------------
void	DebugPutHex( S32 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	DebugPutch( '0' );
	DebugPutch( 'x' );

	for( i=0 ; i<8 ; i++ )
	{
		ch = (S8)(( data >> (28 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutByte( S8 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<2 ; i++ )
	{
		ch = (S8)(( data >> (4 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutWord( S16 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<4 ; i++ )
	{
		ch = (S8)(( data >> (12 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutDWord( S32 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<8 ; i++ )
	{
		ch = (S8)(( data >> (28 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}
#endif
