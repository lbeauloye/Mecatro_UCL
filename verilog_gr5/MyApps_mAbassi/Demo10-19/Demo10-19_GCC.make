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

MAKE_NAME := Demo10-19_GCC.make                # For full re-build if I'm modified

ifeq ($(PLATFORM), 0x0100AAC5)            # DE0 nano
  USE_QSPI := NO
  USE_SPI  := NO
else
  $(error Error : Unsupported value for the defintion of OS_PLATFORM)
endif

LWIP         := ../../mAbassi/lwip-1.4.1
LWIPIF       := ../../mAbassi/lwip-if
FATFS        := ../../mAbassi/FatFS-0.13a
FULLFAT      := ../../mAbassi/FullFAT-2.0.1
UEFAT        := ../../mAbassi/ueFat-2.6.11

                                        # The path order of in VPATH is important
VPATH   :=  ../src						# This is due to multiple file with same names
VPATH   += :../../mAbassi/Abassi
VPATH   += :../../mAbassi/Platform/src
VPATH   += :../../mAbassi/Drivers/src
VPATH   += :$(LWIPIF)/Abassi
VPATH   += :$(LWIPIF)/dw/RTOS
VPATH   += :../../mAbassi/Share/src
VPATH   += :$(LWIP)/src/api
VPATH   += :$(LWIP)/src/core
VPATH   += :$(LWIP)/src/core/ipv4
VPATH   += :$(LWIP)/src/netif

C_SRC   :=
C_SRC   += Demo_10-19_CORTEXA9.c
C_SRC   += TIMERinit.c
C_SRC   += CGI_SSI.c
C_SRC   += NetAddr.c
C_SRC   += WebServer.c
C_SRC   += alt_gpio.c
C_SRC   += dw_ethernet.c
C_SRC   += dw_i2c.c
C_SRC   += dw_uart.c
C_SRC   += api_lib.c
C_SRC   += api_msg.c
C_SRC   += autoip.c
C_SRC   += def.c
C_SRC   += dhcp.c
C_SRC   += dns.c
C_SRC   += err.c
C_SRC   += etharp.c
C_SRC   += ethernetif.c
C_SRC   += icmp.c
C_SRC   += igmp.c
C_SRC   += inet.c
C_SRC   += inet_chksum.c
C_SRC   += init.c
C_SRC   += ip.c
C_SRC   += ip_addr.c
C_SRC   += ip_frag.c
C_SRC   += mem.c
C_SRC   += memp.c
C_SRC   += netbuf.c
C_SRC   += netdb.c
C_SRC   += netif.c
C_SRC   += netifapi.c
C_SRC   += pbuf.c
C_SRC   += raw.c
C_SRC   += stats.c
C_SRC   += sys.c
C_SRC   += sys_arch.c
C_SRC   += tcp.c
C_SRC   += tcp_in.c
C_SRC   += tcp_out.c
C_SRC   += tcpip.c
C_SRC   += timers.c
C_SRC   += udp.c
ifeq ($(RXPERF), YES)
C_SRC += IperfRX.c
else ifeq ($(TXPERF), YES)
C_SRC += IperfTX.c
endif
											# Assembly files
S_SRC   :=
											# Object files
O_SRC   :=

AFLAGS  :=
CFLAGS  :=
LFLAGS  :=
LIBS    := 

# ----------------------------------------------------------------------------------------------------

C_INC   :=                                     # All "C" include files for dependencies
C_INC   += ../../mAbassi/Abassi/mAbassi.h
C_INC   += ../../mAbassi/Abassi/SysCall.h
C_INC   += ../../mAbassi/Platform/inc/Platform.h
C_INC   += ../../mAbassi/Platform/inc/HWinfo.h
C_INC   += ../../mAbassi/Platform/inc/AbassiLib.h
C_INC   += ../../mAbassi/Drivers/inc/alt_gpio.h
C_INC   += ../../mAbassi/Drivers/inc/dw_ethernet.h
C_INC   += ../../mAbassi/Drivers/inc/dw_i2c.h
C_INC   += ../../mAbassi/Drivers/inc/dw_uart.h
C_INC   += $(LWIPIF)/Abassi/sys_arch.h
C_INC   += $(LWIPIF)/dw/RTOS/ethernetif.h
C_INC   += ../../mAbassi/Share/inc/lwipopts.h
C_INC   += ../../mAbassi/Share/inc/WebApp.h
C_INC   += ../../mAbassi/Share/inc/WebServer.h
C_INC   += $(LWIP)/src/include/lwip/api.h
C_INC   += $(LWIP)/src/include/lwip/api_msg.h
C_INC   += $(LWIP)/src/include/lwip/arch.h
C_INC   += $(LWIP)/src/include/lwip/debug.h
C_INC   += $(LWIP)/src/include/lwip/def.h
C_INC   += $(LWIP)/src/include/lwip/dhcp.h
C_INC   += $(LWIP)/src/include/lwip/dns.h
C_INC   += $(LWIP)/src/include/lwip/err.h
C_INC   += $(LWIP)/src/include/lwip/init.h
C_INC   += $(LWIP)/src/include/lwip/mem.h
C_INC   += $(LWIP)/src/include/lwip/memp.h
C_INC   += $(LWIP)/src/include/lwip/memp_std.h
C_INC   += $(LWIP)/src/include/lwip/netbuf.h
C_INC   += $(LWIP)/src/include/lwip/netdb.h
C_INC   += $(LWIP)/src/include/lwip/netif.h
C_INC   += $(LWIP)/src/include/lwip/netifapi.h
C_INC   += $(LWIP)/src/include/lwip/opt.h
C_INC   += $(LWIP)/src/include/lwip/pbuf.h
C_INC   += $(LWIP)/src/include/lwip/raw.h
C_INC   += $(LWIP)/src/include/lwip/sio.h
C_INC   += $(LWIP)/src/include/lwip/snmp.h
C_INC   += $(LWIP)/src/include/lwip/snmp_asn1.h
C_INC   += $(LWIP)/src/include/lwip/snmp_msg.h
C_INC   += $(LWIP)/src/include/lwip/snmp_structs.h
C_INC   += $(LWIP)/src/include/lwip/sys.h
C_INC   += $(LWIP)/src/include/lwip/tcp.h
C_INC   += $(LWIP)/src/include/lwip/tcp_impl.h
C_INC   += $(LWIP)/src/include/lwip/tcpip.h
C_INC   += $(LWIP)/src/include/lwip/timers.h
C_INC   += $(LWIP)/src/include/lwip/udp.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/autoip.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/icmp.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/igmp.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/inet.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/inet_chksum.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/ip.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/ip_addr.h
C_INC   += $(LWIP)/src/include/ipv4/lwip/ip_frag.h
C_INC   += $(LWIP)/src/include/netif/etharp.h
C_INC   += $(LWIP)/src/include/netif/ppp_oe.h
C_INC   += $(LWIP)/src/include/netif/slipif.h

											# Compiler command line options. The -I order is important
CFLAGS  += -g -O3 -Wall
CFLAGS  += -I../inc
CFLAGS  += -I../../mAbassi/Abassi
CFLAGS  += -I../../mAbassi/Platform/inc
CFLAGS  += -I../../mAbassi/Drivers/inc
CFLAGS  += -I$(LWIPIF)
CFLAGS  += -I$(LWIPIF)/dw/RTOS
CFLAGS  += -I$(LWIPIF)/Abassi
CFLAGS  += -I../../mAbassi/Share/inc
CFLAGS  += -I$(LWIP)/src/include
CFLAGS  += -I$(LWIP)/src/include/lwip
CFLAGS  += -I$(LWIP)/src/include/ipv4
CFLAGS  += -I$(LWIP)/src/include/netif

CFLAGS  += -DOS_START_STACK=8192
CFLAGS  += -DETH_DEBUG=1
CFLAGS  += -DI2C_OPERATION=0x00303
CFLAGS  += -DSDMMC_BUFFER_TYPE=SDMMC_BUFFER_CACHED
CFLAGS  += -DUART_FULL_PROTECT=1

ifeq ($(RXPERF), YES)
  CFLAGS += -DTEST_RXPERF=1
  CFLAGS += -DTEST_TXPERF=0
  CFLAGS += -DOS_TIMER_USE=1000
  CFLAGS += -DETH_N_RXBUF=1024
  CFLAGS += -DETH_N_TXBUF=1024
  CFLAGS += -DRXPERF_CON_TYPE=0
  CFLAGS += -DETH_BUFFER_TYPE=ETH_BUFFER_PBUF
else ifeq ($(TXPERF), YES)
  CFLAGS += -DTEST_RXPERF=0
  CFLAGS += -DTEST_TXPERF=1
  CFLAGS += -DOS_TIMER_USE=1000
  CFLAGS += -DETH_N_RXBUF=1024
  CFLAGS += -DETH_N_TXBUF=1024
  CFLAGS += -DTXPERF_CON_TYPE=0
  CFLAGS += -DTXPERF_BUFSIZE=4*TCP_MSS
  CFLAGS += -DETH_BUFFER_TYPE=ETH_BUFFER_PBUF
else
  CFLAGS += -DTEST_RXPERF=0
  CFLAGS += -DTEST_TXPERF=0
  CFLAGS += -DETH_BUFFER_TYPE=ETH_BUFFER_CACHED
endif
											# Assembler command line options
AFLAGS  += -g

# ----------------------------------------------------------------------------------------------------
# Each Demo # has some specifics

ifeq ($(DEMO_NMB), 10)
  NET_TYPE := NETCONN
  FS_TYPE  := FS_INMEM
  SC_TYPE  := NONE
else ifeq ($(DEMO_NMB), 11)
  NET_TYPE := SOCKET
  FS_TYPE  := FS_INMEM
  SC_TYPE  := NONE
else ifeq ($(DEMO_NMB), 12)
  NET_TYPE := NETCONN
  FS_TYPE  := FS_FATFS
  SC_TYPE  := NONE
else ifeq ($(DEMO_NMB), 13)
  NET_TYPE := SOCKET
  FS_TYPE  := FS_FATFS
  SC_TYPE  := NONE
else ifeq ($(DEMO_NMB), 14)
  NET_TYPE := NETCONN
  FS_TYPE  := FS_FATFS
  SC_TYPE  := SYS_CALL
else ifeq ($(DEMO_NMB), 15)
  NET_TYPE := SOCKET
  FS_TYPE  := FS_FATFS
  SC_TYPE  := SYS_CALL
else ifeq ($(DEMO_NMB), 16)
  NET_TYPE := NETCONN
  FS_TYPE  := FS_FULLFAT
  SC_TYPE  := SYS_CALL
else ifeq ($(DEMO_NMB), 17)
  NET_TYPE := SOCKET
  FS_TYPE  := FS_FULLFAT
  SC_TYPE  := SYS_CALL
else ifeq ($(DEMO_NMB), 18)
  NET_TYPE := NETCONN
  FS_TYPE  := FS_UEFAT
  SC_TYPE  := SYS_CALL
else ifeq ($(DEMO_NMB), 19)
  NET_TYPE := SOCKET
  FS_TYPE  := FS_UEFAT
  SC_TYPE  := SYS_CALL
else
  $(error Error : Unsupported demo #)
endif

ifeq ($(NET_TYPE), NETCONN)
  C_SRC   += httpserver-netconn.c
else
  CFLAGS  += -DLWIP_SOCKET=1
  C_INC   += $(LWIP)/src/include/lwip/sockets.h
  C_SRC   += httpserver-socket.c
  C_SRC   += sockets.c
endif

ifeq ($(SC_TYPE), SYS_CALL)
  CFLAGS  += -DOS_SYS_CALL=1
else
  C_SRC   += SysCall_noFS.c
endif

ifneq ($(FS_TYPE), FS_INMEM)
  CFLAGS  += -DSDMMC_BUFFER_TYPE=SDMMC_BUFFER_CACHED
  CFLAGS  += -DSDMMC_NUM_DMA_DESC=64
  C_INC   += ../../mAbassi/Drivers/inc/dw_sdmmc.h
  C_SRC   += dw_sdmmc.c
 ifeq ($(USE_QSPI), YES)
  CFLAGS  += -DQSPI_OPERATION=0x10707
  CFLAGS  += -DSYS_CALL_N_DRV=2
  CFLAGS  += -D_VOLUMES=2
  C_INC   += ../../mAbassi/Drivers/inc/cd_qspi.h
  C_SRC   += cd_qspi.c
 else
  CFLAGS  += -DSYS_CALL_N_DRV=1
 endif
 ifeq ($(USE_SPI), YES)
  CFLAGS  += -DSYS_CALL_DEV_SPI=1
  CFLAGS  += -DSPI_OPERATION=0x00707
  C_INC   += ../../mAbassi/Drivers/inc/dw_spi.h
  C_SRC   += dw_spi.c
 endif
 ifeq ($(USE_QSPI), YES)
  C_SRC   += arm_pl330.c
 else ifeq ($(USE_SPI), YES)
  C_SRC   += arm_pl330.c
 endif
endif

ifeq ($(FS_TYPE), FS_INMEM)
  C_INC   += ../../mAbassi/Share/inc/fs.h
  C_INC   += ../../mAbassi/Share/src/fsdata.c
  C_INC   += ../../mAbassi/Share/inc/fsdata.h
  C_SRC   += fs.c
else ifeq ($(FS_TYPE), FS_FATFS)
  VPATH   += :$(FATFS)/src
  VPATH   += :$(FATFS)/src/option
  CFLAGS  += -I$(FATFS)/inc
  C_INC   += ../../../Drivers/inc/MediaIF.h
  C_INC   += $(FATFS)/inc/diskio.h
  C_INC   += $(FATFS)/inc/ff.h
  C_INC   += $(FATFS)/inc/integer.h
  C_INC   += ../../mAbassi/Share/inc/ffconf.h
  C_SRC   += ff.c
  C_SRC   += ffunicode.c
  C_SRC   += Abassi_FatFS.c
  C_SRC   += Media_FatFS.c
  C_SRC   += MediaIF.c
 ifeq ($(SC_TYPE), SYS_CALL)
  C_SRC   += SysCall_FatFS.c
  C_SRC   += SysCall_CY5.c
 endif
else ifeq ($(FS_TYPE), FS_FULLFAT)
  VPATH   += :$(FULLFAT)/src
  CFLAGS  += -I$(FULLFAT)/src/
  C_INC   += ../../mAbassi/Drivers/inc/MediaIF.h
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
  C_SRC   += MediaIF.c
 ifeq ($(SC_TYPE), SYS_CALL)
  C_SRC   += SysCall_FullFAT.c
  C_SRC   += SysCall_CY5.c
 endif
else ifeq ($(FS_TYPE), FS_UEFAT)
  VPATH   += :$(UEFAT)
  CFLAGS  += -I$(UEFAT)
  C_INC   += ../../mAbassi/Drivers/inc/MediaIF.h
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
  C_SRC   += MediaIF.c
 ifeq ($(SC_TYPE), SYS_CALL)
  C_SRC   += SysCall_ueFAT.c
  C_SRC   += SysCall_CY5.c
 endif

endif

# ----------------------------------------------------------------------------------------------------

include ../../mAbassi/Common_GCC.make

# EOF

