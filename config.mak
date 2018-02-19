 #
 #      Copyright (C) 2012 Nexell Co., All Rights Reserved
 #      Nexell Co. Proprietary & Confidential
 #
 #      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 #      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 #      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 #      FOR A PARTICULAR PURPOSE.
 #
 #      Moudle          : Base
 #      File            : config.mak
 #      Description     :
 #      Author          : Firware Team
 #      History         : 2016.06.10 Deoks Modify
 #
###########################################################################
# Build Version info
###########################################################################
VERINFO				= V062

###########################################################################
# Build Environment
###########################################################################
CHIPNAME			= NXP4330
#CHIPNAME			= S5P4418

DEBUG				= n

MEMTYPE				= DDR3
#MEMTYPE			= LPDDR3
MEMTEST				= n

INITPMIC			= YES
#INITPMIC			= NO

CRC_CHECK			= n

CFLAGS				:=

SUPPORT_USB_BOOT		= n
SUPPORT_SPI_BOOT		= n
SUPPORT_SDMMC_BOOT		= y
SUPPORT_SDFS_BOOT		= n
SUPPORT_NAND_BOOT		= n
SUPPORT_UART_BOOT		= n
SUPPORT_Q100			= n
SUPPORT_ASVTBL			= n

ifeq ($(CHIPNAME), NXP4330)
#BOARD				= LEPUS
#BOARD				= SVM
BOARD				= NAVI
else
#BOARD				= SVT
#BOARD				= ASB
#BOARD				= DRONE
#BOARD				= AVN
#BOARD				= SVM
#BOARD				= LAVENDA
#BOARD				?= RAPTOR
endif

# cross-tool pre-header
ifeq ($(OS),Windows_NT)
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-none-eabi-
else
CROSS_TOOL_TOP			=
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-eabi-
endif

###########################################################################
# Top Names
###########################################################################
PROJECT_NAME			= $(CHIPNAME)_2ndboot_$(MEMTYPE)_$(VERINFO)
TARGET_NAME			= bl1-$(shell echo $(BOARD) | tr A-Z a-z)
LDS_NAME			= pyrope_2ndboot

###########################################################################
# Directories
###########################################################################
DIR_PROJECT_TOP			=

DIR_OBJOUTPUT			= obj
DIR_TARGETOUTPUT		= out

CODE_MAIN_INCLUDE		=

###########################################################################
# Build Environment
###########################################################################
CPU				= cortex-a9
CC				= $(CROSS_TOOL)gcc
LD 				= $(CROSS_TOOL)ld
AS 				= $(CROSS_TOOL)as
AR 				= $(CROSS_TOOL)ar
MAKEBIN				= $(CROSS_TOOL)objcopy
OBJCOPY				= $(CROSS_TOOL)objcopy
RANLIB 				= $(CROSS_TOOL)ranlib

GCC_LIB				= $(shell $(CC) -print-libgcc-file-name)

ifeq ($(DEBUG), y)
CFLAGS				= -DNX_DEBUG -O0
Q					=
else
CFLAGS				= -DNX_RELEASE -Os
Q					= @
endif

###########################################################################
# MISC tools for MS-DOS
###########################################################################
ifeq ($(OS),Windows_NT)
MKDIR				= mkdir
RM				= del /q /F
MV				= move
CD				= cd
CP				= copy
ECHO				= echo
RMDIR				= rmdir /S /Q
else
MKDIR				= mkdir
RM				= rm -f
MV				= mv
CD				= cd
CP				= cp
ECHO				= echo
RMDIR				= rm -rf
endif
###########################################################################
# FLAGS
###########################################################################
ARFLAGS				= rcs
ARFLAGS_REMOVE			= -d
ARLIBFLAGS			= -v -s

ASFLAG				= -D__ASSEMBLY__

CFLAGS				+=	-g -Wall				\
					-Wextra -ffreestanding -fno-builtin	\
					-msoft-float				\
					-mlittle-endian				\
					-mcpu=$(CPU)				\
					-mstructure-size-boundary=32		\
					$(CODE_MAIN_INCLUDE)			\
					-D__arm -DLOAD_FROM_$(BOOTFROM)		\
					-DMEMTYPE_$(MEMTYPE)			\
					-DINITPMIC_$(INITPMIC)			\
					-DCHIPID_$(CHIPNAME)			\
					-D_2NDBOOT_MODE -D$(BOARD)

ifeq ($(INITPMIC), YES)
CFLAGS				+=	-D$(BOARD)_PMIC_INIT
endif
ifeq ($(MEMTEST), y)
#MEMTEST_TYPE			+=	STANDARD
MEMTEST_TYPE			+=	SIMPLE
CFLAGS				+=	-D$(MEMTEST_TYPE)_MEMTEST
endif

ifeq ($(CRC_CHECK), y)
CHECKSUM			+=	CRC_CHECK
CFLAGS				+=	-D$(CHECKSUM)_ON
endif
