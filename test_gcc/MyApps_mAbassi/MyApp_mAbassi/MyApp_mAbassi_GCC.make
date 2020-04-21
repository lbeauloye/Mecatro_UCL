# ----------------------------------------------------------------------------------------------------

MAKE_NAME := ${MyAPP}_GCC.make		        # For full re-build if I'm modified

SOCEDS_DEST_ROOT := C:/intelFPGA/18.1/embedded

											# The path order of in VPATH is important
VPATH   := ../src
VPATH   += :../inc/libcanard	
VPATH   += :../inc/o1heap        				# This is due to multiple file with same names
VPATH   += :../inc/motor
VPATH   += :../hwlib
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src

C_SRC   :=									# C sources with/without thumb (see THUMB above)
CPP_SRC :=
#C_SRC   += Main_mAbassi.c
#C_SRC   += MyApp_mAbassi.c
CPP_SRC   += Main_mAbassi.cpp
CPP_SRC   += MyApp_mAbassi.cpp
C_SRC   += SysCall_noFS.c
C_SRC   += TIMERinit.c
C_SRC   += dw_uart.c
C_SRC   += alt_gpio.c
C_SRC   += dw_spi.c
C_SRC   += alt_generalpurpose_io.c
C_SRC   += canard.c
C_SRC   += o1heap.c


CPP_SRC   += motor_card.cpp

CPP_SRC   += middle_level.cpp


CPP_SRC += test.cpp
CPP_SRC   += MyDriver_CAN.cpp

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
CPP_INC :=
C_INC   += ../../mAbassi/Abassi/mAbassi.h
C_INC   += ../../mAbassi/Abassi/SysCall.h
C_INC   += ../../mAbassi/Platform/inc/Platform.h
C_INC   += ../../mAbassi/Platform/inc/HWinfo.h
C_INC   += ../../mAbassi/Platform/inc/AbassiLib.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
C_INC   += ../../mAbassi/Drivers/inc/dw_spi.h
CPP_INC   += ../inc/libcanard/canard.h
CPP_INC += ../inc/o1heap/o1heap.h

CPP_INC   += ../inc/hps_0.h

CPP_INC += ../inc/motor/motor_card.h
CPP_INC += ../inc/motor/middle_level.h

CPP_INC += ../inc/test.h



CPP_INC   += ../inc/MyApp_mAbassi.h
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


CFLAGS  += -D soc_cv_av

											# Assembler command line options
AFLAGS  += -g
# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

