//------------------------------------------------------------------------------
//
//  Copyright (C) 2009 Nexell Co., All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module     : I2C GPIO
//  File       : nx_i2c_gpio.c
//  Description:
//  Author     : Firmware Team
//  History    : 2014.11.21 hans
//				 2015.03.13 hans change to no use prototype
//				 2015.07.22 dahye change
//------------------------------------------------------------------------------
#include <nx_type.h>
#include <nx_gpio.h>
#include <nx_i2c.h>
#include "sysHeader.h"

//#define dprintf(x)
#define dprintf printf

#define I2C_DELAY_TIME			2

#define I2C_Divider				1		// Divider 16 : 0, 256 : 1
#define I2C_Prescaler			16		// from 1 to 16
#define GPIO_GROUP_D			3		// I2C0,1,2 GPIO_GROUP_D


static U32 I2C_PAD[NUMBER_OF_I2C_MODULE][2] =
{
	{2, 3},				// GPIO_I2C_CH0_SCL, GPIO_I2C_CH0_SDA
	{4, 5},				// GPIO_I2C_CH1_SCL, GPIO_I2C_CH1_SDA
	{6, 7}				// GPIO_I2C_CH2_SCL, GPIO_I2C_CH2_SDA
};
static struct NX_CLKGEN_RegisterSet * const pgI2CClkGenReg[3] =
{
	(struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN6_MODULE,
	(struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN7_MODULE,
	(struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN8_MODULE
};
static struct NX_I2C_RegisterSet * const pgI2CReg[3] = 
{
	(struct NX_I2C_RegisterSet *)PHY_BASEADDR_I2C0_MODULE,
	(struct NX_I2C_RegisterSet *)PHY_BASEADDR_I2C1_MODULE,
	(struct NX_I2C_RegisterSet *)PHY_BASEADDR_I2C2_MODULE
};

static inline void I2CDEVDELAY(U32 us)
{
    volatile U32 i = 0, j = 0;
    for(i = 0; i < us * 2; i++)
        for(j = 0; j < 5; j++);
}

void I2C_Init(U32 Channel)
{
	register struct NX_I2C_RegisterSet * const pI2CReg = pgI2CReg[Channel];
	struct NX_CLKGEN_RegisterSet * const pI2CClkGenReg = pgI2CClkGenReg[Channel];

	/* Clock Generation */
	SetIO32( &pI2CClkGenReg->CLKENB, 0x1<<3 );									// always pclk mode 
	/* Reset Generation (0:Reset, 1:NoReset) */
	ResetCon(RESETINDEX_OF_I2C2_MODULE_PRESETn, CFALSE);						// reset negate
	/* Deinitialize */
	SetIO32( &pI2CReg->ICCR, (0x0<<7 | 0x0<<5 | ~(0x1<<4)) );					// Set Ack generation disable, Interrupt disable, Bus disable
	/* I2C Initialize */
	SetIO32( &pI2CReg->ICCR, ( I2C_Divider<<0x6 | I2C_Prescaler-1 | 0x1<<7 | 0x1<<5 ) );	// Set Clock Divider, Prescaler, Ack generation enable, Interrupt enable

	/* GPIOx PAD Alternate Function */
	WriteIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0], WriteIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0], ~( 3<<(I2C_PAD[Channel][0]*2) | 3<<(I2C_PAD[Channel][1]*2) ) ) | ( 1<<(I2C_PAD[Channel][0]*2) | 1<<(I2C_PAD[Channel][1]*2) ) );
	SetIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxOUTENB, (1<<I2C_PAD[Channel][1]) );	// GPIO Output enable
	SetIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOx_SLEW_DISABLE_DEFAULT, (1<<I2C_PAD[Channel][0] | 1<<I2C_PAD[Channel][1]) );
	SetIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOx_DRV0_DISABLE_DEFAULT, (1<<I2C_PAD[Channel][0] | 1<<I2C_PAD[Channel][1]) );
	SetIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOx_DRV1_DISABLE_DEFAULT, (1<<I2C_PAD[Channel][0] | 1<<I2C_PAD[Channel][1]) );

}

void I2C_StopGen(U32 Channel)
{
	register struct NX_I2C_RegisterSet * const pI2CReg = pgI2CReg[Channel];

	/* SDA Line -> Low (GPIO) */
	WriteIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0], ( pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0] & ~( 3<<(I2C_PAD[Channel][1]*2) ) ) | ( 0<<(I2C_PAD[Channel][1]*2) ) );	// I2C2 GPIO D 7(SDA) alt 0
	WriteIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxOUT, ~(1<<I2C_PAD[Channel][1]) ); // GPIO Output value Low
	while( CFALSE != ( ( ReadIO32(&pReg_GPIO[GPIO_GROUP_D]->GPIOxOUT)>>I2C_PAD[Channel][1] )&1) );

	/* SCL Line Stop */
	I2CDEVDELAY(I2C_DELAY_TIME);		
	WriteIO32( &pI2CReg->STOPCON, 0x1<<0 );										// Clock line release for the stop condition	
	I2CDEVDELAY(I2C_DELAY_TIME);

	/* SDA Line Stop (GPIO) */
	SetIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxOUT, (1<<I2C_PAD[Channel][1]) );	// GPIO Output value High
	while( CTRUE != ( ( ReadIO32(&pReg_GPIO[GPIO_GROUP_D]->GPIOxOUT)>>I2C_PAD[Channel][1] )&1) );
	I2CDEVDELAY(I2C_DELAY_TIME);
	WriteIO32( &pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0], ( pReg_GPIO[GPIO_GROUP_D]->GPIOxALTFN[0] & ~( 3<<(I2C_PAD[Channel][1]*2) ) ) | ( 1<<(I2C_PAD[Channel][1]*2) ) );	// I2C2 GPIO D 7(SDA) alt 1
	
}

CBOOL I2C_Read(U32 Channel, U8 DeviceAddress, U8 RegisterAddress, U8* pData)
{
	U32 ErrorChk;
	
	register struct NX_I2C_RegisterSet * const pI2CReg = pgI2CReg[Channel];

	/* Start Bit Generation */
	WriteIO32( &pI2CReg->ICSR, (0x1<<4 | 0x1<<5 | 0x3<<6) );	// Enable Tx
																// I2C-bus interface START signal generation
																// select Master transmit mode
	/* Slave Address */
	WriteIO32( &pI2CReg->IDSR, DeviceAddress);					// slave address
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}	

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	/* Sub Address (Register Address) */
	WriteIO32( &pI2CReg->IDSR, RegisterAddress);				// Register address
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Register Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Register Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	/* ReStart */
	WriteIO32( &pI2CReg->ICSR, (0x2<<6 | 0x1<<5 | 0x1<<4) );	// select Master receive mode
																// I2C-bus interface START signal generation
																// Enable Rx
	/* Slave Address */
	WriteIO32( &pI2CReg->IDSR, DeviceAddress);					// slave address
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	/* Read Data (Last Data) */
	WriteIO32( &pI2CReg->ICCR, ~(0x1<<7) );						// Ack generation Disable
	WriteIO32( &pI2CReg->STOPCON, 0x1<<2 );						// Not Ack generation
	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit

	*pData = ReadIO8( &pI2CReg->IDSR );							// Read data
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Data Read Abitration Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	I2C_StopGen( Channel);

	return CTRUE;
	
}

CBOOL I2C_Write(U32 Channel, U8 DeviceAddress, U8 RegisterAddress, U8 Data)
{
	U32 ErrorChk;
	printf("PMIC DBG2\r\n");
	register struct NX_I2C_RegisterSet * const pI2CReg = pgI2CReg[Channel];

	/* Start Bit Generation */
	WriteIO32( &pI2CReg->ICSR, (0x1<<4 | 0x3<<6 | 0x1<<5) );	// Enable Tx
																// select Master transmit mode
																// I2C-bus interface START signal generation
	/* Slave Address */
	WriteIO32( &pI2CReg->IDSR, DeviceAddress );					// slave address
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear
	
	/* Sub Address (Register Address) */
	WriteIO32( &pI2CReg->IDSR, RegisterAddress );				// Register address
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Register Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Register Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	/* Write Data */
	WriteIO32( &pI2CReg->IDSR, Data );							// Write data
	while(!( ReadIO32( &pI2CReg->ICCR ) & 0x1<<4 ));			// wait for 1-byte transmit
	ErrorChk = ReadIO32( &pI2CReg->ICSR );
	if( ErrorChk & 0x1<<3 )
	{
		dprintf("I2C Data Write Abitration Error\r\n");
		return CFALSE;
	}
	if( ErrorChk & 0x1<<0 )
	{
		dprintf("I2C Data Write Acknowledge Error\r\n");
		return CFALSE;
	}

	WriteIO32( &pI2CReg->ICCR, (ReadIO32(&pI2CReg->ICCR) & ~(0x1<<4)) | 0x1<<8 );	// interrupt clear

	I2C_StopGen( Channel);

	return CTRUE;
	
}
