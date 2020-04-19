/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_50_CORTEXA9.c																	*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				Test suite for DMA																	*/
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
/*	$Revision: 1.24 $																				*/
/*	$Date: 2019/01/10 18:07:15 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if (((OS_DEMO) != 50) && ((OS_DEMO) != -50))
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
  #include "cd_uart.h"
#else
  #error "This platform does not support the DMA Driver"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define BUF_SIZE (64*1024*1024)

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer				*/
													/* Override cache defintions: OS_MMU_EXTERN_DEF	*/
													/* Using larger memory area than default cache	*/
#if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00AAC5)														\
 ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x00AA10))
  const uint32_t G_MMUsharedTbl[]    = {0x1E000000, 0x02000000, 0};
  const uint32_t G_MMUnonCachedTbl[] = {0x01000000, 0xFF000000, 0};
  const uint32_t G_MMUprivateTbl[]   = {0,0 };
  const uint32_t G_MMUnonCprivTbl[]  = {0,0 };
#elif (((OS_PLATFORM) & 0x00FFFFFF) == 0x007020)
  const uint32_t G_MMUsharedTbl[]    = {0x40000000, 0x00000000, 0};
  const uint32_t G_MMUnonCachedTbl[] = {0x40000000, 0xC0000000, 0};
  const uint32_t G_MMUprivateTbl[]   = {0,0 };
  const uint32_t G_MMUnonCprivTbl[]  = {0,0 };
#endif

volatile uint8_t __attribute__((aligned (OX_CACHE_LSIZE))) G_Src[BUF_SIZE+512];
volatile uint8_t __attribute__((aligned (OX_CACHE_LSIZE))) G_Dst[BUF_SIZE+512];

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
uint32_t ACPmaskRd;
uint32_t ACPmaskWrt;
int Err;
int ErrType;
uint32_t OpMode[16];
int OpIdx;
int RetVal1=0;
int RetVal2=0;
int XferID1;
int XferID2;
int ii;
int OffDst;
int OffSrc;
int SeedData;
int SeedParam;
int Size;
int TestCnt;
int Tick;
int TimerUS; 
int UseACP;

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

	TimerUS = OS_TIMER_US;

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
/* DMA set-up										*/

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

/* ------------------------------------------------ */
/* Star of the application							*/

	puts("\n\nMemory to Memory Test");
	puts("\n   The DMA transfer time measurement is accurate");
	puts("   Some tests may look like taking a long time but this is due to:");
	puts("     - Filling the source memory with a random pattern");
	puts("     - Verifying the copying was done without errors");
	puts("   The transfer info is updated after the transfer & check\n");
  #if defined(_STANDALONE_)
   #if ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5)												\
   ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10))
	puts("\n------------------------------------------------------------------------");
	puts("     As the cache is not enable in the Standalone demos, it will take");
	puts("  a very very long time to generate & check, alike 30 s per iteration\n");
	puts("------------------------------------------------------------------------\n");
   #endif
  #endif

/* ------------------------------------------------ */
/* ACP set-up										*/

#if (defined(_STANDALONE_)																			\
 ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
 ||  (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
	ACPwrt = 0xFFFFFFFF;
	ACPrd  = 0xFFFFFFFF;

#else
	ACPwrt = acp_enable(-1, 0, 0, 0);				/* Page 0 (0x00000000->0x3FFFFFFF) is set-up	*/
	ACPrd  = acp_enable(-1, 0, 0, 1);				/* to use ACP for both read & write				*/


	if ((ACPrd != 0xFFFFFFFF) || (ACPwrt != 0xFFFFFFFF)) {
		puts("\nOn the right of the printout, these mean");
		puts("    -- : No ACP");
		if (ACPrd != 0xFFFFFFFF) {
			puts("    R- : ACP on read, no ACP on write");
		}
		if (ACPwrt != 0xFFFFFFFF) {
			puts("    -W : No ACP on read, ACP on write");
		}
		if ((ACPrd != 0xFFFFFFFF) && (ACPwrt != 0xFFFFFFFF)) {
			puts("    RW : ACP on read and write\n");
		}
	}
#endif

	SeedData  = 0;
	SeedParam = 0;
	TestCnt   = 0;
	for(;;) {
		do {										/* Random values for:							*/
			SeedParam *= 1664525;					/*		Number of bytes to copy					*/
			SeedParam += 1013904223;				/*		Input buffer memory alignment			*/
			OffDst = (SeedParam   ) & 0x1F;			/*		Output buffer memory alignment			*/
			OffSrc = (SeedParam>>3) & 0x1F;
			Size   = (SeedParam>>5) & ((BUF_SIZE/2)-1);
		} while(Size < 256);

		UseACP = ((OffSrc + OffDst) & 7);			/* Select if using ACP on read and/or write		*/
		if (UseACP > 3) {
			UseACP = 0;
		}
		if (ACPrd == 0xFFFFFFFF) {					/* If ACP I/F not available, no ACP				*/
			UseACP &= ~1;
		}
		if (ACPwrt == 0xFFFFFFFF) {					/* If ACP I/F not available, no ACP				*/
			UseACP &= ~2;
		}

		ACPmaskRd  = 0;								/* ORmask to apply on the address to use ACP	*/
		ACPmaskWrt = 0;
		if (UseACP & 1) { 
			ACPmaskRd  = ACPrd; 	
		}
		if (UseACP & 2) { 
			ACPmaskWrt = ACPwrt;
		}

		memset((void *)&G_Dst[0], 0x55, 2*Size);
		memset((void *)&G_Src[0], 0xAA, 2*Size);

		for (ii=0 ; ii<Size ; ii++) {				/* Put new data in the source buffer.			*/
			SeedData        *= 1664525;
			SeedData        += 1013904223;
			G_Src[OffSrc+ii] = (uint8_t)SeedData;
		}


		Tick      = G_OStimCnt;						/* Tiem measurement start before DMA set-up		*/

		OpMode[0] = DMA_CFG_WAIT_TRG;				/* This DMA transfer wait for the EOT of other	*/
		OpMode[1] = DMA_CFG_NOWAIT;					/* The other DMA will wait on this one EOT		*/
		OpIdx     = 2;
		if (UseACP & 1) { 
			OpMode[OpIdx++] = DMA_CFG_NOCACHE_SRC;
		}
		if (UseACP & 2) { 
			OpMode[OpIdx++] = DMA_CFG_NOCACHE_DST;
		}

		OpMode[OpIdx] = 0;

		RetVal1 = dma_xfer(0,
	                      (uint8_t *)(ACPmaskWrt + (uintptr_t)(&G_Dst[OffDst+128])), 1, MEMORY_DMA_ID,
	                      (uint8_t *)(ACPmaskRd  + (uintptr_t)(&G_Src[OffSrc+128])), 1, MEMORY_DMA_ID,
	                      1, 1, Size-128,
	                      DMA_OPEND_NONE, NULL, (intptr_t)0,
		                  &OpMode[0], &XferID1, OS_MS_TO_TICK(1000));

		OpMode[0] = DMA_CFG_TRG_ON_END(XferID1);	/* This DMA trigger the start of other when EOT	*/
		OpMode[1] = DMA_CFG_EOT_ISR;				/* Use ISRs for the EOT							*/
		OpMode[2] = DMA_CFG_WAIT_ON_END(XferID1);	/* Wait on the other DMA EOT					*/
		OpIdx     = 3;
		if (UseACP & 1) {
			OpMode[OpIdx++] = DMA_CFG_NOCACHE_SRC;
		}
		if (UseACP & 2) {
			OpMode[OpIdx++] = DMA_CFG_NOCACHE_DST;
		}
		OpMode[OpIdx] = 0;

		RetVal2 = dma_xfer(0,
	                      (uint8_t *)(ACPmaskWrt + (uintptr_t)(&G_Dst[OffDst])), 1, MEMORY_DMA_ID,
	                      (uint8_t *)(ACPmaskRd  + (uintptr_t)(&G_Src[OffSrc])), 1, MEMORY_DMA_ID,
		                  1, 1, 128,
		                  DMA_OPEND_NONE, NULL, (intptr_t)0,
		                  &OpMode[0], &XferID2, OS_MS_TO_TICK(1000));

		Tick    = G_OStimCnt							/* Transfer done, look at the time it took	*/
	            - Tick;									/* set-up & transfer evrything				*/

		Err     = 0;
		ErrType = 0;

		for (ii=0 ; ii<OffDst ; ii++) {					/* Check if area before dst is untouched	*/
			if (G_Dst[ii] != 0x55) {
				Err++;
				ErrType |= 1;
			}
		}

		if ((0 != memcmp((void *)&G_Src[OffSrc], (void *)&G_Dst[OffDst], Size))
		&&  (Err == 0)) {
			for (ii=0 ; ii<(Size) ; ii++) {				/* Check all data was corecctly copied		*/
				if (G_Dst[ii+OffDst] != G_Src[ii+OffSrc]) {
					Err++;
					ErrType |= 2;
				}
			}
			ErrType |= 2;
		}

		if (Err == 0) {									/* Check if area bafter dst is untouched	*/
			for (ii=(OffDst+Size) ; ii<(2*Size) ; ii++) {
				if (G_Dst[ii] != 0x55) {
					Err++;
					ErrType |= 4;
				}
			}
		}

		if (RetVal1 != 0) {
			printf("\n1st dma_xfer() reported the error #%d\n", RetVal1);
		}
		else if (RetVal2 != 0) {
			printf("\n2nd dma_xfer() reported the error #%d\n", RetVal2);
		}
		else if (Err != 0) {
			printf("\nERROR: DMA transfer completed but there is incorrect data\n");
			printf("ERROR: Number of errors: %d\n", Err);
			if (ErrType & 1) {
				printf("ERROR: area before destination buffer is not left untouched\n");
			}
			if (ErrType & 2) {
				printf("ERROR: mismatch in copied data\n");
			}
			if (ErrType & 3) {
				printf("ERROR: area after destination buffer is not left untouched\n");
			}
		}

		if ((ACPrd != 0xFFFFFFFF) || (ACPwrt != 0xFFFFFFFF)) {
			printf("\rTest #%4d: &Src[%2d] &Dst[%2d] Size:%9d    Xfer:%4d ms %5dMB/s %c%c",
		                             TestCnt++, OffSrc, OffDst, Size, (TimerUS*Tick)/1000,
			                         (Tick == 0) ? 0 : Size/((TimerUS*Tick)),
		                            (UseACP&1) ? 'R' : '-', (UseACP&2) ? 'W' : '-');
		}
		else {
			printf("\rTest #%4d: &Src[%2d] &Dst[%2d] Size:%9d    Xfer time:%4d ms %5dMB/s ",
		                             TestCnt++, OffSrc, OffDst, Size, (TimerUS*Tick)/1000,
			                         (Tick == 0) ? 0 : Size/((TimerUS*Tick)));
		}
	}
}

/* ------------------------------------------------------------------------------------------------ */
/* Not used but required because of the the generic OS_DEMO value used								*/

void TIMcallBack(void)
{
	return;
}

/* EOF */

