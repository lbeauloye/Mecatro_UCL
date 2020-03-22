# ----------------------------------------------------------------------------------------------------

LWIP         := ../../mAbassi/lwip-1.4.1
LWIPIF       := ../../mAbassi/lwip-if

VPATH   += :$(LWIP)/src/api
VPATH   += :$(LWIP)/src/core
VPATH   += :$(LWIP)/src/core/ipv4
VPATH   += :$(LWIP)/src/netif

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
C_SRC	+= sockets.c
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

# ----------------------------------------------------------------------------------------------------


C_INC   += $(LWIPIF)/Abassi/sys_arch.h
C_INC   += $(LWIPIF)/dw/RTOS/ethernetif.h

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
C_INC   += $(LWIP)/src/include/lwip/sockets.h
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

CFLAGS  += -I$(LWIPIF)
CFLAGS  += -I$(LWIPIF)/dw/RTOS
CFLAGS  += -I$(LWIPIF)/Abassi
CFLAGS  += -I$(LWIP)/src/include
CFLAGS  += -I$(LWIP)/src/include/lwip
CFLAGS  += -I$(LWIP)/src/include/ipv4
CFLAGS  += -I$(LWIP)/src/include/netif

CFLAGS  += -DETH_DEBUG=0

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

# ----------------------------------------------------------------------------------------------------

# EOF
