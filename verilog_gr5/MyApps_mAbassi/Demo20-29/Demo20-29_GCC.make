#
# File: Demo20-29_GCC.make
#
# Copyright (c) 2017-2018, Code-Time Technologies Inc.
# All rights reserved.		
#

# NOTE:
# This Makefile includes "Common_GCC.make" which contains the common part of all makefiles
# It is done to simplify the work needed when something is changed for a target or for a demo #
#
# ----------------------------------------------------------------------------------------------------

MAKE_NAME := Demo20-29_GCC.make					# For full re-build if I'm modified

ifeq ($(PLATFORM), 0x0100AAC5)            # DE0 nano
USE_MDRV := 1
USE_MMC  := 1
USE_QSPI := 0
USE_SPI  := 0
USE_RTC  := 0
else
$(error Error : Unsupported value for the defintion of OS_PLATFORM)
endif

FATFS        := ../../mAbassi/FatFS-0.13a
FULLFAT      := ../../mAbassi/FullFAT-2.0.1
UEFAT        := ../../mAbassi/ueFat-2.6.11

                                        # The path order of in VPATH is important
VPATH   :=  ../src						# This is due to multiple file with same names
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src

C_SRC   :=
C_SRC   += Demo_20-29_CORTEXA9.c
C_SRC   += TIMERinit.c
C_SRC   += alt_gpio.c
C_SRC   += arm_acp.c
C_SRC   += arm_pl330.c
C_SRC   += dw_i2c.c
C_SRC   += dw_uart.c
C_SRC   += MediaIF.c
C_SRC   += SysCall_CY5.c
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
C_INC   += ../../mAbassi/Drivers/inc/alt_gpio.h
C_INC   += ../../mAbassi/Drivers/inc/arm_acp.h
C_INC   += ../../mAbassi/Drivers/inc/arm_pl330.h
C_INC   += ../../mAbassi/Drivers/inc/dw_i2c.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
C_INC   += ../../mAbassi/Drivers/inc/MediaIF.h
											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I../inc
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DOS_SYS_CALL=1
CFLAGS  += -DSYS_CALL_DEV_I2C=1
CFLAGS  += -DSYS_CALL_DEV_TTY=1
CFLAGS  += -DSYS_CALL_TTY_EOF=4
CFLAGS  += -DI2C_DEBUG=0
CFLAGS  += -DI2C_OPERATION=0x00707
CFLAGS  += -DI2C_USE_MUTEX=1
CFLAGS  += -DMEDIA_DEBUG=1
CFLAGS  += -DSDMMC_BUFFER_TYPE=SDMMC_BUFFER_CACHED
CFLAGS  += -DSDMMC_NUM_DMA_DESC=64
CFLAGS  += -DSDMMC_USE_MUTEX=1
CFLAGS  += -DUART_FULL_PROTECT=1
											# Assembler command line options
AFLAGS  += -g

# ----------------------------------------------------------------------------------------------------
# Each Demo # has some specifics

ifeq ($(DEMO_NMB), 20)
  USE_FATFS   := YES
  USE_FULLFAT := NO
  USE_UEFAT   := NO
  USE_MULTI   := NO
else ifeq ($(DEMO_NMB), 21)
  USE_FATFS   := NO
  USE_FULLFAT := YES
  USE_UEFAT   := NO
  USE_MULTI   := NO
else ifeq ($(DEMO_NMB), 22)
  USE_FATFS   := NO
  USE_FULLFAT := NO
  USE_UEFAT   := YES
  USE_MULTI   := NO
else ifeq ($(DEMO_NMB), 29)
  USE_FATFS   := YES
  USE_FULLFAT := YES
  USE_UEFAT   := YES
  USE_MULTI   := YES
else
  $(error Error : Unsupported demo #)
endif

ifeq ($(USE_FATFS), YES)
  VPATH   += :$(FATFS)/src
  VPATH   += :$(FATFS)/src/option
  CFLAGS  += -I$(FATFS)/inc
  CFLAGS  += -I../../Drivers/inc
 ifneq ($(USE_RTC), 0)
  CFLAGS  += -DFF_FS_NORTC=0
 endif
 ifeq ($(USE_MULTI), YES)
  CFLAGS  += -DSYS_CALL_MULTI_FATFS=1
 endif
  C_INC   += $(FATFS)/inc/diskio.h
  C_INC   += $(FATFS)/inc/ff.h
  C_INC   += $(FATFS)/inc/integer.h
  C_INC   += ../../mAbassi/Share/inc/ffconf.h
  C_SRC   += ff.c
  C_SRC   += ffunicode.c
  C_SRC   += Abassi_FatFS.c
  C_SRC   += Media_FatFS.c
  C_SRC   += SysCall_FatFS.c
endif
ifeq ($(USE_FULLFAT), YES)
  VPATH   += :$(FULLFAT)/src
  CFLAGS  += -I$(FULLFAT)/src/
  CFLAGS  += -I../../Drivers/inc
 ifneq ($(USE_RTC), 0)
  CFLAGS  += -DFF_TIME_SUPPORT=1
 endif
 ifeq ($(USE_MULTI), YES)
  CFLAGS  += -DSYS_CALL_MULTI_FULLFAT=1
 endif
  C_INC   += $(FULLFAT)/src/ff_blk.h
  C_INC   += $(FULLFAT)/src/ff_crc.h
  C_INC   += $(FULLFAT)/src/ff_dir.h
  C_INC   += $(FULLFAT)/src/ff_error.h
  C_INC   += $(FULLFAT)/src/ff_fat.h
  C_INC   += $(FULLFAT)/src/ff_fatdef.h
  C_INC   += $(FULLFAT)/src/ff_file.h
  C_INC   += $(FULLFAT)/src/ff_format.h
  C_INC   += $(FULLFAT)/src/ff_hash.h
  C_INC   += $(FULLFAT)/src/ff_ioman.h
  C_INC   += $(FULLFAT)/src/ff_memory.h
  C_INC   += $(FULLFAT)/src/ff_safety.h
  C_INC   += $(FULLFAT)/src/ff_string.h
  C_INC   += $(FULLFAT)/src/ff_time.h
  C_INC   += $(FULLFAT)/src/ff_types.h
  C_INC   += $(FULLFAT)/src/fullfat.h
  C_INC   += ../../mAbassi/Share/inc/ff_config.h
  C_SRC   += ff_blk.c
  C_SRC   += ff_crc.c
  C_SRC   += ff_dir.c
  C_SRC   += ff_error.c
  C_SRC   += ff_fat.c
  C_SRC   += ff_file.c
  C_SRC   += ff_format.c
  C_SRC   += ff_hash.c
  C_SRC   += ff_ioman.c
  C_SRC   += ff_memory.c
  C_SRC   += ff_string.c
  C_SRC   += Abassi_FullFAT.c
  C_SRC   += Media_FullFAT.c
  C_SRC   += SysCall_FullFAT.c
endif
ifeq ($(USE_UEFAT), YES)
  VPATH   += :$(UEFAT)
  CFLAGS  += -I$(UEFAT)
  CFLAGS  += -I../../Drivers/inc
 ifneq ($(USE_RTC), 0)
  CFLAGS  += -DFATFS_INC_TIME_DATE_SUPPORT=1
 endif
 ifeq ($(USE_MULTI), YES)
  CFLAGS  += -DSYS_CALL_MULTI_UEFAT=1
 endif
  C_INC   += $(UEFAT)/fat_access.h
  C_INC   += $(UEFAT)/fat_cache.h
  C_INC   += $(UEFAT)/fat_filelib.h
  C_INC   += $(UEFAT)/fat_format.h
  C_INC   += $(UEFAT)/fat_list.h
  C_INC   += $(UEFAT)/fat_misc.h
  C_INC   += $(UEFAT)/fat_string.h
  C_INC   += $(UEFAT)/fat_table.h
  C_INC   += $(UEFAT)/fat_types.h
  C_INC   += $(UEFAT)/fat_write.h
  C_INC   += ../../mAbassi/Share/inc/fat_opts.h
  C_SRC   += fat_access.c
  C_SRC   += fat_cache.c
  C_SRC   += fat_filelib.c
  C_SRC   += fat_format.c
  C_SRC   += fat_misc.c
  C_SRC   += fat_string.c
  C_SRC   += fat_table.c
  C_SRC   += fat_write.c
  C_SRC   += Abassi_ueFAT.c
  C_SRC   += Media_ueFAT.c
  C_SRC   += SysCall_ueFAT.c
endif

ifeq ($(USE_MULTI), YES)
  C_SRC   += SysCall_MultiFS.c
endif

#ifneq ($(USE_MMC), 0)
  C_INC   += ../../mAbassi/Drivers/inc/dw_sdmmc.h
  C_SRC   += dw_sdmmc.c
#endif

ifneq ($(USE_QSPI), 0)
  CFLAGS  += -DQSPI_DEBUG=0
  CFLAGS  += -DMEDIA_QSPI_SECT_SZ=512
  CFLAGS  += -DMEDIA_QSPI_CHK_WRT=2
  CFLAGS  += -DQSPI_OPERATION=0x10707
  CFLAGS  += -DQSPI_USE_MUTEX=1
  C_INC   += ../../mAbassi/Drivers/inc/cd_qspi.h
  C_SRC   += cd_qspi.c
endif

ifneq ($(USE_MDRV), 0)
  CFLAGS  += -DMEDIA_MDRV_SIZE=-1
endif

ifneq ($(USE_MMC), 0)
 ifneq ($(USE_QSPI), 0)
  ifneq ($(USE_MDRV), 0)
   NMB_DRV := 3
  else
   NMB_DRV := 2
  endif
 else
  ifneq ($(USE_MDRV), 0)
   NMB_DRV := 2
  else
   NMB_DRV := 1
  endif
 endif
else
 ifneq ($(USE_QSPI), 0)
  ifneq ($(USE_MDRV), 0)
   NMB_DRV := 2
  else
   NMB_DRV := 1
  endif
 else
  ifneq ($(USE_MDRV), 0)
   NMB_DRV := 1
  else
   NMB_DRV := 0
  endif
 endif
endif
CFLAGS  += -D_VOLUMES=$(NMB_DRV)
CFLAGS  += -DSYS_CALL_N_DRV=$(NMB_DRV)

ifneq ($(USE_SPI), 0)
  CFLAGS  += -DSYS_CALL_DEV_SPI=1
  CFLAGS  += -DSPI_OPERATION=0x00707
  CFLAGS  += -DSPI_USE_MUTEX=1
  C_INC   += ../../mAbassi/Drivers/inc/dw_spi.h
  C_SRC   += dw_spi.c
endif
											# We have nothing to do with auto select
ifneq ($(AUTO_SELECT), YES)
  CFLAGS  += -DMEDIA_AUTO_SELECT=0
  ifneq ($(USE_MMC), 0)
	CFLAGS   += -DMEDIA_SDMMC0_IDX=0
	CFLAGS   += -DMEDIA_SDMMC0_DEV=SDMMC_DEV
  endif
  ifneq ($(USE_QSPI), 0)
   ifneq ($(USE_MMC), 0)
	CFLAGS   += -DMEDIA_QSPI0_IDX=1
   endif
	CFLAGS   += -DMEDIA_QSPI0_DEV=QSPI_DEV
	CFLAGS   += -DMEDIA_QSPI0_SLV=QSPI_SLV
  endif
  ifneq ($(USE_MDRV), 0)
   ifneq ($(USE_QSPI), 0)
    ifneq ($(USE_MMC), 0)
     CFLAGS += -DMEDIA_MDRV_IDX=2
    else
     CFLAGS += -DMEDIA_MDRV_IDX=1
    endif
   else
    ifneq ($(USE_MMC), 0)
     CFLAGS += -DMEDIA_MDRV_IDX=1
    else
     CFLAGS += -DMEDIA_MDRV_IDX=0
    endif
   endif
  endif
endif

ifneq ($(USE_MMC), 0)
  CFLAGS += -DDEMO_USE_SDMMC=1
endif
ifneq ($(USE_QSPI), 0)
  CFLAGS += -DDEMO_USE_QSPI=1
endif
ifneq ($(USE_SPI), 0)
  CFLAGS += -DDEMO_USE_SPI=1
endif

# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

