/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_40_CORTEXA9.c																	*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				Test suite for SPI																	*/
/*							Baud Rate: 115200														*/
/*							Data bits:      8														*/
/*							Stop bits:      1														*/
/*							Parity:      none														*/
/*							Flow Ctrl:   none														*/
/*																									*/
/*																									*/
/* Copyright (c) 2016-2019, Code-Time Technologies Inc. All rights reserved.						*/
/*																									*/
/* Code-Time Technologies retains all right, title, and interest in and to this work				*/
/*																									*/
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS							*/
/* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF										*/
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL							*/
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR								*/
/* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,							*/
/* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR							*/
/* OTHER DEALINGS IN THE SOFTWARE.																	*/
/*																									*/
/*																									*/
/*	$Revision: 1.27 $																				*/
/*	$Date: 2019/01/10 18:07:15 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */
/*																									*/
/* NOTES:																							*/
/*		As this is a demo, the RX FIFO watermark is set to a very conservative value to make sure	*/
/*		there couldn't be any FIFO overflow.  This value is definitively not providing good			*/
/*		 performance but this demo is about functionnality, not one about performance.				*/
/*			SPI_ISR_RX_THRS == 10% : set very low to give plenty of room to accumulate RX data		*/
/*		It is set-up in the makefile or DS5 GUI C preprocessor defintions							*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */
/*																									*/
/*		Tests #5 and #6 use a 22V10 PLD as a pseudo-random bit generator & verifier.				*/
/*		The JEDEC file for the PLD is provided in the src directory: spicnt.jed.					*/
/*		The PLD pin-out (DIP-24) is :																*/
/*			pin #1  : SPI clock																		*/
/*			pin #2  : SPI Chip Select																*/
/*			pin #3  : SPI MOSI (master out, 22V10 in)												*/
/*			pin #10 : GND																			*/
/*			pin #19 : Error (when sending to 22V10), high for each RXed bits that don't match		*/
/*			pin #23 : SPI MISO (master in, 22V10 out)												*/
/*			Pin #24 : VCC																			*/
/*			- All other pins can be left un-connected												*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if (((OS_DEMO) != 40) && ((OS_DEMO) != -40))
	#error "Wrong OS_DEMO value" 
#endif

#if defined(_STANDALONE_)						 	/* It is the same include file in all cases.	*/
  #include "mAbassi.h"									/* There is a substitution during the release 	*/
#elif defined(_UABASSI_)							/* This file is the same for Abassi, mAbassi,	*/
  #include "mAbassi.h"								/* and uAbassi, stamdalone so Code Time uses	*/
  #include "Xtra_uAbassi.h"
#elif defined (OS_N_CORE)							/* these checks to not have to keep 4 quasi		*/
  #include "mAbassi.h"								/* identical copies of this file				*/
#else
  #include "mAbassi.h"
#endif

#include "SysCall.h"								/* System call layer definitions				*/
#include "Platform.h"								/* Everything about the target platform is here	*/
#include "HWinfo.h"									/* Everything about the target board is here	*/

#if  ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)													\
 ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5))
 #if !defined(_STANDALONE_)
  #include "arm_acp.h"
 #endif
  #include "arm_pl330.h"
  #include "dw_spi.h"
  #include "dw_uart.h"
 #if ((OS_PLATFORM) == 0x0100AAC5)					/* DE0-nano / DE10-nano use GPIO 40 to select	*/
  #include "alt_gpio.h"								/* I2C or SPI 									*/
 #endif
#elif ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
  #include "arm_pl330.h"
  #include "cd_spi.h"
  #include "cd_uart.h"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define ALIGN_TO_USE		SPI_ALIGN_RIGHT

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer				*/

#define DMA_ATTRIB __attribute__ ((aligned (OX_CACHE_LSIZE)))

uint8_t  Buf8[1024*1024]   DMA_ATTRIB;
uint8_t  Buf8A[1024*1024]  DMA_ATTRIB;
uint8_t  Buf8B[1024*1024]  DMA_ATTRIB;
uint16_t Buf16[1024*1024]  DMA_ATTRIB;

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(void)
{
uint32_t ACPrd;
uint32_t ACPwrt;
int Align;
int BitsFrame;										/* Bits per frame								*/
int Burst;
int Count;
int CPha;
int CPol;
int Err;
int ii;
int jj;
int kk;
int mm;
int Frq;
int LeftAlign;										/* If non 8/16 bits are left or right aligned	*/
int Proto;
uint16_t *Ptr16;
int      QSPImodeEEPROM;
uint32_t ShfReg;
int      Test;
uint32_t Tmp32;

#if (((OS_PLATFORM) & 0x00FFFFFF) == 0x7020)
  int MaskByte0;
  int MaskByte1;
#endif


/* ------------------------------------------------ */
/* Set buffering here: tasks inherit main's stdios	*/
/* when Newlib reent is used & stdio are shared		*/ 

	setvbuf(stdin,  NULL, _IONBF, 0);				/* By default, Newlib library flush the I/O		*/
	setvbuf(stdout, NULL, _IONBF, 0);				/* buffer when full or when new-line			*/
	setvbuf(stderr, NULL, _IONBF, 0);				/* Done before OSstart() to have all tasks with	*/
													/* the same stdio set-up						*/
/* ------------------------------------------------ */
/* Start mAbassi									*/

	OSstart();										/* Start mAbassi								*/

	SysCallInit();									/* Initialize the System Call layer				*/

	OSintOn();										/* Enable the interrupts						*/

/* ------------------------------------------------ */
/* SysTick Set-up									*/

	OSisrInstall(OS_TICK_INT, &TIMtick);			/* Install the RTOS timer tick function			*/
	TIMERinit(((((OS_TICK_SRC_FREQ/OS_TICK_PRESCL)/100000)*(OX_TIMER_US))/(10)));
	GICenable(OS_TICK_INT, 128, 1);					/* Priority == mid / Edge trigger				*/

/* ------------------------------------------------ */
/* UART set-up										*/
/* 8 bits, No Parity, 1 Stop						*/

   #if (0U == ((UART_LIST_DEVICE) & (1U << (UART_DEV))))
	#error "Selected UART device is not in UART_LIST_DEVICE"
   #endif
													/* UART_Q_SIZE are defined in Platform.h		*/
													/* See ??_uart.h for the UART_FILT etc defs		*/
	uart_init(UART_DEV, BAUDRATE, 8, 0, 10, UART_Q_SIZE_RX, UART_Q_SIZE_TX, UART_FILT_OUT_LF_CRLF
	                                                                      | UART_FILT_IN_CR_LF
	                                                                      | UART_ECHO
	                                                                      | UART_ECHO_BS_EXPAND);

  #if (((UART_LIST_DEVICE) & 0x01U) != 0U)			/* Install the UART interrupt handler			*/
	OSisrInstall(UART0_INT, &UARTintHndl_0);
   #if ((OX_N_CORE) > 1)
	GICenable(-UART0_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #else											/* Int # -ve: the interrupt targets all cores	*/
	GICenable(UART0_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #endif
  #endif

  #if (((UART_LIST_DEVICE) & 0x02U) != 0U)			/* Install the UART interrupt handler			*/
	OSisrInstall(UART1_INT, &UARTintHndl_1);
   #if ((OX_N_CORE) > 1)
	GICenable(-UART1_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #else											/* Int # -ve: the interrupt targets all cores	*/
	GICenable(UART1_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #endif
  #endif

  #if (((UART_LIST_DEVICE) & 0x04U) != 0U)			/* Install the UART interrupt handler			*/
	OSisrInstall(UART2_INT, &UARTintHndl_2);
   #if ((OX_N_CORE) > 1)
	GICenable(-UART2_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #else											/* Int # -ve: the interrupt targets all cores	*/
	GICenable(UART2_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #endif
  #endif

  #if (((UART_LIST_DEVICE) & 0x08U) != 0U)			/* Install the UART interrupt handler			*/
	OSisrInstall(UART3_INT, &UARTintHndl_3);
   #if ((OX_N_CORE) > 1)
	GICenable(-UART3_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
  #else											/* Int # -ve: the interrupt targets all cores	*/
	GICenable(UART3_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
   #endif
  #endif

/* ------------------------------------------------ */
/* SPI set-up										*/

  #if (0U == ((SPI_LIST_DEVICE) & (1U << (SPI_DEV))))
	#error "Selected SPI device is not in SPI_LIST_DEVICE"
  #endif

   #if (((SPI_LIST_DEVICE) & 0x01U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI0_INT, &SPIintHndl_0);
	GICenable(SPI0_INT, 64, 0);						/* Must be level triggered and not edge			*/
   #endif											/* Prio set to 64, above timer tick & UART		*/

   #if (((SPI_LIST_DEVICE) & 0x02U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI1_INT, &SPIintHndl_1);
	GICenable(SPI1_INT, 64, 0);						/* Must be level triggered and not edge			*/
   #endif											/* Prio set to 64, above timer tick & UART		*/

   #if (((SPI_LIST_DEVICE) & 0x04U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI2_INT, &SPIintHndl_2);
	GICenable(SPI2_INT, 64, 0);						/* Must be level triggered and not edge			*/
   #endif											/* Prio set to 64, above timer tick & UART		*/

   #if (((SPI_LIST_DEVICE) & 0x08U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI3_INT, &SPIintHndl_3);
	GICenable(SPI3_INT, 64, 0);						/* Must be level triggered and not edge			*/
   #endif											/* Prio set to 64, above timer tick & UART		*/

/* ------------------------------------------------ */
/* DMA set-up										*/

  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)													\
   ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5))
	ii = dma_init(0);
	if (ii != 0) {
		printf("ERROR : dma_init() returned %d\n", ii);
	}

   #ifdef DMA0_INT
	OSisrInstall(DMA0_INT, &DMAintHndl_0);
	GICenable(DMA0_INT, 128, 0);
   #endif

   #ifdef DMA1_INT
	OSisrInstall(DMA1_INT, &DMAintHndl_1);
	GICenable(DMA1_INT, 128, 0);
   #endif

   #ifdef DMA2_INT
	OSisrInstall(DMA2_INT, &DMAintHndl_2);
	GICenable(DMA2_INT, 128, 0);
   #endif

   #ifdef DMA3_INT
	OSisrInstall(DMA3_INT, &DMAintHndl_3);
	GICenable(DMA3_INT, 128, 0);
   #endif

   #ifdef DMA4_INT
	OSisrInstall(DMA4_INT, &DMAintHndl_4);
	GICenable(DMA4_INT, 128, 0);
   #endif

   #ifdef DMA5_INT
	OSisrInstall(DMA5_INT, &DMAintHndl_5);
	GICenable(DMA5_INT, 128, 0);
   #endif

   #ifdef DMA6_INT
	OSisrInstall(DMA6_INT, &DMAintHndl_6);
	GICenable(DMA6_INT, 128, 0);
   #endif

   #ifdef DMA7_INT
	OSisrInstall(DMA7_INT, &DMAintHndl_7);
	GICenable(DMA7_INT, 128, 0);
   #endif

   #ifdef DMA8_INT
	OSisrInstall(DMA8_INT, &DMAintHndl_8);
	GICenable(DMA8_INT, 128, 0);
   #endif

   #ifdef DMA9_INT
	OSisrInstall(DMA9_INT, &DMAintHndl_9);
	GICenable(DMA9_INT, 128, 0);
   #endif

   #ifdef DMA10_INT
	OSisrInstall(DMA10_INT, &DMAintHndl_10);
	GICenable(DMA10_INT, 128, 0);
   #endif

   #ifdef DMA11_INT
	OSisrInstall(DMA11_INT, &DMAintHndl_11);
	GICenable(DMA11_INT, 128, 0);
   #endif

   #ifdef DMA12_INT
	OSisrInstall(DMA12_INT, &DMAintHndl_12);
	GICenable(DMA12_INT, 128, 0);
   #endif

   #ifdef DMA13_INT
	OSisrInstall(DMA13_INT, &DMAintHndl_13);
	GICenable(DMA13_INT, 128, 0);
   #endif

   #ifdef DMA14_INT
	OSisrInstall(DMA14_INT, &DMAintHndl_14);
	GICenable(DMA14_INT, 128, 0);
   #endif

   #ifdef DMA15_INT
	OSisrInstall(DMA15_INT, &DMAintHndl_15);
	GICenable(DMA15_INT, 128, 0);
   #endif
  #endif

/* ------------------------------------------------ */
/* ACP set-up										*/

  #if (defined(_STANDALONE_)																		\
   ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x007020)													\
   ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x007753))
	ACPrd  = 0xFFFFFFFF;
	ACPwrt = 0xFFFFFFFF;
  #else

	ACPwrt = acp_enable(-1, 0, 0, 0);				/* Page 0 (0x00000000->0x3FFFFFFF) is set-up	*/
	ACPrd  = acp_enable(-1, 0, 0, 1);				/* to use ACP for both read & write				*/

   #if (((SPI_OPERATION) & 0x00004) == 0)
	ACPrd = 0xFFFFFFFF;
   #endif
   #if (((SPI_OPERATION) & 0x00400) == 0)
	ACPwrt = 0xFFFFFFFF;
   #endif
  #endif

/* ------------------------------------------------ */
/* DE0-nano / DE10-nano:							*/
/* GPIO selects SPI / I2C on LT connector			*/

  #if ((OS_PLATFORM) == 0x0100AAC5)

	DE0_SELECT_LT_SPI();

  #endif

/* ------------------------------------------------ */
/* The application itself							*/ 

	printf("\nSPI test started\n\n");

/* ------------------------------------------------------------------------------------------------ */

#define TEST_QSPI			0
#define TEST_EEPROM_25LC512	1
#define TEST_EEPROM_93C86	2
#define	TEST_DAC_LTC2640	3
#define	TEST_ADC_LTC2452	4
#define TEST_RANDOM_READ	5
#define TEST_RANDOM_WRITE	6
#define TEST_MASTER_SEND	7
#define TEST_SLAVE_RECV		8
#define TEST_MASTER_RECV	9
#define TEST_SLAVE_SEND		10
  #if (((OS_PLATFORM) == 0x0000AAC5)					/* Cyclone V SocFPGA dev board			*/	\
   ||  ((OS_PLATFORM) == 0x0100AAC5))					/* DE0-nano (Atlas) / DE10-nano boards	*/
	puts("On this board, the SPI devices are on the LT connector\n");
  #endif

	do {
		puts("Test to run:");
		puts("0 - QSPI flash");
		puts("1 - 25LC512 EEPROM");
		puts("2 - 93C86   EEPROM");
		puts("3 - LTC2640 DAC");
		puts("4 - LTC2452 ADC");
		puts("5 - Custom board: random sequence read");
		puts("6 - Custom board: random sequence write");
  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
		puts("7 - Master-slave: master send");
		puts("8 - Master-slave: slave  receive");
		puts("9 - Master-slave: master receive");
		puts("A - Master-slave: slave  send");
		printf("\n Press key 0 to A : ");
		do {
			Test = GetKey();
		} while (Test == 0);
	} while (NULL == strchr("0123456789aA", Test));
  #else
		printf("\n Press key 0 to 6 : ");
		do {
			Test = GetKey();
		} while (Test == 0);
	} while (NULL == strchr("0123456", Test));
  #endif
	printf("\n");

	if ((Test == 'a')
	||  (Test == 'A')) {
		Test = 10;
	}
	else {
		Test -= '0';
	}

	QSPImodeEEPROM = 0;
	if ((Test == TEST_QSPI)
	||  (Test == TEST_EEPROM_25LC512)) {
		puts("0 - use non-EEPROM mode for the SPI");
		puts("1 - use EEPROM mode for the SPI");
		printf("\n    Press key 0 or 1 : ");
		do {
			QSPImodeEEPROM = GetKey();
		} while (QSPImodeEEPROM == 0);
		QSPImodeEEPROM -= '0';
	} while ((QSPImodeEEPROM != 0)
	  &&     (QSPImodeEEPROM != 1));

	printf("\n");

	BitsFrame = 8;
	LeftAlign = 0;

	if ((Test == TEST_RANDOM_READ)
	||  (Test == TEST_RANDOM_WRITE)) {
		printf("Number of bits per frame : ");
		BitsFrame = 0;
		do {
			ii = GetKey();
			if ((ii >= '0')
			&&  (ii <= '9')) {
				BitsFrame = (BitsFrame * 10)
				          + (ii - '0');
			}
		} while ((ii != '\r')
		  &&     (ii != '\n'));
	}

	if (Test == (TEST_QSPI)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 10 * 1000000;
		Proto = SPI_PROTO_SPI;
  	}
	else if (Test == (TEST_EEPROM_25LC512)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;						/* Should be CPOL0, but ZYNC SPI controller		*/
		Frq   = 1 * 1000000;						/* does funnies with CPOL0						*/
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_EEPROM_93C86)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1000000/10;
		Proto = SPI_PROTO_UWIRE;
	}
	else if (Test == (TEST_DAC_LTC2640)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_ADC_LTC2452)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL0;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if  (Test == (TEST_RANDOM_READ)) {
		CPha  = SPI_CLK_CPHA1;						/* Zynq TRM says this shouldn't work... It does	*/
		CPol  = SPI_CLK_CPOL0;
		Frq   = 12 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_RANDOM_WRITE)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 12 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_MASTER_SEND)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_SLAVE_RECV)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else if (Test == (TEST_MASTER_RECV)) {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}
	else {
		CPha  = SPI_CLK_CPHA1;
		CPol  = SPI_CLK_CPOL1;
		Frq   = 1 * 1000000;
		Proto = SPI_PROTO_SPI;
	}

													/* DMA can only be used with 8 or 16 bit frames	*/
	Err = spi_init(SPI_DEV, SPI_SLV, Frq, BitsFrame, Proto
	                                               | CPha
	                                               | CPol
	                                               | ALIGN_TO_USE
	                                               | (((QSPImodeEEPROM) != 0)
	                                                 ? SPI_TX_RX_EEPROM
	                                                 : 0)
	                                               | SPI_MASTER
 	                                               | SPI_XFER_DMA		/* Driver deal with non-N*8	*/
	                                               | SPI_XFER_ISR		/* ISR if DMA not avail		*/
	                                               | SPI_XFER_POLLING 	/* Polling if no ISR/DMA	*/
	                                               | SPI_EOT_ISR
	                                               | ((ACPwrt != 0xFFFFFFFF)
	                                                  ? SPI_CFG_CACHE_RX(1,0)
	                                                  : 0)
	                                               | ((ACPrd != 0xFFFFFFFF)
	                                                  ? SPI_CFG_CACHE_TX(1,0)
	                                                  : 0)
	                                       );

	if ((Test == (TEST_SLAVE_RECV))
	||  (Test == (TEST_SLAVE_SEND))) {
	  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)												\
	  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
		spi_set(SPI_DEV, SPI_SLV, SPI_TIMEOUT, 4*OS_TICK_PER_SEC);
	  #endif
	}

	if (Err != 0) {
		printf("Error initializing SPI [%d]\n", Err);
		for(;;);
	}

	putchar('\n');

/* ------------------------------------------------------------------------------------------------ */

  if (Test == TEST_QSPI) {
	puts("QSPI test\n");
	puts("The QSPI part must have these characteristics:\n");
	puts("     - single lane");
	puts("     - 3 byte address");
	puts("     - 0x02: write command (page program)");
	puts("             minimum page size of 256");
	puts("     - 0x03: read command ");
	puts("     - 0x06: write enable");
	puts("     - 0x20: sector erase command ");
	puts("             minimum sector size of 4096\n\n");
	puts("The test erases the first sector and program the 1st page.\n");
	puts("Then all these combinations of QSPI read are checked:\n");
	if (QSPImodeEEPROM != 0) {
		puts("   TX : 4 (4 to send op-code + address)");
		puts("   RX : 1 to 512 bytes");
	}
	else {
		puts("   TX : 5 to 512 bytes (4 sends op-code + address, rest are dummies)");
		puts("   RX : 5 to 516 bytes (4 initial are ignored, rest are true reads");
 	}

	Burst = 1024;

  #define N_FRAMES_RX	(Burst*8+BitsFrame-1)/(BitsFrame)

	if (BitsFrame == 16) {

		puts("It's not possible to re-program the QSPI flash when using 16 bit frames");
		puts("To get the correct data in the QSPI flash");
		puts("use 8 bit frames and re-program it\n");

		memset((void *)&Buf16[0], 0x55, sizeof(Buf16));

		Buf16[0] = 0x0300;
		Buf16[1] = 0x0000;

		ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf16[0], 2, (void *)&Buf16[0], Burst);
		if (ii != 0) {
			puts("\nError : overflow / underrun\n");
		}

		for(ii=0 ; ii<Burst ; ii++) {
			printf("%02X %02X", Buf16[ii] >> 8, 0xFF & Buf16[ii]);
			printf("%c", ((ii&7) == 7) ? '\n' : ' ');
		}
	}
	else if (BitsFrame == 8) {
		int BurstRX;
		int BurstTX;

		printf("\nDo we re-program the flash memory (y/n) ? ");
		do {
			ii = GetKey();
		} while ((ii != 'y')
		  &&     (ii != 'Y')
		  &&     (ii != 'n')
		  &&     (ii != 'N'));

		putchar('\n');
		putchar('\n');

		if ((ii == 'y')
		||  (ii == 'Y')) {
			puts("\n- Erasing the sector then waiting for 2 s.");

			Buf8[0] = 0x06;							/* Write enable									*/
			spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);

			Buf8[0] = 0x20;							/* Sector erase command							*/
			Buf8[1] = 0x00;							/* Address : 0x000000							*/
			Buf8[2] = 0x00;
			Buf8[3] = 0x00;
			spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 4);

			TSKsleep(2*OS_TICK_PER_SEC);			/* Wait long enough for the erase to occur		*/

			puts("- Programming page #0 then waiting for 2 s.\n");

			Buf8[0] = 0x06;							/* Write enable									*/
			spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);

			Buf8[0] = 0x02;							/* Page program command							*/
			Buf8[1] = 0x00;							/* Address : 0x000000							*/
			Buf8[2] = 0x00;
			Buf8[3] = 0x00;
			for (ii=0 ; ii<256 ; ii++) {
				Buf8[4+ii] = ii;
			}
			spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 256+4);

			TSKsleep(2*OS_TICK_PER_SEC);			/* Wait long enough for programming to be done	*/
		}

		memset((void *)&Buf8[0], 0, sizeof(Buf8));

		Buf8[0] = 0x03;								/* Read command									*/
		Buf8[1] = 0x00;								/* 3 byte addresses								*/
		Buf8[2] = 0x00;
		Buf8[3] = 0x00;

		kk = (QSPImodeEEPROM != 0)
		   ? 0
		   : 4;
		for (Count=0 ; Count<10 ; Count++) {
			for (BurstRX=1+kk ; BurstRX<=(512+kk) ; BurstRX++) {
				for (BurstTX=4 ; BurstTX<=512 ; BurstTX += (QSPImodeEEPROM != 0) ? 10000000 : 1) {
					if (QSPImodeEEPROM == 0) {
						printf("\r%d - Testing read of %3d bytes, TX length of %3d ",
						       Count, BurstRX, BurstTX);
					}
					else {
						printf("\r%d - Testing read of %3d bytes ", Count, BurstRX);
					}

					memset((void *)&Buf8A[0], 0xAA, 1024);

					ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0],  BurstTX,
					                                     (void *)&Buf8A[0], BurstRX);
					if (ii != 0) {
						printf("\n\n*******Error number from spi_send_recv() : #%d\n", ii);
						for(;;);
					}

					if  ((QSPImodeEEPROM) != 0) {
						jj = 0;
						for (ii=0 ; ii<BurstRX ; ii++) {
							if (((ii <  256) && (Buf8A[ii] != ii))
							||  ((ii >= 256) && (Buf8A[ii] != 0xFF))) {
								jj = 1;
							}
						}
						for ( ; ii<BurstRX+16 ; ii++) {
							if (Buf8A[ii] != 0xAA) {
								jj = 1;
							}
						}
					}
					else {
						jj = 0;
						for (ii=0 ; ii<BurstRX ; ii++) {
							if (ii < 4) {			/* Op-code + Address, need to ignore RX			*/
							}
							else if (ii < (256+4)) {/* Valid data, must be 0x00 -> 0xFF				*/
								if (Buf8A[ii] != (ii-4)) {
									jj = 1;
								}
							}
							else {
								if (Buf8A[ii] != 0xFF) {
									jj = 1;
								}
							}
						}

						for ( ; ii<(BurstRX+16) ; ii++) {
							if (Buf8A[ii] != 0xAA) {
								jj = 1;
							}
						}
					}
	
					if (jj != 0) {
						printf("\n\nError\n");
						for (jj=0 ; jj<(BurstRX+16) ; jj++) {
							if ((jj&15) == 0) {
								printf("\n0x%03X:", jj);
							}
							printf(" 0x%02X", Buf8A[jj]);
						}
						printf("\n");
						for(;;);
					}
					else {
						printf("OK     ");
					}
				}
			}
		}
	}
	else if (BitsFrame < 8) {
		memset((void *)&Buf8[0],  0x55, sizeof(Buf8));

		Tmp32 = 0x03000000;							/* Read cmd 0x03 & 3 byte address				*/
		jj    = 0;
		if ((ALIGN_TO_USE) == (SPI_ALIGN_LEFT)) {
			for (ii=0 ; ii<32 ; ii+=BitsFrame) {
				Buf8A[jj++] = Tmp32 >> 24;
				Tmp32     <<= BitsFrame;
			}
		}
		else {
			for (ii=0 ; ii<32 ; ii+=BitsFrame) {
				Buf8A[jj++] = Tmp32 >> (32-BitsFrame);
				Tmp32     <<= BitsFrame;
			}
		}

		ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], jj, (void *)&Buf8[0], N_FRAMES_RX);
		if (ii != 0) {
			puts("\nError : overflow / underrun\n");
		}

		Tmp32 = 0;
		jj    = 0;
		kk    = 0;
		if ((ALIGN_TO_USE) == (SPI_ALIGN_LEFT)) {
			for(ii=0 ; ii<N_FRAMES_RX ; ii++) {
				Tmp32 <<= BitsFrame;
				Tmp32  |= Buf8[ii] >> (8-BitsFrame);
				jj     += BitsFrame;
				if (jj >= 8) {
					jj -= 8;
					printf("%02X", (unsigned int)(0xFF & (Tmp32 >> jj)));
					printf("%c", ((kk&15) == 15) ? '\n' : ' ');
					kk++;
				}
			}
		}
		else {
			for(ii=0 ; ii<N_FRAMES_RX ; ii++) {
				Tmp32 <<= BitsFrame;
				Tmp32  |= Buf8[ii] & ((1 << BitsFrame) -1);
				jj     += BitsFrame;
				if (jj >= 8) {
					jj -= 8;
					printf("%02X", (unsigned int)(0xFF & Tmp32));
					printf("%c", ((kk&15) == 15) ? '\n' : ' ');
					kk++;
				}
			}
		}
	}
	else {
		memset((void *)&Buf16[0],  0x00, sizeof(Buf8));

		Tmp32 = 0x03000000;							/* Read cmd 0x03 & 3 byte address				*/
		jj    = 0;
		if ((ALIGN_TO_USE) == (SPI_ALIGN_LEFT)) {
			for (ii=0 ; ii<32 ; ii+=BitsFrame) {
				Buf16[jj++] = Tmp32 >> 16;
				Tmp32     <<= BitsFrame;
			}
		}
		else {
			for (ii=0 ; ii<32 ; ii+=BitsFrame) {
				Buf16[jj++] = Tmp32 >> (32-BitsFrame);
				Tmp32     <<= BitsFrame;
			}
		}

		ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf16[0], jj, (void *)&Buf16[0], N_FRAMES_RX);
		if (ii != 0) {
			puts("\nError : overflow / underrun\n");
		}

		Tmp32 = 0;
		jj    = 0;
		kk    = 0;
		if ((ALIGN_TO_USE) == (SPI_ALIGN_LEFT)) {
			for(ii=0 ; ii<N_FRAMES_RX ; ii++) {
				Tmp32 <<= BitsFrame;
				Tmp32  |= Buf16[ii] >> (16-BitsFrame);
				jj     += BitsFrame;
				if (jj >= 16) {
					jj -= 16;
					printf("%02X %02X", (unsigned int)(0xFF & (Tmp32 >> (jj+8))),
					                    (unsigned int)(0xFF & (Tmp32 >> jj)));
					printf("%c", ((kk&7) == 7) ? '\n' : ' ');
					kk++;
				}
			}
		}
		else {
			for(ii=0 ; ii<N_FRAMES_RX ; ii++) {
				Tmp32 <<= BitsFrame;
				Tmp32  |= Buf16[ii] & ((1 << BitsFrame) -1);
				jj     += BitsFrame;
				if (jj >= 16) {
					jj -= 16;
					printf("%02X %02X", (unsigned int)(0xFF & (Tmp32 >> (jj+8))),
					                    (unsigned int)(0xFF & (Tmp32 >> jj)));
					printf("%c", ((kk&7) == 7) ? '\n' : ' ');
					kk++;
				}
			}
		}
	}

 	puts("\nTest Done");

	for(;;);
  }

/* ------------------------------------------------------------------------------------------------ */

  else if (Test == (TEST_EEPROM_25LC512)) {

	Buf8A[0] = 0x05;								/* Read status register command					*/
	Buf8[0]  = 0x55;
	ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 1, (void *)&Buf8[0], 1);
	if (ii != 0) {
		printf("\n[1] Error : overflow / underrun %d\n", ii);
	}

	puts("Erasing & waiting for done");

	Buf8A[0] = 0x06;								/* Write enable command							*/
	ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 1);
	if (ii != 0) {
		printf("\n[2] Error : overflow / underrun %d\n", ii);
	}

	Buf8A[0] = 0x42;								/* Page erase command							*/
	Buf8A[1] = 0x00;								/* 2 byte address								*/
	Buf8A[2] = 0x00;
	ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 3);
													/* Wait for erase to be done					*/
	jj = (QSPImodeEEPROM == 0)						/* Is using EEPROM mode or not					*/
	   ? 1											/* When non-EEPROM, read 2 bytes: opcode + data	*/
	   : 0;											/* When EEPROM, it skips op-code, read 1 bytes	*/

	Buf8A[0] = 0x05;								/* Read status register command					*/
	kk       = 0;
	do {
		Buf8[0] = 0xAA;
		ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 1, (void *)&Buf8[0], 1 + jj);
		if (ii != 0) {
			printf("\n[3] Error : overflow / underrun %d\n", ii);
		}
	} while((0 != (Buf8[jj] & 0x01))
	  &&    (++kk < 10000));

	if (kk >= 10000) {
		printf("\nError : did not get status OK after 10000 tries\n");
		for(;;);		
	}
	puts("checking if EEPROM has been erased");

	jj *= 3;
	memset((void *)&Buf8[0], 0x55, sizeof(Buf8));
	Buf8A[0] = 0x03;								/* Read command									*/
	Buf8A[1] = 0x00;								/* 2 byte addresses								*/
	Buf8A[2] = 0x00;
	ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 3, (void *)&Buf8[0], 256+jj);
	if (ii != 0) {
		printf("\n[4] Error : overflow / underrun %d\n", ii);
	}

	Err = 0;
	for(ii=jj ; ii<(128+jj) ; ii++) {
		if (Buf8[ii] != 0xFF) {
			printf("\nError : erase failed at address 0x%04X\n", ii-jj);
			Err = 1;
			break;
		}
	}

	if (Err == 0) {
		printf("Page erase successful\n");
	}

	Buf8A[0] = 0x06;								/* Write enable command							*/
	ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 1);
	if (ii != 0) {
		printf("\n[5] Error : overflow / underrun %d\n", ii);
	}

	Buf8A[0] = 0x02;								/* Write command								*/
	Buf8A[1] = 0x00;								/* 2 byte address								*/
	Buf8A[2] = 0x00;

	for(ii=0 ; ii<128 ; ii++) {						/* 25LC512 max write burst is 128 bytes			*/
		Buf8A[ii+3] = ii;							/* Does not require an erase before write		*/
	}

	ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 128+3);
	if (ii != 0) {
		printf("\n[6] Error : overflow / underrun %d\n", ii);
	}

	jj /= 3;										/* Wait for write to be done					*/
	Buf8A[0] = 0x05;								/* Read status register command					*/
	do {
		ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 1, (void *)&Buf8[0], 1+jj);
		if (ii != 0) {
			printf("\n[7] Error : overflow / underrun %d\n", ii);
		}
	} while(0 != (Buf8[jj] & 0x01));

	memset((void *)&Buf8[0],  0x55, sizeof(Buf8));
	memset((void *)&Buf8A[0], 0x55, sizeof(Buf8A));

	jj *= 3;

	Buf8A[0] = 0x03;								/* Read command									*/
	Buf8A[1] = 0x00;								/* 2 byte addresses								*/
	Buf8A[2] = 0x00;
	ii = spi_send_recv(SPI_DEV, SPI_SLV, (void *)&Buf8A[0], 3, (void *)&Buf8[0], 256+jj);
	if (ii != 0) {
		printf("\n[8] Error : overflow / underrun %d\n", ii);
	}

	Err = 0;
	for(ii=jj ; ii<(128+jj) ; ii++) {
		if (Buf8[ii] != ii-jj) {
			printf("\nError : write failed at address 0x%04X\n", ii-jj);
			printf("        Expected 0x%02X, got 0x%02X\n", ii-jj, Buf8[ii]);
			Err = 1;
			break;
		}
		printf("%02X", Buf8[ii]);
		printf("%c", (((ii-jj)&15) == 15) ? '\n' : ' ');
	}

	if (Err == 0) {
		printf("Page write successful\n");
	}

 	puts("Test Done");

	for(;;);
  }

/* ------------------------------------------------------------------------------------------------ */

  else if (Test == (TEST_EEPROM_93C86)) {

   #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)												\
   ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
	puts("The ZYNQ controller does not support SST flash memories");
	for(;;);
   #endif

	printf("\nErasing the memory\n");

	Buf8[0] = 14;									/* 14 bits in the WEN control word				*/
	Buf8[1] = 0;									/* Don't care									*/
	Buf8[2] = 0x26;									/* Write enable op-code 0x2600					*/
	Buf8[3] = 0x00;
	ii      = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 0);
	if (ii != 0) {
		puts("\nError : overflow / underrun\n");
	}

	Buf8[0] = 14;									/* 14 bits in the ERAL control word				*/
	Buf8[1] = 0;									/* Don't care									*/
	Buf8[2] = 0x24;									/* Erase all memory op-code 0x2400				*/
	Buf8[3] = 0x00;
	ii      = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 0);
	if (ii != 0) {
		puts("\nError : overflow / underrun\n");
	}

	TSKsleep(OS_MS_TO_TICK(100));

	jj = 0;
	printf("Checking if memory is blank\n");
	for (ii=0x3000 ; ii<0x3100 ; ii++) {			/* 1st read addresses 0x00-> 0x7F				*/
		Buf8[0] = 14;								/* 14 bits in the control word					*/
		Buf8[1] = 0;								/* Don't care									*/
		Buf8[2] = ii >> 8; 							/* CtrlWord is 11 0AAA AAAA AAAA				*/
		Buf8[3] = ii & 0xFF;
		spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);
		if (Buf8[0] != 0xFF) {
			printf("Did not erase properly\n");
			jj=1;
			break;
		}
	}

	if (jj == 0) {
		printf("Memory is blank\n");
		printf("Writing 0x00 -> 0x7F in first 128 bytes\n");
		for (ii=0 ; ii<0x80 ; ii++) {				/* 1st write 0x00-> 0x7F						*/
			Buf8[0] = 14;							/* 14 bits in the control word					*/
			Buf8[1] = 0;							/* Don't care									*/
			Buf8[2] = (0x2800 | ii) >> 8;			/* MSB of the control word						*/
			Buf8[3] = (0x2800 | ii) & 0xFF;			/* LSB of the control word						*/
			Buf8[4] = ii & 0xFF;	 				/* CtrlWord is 10 1AAA AAAA AAAA				*/
			spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);
			TSKsleep(2);
		}
		printf("Checking write\n");
	}
	else {
		printf("Contents after erase failure\n");
	}

	Buf8[0] = 14;									/* 14 bits in the WDS control word				*/
	Buf8[1] = 0x00;									/* Don't care									*/
	Buf8[2] = 0x20;									/* Write disable op-code 0x2000					*/
	Buf8[3] = 0x00;
	for (ii=0x3000 ; ii<0x3100 ; ii++) {			/* 1st read addresses 0x00-> 0x7F				*/
		Buf8[0] = 14;								/* 14 bits in the control word					*/
		Buf8[1] = 0;								/* Don't care									*/
		Buf8[2] = ii >> 8; 							/* CtrlWord is 11 0AAA AAAA AAAA				*/
		Buf8[3] = ii & 0xFF;
		spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);
		printf("%02X ", Buf8[0]);
		if ((ii & 0xF) == 0xF) {
			printf("\n");
		}
	}

	printf("\nRedoing: read all 256 in one transfer\n");
	Buf8[0] = 14;								/* 14 bits in the control word					*/
	Buf8[1] = 0;								/* Don't care									*/
	Buf8[2] = 0x30; 							/* CtrlWord is 11 0AAA AAAA AAAA				*/
	Buf8[3] = 0x00;
	spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 256);
	for (ii=0 ; ii<256 ; ii++) {
		printf("%02X ", Buf8[ii]);
		if ((ii & 0xF) == 0xF) {
		printf("\n");
		}
	}


 	puts("Test Done");

	for(;;);
  }

/* ------------------------------------------------------------------------------------------------ */

  else if  (Test == (TEST_DAC_LTC2640)) {
													/* LT2640 reads only the last 3 bytes			*/
	Burst = 3;										/* Change BURST to send more than 3 bytes		*/
	jj    = 0;										/* jj is use for the DAC value					*/

	Buf8[0] = 0x40;									/* Do a power down, as it reset the operations	*/
	spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], 1);

	for(;;) {
		memset((void *)&Buf8, 0x11, Burst);
		Buf8[Burst-3] = 0x30;						/* Command : Write and update					*/
		Buf8[Burst-2] = jj >> 8;
		Buf8[Burst-1] = jj;

		ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], Burst);
		if (ii != 0) {
			printf("\nError #%d / %d bytes\n", ii, Burst);
		}

		jj += 0x10;									/* It's a 12 bit DAC, 4 LSBits are don't care	*/
		Burst++;									/* therefore increment by 16					*/
		if ((jj & 0xFFFF0000) != 0) {				/* 12 bit wrap around, restart					*/
			Burst = 3;
			jj    = 0;
		}
	}
  }

/* ------------------------------------------------------------------------------------------------ */

  else if (Test == (TEST_ADC_LTC2452)) {

	Burst = 16;										/* ADC data are in the first 2 bytes			*/

	for(;;) {										/* Set to >2 for larger buffer read				*/
	  int Data;

		memset((void *)&Buf8[0], 0xFF, sizeof(Buf8));
		Buf8[0] = 0;
		Buf8[1] = 0;

		ii = spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0], Burst);
		if (ii != 0) {
			puts("\nError : overflow / underrun\n");
		}

		Data = (((int)Buf8[0]) << 8)
		     | Buf8[1];
		printf("Length:%3d   Data:%5d [0x%02X 0x%02X 0x%02X 0x%02X ... 0x%02X]     \r",
		        Burst, Data, Buf8[0], Buf8[1], Buf8[2], Buf8[3], Buf8[Burst-1]);

		Burst += 16;
		if (Burst > 2048) {
			Burst = 16;
		}

		TSKsleep(OS_MS_TO_TICK(20));				/* Maximum conversion rate is 60Hz (16.666ms)	*/
	}

  }

/* ------------------------------------------------------------------------------------------------ */

  else if (Test == (TEST_RANDOM_READ)) {

	Ptr16  = (void *)&Buf8A[0];
	ShfReg = 0x001;									/* Prepare the sequence to compare to			*/
	for (ii=0 ; ii<32768 ; ii++) {
		Tmp32 = 0;									/* Init the next frame to read					*/
		for (jj=0 ; jj<BitsFrame ; jj++) {
			Tmp32   = (Tmp32 << 1)					/* This is what is implemented in the 22V10		*/
			        | (ShfReg & 1);					/* Bit to insert when shifting left				*/
			ShfReg  = (ShfReg << 1)					/* Next bit to insert: Bit#8 XOR bit#3			*/
			        | (1 & ((ShfReg >> 8) + (ShfReg >> 3)));
		}
		if (BitsFrame <= 8) {						/* Write 8 bit or 16 bit (frame size)			*/
			if (LeftAlign == 0) {
				Buf8A[ii] = Tmp32;			
			}
			else {
				Buf8A[ii] = Tmp32 << (8-BitsFrame);
			}
		}
		else {
			if (LeftAlign == 0) {
				Ptr16[ii] = Tmp32;
			}
			else {
				Ptr16[ii] = Tmp32 << (16-BitsFrame);
			}
		}
	}

  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
	if (BitsFrame <= 8) {
		MaskByte0 = (1 << BitsFrame) - 1;
		MaskByte1 = MaskByte0;
	}
	else {
		MaskByte1 = (1 << (BitsFrame-8)) - 1;
		MaskByte0 = 0xFF;
	}
  #endif

	for (Count=0 ;; Count++) {
		for (Burst=1 ; Burst<=32768 ; Burst++) {	/* Check lots of burst sizes					*/
		  #if !defined(_STANDALONE_)				/* In case cache not enabled in standalone		*/
			for (Align=0 ; Align<((BitsFrame <= 8) ? 1 : 8) ; Align++) {	/* Check alignments		*/
				if (BitsFrame <= 8) {
					printf("\r#%d - Testing read of %5d frames", Count, Burst);
				}
				else {
					printf("\r#%d - Testing read of %5d frames / memory aligned on %1d",
					        Count, Burst, Align);
				}
		  #else
			{
				Align = 0;
				printf("\r#%d - Testing read of %5d frames", Count, Burst);
		  #endif
				memset((void *)&Buf8, 0xAA, ((2*Burst)+256));

				if (((Burst % 7) == 0)				/* Use ACP once in a while						*/
				&&  (ACPwrt != 0xFFFFFFFF)) {
					ii = spi_recv(SPI_DEV, SPI_SLV, (void *)(ACPwrt|(uintptr_t)&Buf8[Align]), Burst);
				}
				else {
					ii = spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[Align], Burst);
				}

				if (ii != 0) {
					printf("\nSPI driver reported error #%d\n", ii);
					printf("Test aborted\n");
					for(;;);
				}

				kk = (BitsFrame <= 8)
				   ? 1
				   : 2;

			  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)										\
			  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
				jj = 0;								/* For the comparison, must remove extra bits	*/
				for (ii=Align ; ii<((Burst*kk)+Align) ; ii++) {	/* when not 8 or 16 bits			*/
					Buf8[ii] &= (1 & jj++)
					          ? MaskByte1
					          : MaskByte0;
				}
			  #endif

				jj = 0;
				for(ii=0 ; ii<Align ; ii++) {		/* Check the area before the RX data			*/
					if (Buf8[ii] != 0xAA) {
						puts("\nMemory before the write area isn't 0xAA"); 
						jj = 1;
						break;
					}
				}
													/* Check all matching data						*/
				if ((0 !=  memcmp(&Buf8[ii], &Buf8A[0], Burst*kk))
				&& (jj ==0)) {
					puts("\nData mismatch");
					jj = 1;
				}

				ii += kk*Burst;
													/* Check read don't spill						*/
			  #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)										\
			  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
				if (((Burst*BitsFrame) & 7) != 0) {
					ii++;
				}
			  #endif

				for ( ; ((ii<((2*Burst)+256)) && (jj==0)) ; ii++) {
					if (Buf8[ii] != 0xAA) {			/* Check the area past the RX data				*/
						puts("\nMemory after the write area isn't 0xAA"); 						
						jj = 1;
						break;
					}
				}

				if (jj != 0) {						/* jj != 0 is a mismatch						*/
				  #if 1
					for (jj=0 ; jj<(kk*Burst+16) ; jj++) {
						if ((jj&15) == 0) {
							printf("\n0x%03X:", jj);
						}
						if (jj < (kk*Burst)) {
							printf("%c0x%02X", (Buf8[jj+Align]==Buf8A[jj])?' ':'*', Buf8[jj+Align]);
						}
						else {
							printf("%c0x%02X", (Buf8[jj+Align]==0xAA)?' ':'*', Buf8[jj+Align]);
						}
					}
				  #endif
					for(;;);
				}
			}
		}
	}
  }

/* ------------------------------------------------------------------------------------------------ */

  else if (Test == (TEST_RANDOM_WRITE)) {

	Ptr16  = (void *)&Buf8A[0];						/* Prepare the sequence to compare to			*/
	ShfReg = 0x001;


	for (ii=0 ; ii<=65536 ; ii++) {					/* Need to be 1 bit in advance vs read			*/
		Tmp32 = 0;									/* Init the next frame to read					*/
		for (jj=0 ; jj<BitsFrame ; jj++) {
			Tmp32   = (Tmp32 << 1)					/* This is what is implemented in the 22V10		*/
			        | (ShfReg & 1);					/* Bit to insert when shifting left				*/
			ShfReg  = (ShfReg << 1)					/* Next bit to insert: Bit#8 XOR bit#3			*/
			        | (1 & ((ShfReg >> 8) + (ShfReg >> 3)));
		}
		if (BitsFrame <= 8) {						/* Write 8 bit or 16 bit (frame size)			*/
			if (LeftAlign == 0) {
				Buf8A[ii] = Tmp32;			
			}
			else {
				Buf8A[ii] = Tmp32 << (8-BitsFrame);
			}
		}
		else {
			if (LeftAlign == 0) {
				Ptr16[ii] = Tmp32;
			}
			else {
				Ptr16[ii] = Tmp32 << (16-BitsFrame);
			}
		}
	}

	kk = (BitsFrame <= 8)
	   ? 1											/* No need to check for memory align on bytes	*/
	   : 8;											/* Try 8 possible memory alignment for 2 bytes	*/

	for (Count=0 ;; Count++) {
		for (Burst=1; Burst<=65536 ; Burst++) {		/* Check lots of burst sizes					*/
		  #if !defined(_STANDALONE_)				/* In case cache not enabled in standalone		*/
			for (Align=0 ; Align<kk ; Align++) {	/* Check memory address alignments				*/
				memmove(&Buf8[Align], &Buf8A[0], (2*Burst)+2);
													/* +2: Zynq needs exact multiple of 8/16 bits	*/
				if (BitsFrame <= 8) {
					printf("\r#%d - Testing write of %5d frames", Count, Burst);
				}
				else {
					printf("\r#%d - Testing write of %5d frames / memory aligned on %1d",
					        Count, Burst, Align);
				}
		  #else
			{
				Align = 0;
				memmove(&Buf8[Align], &Buf8A[0], (2*Burst)+2);
				printf("\r#%d - Testing write of %5d frames", Count, Burst);
		  #endif

				if (((Burst %7) == 0)				/* Use ACP once in a while						*/
				&&  (ACPrd != 0xFFFFFFFF)) {
					ii = spi_send(SPI_DEV, SPI_SLV, (void *)(ACPrd|(uintptr_t)&Buf8[Align]), Burst);
				}
				else {
					ii = spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[Align], Burst);
				}

				if (ii != 0) {
					printf("\nSPI driver reported error #%d\n", ii);
					printf("Test aborted\n");
					for(;;);
				}

				TSKsleep(OS_MS_TO_TICK(50));		/* Needed when using scope with infinite 		*/
			}										/* persistence									*/
		}
	}
  }

/* ------------------------------------------------------------------------------------------------ */

  else if ((Test == (TEST_MASTER_SEND))
       ||  (Test == (TEST_SLAVE_SEND ))) {

	printf("Burst size? : "); 
	scanf ("%d", &Burst);

	for (ii=0 ; ; ii++) {
		printf("Sending %d bytes starting with %02X\r", Burst, ii&0xFF);
		kk = ii & 0xFF;
		mm = 1;
		for (jj=0 ; jj< Burst ; jj++) {
			Buf8[jj] = kk;
			kk += mm;
			if (kk == 0x100) {
				mm = -1;
				kk = 0xFF;
			}
			if (kk == -1) {
				mm = 1;
				kk = 0x00;
			}
		}

		spi_send(SPI_DEV, SPI_SLV, (void *)&Buf8[0], Burst);

		if (Test == (TEST_MASTER_SEND)) {			/* Pacing of the exchanges						*/
			TSKsleep(OS_TICK_PER_SEC);
		}
	}
  }
/* ------------------------------------------------------------------------------------------------ */

  else if ((Test == (TEST_SLAVE_RECV ))
       ||  (Test == (TEST_MASTER_RECV))) {

	printf("Burst size? : "); 
	scanf ("%d", &Burst);

	for(;;) {
		spi_recv(SPI_DEV, SPI_SLV, (void *)&Buf8[0], Burst);

		for (ii=0 ; ii<Burst ; ii++) {
			if ((ii & 0xF) == 0) {
				putchar('\n');
			}
			printf("%02X ", Buf8[ii]);
		}

		putchar('\n');

		if (Test == (TEST_MASTER_RECV)) {			/* Pacing of the exchanges						*/
			TSKsleep(OS_TICK_PER_SEC);
		}
	}
  }

/* ------------------------------------------------------------------------------------------------ */

	TSKsleep(OS_TICK_PER_SEC);							/* To givwe time to UART to finish if iSRs	*/

	return(0);
}

/* ------------------------------------------------------------------------------------------------ */
/* Not used but required because of the the generic OS_DEMO value used								*/

void TIMcallBack(void)
{
	return;
}

/* ------------------------------------------------------------------------------------------------ */
/* Needed for the interrupt call-back of GPIOs when target platform rerquites it					*/
/* ------------------------------------------------------------------------------------------------ */

void GPIOintHndl(int IOPin)
{
	return;
}

/* EOF */
