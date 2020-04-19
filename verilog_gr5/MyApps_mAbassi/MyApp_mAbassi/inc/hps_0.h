#ifndef _ALTERA_HPS_0_H_
#define _ALTERA_HPS_0_H_

/*
 * This file was automatically generated by the swinfo2header utility.
 * 
 * Created from SOPC Builder system 'soc_system' in
 * file './soc_system.sopcinfo'.
 */

/*
 * This file contains macros for module 'hps_0' and devices
 * connected to the following masters:
 *   h2f_axi_master
 *   h2f_lw_axi_master
 * 
 * Do not include this header file and another header file created for a
 * different module or master group at the same time.
 * Doing so may result in duplicate macro names.
 * Instead, use the system header file which has macros with unique names.
 */

/*
 * Macros for device 'sysid_qsys', class 'altera_avalon_sysid_qsys'
 * The macros are prefixed with 'SYSID_QSYS_'.
 * The prefix is the slave descriptor.
 */
#define SYSID_QSYS_COMPONENT_TYPE altera_avalon_sysid_qsys
#define SYSID_QSYS_COMPONENT_NAME sysid_qsys
#define SYSID_QSYS_BASE 0x1000
#define SYSID_QSYS_SPAN 8
#define SYSID_QSYS_END 0x1007
#define SYSID_QSYS_ID 2899645186
#define SYSID_QSYS_TIMESTAMP 1587308934

/*
 * Macros for device 'jtag_uart', class 'altera_avalon_jtag_uart'
 * The macros are prefixed with 'JTAG_UART_'.
 * The prefix is the slave descriptor.
 */
#define JTAG_UART_COMPONENT_TYPE altera_avalon_jtag_uart
#define JTAG_UART_COMPONENT_NAME jtag_uart
#define JTAG_UART_BASE 0x2000
#define JTAG_UART_SPAN 8
#define JTAG_UART_END 0x2007
#define JTAG_UART_IRQ 2
#define JTAG_UART_READ_DEPTH 64
#define JTAG_UART_READ_THRESHOLD 8
#define JTAG_UART_WRITE_DEPTH 64
#define JTAG_UART_WRITE_THRESHOLD 8

/*
 * Macros for device 'led_pio', class 'altera_avalon_pio'
 * The macros are prefixed with 'LED_PIO_'.
 * The prefix is the slave descriptor.
 */
#define LED_PIO_COMPONENT_TYPE altera_avalon_pio
#define LED_PIO_COMPONENT_NAME led_pio
#define LED_PIO_BASE 0x3000
#define LED_PIO_SPAN 16
#define LED_PIO_END 0x300f
#define LED_PIO_BIT_CLEARING_EDGE_REGISTER 0
#define LED_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define LED_PIO_CAPTURE 0
#define LED_PIO_DATA_WIDTH 7
#define LED_PIO_DO_TEST_BENCH_WIRING 0
#define LED_PIO_DRIVEN_SIM_VALUE 0
#define LED_PIO_EDGE_TYPE NONE
#define LED_PIO_FREQ 50000000
#define LED_PIO_HAS_IN 0
#define LED_PIO_HAS_OUT 1
#define LED_PIO_HAS_TRI 0
#define LED_PIO_IRQ_TYPE NONE
#define LED_PIO_RESET_VALUE 127

/*
 * Macros for device 'dipsw_pio', class 'altera_avalon_pio'
 * The macros are prefixed with 'DIPSW_PIO_'.
 * The prefix is the slave descriptor.
 */
#define DIPSW_PIO_COMPONENT_TYPE altera_avalon_pio
#define DIPSW_PIO_COMPONENT_NAME dipsw_pio
#define DIPSW_PIO_BASE 0x4000
#define DIPSW_PIO_SPAN 16
#define DIPSW_PIO_END 0x400f
#define DIPSW_PIO_IRQ 0
#define DIPSW_PIO_BIT_CLEARING_EDGE_REGISTER 1
#define DIPSW_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define DIPSW_PIO_CAPTURE 1
#define DIPSW_PIO_DATA_WIDTH 4
#define DIPSW_PIO_DO_TEST_BENCH_WIRING 0
#define DIPSW_PIO_DRIVEN_SIM_VALUE 0
#define DIPSW_PIO_EDGE_TYPE ANY
#define DIPSW_PIO_FREQ 50000000
#define DIPSW_PIO_HAS_IN 1
#define DIPSW_PIO_HAS_OUT 0
#define DIPSW_PIO_HAS_TRI 0
#define DIPSW_PIO_IRQ_TYPE EDGE
#define DIPSW_PIO_RESET_VALUE 0

/*
 * Macros for device 'button_pio', class 'altera_avalon_pio'
 * The macros are prefixed with 'BUTTON_PIO_'.
 * The prefix is the slave descriptor.
 */
#define BUTTON_PIO_COMPONENT_TYPE altera_avalon_pio
#define BUTTON_PIO_COMPONENT_NAME button_pio
#define BUTTON_PIO_BASE 0x5000
#define BUTTON_PIO_SPAN 16
#define BUTTON_PIO_END 0x500f
#define BUTTON_PIO_IRQ 1
#define BUTTON_PIO_BIT_CLEARING_EDGE_REGISTER 1
#define BUTTON_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define BUTTON_PIO_CAPTURE 1
#define BUTTON_PIO_DATA_WIDTH 2
#define BUTTON_PIO_DO_TEST_BENCH_WIRING 0
#define BUTTON_PIO_DRIVEN_SIM_VALUE 0
#define BUTTON_PIO_EDGE_TYPE FALLING
#define BUTTON_PIO_FREQ 50000000
#define BUTTON_PIO_HAS_IN 1
#define BUTTON_PIO_HAS_OUT 0
#define BUTTON_PIO_HAS_TRI 0
#define BUTTON_PIO_IRQ_TYPE EDGE
#define BUTTON_PIO_RESET_VALUE 0

/*
 * Macros for device 'spi_raspberrypi', class 'altera_avalon_spi'
 * The macros are prefixed with 'SPI_RASPBERRYPI_'.
 * The prefix is the slave descriptor.
 */
#define SPI_RASPBERRYPI_COMPONENT_TYPE altera_avalon_spi
#define SPI_RASPBERRYPI_COMPONENT_NAME spi_raspberrypi
#define SPI_RASPBERRYPI_BASE 0x6000
#define SPI_RASPBERRYPI_SPAN 32
#define SPI_RASPBERRYPI_END 0x601f
#define SPI_RASPBERRYPI_IRQ 3
#define SPI_RASPBERRYPI_CLOCKMULT 1
#define SPI_RASPBERRYPI_CLOCKPHASE 0
#define SPI_RASPBERRYPI_CLOCKPOLARITY 0
#define SPI_RASPBERRYPI_CLOCKUNITS "Hz"
#define SPI_RASPBERRYPI_DATABITS 32
#define SPI_RASPBERRYPI_DATAWIDTH 32
#define SPI_RASPBERRYPI_DELAYMULT "1.0E-9"
#define SPI_RASPBERRYPI_DELAYUNITS "ns"
#define SPI_RASPBERRYPI_EXTRADELAY 0
#define SPI_RASPBERRYPI_INSERT_SYNC 1
#define SPI_RASPBERRYPI_ISMASTER 0
#define SPI_RASPBERRYPI_LSBFIRST 0
#define SPI_RASPBERRYPI_NUMSLAVES 1
#define SPI_RASPBERRYPI_PREFIX "spi_"
#define SPI_RASPBERRYPI_SYNC_REG_DEPTH 2
#define SPI_RASPBERRYPI_TARGETCLOCK 128000
#define SPI_RASPBERRYPI_TARGETSSDELAY "0.0"

/*
 * Macros for device 'pio_0', class 'altera_avalon_pio'
 * The macros are prefixed with 'PIO_0_'.
 * The prefix is the slave descriptor.
 */
#define PIO_0_COMPONENT_TYPE altera_avalon_pio
#define PIO_0_COMPONENT_NAME pio_0
#define PIO_0_BASE 0x11000
#define PIO_0_SPAN 16
#define PIO_0_END 0x1100f
#define PIO_0_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_0_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_0_CAPTURE 0
#define PIO_0_DATA_WIDTH 32
#define PIO_0_DO_TEST_BENCH_WIRING 0
#define PIO_0_DRIVEN_SIM_VALUE 0
#define PIO_0_EDGE_TYPE NONE
#define PIO_0_FREQ 50000000
#define PIO_0_HAS_IN 1
#define PIO_0_HAS_OUT 0
#define PIO_0_HAS_TRI 0
#define PIO_0_IRQ_TYPE NONE
#define PIO_0_RESET_VALUE 0

/*
 * Macros for device 'pio_1', class 'altera_avalon_pio'
 * The macros are prefixed with 'PIO_1_'.
 * The prefix is the slave descriptor.
 */
#define PIO_1_COMPONENT_TYPE altera_avalon_pio
#define PIO_1_COMPONENT_NAME pio_1
#define PIO_1_BASE 0x12000
#define PIO_1_SPAN 16
#define PIO_1_END 0x1200f
#define PIO_1_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_1_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_1_CAPTURE 0
#define PIO_1_DATA_WIDTH 32
#define PIO_1_DO_TEST_BENCH_WIRING 0
#define PIO_1_DRIVEN_SIM_VALUE 0
#define PIO_1_EDGE_TYPE NONE
#define PIO_1_FREQ 50000000
#define PIO_1_HAS_IN 1
#define PIO_1_HAS_OUT 0
#define PIO_1_HAS_TRI 0
#define PIO_1_IRQ_TYPE NONE
#define PIO_1_RESET_VALUE 0

/*
 * Macros for device 'pio_2', class 'altera_avalon_pio'
 * The macros are prefixed with 'PIO_2_'.
 * The prefix is the slave descriptor.
 */
#define PIO_2_COMPONENT_TYPE altera_avalon_pio
#define PIO_2_COMPONENT_NAME pio_2
#define PIO_2_BASE 0x13000
#define PIO_2_SPAN 16
#define PIO_2_END 0x1300f
#define PIO_2_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_2_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_2_CAPTURE 0
#define PIO_2_DATA_WIDTH 32
#define PIO_2_DO_TEST_BENCH_WIRING 0
#define PIO_2_DRIVEN_SIM_VALUE 0
#define PIO_2_EDGE_TYPE NONE
#define PIO_2_FREQ 50000000
#define PIO_2_HAS_IN 1
#define PIO_2_HAS_OUT 0
#define PIO_2_HAS_TRI 0
#define PIO_2_IRQ_TYPE NONE
#define PIO_2_RESET_VALUE 0

/*
 * Macros for device 'pio_3', class 'altera_avalon_pio'
 * The macros are prefixed with 'PIO_3_'.
 * The prefix is the slave descriptor.
 */
#define PIO_3_COMPONENT_TYPE altera_avalon_pio
#define PIO_3_COMPONENT_NAME pio_3
#define PIO_3_BASE 0x14000
#define PIO_3_SPAN 16
#define PIO_3_END 0x1400f
#define PIO_3_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_3_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_3_CAPTURE 0
#define PIO_3_DATA_WIDTH 32
#define PIO_3_DO_TEST_BENCH_WIRING 0
#define PIO_3_DRIVEN_SIM_VALUE 0
#define PIO_3_EDGE_TYPE NONE
#define PIO_3_FREQ 50000000
#define PIO_3_HAS_IN 1
#define PIO_3_HAS_OUT 0
#define PIO_3_HAS_TRI 0
#define PIO_3_IRQ_TYPE NONE
#define PIO_3_RESET_VALUE 0

/*
 * Macros for device 'ILC', class 'interrupt_latency_counter'
 * The macros are prefixed with 'ILC_'.
 * The prefix is the slave descriptor.
 */
#define ILC_COMPONENT_TYPE interrupt_latency_counter
#define ILC_COMPONENT_NAME ILC
#define ILC_BASE 0x30000
#define ILC_SPAN 256
#define ILC_END 0x300ff


#endif /* _ALTERA_HPS_0_H_ */