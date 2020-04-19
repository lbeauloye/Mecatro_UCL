# ----------------------------------------------------------------------------------------------------

MAKE_NAME := MyApp_MTL2_GCC.make		    # For full re-build if I'm modified

											# The path order of in VPATH is important
VPATH   := ../src	        				# This is due to multiple file with same names
VPATH   += :../hwlib
VPATH   += :../painter
VPATH   += :../painter/fonts
VPATH   += :../painter/graphic_lib
VPATH   += :../painter/terasic_lib
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src

C_SRC   :=									# C sources with/without thumb (see THUMB above)
C_SRC   += Main_mAbassi.c
C_SRC   += MyApp_MTL2.c
C_SRC   += MyFatFS.c
C_SRC   += SysCall_noFS.c
C_SRC   += TIMERinit.c

C_SRC   += arm_acp.c
C_SRC   += arm_pl330.c
C_SRC   += alt_gpio.c
C_SRC   += dw_i2c.c
C_SRC   += dw_sdmmc.c
C_SRC   += dw_uart.c

C_SRC   += tahomabold_20.c
C_SRC   += tahomabold_32.c
C_SRC   += geometry.c
C_SRC   += gesture.c
C_SRC   += gui_vpg.c
C_SRC   += gui.c
C_SRC   += vip_fr.c
C_SRC   += simple_graphics.c
C_SRC   += simple_text.c
C_SRC   += debug.c
C_SRC   += I2C_core.c
C_SRC   += I2C.c
C_SRC   += multi_touch2.c
C_SRC   += queue.c

C_SRC   += alt_generalpurpose_io.c
C_SRC   += alt_globaltmr.c
C_SRC   += alt_clock_manager.c
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

C_INC   += ../../mAbassi/Drivers/inc/arm_acp.h
C_INC   += ../../mAbassi/Drivers/inc/arm_pl330.h
C_INC   += ../../mAbassi/Drivers/inc/alt_gpio.h
C_INC   += ../../mAbassi/Drivers/inc/dw_i2c.h
C_INC   += ../../mAbassi/Drivers/inc/dw_sdmmc.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h

C_INC   += ../inc/MyApp_MTL2.h
C_INC   += ../inc/hps_0.h

C_INC   += ../painter/geometry.h
C_INC   += ../painter/gesture.h
C_INC   += ../painter/gui.h
C_INC   += ../painter/vip_fr.h
C_INC   += ../painter/fonts/fonts.h
C_INC   += ../painter/graphic_lib/simple_graphics.h
C_INC   += ../painter/graphic_lib/simple_text.h
C_INC   += ../painter/terasic_lib/debug.h
C_INC   += ../painter/terasic_lib/I2C_core.h
C_INC   += ../painter/terasic_lib/I2C.h
C_INC   += ../painter/terasic_lib/multi_touch2.h
C_INC   += ../painter/terasic_lib/queue.h
C_INC   += ../painter/terasic_lib/terasic_includes.h

											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I ../inc
CFLAGS  += -I ../painter
CFLAGS  += -I ../painter/fonts
CFLAGS  += -I ../painter/graphic_lib
CFLAGS  += -I ../painter/terasic_lib

CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc

CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av
CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DUART_FULL_PROTECT=1

CFLAGS  += -D soc_cv_av
CFLAGS  += -D MYAPP_MTL

											# Assembler command line options
AFLAGS  += -g

NMB_DRV := 2

# ----------------------------------------------------------------------------------------------------

MAKE_NAME += ../MyFatFS_GCC.make

include ../MyFatFS_GCC.make
include ../../mAbassi/Common_GCC.make

# EOF
