/*
 * "Hello World" example.
 *#include "alt_types.h"#include "sys/alt_stdio.h"#include "io.h"#include "system.h"#include "sys/alt_cache.h"#include "altera_avalon_spi.h"#include "altera_avalon_spi_regs.h"#include "sys/alt_irq.h"
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <stdlib.h> // malloc, free
#include <string.h>
#include <stddef.h>
#include <unistd.h>  // usleep (unix standard?)
#include "io.h"
#include "alt_types.h"  // alt_u32
#include <sys/alt_cache.h>
#include "system.h"
#include <fcntl.h>
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_spi_regs.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"


typedef int bool;

#define TRUE    1
#define FALSE   0

static void spi_rx_isr(void* isr_context){

        alt_printf("ISR :) %x \n" ,  IORD_ALTERA_AVALON_SPI_RXDATA(SPI_NIOS_BASE));

        //This resets the IRQ flag. Otherwise the IRQ will continuously run.
        IOWR_ALTERA_AVALON_SPI_STATUS(SPI_NIOS_BASE, 0x0);
}


int main()
{
    unsigned int value = 0;
    alt_u8 val;
    char spi_command_string_tx[10] = "$HELLOABC*";

      char spi_command_string_rx[10] = "$HELLOABC*";

    
    printf("Hello from MyApp_Nios ! \r\n");
    printf("SPI irq: %x\r\n",SPI_NIOS_IRQ);
  int ret = alt_ic_isr_register(SPI_NIOS_IRQ_INTERRUPT_CONTROLLER_ID,SPI_NIOS_IRQ,spi_rx_isr,(void *)spi_command_string_tx,0x0);
    IOWR_ALTERA_AVALON_SPI_CONTROL(SPI_NIOS_BASE,ALTERA_AVALON_SPI_CONTROL_SSO_MSK | ALTERA_AVALON_SPI_CONTROL_IRRDY_MSK);
  alt_printf("IRQ register return %x \n", ret);

    //Just calling the ISR to see if the function is OK.
    spi_rx_isr(NULL);

    while(1) {
      IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_NIOS_BASE, value++);

    //You need to enable the IRQ in the IP core control register as well


      usleep(100000);
    }
}
