/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2_gen2_0' in SOPC Builder design 'soc_system'
 * SOPC Builder design path: ../../../soc_system.sopcinfo
 *
 * Generated: Sun Mar 22 11:48:29 CET 2020
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00021020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "tiny"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1c
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 0
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 0
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_ICACHE_SIZE 0
#define ALT_CPU_INST_ADDR_WIDTH 0x12
#define ALT_CPU_NAME "nios2_gen2_0"
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00021020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "tiny"
#define NIOS2_DATA_ADDR_WIDTH 0x1c
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00000020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 0
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 0
#define NIOS2_ICACHE_LINE_SIZE_LOG2 0
#define NIOS2_ICACHE_SIZE 0
#define NIOS2_INST_ADDR_WIDTH 0x12
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_ADDRESS_SPAN_EXTENDER
#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SPI
#define __ALTERA_AVALON_TIMER
#define __ALTERA_NIOS2_GEN2


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone V"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart_0"
#define ALT_STDERR_BASE 0x20000
#define ALT_STDERR_DEV jtag_uart_0
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart_0"
#define ALT_STDIN_BASE 0x20000
#define ALT_STDIN_DEV jtag_uart_0
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart_0"
#define ALT_STDOUT_BASE 0x20000
#define ALT_STDOUT_DEV jtag_uart_0
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "soc_system"


/*
 * address_span_extender_0_cntl configuration
 *
 */

#define ADDRESS_SPAN_EXTENDER_0_CNTL_BASE 0x20008
#define ADDRESS_SPAN_EXTENDER_0_CNTL_BURSTCOUNT_WIDTH 1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_BYTEENABLE_WIDTH 4
#define ADDRESS_SPAN_EXTENDER_0_CNTL_CNTL_ADDRESS_WIDTH 1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_DATA_WIDTH 32
#define ADDRESS_SPAN_EXTENDER_0_CNTL_IRQ -1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_MASTER_ADDRESS_WIDTH 0x20
#define ADDRESS_SPAN_EXTENDER_0_CNTL_MAX_BURST_BYTES 4
#define ADDRESS_SPAN_EXTENDER_0_CNTL_MAX_BURST_WORDS 1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_NAME "/dev/address_span_extender_0_cntl"
#define ADDRESS_SPAN_EXTENDER_0_CNTL_SLAVE_ADDRESS_SHIFT 0x2
#define ADDRESS_SPAN_EXTENDER_0_CNTL_SLAVE_ADDRESS_WIDTH 0x19
#define ADDRESS_SPAN_EXTENDER_0_CNTL_SPAN 8
#define ADDRESS_SPAN_EXTENDER_0_CNTL_SUB_WINDOW_COUNT 1
#define ADDRESS_SPAN_EXTENDER_0_CNTL_TYPE "altera_address_span_extender"
#define ALT_MODULE_CLASS_address_span_extender_0_cntl altera_address_span_extender


/*
 * address_span_extender_0_windowed_slave configuration
 *
 */

#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_BASE 0x8000000
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_BURSTCOUNT_WIDTH 1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_BYTEENABLE_WIDTH 4
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_CNTL_ADDRESS_WIDTH 1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_DATA_WIDTH 32
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_IRQ -1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_MASTER_ADDRESS_WIDTH 0x20
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_MAX_BURST_BYTES 4
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_MAX_BURST_WORDS 1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_NAME "/dev/address_span_extender_0_windowed_slave"
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_SLAVE_ADDRESS_SHIFT 0x2
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_SLAVE_ADDRESS_WIDTH 0x19
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_SPAN 134217728
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_SUB_WINDOW_COUNT 1
#define ADDRESS_SPAN_EXTENDER_0_WINDOWED_SLAVE_TYPE "altera_address_span_extender"
#define ALT_MODULE_CLASS_address_span_extender_0_windowed_slave altera_address_span_extender


/*
 * dipsw_pio_nios configuration
 *
 */

#define ALT_MODULE_CLASS_dipsw_pio_nios altera_avalon_pio
#define DIPSW_PIO_NIOS_BASE 0x20040
#define DIPSW_PIO_NIOS_BIT_CLEARING_EDGE_REGISTER 1
#define DIPSW_PIO_NIOS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define DIPSW_PIO_NIOS_CAPTURE 1
#define DIPSW_PIO_NIOS_DATA_WIDTH 4
#define DIPSW_PIO_NIOS_DO_TEST_BENCH_WIRING 0
#define DIPSW_PIO_NIOS_DRIVEN_SIM_VALUE 0
#define DIPSW_PIO_NIOS_EDGE_TYPE "ANY"
#define DIPSW_PIO_NIOS_FREQ 50000000
#define DIPSW_PIO_NIOS_HAS_IN 1
#define DIPSW_PIO_NIOS_HAS_OUT 0
#define DIPSW_PIO_NIOS_HAS_TRI 0
#define DIPSW_PIO_NIOS_IRQ -1
#define DIPSW_PIO_NIOS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DIPSW_PIO_NIOS_IRQ_TYPE "NONE"
#define DIPSW_PIO_NIOS_NAME "/dev/dipsw_pio_nios"
#define DIPSW_PIO_NIOS_RESET_VALUE 0
#define DIPSW_PIO_NIOS_SPAN 16
#define DIPSW_PIO_NIOS_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK TIMER_0
#define ALT_TIMESTAMP_CLK none


/*
 * jtag_uart_0 configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart_0 altera_avalon_jtag_uart
#define JTAG_UART_0_BASE 0x20000
#define JTAG_UART_0_IRQ 0
#define JTAG_UART_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_0_NAME "/dev/jtag_uart_0"
#define JTAG_UART_0_READ_DEPTH 64
#define JTAG_UART_0_READ_THRESHOLD 8
#define JTAG_UART_0_SPAN 8
#define JTAG_UART_0_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_0_WRITE_DEPTH 64
#define JTAG_UART_0_WRITE_THRESHOLD 8


/*
 * led_pio_nios configuration
 *
 */

#define ALT_MODULE_CLASS_led_pio_nios altera_avalon_pio
#define LED_PIO_NIOS_BASE 0x20060
#define LED_PIO_NIOS_BIT_CLEARING_EDGE_REGISTER 0
#define LED_PIO_NIOS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define LED_PIO_NIOS_CAPTURE 0
#define LED_PIO_NIOS_DATA_WIDTH 7
#define LED_PIO_NIOS_DO_TEST_BENCH_WIRING 0
#define LED_PIO_NIOS_DRIVEN_SIM_VALUE 0
#define LED_PIO_NIOS_EDGE_TYPE "NONE"
#define LED_PIO_NIOS_FREQ 50000000
#define LED_PIO_NIOS_HAS_IN 0
#define LED_PIO_NIOS_HAS_OUT 1
#define LED_PIO_NIOS_HAS_TRI 0
#define LED_PIO_NIOS_IRQ -1
#define LED_PIO_NIOS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define LED_PIO_NIOS_IRQ_TYPE "NONE"
#define LED_PIO_NIOS_NAME "/dev/led_pio_nios"
#define LED_PIO_NIOS_RESET_VALUE 127
#define LED_PIO_NIOS_SPAN 16
#define LED_PIO_NIOS_TYPE "altera_avalon_pio"


/*
 * onchip_memory2_0 configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory2_0 altera_avalon_onchip_memory2
#define ONCHIP_MEMORY2_0_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY2_0_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY2_0_BASE 0x0
#define ONCHIP_MEMORY2_0_CONTENTS_INFO ""
#define ONCHIP_MEMORY2_0_DUAL_PORT 0
#define ONCHIP_MEMORY2_0_GUI_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY2_0_INIT_CONTENTS_FILE "soc_system_onchip_memory2_0"
#define ONCHIP_MEMORY2_0_INIT_MEM_CONTENT 1
#define ONCHIP_MEMORY2_0_INSTANCE_ID "NONE"
#define ONCHIP_MEMORY2_0_IRQ -1
#define ONCHIP_MEMORY2_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_MEMORY2_0_NAME "/dev/onchip_memory2_0"
#define ONCHIP_MEMORY2_0_NON_DEFAULT_INIT_FILE_ENABLED 0
#define ONCHIP_MEMORY2_0_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY2_0_READ_DURING_WRITE_MODE "DONT_CARE"
#define ONCHIP_MEMORY2_0_SINGLE_CLOCK_OP 0
#define ONCHIP_MEMORY2_0_SIZE_MULTIPLE 1
#define ONCHIP_MEMORY2_0_SIZE_VALUE 100000
#define ONCHIP_MEMORY2_0_SPAN 100000
#define ONCHIP_MEMORY2_0_TYPE "altera_avalon_onchip_memory2"
#define ONCHIP_MEMORY2_0_WRITABLE 1


/*
 * spi_nios configuration
 *
 */

#define ALT_MODULE_CLASS_spi_nios altera_avalon_spi
#define SPI_NIOS_BASE 0x20080
#define SPI_NIOS_CLOCKMULT 1
#define SPI_NIOS_CLOCKPHASE 0
#define SPI_NIOS_CLOCKPOLARITY 0
#define SPI_NIOS_CLOCKUNITS "Hz"
#define SPI_NIOS_DATABITS 32
#define SPI_NIOS_DATAWIDTH 32
#define SPI_NIOS_DELAYMULT "1.0E-9"
#define SPI_NIOS_DELAYUNITS "ns"
#define SPI_NIOS_EXTRADELAY 0
#define SPI_NIOS_INSERT_SYNC 0
#define SPI_NIOS_IRQ 2
#define SPI_NIOS_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SPI_NIOS_ISMASTER 0
#define SPI_NIOS_LSBFIRST 0
#define SPI_NIOS_NAME "/dev/spi_nios"
#define SPI_NIOS_NUMSLAVES 1
#define SPI_NIOS_PREFIX "spi_"
#define SPI_NIOS_SPAN 32
#define SPI_NIOS_SYNC_REG_DEPTH 2
#define SPI_NIOS_TARGETCLOCK 1000000u
#define SPI_NIOS_TARGETSSDELAY "0.0"
#define SPI_NIOS_TYPE "altera_avalon_spi"


/*
 * timer_0 configuration
 *
 */

#define ALT_MODULE_CLASS_timer_0 altera_avalon_timer
#define TIMER_0_ALWAYS_RUN 0
#define TIMER_0_BASE 0x20020
#define TIMER_0_COUNTER_SIZE 32
#define TIMER_0_FIXED_PERIOD 0
#define TIMER_0_FREQ 50000000
#define TIMER_0_IRQ 1
#define TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_0_LOAD_VALUE 49999
#define TIMER_0_MULT 0.001
#define TIMER_0_NAME "/dev/timer_0"
#define TIMER_0_PERIOD 1
#define TIMER_0_PERIOD_UNITS "ms"
#define TIMER_0_RESET_OUTPUT 0
#define TIMER_0_SNAPSHOT 1
#define TIMER_0_SPAN 32
#define TIMER_0_TICKS_PER_SEC 1000
#define TIMER_0_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_0_TYPE "altera_avalon_timer"

#endif /* __SYSTEM_H_ */