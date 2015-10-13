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
void DMC_Delay(int milisecond);

#define AUTO_VOLTAGE_CONTROL        (1)
#define ARM_VOLTAGE_CONTROL_SKIP    (0)
#define ARRAY_SIZE(x)               (sizeof(x) / sizeof((x)[0]))

//------------------------------------------------------------------------------
#if defined( INITPMIC_YES )

//#define DRONE_PMIC_INIT
//#define AVN_PMIC_INIT
//#define SVT_PMIC_INIT
//#define ASB_PMIC_INIT

#define AXP_I2C_GPIO_GRP            (-1)
#define NXE2000_I2C_GPIO_GRP        (-1)

#if defined( DRONE_PMIC_INIT )
#if 0
#undef  NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP        4   // E group, NXE2000
#define NXE2000_I2C_SCL             14
#define NXE2000_I2C_SDA             15
#else
#undef  AXP_I2C_GPIO_GRP
#define AXP_I2C_GPIO_GRP            3   // D group
#define AXP_I2C_SCL                 20
#define AXP_I2C_SDA                 16
#endif
#endif

#if defined( LEPUS_PMIC_INIT )
#undef  NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP        4   // E group, NXE2000
#define NXE2000_I2C_SCL             14
#define NXE2000_I2C_SDA             15
#endif

#if defined( AVN_PMIC_INIT )
#define MP8845_CORE_I2C_GPIO_GRP    4   // E group, FineDigital VDDA_1.2V (core)
#define MP8845_CORE_I2C_SCL         11
#define MP8845_CORE_I2C_SDA         10

#define MP8845_ARM_I2C_GPIO_GRP     4   // E group, FineDigital VDDB_1.2V (arm)
#define MP8845_ARM_I2C_SCL          9
#define MP8845_ARM_I2C_SDA          8

#define MP8845_PMIC_INIT            (1)
#endif

#if defined( SVT_PMIC_INIT ) || defined( ASB_PMIC_INIT )
#undef  NXE2000_I2C_GPIO_GRP
#define NXE2000_I2C_GPIO_GRP        3   // D group, NXE2000
#define NXE2000_I2C_SCL             2
#define NXE2000_I2C_SDA             3
#endif

#if (AXP_I2C_GPIO_GRP > -1)
#include "pmic_axp228.h"
#endif
#if (NXE2000_I2C_GPIO_GRP > -1)
#include "pmic_nxe2000.h"
#endif
#if (MP8845_PMIC_INIT == 1)
#include "pmic_mp8845.h"
#endif

extern void  I2C_Init( U8 gpioGRP, U8 gpioSCL, U8 gpioSDA );
//extern void  I2C_Deinit( void );
extern CBOOL I2C_Read(U8 DeviceAddress, U8 RegisterAddress, U8* pData, U32 Length);
extern CBOOL I2C_Write(U8 DeviceAddress, U8 RegisterAddress, U8* pData, U32 Length);

#if (AXP_I2C_GPIO_GRP > -1)
static U8 axp228_get_dcdc_step(int want_vol, int step, int min, int max)
{
    U32 vol_step = 0;

    if (want_vol < min)
    {
        want_vol = min;
    }
    else if (want_vol > max)
    {
        want_vol = max;
    }

    vol_step = (want_vol - min + step - 1) / step;

    return (U8)(vol_step & 0xFF);
}
#endif

#if (NXE2000_I2C_GPIO_GRP > -1)
static U8 nxe2000_get_dcdc_step(int want_vol)
{
    U32 vol_step = 0;

    if (want_vol < NXE2000_DEF_DDCx_VOL_MIN)
    {
        want_vol = NXE2000_DEF_DDCx_VOL_MIN;
    }
    else if (want_vol > NXE2000_DEF_DDCx_VOL_MAX)
    {
        want_vol = NXE2000_DEF_DDCx_VOL_MAX;
    }

    vol_step    = (want_vol - NXE2000_DEF_DDCx_VOL_MIN + NXE2000_DEF_DDCx_VOL_STEP - 1) / NXE2000_DEF_DDCx_VOL_STEP;

    return (U8)(vol_step & 0xFF);
}
#endif

#if (AXP_I2C_GPIO_GRP > -1)
void PMIC_AXP228(void)
{
    U8 pData[4];

    I2C_Init(AXP_I2C_GPIO_GRP, AXP_I2C_SCL, AXP_I2C_SDA);

    I2C_Read(I2C_ADDR_AXP228, 0x80, pData, 1);
    pData[0] = (pData[0] & 0x1F) | DCDC_SYS | DCDC_DDR;
    I2C_Write(I2C_ADDR_AXP228, 0x80, pData, 1);

    // Set bridge DCDC2 and DCDC3
    I2C_Read(I2C_ADDR_AXP228, 0x37, pData, 1);
    pData[0] |= 0x10;
    I2C_Write(I2C_ADDR_AXP228, 0x37, pData, 1);

    //
    // ARM voltage change
    //
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
    pData[0] = axp228_get_dcdc_step(AXP228_DEF_DDC2_VOL, AXP228_DEF_DDC234_VOL_STEP, AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
    I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC2VOL, pData, 1);
#endif

#if 0
    // Set voltage of DCDC4.
    pData[0] = axp228_get_dcdc_step(AXP228_DEF_DDC4_VOL, AXP228_DEF_DDC234_VOL_STEP, AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
    I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC4VOL, pData, 1);

    // Set voltage of DCDC5.
    pData[0] = axp228_get_dcdc_step(AXP228_DEF_DDC5_VOL, AXP228_DEF_DDC5_VOL_STEP, AXP228_DEF_DDC5_VOL_MIN, AXP228_DEF_DDC5_VOL_MAX);
    I2C_Write(I2C_ADDR_AXP228, AXP228_REG_DC5VOL, pData, 1);
#endif
}
#endif

#if ( MP8845_PMIC_INIT == 1 )
void PMIC_MP8845(void)
{
    U8 pData[4];

    //
    // I2C init for CORE power.
    //
    I2C_Init(MP8845_CORE_I2C_GPIO_GRP, MP8845_CORE_I2C_SCL, MP8845_CORE_I2C_SDA);

    // PFM -> PWM mode
    I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
    pData[0] |= 1<<0;
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

    //
    // Core voltage change
    //
    I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
    pData[0] |= 1<<5;
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

#if defined( BF700_PMIC_INIT )
    pData[0] = 90 | 1<<7;     // 1.2V
#endif
#if defined( AVN_PMIC_INIT )
//    pData[0] = 75 | 1<<7;     // 1.1V
    pData[0] = 79 | 1<<7;     // 1.125V
#endif
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);

    //
    // I2C init for ARM power.
    //
    I2C_Init(MP8845_ARM_I2C_GPIO_GRP, MP8845_ARM_I2C_SCL, MP8845_ARM_I2C_SDA);

    // PFM -> PWM mode
    I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);
    pData[0] |= 1<<0;
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL1, pData, 1);

    //
    // ARM voltage change
    //
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
    I2C_Read(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);
    pData[0] |= 1<<5;
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_SYSCNTL2, pData, 1);

//    pData[0] = 75 | 1<<7;   // 1.1V
    pData[0] = 80 | 1<<7;   // 1.135V
    I2C_Write(I2C_ADDR_MP8845, MP8845C_REG_VSEL, pData, 1);
#endif
}
#endif

#if (NXE2000_I2C_GPIO_GRP > -1)
void PMIC_NXE2000(void)
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
}
#endif

void initPMIC(void)
{
#if (AXP_I2C_GPIO_GRP > -1)
    PMIC_AXP228();
#endif

#if (NXE2000_I2C_GPIO_GRP > -1)
    PMIC_NXE2000();
#endif

#if (MP8845_PMIC_INIT == 1)
    PMIC_MP8845();
#endif

    DMC_Delay(100 * 1000);
}

#endif  // #if defined( INITPMIC_YES )
