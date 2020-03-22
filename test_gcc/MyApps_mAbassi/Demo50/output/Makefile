#
# Copyright (c) 2014-2018, Code-Time Technologies Inc.
# All rights reserved.		
#

# NOTE:
# This Makefile includes another one: ../Demo50_GCC.make
# It is done to simplify the work needed when something is changed for a target or for a demo #
# 
# This makefile must be tailored to the mAbassi package used
# Depending on the availability of the drivers in source form or library
# You need to select the mAbassi type of code
#
# TYPE_SRC   : Source code
# TYPE_EVAL  : Evaluation library (with time expiry)
# TYPE_TRIAL : Evaluation library (no time expiry)
#
BUILD_TYPE   := TYPE_EVAL

# If using GCC ARM (Linaro), set LINARO to YES, otherwise set to NO
LINARO       := NO

# How to set-up the UART:
#      UART_TYPE := POLL	- Polling
#      UART_TYPE := MBX		- RTOS mailboxes with ISR
#      UART_TYPE := # 		- Circular buffer with ISR (# is size of the RX & TX circular buffers)
UART_TYPE     := 512

# ----------------------------------------------------------------------------------------------------
#
# Defines to keep "C" / "ASM" / linker in sync

DEMO_NMB     := 50
PLATFORM     := 0x0100AAC5
N_CORE       := 2
LIB_REENT    := 0

# ----------------------------------------------------------------------------------------------------

include ../Demo50_GCC.make

#EOF
