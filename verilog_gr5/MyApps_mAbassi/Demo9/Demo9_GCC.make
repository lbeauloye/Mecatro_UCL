#
# File: Demo9_GCC.make
#
# Copyright (c) 2017-2018, Code-Time Technologies Inc.
# All rights reserved.		
#

# NOTE:
# This Makefile includes "Common_GCC.make" which contains the common part of all makefiles
# It is done to simplify the work needed when something is changed for a target or for a demo #
#
# ----------------------------------------------------------------------------------------------------

MAKE_NAME := Demo9_GCC.make					# For full re-build if I'm modified

ifeq ($(PLATFORM), 0x0100AAC5)            # DE0 nano
  USE_QSPI := NO
  USE_RTC  := 0
else
  $(error Error : Unsupported value for the defintion of OS_PLATFORM)
endif

FATFS   := ../../mAbassi/FatFS-0.13a
                                        # The path order of in VPATH is important
VPATH   :=  ../src						# This is due to multiple file with same names
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src
VPATH   += :$(FATFS)/src
VPATH   += :$(FATFS)/src/option

C_SRC   :=
C_SRC   += Demo_9_CORTEXA9.c
C_SRC   += SysCall_noFS.c
C_SRC   += TIMERinit.c
C_SRC   += arm_acp.c
C_SRC   += arm_pl330.c
C_SRC   += alt_gpio.c
C_SRC   += dw_i2c.c
ifeq ($(USE_QSPI), YES)
C_SRC   += cd_qspi.c
endif
C_SRC   += dw_sdmmc.c
C_SRC   += dw_uart.c
C_SRC   += Abassi_FatFS.c
C_SRC   += Media_FatFS.c
C_SRC   += MediaIF.c
C_SRC   += SysCall_CY5.c
C_SRC   += ff.c
C_SRC   += ffunicode.c
											# Assembly files
S_SRC   :=
											# Object files
O_SRC   :=

AFLAGS  :=
CFLAGS  :=
LFLAGS  :=
LIBS    := 

# ----------------------------------------------------------------------------------------------------

C_INC   := 									# All "C" include files for dependencies
C_INC   += ../../mAbassi/Abassi/mAbassi.h
C_INC   += ../../mAbassi/Abassi/SysCall.h
C_INC   += ../../mAbassi/Platform/inc/Platform.h
C_INC   += ../../mAbassi/Platform/inc/HWinfo.h
C_INC   += ../../mAbassi/Platform/inc/AbassiLib.h
C_INC   += ../../mAbassi/Drivers/inc/arm_acp.h
C_INC   += ../../mAbassi/Drivers/inc/arm_pl330.h
C_INC   += ../../mAbassi/Drivers/inc/alt_gpio.h
C_INC   += ../../mAbassi/Drivers/inc/dw_i2c.h
ifeq ($(USE_QSPI), YES)
C_INC   += ../../mAbassi/Drivers/inc/cd_qspi.h
endif
C_INC   += ../../mAbassi/Drivers/inc/dw_sdmmc.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
C_INC   += ../../mAbassi/Drivers/inc/MediaIF.h
C_INC   += ../../mAbassi/Share/inc/ffconf.h
C_INC   += $(FATFS)/inc/diskio.h
C_INC   += $(FATFS)/inc/ff.h
C_INC   += $(FATFS)/inc/integer.h
											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I../inc
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc
CFLAGS  += -I$(FATFS)/inc/

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DI2C_OPERATION=0x00707
CFLAGS  += -DSDMMC_BUFFER_TYPE=SDMMC_BUFFER_CACHED
CFLAGS  += -DSDMMC_NUM_DMA_DESC=64
CFLAGS  += -DSDMMC_USE_MUTEX=0
CFALGS  += -DMEDIA_AUTO_SELECT=1
CFLAGS  += -DMEDIA_MDRV_SIZE=-1
ifneq ($(USE_RTC), 0)
  CFLAGS  += -DFF_FS_NORTC=0
endif
ifeq ($(USE_QSPI), YES)
  CFLAGS += -D_VOLUMES=3
  CFLAGS += -DQSPI_OPERATION=0x00202
  CFLAGS += -DQSPI_USE_MUTEX=0
  CFLAGS += -DMEDIA_QSPI_SECT_SZ=512
else
  CFLAGS += -D_VOLUMES=2
endif
CFLAGS  += -DUART_FULL_PROTECT=1
											# Assembler command line options
AFLAGS  += -g

# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

