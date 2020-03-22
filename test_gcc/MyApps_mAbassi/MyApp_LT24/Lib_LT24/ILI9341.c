/*
 * ILI9341.c
 *
 *  Created on: 2014-2-13
 *      Author: Administrator
 */

#include "alt_types.h"
#include "alt_video_display.h"
#include "stdio.h"
#include <unistd.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "geometry.h"
#include "AD7843.h"

extern volatile unsigned long *fpga_LT24_ADC_SPI_addr;
extern volatile unsigned long *fpga_LT24_ADC_IRQ_N_addr;
extern volatile unsigned long *fpga_LT24_ADC_BUSY_addr;

#define SPI_RXDATA  fpga_LT24_ADC_SPI_addr+0
#define SPI_TXDATA  fpga_LT24_ADC_SPI_addr+1
#define SPI_STATUS  fpga_LT24_ADC_SPI_addr+2
#define SPI_CONTROL fpga_LT24_ADC_SPI_addr+3

#define SPI_STATUS_TMT  0x20	// Transmitter shift-register empty
#define SPI_STATUS_TRDY 0x40	// Transmitter Ready
#define SPI_STATUS_RRDY 0x80	// Receiver Ready

void LT24_SPI_Init()
{
	// Clear the errors
	alt_write_word(SPI_STATUS, 0x0000);
	// Set SPI Control Bits to 0 - Disable all interrupts and SSO
	alt_write_word(SPI_CONTROL, 0x0000);

	printf("LT24 SPI Status  : %x \r\n", alt_read_word(SPI_STATUS));
	printf("LT24 SPI Control : %x \r\n", alt_read_word(SPI_CONTROL));
}

alt_u16 LT24_ADC_Read(alt_u8 ADC_Command)
{
	alt_u16 ADC_Result = 0;

	// Active CS_N : Set SS0 (Bit 10 of SPI Control Register)
	alt_write_word(SPI_CONTROL, 0x0400);

	// Send ADC_Command
	alt_write_word(SPI_TXDATA, ADC_Command);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_TRDY) == 0);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_RRDY) == 0);
	ADC_Result = alt_read_word(SPI_RXDATA);

	// Read Conversion Result (13 bits = 16 + 000)
	alt_write_word(SPI_TXDATA, 0x00);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_TRDY) == 0);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_RRDY) == 0);
	ADC_Result = alt_read_word(SPI_RXDATA);

	alt_write_word(SPI_TXDATA, 0x00);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_TRDY) == 0);
	while ((alt_read_word(SPI_STATUS) & SPI_STATUS_RRDY) == 0);
	ADC_Result = ((ADC_Result << 8) +  alt_read_word(SPI_RXDATA)) >> 3;

	// Release CS_N : Clear SS0 (Bit0 of SPI Control Register)
	alt_write_word(SPI_CONTROL, 0x0000);
	return (ADC_Result);
}

int LT24_IsPressed()
{
	if (alt_read_word(fpga_LT24_ADC_IRQ_N_addr) == 0) return 1;
	return 0;
}

void LT24_GetXY(POINT *thePoint)
{
	thePoint->x = LT24_ADC_Read(0x92);  // Start = 1, A210 = 001, Mode = 0, SER = 0, PD10 = 1 0
	thePoint->y = LT24_ADC_Read(0xD2);  // Start = 1, A210 = 101, Mode = 0, SER = 0, PD10 = 1 0

	// printf("Debug x=%d, y=%d \r\n", thePoint->x, thePoint->y);

	// Calibration
	// Max Output for the ADC is 3600 for x and 3650 for y

	thePoint->x = thePoint->x * SCREEN_WIDTH  / 3600;
	thePoint->y = thePoint->y * SCREEN_HEIGHT / 3650;
	thePoint->y = 240 - thePoint->y;

}





