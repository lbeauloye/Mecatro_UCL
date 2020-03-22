#
# File: Demo8_GCC.make
#
# Copyright (c) 2017-2018, Code-Time Technologies Inc.
# All rights reserved.		
#

# NOTE:
# This Makefile includes "Common_GCC.make" which contains the common part of all makefiles
# It is done to simplify the work needed when something is changed for a target or for a demo #
#
# ----------------------------------------------------------------------------------------------------

MAKE_NAME := Demo8_GCC.make					# For full re-build if I'm modified

											# The path order of in VPATH is important
VPATH   :=  ../src						# This is due to multiple file with same names
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :../../mAbassi/Share/src

C_SRC   :=
C_SRC   += Demo_8_CORTEXA9.c
C_SRC   += SysCall_noFS.c
C_SRC   += TIMERinit.c
C_SRC   += arm_acp.c
C_SRC   += arm_pl330.c
C_SRC   += alt_gpio.c
C_SRC   += dw_i2c.c
C_SRC   += dw_uart.c
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
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I../inc
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I../../mAbassi/Share/inc

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DI2C_DEBUG=1
CFLAGS  += -DI2C_OPERATION=0x00707
CFLAGS  += -DI2C_USE_MUTEX=0
CFLAGS  += -DUART_FULL_PROTECT=1
											# Assembler command line options
AFLAGS  += -g

# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

