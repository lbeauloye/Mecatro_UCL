# ----------------------------------------------------------------------------------------------------
FATFS   := ../../mAbassi/FatFS-0.13a

VPATH   += :$(FATFS)/src
VPATH   += :$(FATFS)/src/option

C_SRC   += Abassi_FatFS.c
C_SRC   += Media_FatFS.c
C_SRC   += MediaIF.c
C_SRC   += SysCall_CY5.c
C_SRC   += ff.c
C_SRC   += ffunicode.c

# ----------------------------------------------------------------------------------------------------

C_INC   += ../../mAbassi/Drivers/inc/MediaIF.h
C_INC   += ../../mAbassi/Share/inc/ffconf.h
C_INC   += $(FATFS)/inc/diskio.h
C_INC   += $(FATFS)/inc/ff.h
C_INC   += $(FATFS)/inc/integer.h

CFLAGS  += -I$(FATFS)/inc/

CFLAGS  += -DSDMMC_BUFFER_TYPE=SDMMC_BUFFER_CACHED
CFLAGS  += -DSDMMC_NUM_DMA_DESC=64
CFLAGS  += -DSDMMC_USE_MUTEX=0
CFALGS  += -DMEDIA_AUTO_SELECT=0
CFLAGS  += -DMEDIA_MDRV_SIZE=-1
CFLAGS  += -D_VOLUMES=2

# ----------------------------------------------------------------------------------------------------

# EOF
