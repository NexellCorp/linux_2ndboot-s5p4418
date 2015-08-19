////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2009 Nexell Co., Ltd All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  Nexell informs that this code and information is provided "as is" base
//  and without warranty of any kind, either expressed or implied, including
//  but not limited to the implied warranties of merchantability and/or fitness
//  for a particular puporse.
//
//
//  Module          :
//  File            : pmic.c
//  Description     :
//  Author          : Hans
//  History         :
//          2015-05-21  Hans
////////////////////////////////////////////////////////////////////////////////

#include "sysHeader.h"


//------------------------------------------------------------------------------
#if defined( INITPMIC_YES )
#define NXE2000_DEF_DDCx_VOL_MIN        600000  /* UINT = 1uV, 0.6V */
#define NXE2000_DEF_DDCx_VOL_MAX        3500000 /* UINT = 1uV, 3.5V */
#define NXE2000_DEF_DDCx_VOL_STEP       12500   /* UINT = 1uV, 12.5mV */

#define NXE2000_DEF_DDC1_VOL            1300000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.3V */
#define NXE2000_DEF_DDC2_VOL            1100000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.2V */
#define NXE2000_DEF_DDC3_VOL            3300000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 3.3V */
#if defined( MEM_TYPE_DDR3 )
#if 0
#define NXE2000_DEF_DDC4_VOL            1500000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#define NXE2000_DEF_DDC5_VOL            1500000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#else	// DDR3L
#define NXE2000_DEF_DDC4_VOL            1350000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#define NXE2000_DEF_DDC5_VOL            1350000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#endif
#endif
#if defined( MEM_TYPE_LPDDR23 )
#define NXE2000_DEF_DDC4_VOL            1200000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#define NXE2000_DEF_DDC5_VOL            1200000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.65V */
#endif

#define NXE2000_REG_DC1VOL              0x36    /* ARM      */
#define NXE2000_REG_DC2VOL              0x37    /* CORE     */
#define NXE2000_REG_DC3VOL              0x38
#define NXE2000_REG_DC4VOL              0x39    /* DDR3     */
#define NXE2000_REG_DC5VOL              0x3A    /* MEM I/O  */

#define I2C_ADDR_NXE2000   (0x64 >> 1)  // SVT & ASB
#define I2C_ADDR_MP8845    (0x38 >> 1)  // SVT & ASB
#define I2C_ADDR_AXP228    (0x68 >> 1)  // DroneL

#define DRONE_PMIC_INIT
//#define SVT_PMIC_INIT
//#define ASB_PMIC_INIT

#if defined( DRONE_PMIC_INIT )
#define NXE2000_I2C_GPIO_GRP        4   // E group, NXE2000
#define NXE2000_I2C_SCL             14
#define NXE2000_I2C_SDA             15
#endif

#if defined( SVT_PMIC_INIT ) || defined( ASB_PMIC_INIT )
#define NXE2000_I2C_GPIO_GRP        3   // D group, NXE2000
#define NXE2000_I2C_SCL             2
#define NXE2000_I2C_SDA             3
#endif


extern void  I2C_Init( U8 gpioGRP, U8 gpioSCL, U8 gpioSDA );
//extern void  I2C_Deinit( void );
extern CBOOL I2C_Read(U8 DeviceAddress, U8 RegisterAddress, U8* pData, U32 Length);
extern CBOOL I2C_Write(U8 DeviceAddress, U8 RegisterAddress, U8* pData, U32 Length);

U8 nxe2000_get_dcdc_step(int want_vol)
{
    U32 vol_step = 0;
    U32 temp = 0;

    if (want_vol < NXE2000_DEF_DDCx_VOL_MIN)
    {
        want_vol = NXE2000_DEF_DDCx_VOL_MIN;
    }
    else if (want_vol > NXE2000_DEF_DDCx_VOL_MAX)
    {
        want_vol = NXE2000_DEF_DDCx_VOL_MAX;
    }

    temp        = (want_vol - NXE2000_DEF_DDCx_VOL_MIN) + (NXE2000_DEF_DDCx_VOL_STEP - 1);
    vol_step    = (temp / NXE2000_DEF_DDCx_VOL_STEP);

    return (U8)(vol_step & 0xFF);
}

void initPMIC(void)
{
    U8 pData[4];

    I2C_Init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA);   // CORE & NXE2000

#if 1
    pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
    I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC1VOL, pData, 1);

    pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
    I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC2VOL, pData, 1);
#endif

    pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
    I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC4VOL, pData, 1);

    pData[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
    I2C_Write(I2C_ADDR_NXE2000, NXE2000_REG_DC5VOL, pData, 1);

//    DMC_Delay(30 * 1000);
}
#endif  // #if defined( INITPMIC_YES )

