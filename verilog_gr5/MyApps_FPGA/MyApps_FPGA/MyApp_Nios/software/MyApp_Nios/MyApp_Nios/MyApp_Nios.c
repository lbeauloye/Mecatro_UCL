/*
 * "Hello World" example.
 *
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

typedef int bool;

#define TRUE    1
#define FALSE   0

int main()
{
    unsigned int value = 0;
    
    printf("Hello from MyApp_Nios ! \r\n");

    while(1) {
    	IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_NIOS_BASE, value++);
    	usleep(100000);
    }
}
