# ----------------------------------------------------------------------------------------------------

MAKE_NAME := ${MyAPP}_GCC.make		        # For full re-build if I'm modified

											# The path order of in VPATH is important
VPATH   := ../src	        				# This is due to multiple file with same names
VPATH   += :../hwlib
VPATH   += :../Lib_LT24
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src

C_SRC   :=									# C sources with/without thumb (see THUMB above)
C_SRC   += Main_mAbassi.c
C_SRC   += MyApp_LT24.c
C_SRC   += MyLT24.c
C_SRC   += SysCall_noFS.c
C_SRC   += TIMERinit.c
C_SRC   += dw_uart.c

C_SRC   += tahomabold_20.c
C_SRC   += tahomabold_32.c
C_SRC   += AD7843.c
C_SRC   += geometry.c
C_SRC   += gui_test1.c
C_SRC   += gui_test2.c
C_SRC   += ILI9341.c
C_SRC   += simple_button.c
C_SRC   += simple_graphics.c
C_SRC   += simple_text.c

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
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h

C_INC   += ../inc/MyApp_LT24.h
C_INC   += ../inc/MyLT24.h
C_INC   += ../inc/hps_0.h

C_INC   += ../Lib_LT24/AD7843.h
C_INC   += ../Lib_LT24/alt_types.h
C_INC   += ../Lib_LT24/alt_video_display.h
C_INC   += ../Lib_LT24/fonts.h
C_INC   += ../Lib_LT24/geometry.h
C_INC   += ../Lib_LT24/ILI9341.h
C_INC   += ../Lib_LT24/simple_button.h
C_INC   += ../Lib_LT24/simple_graphics.h
C_INC   += ../Lib_LT24/simple_text.h
											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I ../inc
CFLAGS  += -I ../Lib_LT24
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc

CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av
CFLAGS  += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DUART_FULL_PROTECT=1

CFLAGS  += -D soc_cv_av

											# Assembler command line options
AFLAGS  += -g

# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

