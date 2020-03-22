# ----------------------------------------------------------------------------------------------------

MAKE_NAME := ${MyAPP}_GCC.make		        # For full re-build if I'm modified

LWIPIF       := ../../mAbassi/lwip-if

											# The path order of in VPATH is important
VPATH   := ../src	        				# This is due to multiple file with same names
VPATH   += :../hwlib
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :$(LWIPIF)/Abassi
VPATH   += :$(LWIPIF)/dw/RTOS
VPATH   += :../../mAbassi/Share/src

C_SRC   :=									# C sources with/without thumb (see THUMB above)
C_SRC   += Main_mAbassi.c
C_SRC   += MyApp_Web.c
C_SRC   += MyFatFS.c
C_SRC   += SysCall_noFS.c

C_SRC   += TIMERinit.c
C_SRC   += CGI_SSI.c
C_SRC   += NetAddr.c
C_SRC   += WebServer.c
C_SRC   += httpserver-netconn.c

C_SRC   += arm_acp.c
C_SRC   += arm_pl330.c
C_SRC   += alt_gpio.c
C_SRC   += dw_ethernet.c
C_SRC   += dw_i2c.c
C_SRC   += dw_uart.c
C_SRC   += dw_sdmmc.c

C_SRC   += alt_generalpurpose_io.c
											# Assembly files
S_SRC   :=
											# Object files
O_SRC   := 

AFLAGS   :=
CFLAGS   :=
LFLAGS   :=
LIBS     := 

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
C_INC   += ../../mAbassi/Drivers/inc/dw_ethernet.h
C_INC   += ../../mAbassi/Drivers/inc/dw_i2c.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
C_INC   += ../../mAbassi/Drivers/inc/dw_sdmmc.h

C_INC   += ../../mAbassi/Share/inc/lwipopts.h
C_INC   += ../../mAbassi/Share/inc/WebApp.h
C_INC   += ../../mAbassi/Share/inc/WebServer.h

C_INC   += ../inc/MyApp_Web.h
C_INC   += ../inc/hps_0.h
											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I ../inc
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc

CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av
CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DUART_FULL_PROTECT=1
CFLAGS  += -DI2C_OPERATION=0x00303
CFLAGS  += -DSYS_CALL_N_DRV=1

CFLAGS  += -D soc_cv_av

											# Assembler command line options
AFLAGS  += -g
# ----------------------------------------------------------------------------------------------------

MAKE_NAME += ../MyLwIP_GCC.make
MAKE_NAME += ../MyFatFS_GCC.make

include ../MyLwIP_GCC.make
include ../MyFatFS_GCC.make

# --- Webserver is based on a memory based file system

C_INC   += ../../mAbassi/Share/inc/fs.h
C_INC   += ../../mAbassi/Share/src/fsdata.c
C_INC   += ../../mAbassi/Share/inc/fsdata.h
C_SRC   += fs.c

include ../../mAbassi/Common_GCC.make

# EOF

