###########################################################################
# Build Environment
###########################################################################
DEBUG				= n

CHIPNAME			= LF3000
#CHIPNAME			= NXP4330

ifeq ($(CHIPNAME),LF3000)
BOOTFROM			= usb
#BOOTFROM			= spi
#BOOTFROM			= sdmmc
#BOOTFROM			= sdfs
#BOOTFROM			= nand
endif

#BOARD				= pyxis
#BOARD				= lynx
#BOARD				= vtk
BOARD				= drone
#BOARD				= svt

# cross-tool pre-header
ifeq ($(OS),Windows_NT)
CROSS_TOOL_TOP		=
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-none-eabi-
else
CROSS_TOOL_TOP		= 
CROSS_TOOL			= $(CROSS_TOOL_TOP)arm-cortex_a9-linux-gnueabi-
endif

###########################################################################
# Top Names
###########################################################################
PROJECT_NAME		= pyrope_2ndboot
ifeq ($(CHIPNAME),LF3000)
TARGET_NAME			= $(PROJECT_NAME)_$(BOARD)_$(BOOTFROM)
else ifeq ($(CHIPNAME),NXP4330)
TARGET_NAME			= $(PROJECT_NAME)
endif
LDS_NAME			= $(PROJECT_NAME)


###########################################################################
# Directories
###########################################################################
DIR_PROJECT_TOP		=

DIR_OBJOUTPUT		= obj
ifeq ($(CHIPNAME),LF3000)
DIR_TARGETOUTPUT	= build_$(BOARD)_$(BOOTFROM)
else ifeq ($(CHIPNAME),NXP4330)
DIR_TARGETOUTPUT	= build_$(BOARD)
endif

CODE_MAIN_INCLUDE	=

###########################################################################
# Build Environment
###########################################################################
CPU					= cortex-a9
CC					= $(CROSS_TOOL)gcc
LD 					= $(CROSS_TOOL)ld
AS 					= $(CROSS_TOOL)as
AR 					= $(CROSS_TOOL)ar
MAKEBIN				= $(CROSS_TOOL)objcopy
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
RM					= del /q /F
MV					= move
CD					= cd
CP					= copy
ECHO				= echo
RMDIR				= rmdir /S /Q
else
MKDIR				= mkdir
RM					= rm -f
MV					= mv
CD					= cd
CP					= cp
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
					-D__arm -D$(BOOTFROM)load -DCHIPID_$(CHIPNAME)
