#ifndef __NX_REG_BASE_H__
#define __NX_REG_BASE_H__

#include "nx_type.h"


//*****************************************************************************
// MCUA Control Register
//*****************************************************************************

//#define ENV_SYSTEM_SYNCMODE
#define NX_BASE_REG_PA_RSTCON   (0xC0012000)
#define NX_BASE_REG_PA_DREX     (0xC00E0000)
#define NX_BASE_REG_PA_DDRPHY   (0xC00E1000)
#define NX_PA_BASE_REG_PL301_DISP  (0xC005E000)         // Temp. PL301 Register
#define NX_PA_BASE_REG_PL301_BOTT  (0xC0050000)         // Temp. PL301 Register
#define NX_PA_BASE_REG_PL301_TOP   (0xC0052000)         // Temp. PL301 Register

#define NX_PL301_QOS_TRDMARK_OFFSET     (0x400)
#define NX_PL301_QOS_CTRL_OFFSET        (0x404)
#define NX_PL301_AR_OFFSET              (0x408)
#define NX_PL301_AW_OFFSET              (0x40C)

#define NX_BASE_REG_PL301_DISP_QOS_TRDMARK  (NX_PA_BASE_REG_PL301_DISP | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_DISP_QOS_CTRL     (NX_PA_BASE_REG_PL301_DISP | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_DISP_AR           (NX_PA_BASE_REG_PL301_DISP | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_DISP_AW           (NX_PA_BASE_REG_PL301_DISP | NX_PL301_AW_OFFSET)

#define NX_BASE_REG_PL301_BOTT_QOS_TRDMARK  (NX_PA_BASE_REG_PL301_BOTT | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_BOTT_QOS_CTRL     (NX_PA_BASE_REG_PL301_BOTT | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_BOTT_AR           (NX_PA_BASE_REG_PL301_BOTT | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_BOTT_AW           (NX_PA_BASE_REG_PL301_BOTT | NX_PL301_AW_OFFSET)

#define NX_BASE_REG_PL301_TOP_QOS_TRDMARK   (NX_PA_BASE_REG_PL301_TOP | NX_PL301_QOS_TRDMARK_OFFSET)
#define NX_BASE_REG_PL301_TOP_QOS_CTRL      (NX_PA_BASE_REG_PL301_TOP | NX_PL301_QOS_CTRL_OFFSET)
#define NX_BASE_REG_PL301_TOP_AR            (NX_PA_BASE_REG_PL301_TOP | NX_PL301_AR_OFFSET)
#define NX_BASE_REG_PL301_TOP_AW            (NX_PA_BASE_REG_PL301_TOP | NX_PL301_AW_OFFSET)

typedef struct {
    U32 QOSCONTROL;
    U32 PAD_0;

} NX_DREX_QOS;

#if 0
typedef struct {
    U32 CONCONTROL;         // 0x00
    U32 MEMCONTROL;         // 0x04
    U32 MEMCONFIG[2];       // 0x08 ~ 0x0C
    U32 DIRECTCMD;          // 0x10
    U32 PRECHCONFIG;        // 0x14
    U32 PHYCONTROL[4];      // 0x18 ~ 0x24
    U32 PWRDNCONFIG;        // 0x28
    U32 TIMINGPZQ;          // 0x2C
    U32 TIMINGAREF;         // 0x30
    U32 TIMINGROW;          // 0x34
    U32 TIMINGDATA;         // 0x38
    U32 TIMINGPOWER;        // 0x3C
    U32 PHYSTATUS;          // 0x40
    U32 PAD_0[1];           // 0x44
    U32 CHIPSTATUS;         // 0x48
    U32 PAD_1[2];           // 0x4C ~ 0x50
    U32 MRSTATUS;           // 0x54
    U32 PAD_2[2];           // 0x58 ~ 0x5C
    NX_DREX_QOS QOS[16];    // 0x60 ~ 0xDC
    U32 PAD_19[5];          // 0xE0 ~ 0xF0

    U32 WRTRA_CONFIG;       // 0xF4
    U32 RDLVL_CONFIG;       // 0xF8
    U32 PAD_20[1];          // 0xFC

    U32 BRBRSVCONTROL;      // 0x100
    U32 BRBRSVCONFIG;       // 0x104
    U32 BRBQOSCONFIG;       // 0x108
    U32 MEMBASECONFIG[2];   // 0x10C ~ 0x110
    U32 PAD_21[3];          // 0x114 ~ 0x11C

    U32 WRLVL_CONFIG[2];    // 0x120 ~ 0x124
    U32 WRLVL_STATUS[2];    // 0x128
    U32 PAD_22[9];          // 0x12C ~ 0x14C

    U32 CTRL_IO_RDATA;      // 0x150
    U32 PAD_23[3];          // 0x154 ~ 0x15C

    U32 CACAL_CONFIG[2];    // 0x160 ~ 0x164
    U32 CACAL_STATUS;       // 0x168

} NX_DREX_REG, *PNX_DREX_REG;

typedef struct {
    U32 PHY_CON[44];
} NX_DDRPHY_REG, *PNX_DDRPHY_REG;
#endif

// PHYCONTROL0
#define PHY_STARPOINT       8
#define PHY_INC             16
#define PHY_DQSDLY          4
#define PHY_DIFDQS          3
#define PHY_DLLON           1
#define PHY_START           0
// PHYCONTROL1
#define PHY_OFFSETD         16
#define PHY_OFFSETC         8
#define PHY_REF             4
#define PHY_SHIFTC          0
// CONCONTROL
#define RD_FETCH            12 // read data fetch cycles
#define QOS_FASTEN          11 //
#define CHIP1_EMPTY         9 //
#define CHIP0_EMPTY         8 //
#define AREFEN              5 // read data fetch cycles
#define DQ_PULLDOWNDIS      4 // read data fetch cycles
#define IO_PD_CON           3 // read data fetch cycles
// MEMCONTROL
//#define MRR_BYTE            25//
//#define MEM_BL              20//
//#define MEM_WIDTH           12
//#define MEM_TYPE            8

// MEMCONFIG0
#define CHIP0_BASE          24
#define CHIP0_MASK          16
#define CHIP0_MAP           12
#define CHIP0_COLADBITS     8    // column address bits
#define CHIP0_ROWADBITS     4
#define CHIP0_BANK          0
// TIMINGROW
#define DREX_T_RFC_SHIFT    24
#define DREX_T_RRD_SHIFT    20
#define DREX_T_RP_SHIFT     16
#define DREX_T_RCD_SHIFT    12
#define DREX_T_RC_SHIFT     6
#define DREX_T_RAS_SHIFT    0

// TIMINGDATA
#define DREX_T_WTR_SHIFT    28
#define DREX_T_WR_SHIFT     24
#define DREX_T_RTP_SHIFT    20
#define DREX_T_CL_SHIFT     16
#define DREX_T_WL_SHIFT     8
#define DREX_T_RL_SHIFT     0

// Direct Command
#define DIRCMD_TYPE_SHIFT       (24)
#define DIRCMD_TYPE_MASK        (0xF)
    #define DIRCMD_MRS_EMRS     (0x0)
    #define DIRCMD_PALL         (0x1)
    #define DIRCMD_PRE          (0x2)
    #define DIRCMD_DPD          (0x3)
    #define DIRCMD_REFS         (0x4)
    #define DIRCMD_REFA         (0x5)
    #define DIRCMD_CKEL         (0x6)
    #define DIRCMD_NOP          (0x7)
    #define DIRCMD_REFSX        (0x8)
    #define DIRCMD_MRR          (0x9)
    #define DIRCMD_ZQINIT       (0xA)
    #define DIRCMD_ZQOPER       (0xB)
    #define DIRCMD_ZQCS         (0xC)

#define DIRCMD_CHIP_SHIFT       (20)
#define DIRCMD_CHIP_MASK        (0x1)
    #define DIRCMD_CHIP_1       (0x1)
    #define DIRCMD_CHIP_0       (0x0)

#define DIRCMD_BANK_SHIFT       (16)
#define DIRCMD_BANK_MASK        (0x7)

#define DIRCMD_ADDR_SHIFT       (0)
#define DIRCMD_ADDR_MASK        (0xFF)

#define DIRCMD_MRS_MODE_SHIFT   (16)
#define DIRCMD_MRS_MODE_MASK    (0x3)
    #define DIRCMD_MR0          (0x0)
    #define DIRCMD_MR1          (0x1)
    #define DIRCMD_MR2          (0x2)
    #define DIRCMD_MR3          (0x3)

#define DIRCMD_MRS_DATA_SHIFT   (0x0)
#define DIRCMD_MRS_DATA_MASK    (0xFF)



// DDR PHY
// DLL Lock
#define PHYCON12            0x30    // start point, inc, force
#define ctrl_startpoint     24
#define ctrl_inc            16
#define ctrl_start          6
#define ctrl_dllon          5
#define ctrl_ref            1

#define PHYCON5             0x14
#define PHYCON8             0x20
#define PHYCON1             0x4
#define PHYCON2             0x8
#define OFF                 0x0
#define ON                  0xFFFFFFFF
#define C5_DEFAULT_RD       0xF
#define C5_VWM_S_FAIL_RD    0xC
#define PHYCON14            0x38
#define PHYCON32            0x84
#define PHYCON33            0x88
#define PHYCON34            0x8C
#define PHYCON16            0x40
#define PHYCON17            0x48
#define PHYCON18            0x4C
#define PHYCON12            0x30
#define PHYCON13            0x34
#define PHYCON26            0x6C
#define PHYCON24            0x64
#define PHYCON42            0xAC
#define PHYCON0             0x0
#define DDR3_CON0           0x17020A40



#endif  // #ifndef __NX_REG_BASE_H__

