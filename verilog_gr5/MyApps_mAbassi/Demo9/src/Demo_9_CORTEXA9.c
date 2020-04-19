/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_9_CORTEXA9.c																	*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				Tiny file system shell for the MMC card on the board								*/
/*				When a QSPI flash is on the board, can also use the QSPI flash						*/
/*																									*/
/*				Serial port settings:																*/
/*							Baud Rate: 115200														*/
/*							Data bits:      8														*/
/*							Stop bits:      1														*/
/*							Parity:      none														*/
/*							Flow Ctrl:   none														*/
/*							Emulation:   none														*/
/*																									*/
/*																									*/
/* Copyright (c) 2013-2019, Code-Time Technologies Inc. All rights reserved.						*/
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
/*	$Revision: 1.50 $																				*/
/*	$Date: 2019/01/10 18:07:15 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if (((OS_DEMO) != 9) && ((OS_DEMO) != -9))
	#error "Wrong OS_DEMO value" 
#endif

#if defined(_STANDALONE_)						 	/* It is the same include file in all cases.	*/
  #include "mAbassi.h"									/* There is a substitution during the release 	*/
#elif defined(_UABASSI_)							/* This file is the same for Abassi, mAbassi,	*/
  #include "mAbassi.h"								/* and uAbassi, stamdalone so Code Time uses	*/
#elif defined (OS_N_CORE)							/* these checks to not have to keep 4 quasi		*/
  #include "mAbassi.h"								/* identical copies of this file				*/
#else
  #include "mAbassi.h"
#endif

#include "SysCall.h"								/* System call layer definitions				*/
#include "Platform.h"								/* Everything about the target platform is here	*/
#include "HWinfo.h"									/* Everything about the target board is here	*/
#include "MediaIF.h"								/* Media interface with SD/MMC, QSPI etc		*/

#include "ff.h"										/* FatFS definitions							*/
#include "diskio.h"									/* Media I/F: needed to access disk_ioctl()		*/

#if  ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)													\
 ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5))
  #include "arm_pl330.h"
  #include "dw_i2c.h"
  #include "cd_qspi.h"
  #include "dw_sdmmc.h"
  #include "dw_uart.h"
#elif ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
  #include "cd_i2c.h"
  #include "xlx_lqspi.h"
  #include "xlx_sdmmc.h"
  #include "cd_uart.h"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#ifndef VERBOSE
  #define VERBOSE			0						/* != 0 print more information					*/
#endif
#if ((VERBOSE) != 0)
  #define PRT_ERROR(x) puts(x)
#else
  #define PRT_ERROR(x) puts(x)
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer				*/

static DIR   g_Dir;
static char  g_DirNowPath[512]  __attribute__ ((aligned (OX_CACHE_LSIZE)));

static struct {										/* See SysCall_FatFS_....c for an explanation 	*/
	FATFS   FileSys;								/* about the padding & alignment				*/
	uint8_t Pad[OX_CACHE_LSIZE];
} gg_FileSys __attribute__ ((aligned (OX_CACHE_LSIZE)));

static char g_Drive[3];								/* Mounted drive								*/

#define g_FileSys (gg_FileSys.FileSys)

typedef struct {
	char *Name;
	int (* FctPtr)(int argc, char *argv[]);
} Cmd_t;
													/* Align on cache lines if cached transfers		*/
static char g_Buffer[16384] __attribute__ ((aligned (OX_CACHE_LSIZE)));	/* Buffer used for all I/O	*/

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void CleanRead(char *Str);

int cmd_cat(int argc, char *argv[]);
int cmd_cd(int argc, char *argv[]);
int cmd_chmod(int argc, char *argv[]);
int cmd_cp(int argc, char *argv[]);
int cmd_du(int argc, char *argv[]);
int cmd_fmt(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);
int cmd_ls(int argc, char *argv[]);
int cmd_mkdir(int argc, char *argv[]);
int cmd_mnt(int argc, char *argv[]);
int cmd_mv(int argc, char *argv[]);
int cmd_perf(int argc, char *argv[]);
int cmd_pwd(int argc, char *argv[]);
int cmd_rm(int argc, char *argv[]);
int cmd_rmdir(int argc, char *argv[]);
int cmd_umnt(int argc, char *argv[]);
Cmd_t CommandLst[] = {
	{ "cat",	&cmd_cat	},
	{ "cd",		&cmd_cd		},
	{ "chmod",	&cmd_chmod	},
	{ "cp",		&cmd_cp		},
	{ "du",		&cmd_du		},
	{ "fmt",	&cmd_fmt	},
	{ "help",   &cmd_help	},
	{ "ls",		&cmd_ls		},
	{ "mkdir",	&cmd_mkdir	},
	{ "mnt",	&cmd_mnt	},
	{ "mv",		&cmd_mv		},
	{ "perf",	&cmd_perf	},
	{ "pwd",	&cmd_pwd	},
	{ "rm",		&cmd_rm		},
	{ "rmdir",	&cmd_rmdir	},
	{ "umnt",	&cmd_umnt	},
	{ "?",      &cmd_help	}
};

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(int argc, char *argv[]) 
{
int  Arg_C;											/* Number of tokens on the command line			*/
int  ii;											/* General purpose								*/
int  IsStr1;										/* Toggle to decode quoted ' strings			*/
int  IsStr2;										/* Toggle to decode quoted " strings			*/
int  jj;											/* General purpose								*/
char PrevC;											/* Previous character during lexical analysis	*/

static char  CmdLine[256];							/* Command line typed by the user				*/
static char *Arg_V[10];								/* Individual tokens from the command line		*/

#if ((FATFS_USE_RTC) && ((I2C_USE_RTC) != 0) && ((I2C_IN_USE)!= 0))
int  Speed[5];										/* Use smallest speed amongst same device		*/
int  Width[5];										/* Use smallest width amongst same device		*/
int  I2CdevToInit[5];								/* To only init a device once					*/
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
	                                                                      | UART_ECHO_BS_EXPAND
	                                                                      | UART_FILT_EOL_CR
	                                                                      | UART_FILT_EOF_CTRLD);

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
/* SD/MMC interrupt handler							*/

  #if (0U == ((SDMMC_LIST_DEVICE) & (1U << (SDMMC_DEV))))
	#error "Selected SDMMC device is not in SDMMC_LIST_DEVICE"
  #endif

  #if (((SDMMC_LIST_DEVICE) & 0x01U) != 0U)			/* Install the SDMMC interrupt handler			*/
	OSisrInstall(SDMMC0_INT, MMCintHndl_0);			/* The SD/MMC driver, mmc_init(), is not called	*/
	GICenable(SDMMC0_INT, 128, 0);					/* because FatFS deals with the initialization	*/
  #endif

  #if (((SDMMC_LIST_DEVICE) & 0x02U) != 0U)			/* Install the SDMMC interrupt handler			*/
	OSisrInstall(SDMMC1_INT, MMCintHndl_1);			/* The SD/MMC driver, mmc_init(), is not called	*/
	GICenable(SDMMC1_INT, 128, 0);					/* because FatFS deals with the initialization	*/
  #endif

  #if (((SDMMC_LIST_DEVICE) & 0x04U) != 0U)			/* Install the SDMMC interrupt handler			*/
	OSisrInstall(SDMMC2_INT, MMCintHndl_2);			/* The SD/MMC driver, mmc_init(), is not called	*/
	GICenable(SDMMC2_INT, 128, 0);					/* because FatFS deals with the initialization	*/
  #endif

  #if (((SDMMC_LIST_DEVICE) & 0x08U) != 0U)			/* Install the SDMMC interrupt handler			*/
	OSisrInstall(SDMMC3_INT, MMCintHndl_3);			/* The SD/MMC driver, mmc_init(), is not called	*/
	GICenable(SDMMC3_INT, 128, 0);					/* because FatFS deals with the initialization	*/
  #endif

/* ------------------------------------------------ */
/* QSPI interrupt handler							*/

 #ifdef QSPI_DEV
   #if (0U == ((QSPI_LIST_DEVICE) & (1U << (QSPI_DEV))))
	#error "Selected QSPI device is not in QSPI_LIST_DEVICE"
   #endif

  #if (((QSPI_LIST_DEVICE) & 0x01U) != 0U)			/* Install the QSPI interrupt handler			*/
	OSisrInstall(QSPI0_INT, &QSPIintHndl_0);		/* The QSPIdriver, qspi_init(), is not called	*/
	GICenable(QSPI0_INT, 128, 0);					/* because the FS deals with the initialization	*/
   #endif

   #if (((QSPI_LIST_DEVICE) & 0x02U) != 0U)			/* Install the QSPI interrupt handler			*/
	OSisrInstall(QSPI1_INT, &QSPIintHndl_1);		/* The QSPIdriver, qspi_init(), is not called	*/
	GICenable(QSPI1_INT, 128, 0);					/* because the FS deals with the initialization	*/
   #endif

   #if (((QSPI_LIST_DEVICE) & 0x04U) != 0U)			/* Install the QSPI interrupt handler			*/
	OSisrInstall(QSPI2_INT, &QSPIintHndl_2);		/* The QSPIdriver, qspi_init(), is not called	*/
	GICenable(QSPI2_INT, 128, 0);					/* because the FS deals with the initialization	*/
   #endif

   #if (((QSPI_LIST_DEVICE) & 0x08U) != 0U)			/* Install the QSPI interrupt handler			*/
	OSisrInstall(QSPI3_INT, &QSPIintHndl_3);		/* The QSPIdriver, qspi_init(), is not called	*/
	GICenable(QSPI3_INT, 128, 0);					/* because the FS deals with the initialization	*/
   #endif
 #endif

/* ------------------------------------------------ */
/* I2C initialization								*/

 #if (((FATFS_USE_RTC) != 0) && ((I2C_USE_RTC) != 0) && ((I2C_IN_USE)!= 0) && defined(I2C_DEV))

  #if (0U == ((I2C_LIST_DEVICE) & (1U << (I2C_DEV))))
	#error "Selected I2C device is not in I2C_LIST_DEVICE"
  #endif

	memset(&I2CdevToInit[0], 0, sizeof(I2CdevToInit));
	for (ii=0 ; ii<sizeof(Speed)/sizeof(Speed[0]) ; ii++) {
		Speed[ii] = 400000;
		Width[ii] = 10;
	}

  #if ((I2C_USE_RTC) != 0)
	if (Speed[I2C_RTC_DEVICE] > I2C_RTC_SPEED) {
		Speed[I2C_RTC_DEVICE] = I2C_RTC_SPEED;
	}
	if (Width[I2C_RTC_DEVICE] > I2C_RTC_WIDTH) {
		Width[I2C_RTC_DEVICE] = I2C_RTC_WIDTH;
	}
	I2CdevToInit[I2C_RTC_DEVICE] = 1;
  #endif

	for (ii=0 ; ii<sizeof(I2CdevToInit)/sizeof(I2CdevToInit[0]) ; ii++) {
		if (I2CdevToInit[ii] != 0) {
		    i2c_init(ii, Width[ii], Speed[ii]);
		}
	}

  #if (((I2C_LIST_DEVICE) & 0x01U) != 0U)
	if (I2CdevToInit[0] != 0) {
		OSisrInstall(I2C0_INT, &I2CintHndl_0);		/* Install the I2C driver interrupt				*/
		GICenable(I2C0_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x02U) != 0U)
	if (I2CdevToInit[1] != 0) {
		OSisrInstall(I2C1_INT, &I2CintHndl_1);		/* Install the I2C driver interrupt				*/
		GICenable(I2C1_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x04U) != 0U)
	if (I2CdevToInit[2] != 0) {
		OSisrInstall(I2C2_INT, &I2CintHndl_2);		/* Install the I2C driver interrupt				*/
		GICenable(I2C2_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x08U) != 0U)
	if (I2CdevToInit[3] != 0) {
		OSisrInstall(I2C3_INT, &I2CintHndl_3);		/* Install the I2C driver interrupt				*/
		GICenable(I2C3_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x10U) != 0U)
	if (I2CdevToInit[4] != 0) {
		OSisrInstall(I2C4_INT, &I2CintHndl_4);		/* Install the I2C driver interrupt				*/
		GICenable(I2C4_INT, 128, 1);
	}
  #endif
 #endif

/* ------------------------------------------------ */
/* Application set-up								*/

	g_DirNowPath[0] = '\0';

	puts("\n\nTiny Shell for FatFS\n");				/* Welcome banner								*/
	puts("Available commands\n\n");

	cmd_help(100, NULL);

	puts("\nThe EOF character for 'cat' is CTRL-D");
	puts("The device must first be mounted using the command \"mnt\"\n");

	puts("\nAvailable drives / devices:");
	for (ii=0 ; ii<10 ; ii++) {
		if (MEDIAinfo(ii) != (char *)NULL) {
			printf("    Drive %d is %s\n", ii, MEDIAinfo(ii));
		}
	}
	putchar('\n');

/* ------------------------------------------------ */
/* Processing loop									*/

	for (;;) {										/* Infinite loop								*/
		if (FR_OK == f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			printf("%s > ", g_DirNowPath);			/* Prompt										*/
		}
		else {
			printf("> ");
		}

		gets(&CmdLine[0]);							/* Get the command from the user				*/

		CleanRead(&CmdLine[0]);						/* Replace tab by space and handle backspaces	*/

		ii     = 0;
		IsStr1 = 0;
		IsStr2 = 0;
		jj     = 0;
		PrevC  = ' ';
		while (CmdLine[ii] != '\0') {				/* Multi-space elimination						*/
			if (CmdLine[ii] == '\'') {				/* Toggle beginning / end of string				*/
				IsStr1 = !IsStr1;
			}
			if (CmdLine[ii] == '\"') {				/* Toggle beginning / end of string				*/
				IsStr2 = !IsStr2;
			}

			while ((PrevC == ' ')
			&&     (CmdLine[ii] == ' ')
			&&     (IsStr1 == 0)					/* Strings are left untouched					*/
			&&     (IsStr2 == 0)) {
				ii++;
			}
			PrevC = CmdLine[ii];
			CmdLine[jj++] = CmdLine[ii++];
		}
		CmdLine[jj] = '\0';

		Arg_C  = 0;									/* Split the command line into tokens			*/
		ii     = 0;
		IsStr1 = 0;
		IsStr2 = 0;
		PrevC  = '\0';
		while (CmdLine[ii] != '\0') {
			if (CmdLine[ii] == '\'') {				/* Properly handle strings enclosed with '		*/
				if (IsStr1 == 0) {
					Arg_V[Arg_C++] = &CmdLine[++ii];
				}
				else {
					CmdLine[ii++] = '\0';
				}
				IsStr1 = !IsStr1;
			}
			else if (CmdLine[ii] == '\"') {			/* Properly handle strings enclosed with "		*/
				if (IsStr2 == 0) {
					Arg_V[Arg_C++] = &CmdLine[++ii];
				}
				else {
					CmdLine[ii++] = '\0';
				}
				IsStr2 = !IsStr2;
			}
			if ((IsStr1 == 0)						/* Not in as string and is a white space		*/
			&&  (IsStr2 == 0)
			&&  (CmdLine[ii] == ' ')) {
				CmdLine[ii] = '\0';					/* This is the end of the token					*/
			}
					
			if ((PrevC == '\0')						/* Start of a new token							*/
			&&  (IsStr1 == 0)
			&&  (IsStr2 == 0)) {
				Arg_V[Arg_C++] = &CmdLine[ii];
			}

			PrevC = CmdLine[ii];
			ii++;
		}

		if (IsStr1 != 0) {							/* Check for incomplete strings					*/
			puts("ERROR: Missing end quote \'");
			Arg_C = 0;
		}
		if (IsStr2 != 0) {
			puts("ERROR: Missing end quote \"");
			Arg_C = 0;
		}

		if (Arg_C != 0) {							/* It is not an empty line						*/
			for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])); ii++) {
				if (0 == strcmp(CommandLst[ii].Name, Arg_V[0])) {
					printf("\n");
					CommandLst[ii].FctPtr(Arg_C, Arg_V);
					break;
				}
			}
			if (ii == (sizeof(CommandLst)/sizeof(CommandLst[0]))) {
				printf("ERROR: unknown command \"%s\"\n", Arg_V[0]); 
			}
		}
	}
}

/* ------------------------------------------------------------------------------------------------ */

void TIMcallBack(void)
{
	return;
}

/* ------------------------------------------------------------------------------------------------ */

void CleanRead(char *Str)
{
int  ii;											/* Read index									*/
int  jj;											/* Write index									*/

	ii = 0;
	jj = 0;
	while(Str[ii] != '\0') {						/* Process the whole string						*/
		if (Str[ii] == '\b') {						/* This is a backspace							*/
			if (ii != 0) {							/* Not the first char in the string, then go	*/
				jj--;								/* back 1 character in the destination			*/
			}
			ii++;									/* Skip the back space							*/
		}
		else {										/* Is not a back space							*/
			if (Str[ii] == '\t') {					/* Replace TABS by white space					*/
				Str[ii] = ' ';
			}
			Str[jj++] = Str[ii++];					/* Copy the src to the destination				*/
		}
	}
	Str[jj] = '\0';									/* Terminate the string							*/

	return;
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cat(int argc, char *argv[])
{
char   *Fname;										/* Name of the file to cat						*/
FIL     FileDsc;									/* File descriptor								*/
int     ii;											/* General purpose								*/
int     IsEOF;										/* If eon of file is reached					*/
int     IsWrite;									/* If writing to the file						*/
UINT    Nrd;										/* Number of bytes read							*/
UINT    Nwrt;										/* Number of bytes written						*/
int     RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cat   : Redirect a file to stdout or redirect stdin to a file");
		return(0);
	}

	if (argc < 2) {									/* Needs 1 or 2 options on the command line		*/
		RetVal = 1;
	}

	IsWrite = 0;
	if (RetVal == 0) {
		if (argv[1][0] == '>') {					/* Is a write to a file							*/
			IsWrite = 1;
			if (argv[1][1] != '\0') {				/* If no space between '>' & file name			*/
				Fname = &argv[1][1];				/* The file name starts at the 2nd character	*/
				if (argc != 2) {					/* Can only have 2 tokens then					*/
					RetVal = 1;
				}
			}
			else {									/* Space between '>' & file name				*/
				Fname = argv[2];					/* The file name is the 3rd token				*/
				if (argc != 3) {					/* Can only have 3 tokens then					*/
					RetVal = 1;
				}
			}
		}
		else if (argv[1][0] == '<'){				/* In case using > for write					*/
			if (argv[1][1] != '\0') {				/* If no space between '>' & file name			*/
				Fname = &argv[1][1];				/* The file name starts at the 2nd character	*/
				if (argc != 2) {					/* Can only have 2 tokens then					*/
					RetVal = 1;
				}
			}
			else {									/* Space between '>' & file name				*/
				Fname = argv[2];					/* The file name is the 3rd token				*/
				if (argc != 3) {					/* Can only have 3 tokens then					*/
					RetVal = 1;
				}
			}
		}
		else {
			Fname = &argv[1][0];
			if (argc != 2) {
				RetVal = 1;
			}
		}
	}


	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cat  source_file           Output the file contents on the screen");
		puts("       cat <target_file           Output the file contents on the screen");
		puts("       cat >target_file           Write from terminal to the file");
		puts("                                  Terminate with EOF<CR>");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (IsWrite == 0) {							/* Reading a file								*/
			if (FR_OK != f_open(&FileDsc, Fname, FA_READ)) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				do {								/* Dump the file contents to stdio until EOF	*/
					if (FR_OK != f_read(&FileDsc, &g_Buffer[0], sizeof(g_Buffer), &Nrd)) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Nrd    = 0;
					}
					for (ii=0 ; ii<Nrd ; ii++) {
						putchar(g_Buffer[ii]);
					}
				} while (Nrd > 0);
				f_close(&FileDsc);
			}
		}
		else {										/* Writing to a file							*/
			if (FR_OK == f_stat(Fname, NULL)) {		/* If the file to write exist, delete it first	*/
				if (FR_OK != f_unlink(Fname)) {
					printf("ERROR: cannot overwrite the file %s\n", Fname);
					RetVal = 1;
				}
			}
			if (RetVal == 0) {
				if  (FR_OK != f_open(&FileDsc, Fname, FA_CREATE_ALWAYS|FA_WRITE)) {
					printf("ERROR: cannot open file %s\n", Fname);
					RetVal = 1;
				}
				else {
					IsEOF = 0;
					Nwrt  = 0;
					do {							/* Write to it until EOF from terminal			*/
						ii = getchar();
						if ((ii == EOF)
						||  (ii == 4)) {			/* 4 is CTRL-D									*/
							IsEOF = 1;
						}
						else {
							g_Buffer[Nwrt++] = ii;
						}
						if ((Nwrt >= sizeof(g_Buffer))	/* Buffer full								*/
						|| (IsEOF != 0)) {			/* CTRL-D typed									*/
							if (FR_OK != f_write(&FileDsc, &g_Buffer[0], Nwrt, &Nrd)) {
								PRT_ERROR("ERROR: problems writing the file");
								RetVal = 1;
							}
							if (Nwrt != Nrd) {
								RetVal = 1;
							}
							Nwrt = 0;
						}
					} while (IsEOF == 0);
					f_close(&FileDsc);
				}
			}
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cd(int argc, char *argv[])
{
int   RetVal;										/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cd    : Change directory");
		return(0);
	}

	if (argc != 2) {								/* Need a directory name or ..					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cd dir_name                Change directory to dir_name or up with dir_name=..");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_chdir(argv[1])) {			/* Use FatFS directly							*/
			PRT_ERROR("ERROR: cannot change directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_chmod(int argc, char *argv[])
{
BYTE ReadOnly;										/* Setting read-only or read-write				*/
int  RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("chmod : Change a file / directory access modes");
		return(0);
	}

	if (argc != 3) {								/* Need a mode + directory name or a file name	*/
		RetVal = 1;
	}

	ReadOnly = (BYTE)0;								/* Assume is chmod +w							*/
	if (0 == strcmp(argv[1], "-w")) {
		ReadOnly = AM_RDO;
	}
	else if (0 != strcmp(argv[1], "+w")) {
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: chmod -w file_name        Change a file / directory to read-only");
		puts("       chmod +w file_name        Change a file / directory to read-write");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_chmod(argv[2], ReadOnly, AM_RDO)) {
			PRT_ERROR("ERROR: cannot chmod the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cp(int argc, char *argv[])
{
FIL   FileDscDst;									/* Destination file descriptor					*/
FIL   FileDscSrc;									/* Source file descriptor						*/
UINT  Nrd;											/* Number of bytes read							*/
UINT  Nwrt;											/* Number of bytes written						*/
int   RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("cp    : Copy a file");
		return(0);
	}

	if (argc != 3) {								/* Need a source and destination file			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: cp src_file dst_file       Copy the file src_file to dst_file");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_open(&FileDscSrc, argv[1], FA_READ)) {
			PRT_ERROR("ERROR: cannot open src file");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK == f_stat(argv[2], NULL)) {
			if (FR_OK != f_unlink(argv[2])) {
				PRT_ERROR("ERROR: cannot overwrite dst file");
				RetVal = 1;
				f_close(&FileDscSrc);
			}
		}
	}

	if (RetVal == 0) {
		if  (FR_OK != f_open(&FileDscDst, argv[2], FA_CREATE_ALWAYS|FA_WRITE)) {
			PRT_ERROR("ERROR: cannot open dst file");
			RetVal = 1;
			f_close(&FileDscSrc);
		}
	}

	if (RetVal == 0) {

		do {
			if (FR_OK != f_read(&FileDscSrc, &g_Buffer[0], sizeof(g_Buffer), &Nrd)) {
				PRT_ERROR("ERROR: problems reading the file");
				RetVal = 1;
				Nrd    = 0;
				Nwrt   = 0;
			}
			if (FR_OK != f_write(&FileDscDst, &g_Buffer[0], Nrd, &Nwrt)) {
				PRT_ERROR("ERROR: problems writing the file");
				RetVal = 1;
				Nrd    = 0;
			}
			if (Nrd != Nwrt) {
				PRT_ERROR("ERROR: problems writing the file");
				Nrd = 0;
			}
		} while (Nrd > 0);
		f_close(&FileDscDst);
		f_close(&FileDscSrc);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_du(int argc, char *argv[])
{
DWORD    Bsize;										/* SD card block size							*/
DWORD    Capacity;									/* SD card number of sectors					*/
uint64_t Free;										/* Free space on the disk						*/
FATFS   *Fsys;										/* Volume to dump the stats						*/
int      ii;										/* General purpose								*/
DWORD    Ncluster;									/* Number of clusters on the disk				*/
char     Path[16];									/* Path of the disk								*/
int      RetVal;									/* Return value									*/
uint64_t Size;										/* Size of the disk								*/
WORD     Ssize;										/* SD card number of bytes per sector			*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("du    : Show disk usage");
		return(0);
	}

	if (argc != 1) {								/* Does not need an argument					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: du                         Show the disk usage");
		RetVal = 1;
	}

	if (RetVal == 0) {
		ii = 0;
		while((g_DirNowPath[ii] != '\0')
		&&    (g_DirNowPath[ii] != '/')) {
			Path[ii] = g_DirNowPath[ii];
			ii++;
		}
		Path[++ii] = '\0';

		if (FR_OK != f_getfree(Path, &Ncluster, &Fsys)) {
			PRT_ERROR("ERROR: problems retrieving the information");
			RetVal = 1;
		}

		if (RetVal == 0) {
			Free  = (uint64_t)Ncluster;
			Size  = (uint64_t)Fsys->n_fatent-2;		/* Number of FAT entries						*/
			Free *= Fsys->csize;					/* Time # sectors per clusters					*/
			Size *= Fsys->csize;
		  #if ((_MAX_SS) == 512)
			Free *= 512;							/* Time # bytes per sectors						*/
			Size *= 512;							/* Time # bytes per sectors						*/
		  #else
			Free *= Fsys->ssize;					/* Time # bytes per sectors						*/
			Size *= Fsys->ssize;					/* Time # bytes per sectors						*/
		  #endif
			printf("Disk size:%12llu bytes\n", Size);
			printf("Disk free:%12llu bytes\n", Free);
			printf("Disk used:%12llu bytes\n", Size-Free);

			ii = g_Drive[0]-'0';
			disk_ioctl(ii, GET_SECTOR_COUNT, &Capacity);
			disk_ioctl(ii, GET_SECTOR_SIZE,  &Ssize);

		  #if (((OS_PLATFORM) & 0x00FFFFFF) == 0x000007020)
			if (0 == strncmp(MEDIAinfo(ii), "SDMMC", 5)) {
				puts("\nOn ZYNQ, the two next fields are invalid\n");
			}
		  #endif
			printf("Dev size :%12llu bytes\n", ((uint64_t)Capacity) * ((uint64_t)Ssize));
			disk_ioctl(ii, GET_BLOCK_SIZE,  &Bsize);
			printf("Blk size :%12lu bytes\n", 512*Bsize);
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_fmt(int argc, char *argv[])
{
char Drive[3];
int  RetVal;										/* Return value									*/
static char Buf[4096];

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("fmt   : Format a drive");
		return(0);
	}

	if (argc != 2) {								/* Need a volume name to format					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: fmt device                 Format the device");
		RetVal = 1;
	}

	if (RetVal == 0) {
	  #if ((VERBOSE) > 0) 
		puts("Formatting started");
	  #endif										/* Let f_mkfs decide the au size & others		*/
		Drive[0] = argv[1][0];
		Drive[1] = ':';
		Drive[2] = '\0';
		if (FR_OK != f_mkfs(Drive, FM_ANY, 0, &Buf[0], sizeof(Buf))) {
			PRT_ERROR("ERROR: format of the drive failed");
			RetVal = 1;
		}
	}

  #if ((VERBOSE) > 0) 
	if (RetVal == 0) {
		puts("Done");
	}
  #endif

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_help(int argc, char *argv[])
{
int ii;												/* General purpose								*/
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("help  : Help");
		return(0);
	}

	if (argc == 1) {								/* Print the help for all commands				*/
		puts("usage: help                       Show help for all commands");
		puts("       help cmd ...               Show help for specified commands");
		putchar('\n');
	}
	if ((argc == 100)
	||  (argc ==   1)) {
		for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])) ; ii++) {
			(void)CommandLst[ii].FctPtr(-1, NULL);
		}
		return(0);
	}

	if (argc == 2) {								/* Print the help for the specified command		*/
		for (ii=0 ; ii<(sizeof(CommandLst)/sizeof(CommandLst[0])) ; ii++) {
			if (0 == strcmp(argv[1], CommandLst[ii].Name)) {
				(void)CommandLst[ii].FctPtr(-1, NULL);
				return(0);
			}
		}
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: help                       Show help for all commands");
		puts("       help cmd ...               Show help for specified commands");
		RetVal = 1;
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_ls(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("ls    : List the current directory contents");
		return(0);
	}

	if (argc != 1) {								/* No argument accepted							*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: ls                         List the directory contents");
		RetVal = 1;
	}

	if (RetVal == 0) {								/* Refresh the current directory path			*/
		if (FR_OK != f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		f_opendir(&g_Dir, &g_DirNowPath[0]);		/* Open the directory							*/

		while((FR_OK == f_readdir(&g_Dir, &FileInfo))	/* Scan all directory entries				*/
		&&    (FileInfo.fname[0] != '\0')) {
			putchar((FileInfo.fattrib & AM_DIR) ? 'd' : ' ');
			putchar('r');
			putchar((FileInfo.fattrib & AM_RDO) ? '-' : 'w');
			putchar('x');
			printf(" (%04d.%02d.%02d %02d:%02d:%02d) ", (((int)FileInfo.fdate>>9) &0x7F)+1980,
			                                            (((int)FileInfo.fdate>>5) &0x0F),
			                                            (((int)FileInfo.fdate)    &0x1F),
			                                            (((int)FileInfo.ftime>>11)&0x1F),
			                                            (((int)FileInfo.ftime>> 5)&0x3F),
			                                            (((int)FileInfo.ftime)    &0x1F)*2);
			printf(" %10lu ", (unsigned long)FileInfo.fsize);
		  #if _USE_LFN
			if (FileInfo.fname[0] != '\0') {
				puts(FileInfo.fname);
			}
			else {
				puts(&FileInfo.fname[0]);
			}
		  #else
			puts(&FileInfo.fname[0]);
		  #endif
		}
	}
	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mkdir(int argc, char *argv[])
{
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mkdir : Make a new directory");
		return(0);
	}

	if (argc != 2) {								/* Need the name of the directory to create		*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mkdir dir_name             Make a new directory with the name dir_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_mkdir(argv[1])) {			/* Direct use of FatFS function					*/
			PRT_ERROR("ERROR: cannot create directory");
			RetVal = 1;
		}
	}
	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mnt(int argc, char *argv[])
{
char DirName[16];
int RetVal;											/* Return value									*/
int VolNmb;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mnt   : Mount a drive");
		return(0);
	}

	if (argc != 2) {								/* Need he volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mnt device                 Mount a file system device number");
		RetVal = 1;
	}

	if (RetVal == 0) {
		VolNmb = strtol(argv[1], NULL, 10);
		if ((VolNmb < 0)
		||  (VolNmb >= _VOLUMES)) {
		  #if ((_VOLUMES) == 1)
			PRT_ERROR("ERROR: volume number must be 0 (only one file system available)");
		  #else
			printf("ERROR: volume number must be between 0 and %d\n", (_VOLUMES)-1);
		  #endif
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		g_Drive[0] = argv[1][0];
		g_Drive[1] = ':';
		g_Drive[2] = '\0';
		if (FR_OK != f_mount(&g_FileSys, g_Drive, 1)) {
			printf("ERROR: cannot mount volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		strcpy(DirName, g_Drive);
		strcat(DirName, "/");
		if (FR_OK != f_opendir(&g_Dir, DirName)) {
			printf("ERROR: cannot opendir on volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

  #if ((_VOLUMES) != 1)
	if (RetVal == 0) {
		if (FR_OK != f_chdrive(g_Drive)) {
			printf("ERROR: cannot chdrive to %s\n", g_Drive);
			RetVal = 1;
		}
	}
  #endif

  #if ((VERBOSE) > 0)
	if (RetVal == 0) {
		puts("The volume is now mounted");
	}
  #endif

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mv(int argc, char *argv[])
{
int  DoCP;											/* If crossing directory, need to copy & delete	*/
int  RetVal;										/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mv    : Move / rename a file");
		return(0);
	}

	if (argc != 3) {								/* Need a source and destination name			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mv file_name new_name      Rename file_name to new_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
	DoCP = (NULL != strchr(argv[1], '/'))
	     | (NULL != strchr(argv[2], '/'));
		if (DoCP == 0) {
			if (FR_OK != f_rename(argv[1], argv[2])) {
				RetVal = 1;
			}
		}
		else {
			RetVal = cmd_cp(argc, argv);
			if (RetVal == 0) {
				if (FR_OK != f_unlink(argv[1])) {
					RetVal = 1;
				}
			}
		}
		if (RetVal != 0) {
			PRT_ERROR("ERROR: renaming the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_pwd(int argc, char *argv[])
{
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("pwd   : Show current directory");
		return(0);
	}

	if (argc != 1) {								/* Does not accept arguments					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: pwd                        Print the current working directory");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (FR_OK != f_getcwd(&g_DirNowPath[0], sizeof(g_DirNowPath))) {
			PRT_ERROR("ERROR: cannot get directory information");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		printf("Current directory: %s\n", &g_DirNowPath[0]);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_perf(int argc, char *argv[])
{
UINT   BlkSize;
char  *Buffer;
char  *Cptr;
FIL    FileDsc;										/* File descriptor								*/
UINT   Left;
UINT   Nrd;
UINT   Nwrt;
int    RetVal;										/* Return value									*/
UINT   Size;
int    StartTime;
double Time;
static const char Fname[] = "__PERF__";
FILINFO FileInfo;


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("perf  : Throughput measurements");
		return(0);
	}

	if (argc != 3) {								/* Need the size of the transfers				*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: perf                       Measure the read and write transfer rates");
		puts("       perf Nbytes BlkSize        Nbytes : file size to write then read");
		puts("                                  BlkSize: block size to use");
		RetVal = 1;
	}

	if (RetVal == 0) {
		Size    = (UINT)strtoul(argv[1], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			Size *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			Size *= 1024*1024;
		}
		BlkSize = (UINT)strtoul(argv[2], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			BlkSize *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			BlkSize *= 1024*1024;
		}
		Buffer  = &g_Buffer[0];
		if (BlkSize > sizeof(g_Buffer)) {
			Buffer = malloc(BlkSize);
			if (Buffer == (char *)NULL) {
				puts("ERROR: cannot allocate memory");
				RetVal = 1;
			}

		}
		memset(&Buffer[0], 0x55, BlkSize);
		TSKsleep(2);								/* Do this to make sure the test always same	*/
		if ((RetVal == 0)
		&& (FR_OK == f_stat(Fname, NULL))) {		/* If the file to write exist, delete it first	*/
			if (FR_OK != f_unlink(Fname)) {
				printf("ERROR: cannot overwrite the file %s\n", Fname);
				RetVal = 1;
			}
		}
		if (RetVal == 0) {
			printf("%d bytes file using R/W block size of %d bytes\n", Size, BlkSize);
			if  (FR_OK != f_open(&FileDsc, Fname, FA_CREATE_ALWAYS|FA_WRITE)) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				puts("Starting test");
				Left      = Size;
				StartTime = G_OStimCnt;
				do {
					Nwrt = Left;
					if (Nwrt > BlkSize) {
						Nwrt = BlkSize;
					}
					Left -= Nwrt;
					if (FR_OK != f_write(&FileDsc, &Buffer[0], Nwrt, &Nrd)) {
						PRT_ERROR("ERROR: problems writing the file");
						RetVal = 1;
						Left   = 0U;
					}
					else {
						if (Nrd != Nwrt) {
							PRT_ERROR("ERROR: problems writing the file");
							RetVal = 1;
							Left   = 0U;;
						}
					}
				} while (Left != 0U);
				StartTime = G_OStimCnt
				          - StartTime;

				f_close(&FileDsc);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					printf("[%7.3lfs] Write rate %9.3lf kB/s\n",
					       Time, ((double)Size/1000.0)/Time);
				}
			}
			if ((RetVal == 0)
			&&  (FR_OK != f_open(&FileDsc, Fname, FA_READ))) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				Left      = Size;
				StartTime = G_OStimCnt;
				do {
					Nrd = Left;
					if (Nrd > BlkSize) {
						Nrd = BlkSize;
					}
					Left -= Nrd;
					if (FR_OK != f_read(&FileDsc, &Buffer[0], BlkSize, &Nrd)) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Left   = 0U;
					}
				} while (Left != 0U);
				StartTime = G_OStimCnt
				          - StartTime;

				f_close(&FileDsc);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					printf("[%7.3lfs] Read rate  %9.3lf kB/s\n",
					       Time, ((double)Size/1000.0)/Time);
				}
				f_close(&FileDsc);
			}
		}
		if ((BlkSize > sizeof(g_Buffer))
		&&  (Buffer != (char *)NULL)) {
			free(Buffer);
		}
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(Fname, &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(Fname)) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rm(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("rm    : Remove / delete a file");
		return(0);
	}

	if (argc != 2) {								/* Need the file name to delete					*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: rm file_name               Delete the file file_name");
		RetVal = 1;
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(argv[1], &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure"	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_DIR) {
			PRT_ERROR("ERROR: this is a directory, use rmdir");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_RDO) {
			PRT_ERROR("ERROR: the file is read-only");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rmdir(int argc, char *argv[])
{
FILINFO FileInfo;
int RetVal;											/* Return value									*/


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("rmdir : Remove / delete a directory");
		return(0);
	}

	if (argc != 2) {								/* Need the directory name to delete	   		*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: rmdir dir_name             Delete the directory dirname");
		RetVal = 1;
	}

	if (RetVal == 0) {
		memset(&FileInfo, 0, sizeof(FileInfo));		/* Must zero otherwise garbage pointer			*/
		if (FR_OK != f_stat(argv[1], &FileInfo)) {	/* doc: Pointer to the blank FILINFO structure"	*/
			PRT_ERROR("ERROR: cannot stat the file");
			RetVal = 1;			
		}
	}

	if (RetVal == 0) {
		if (!(FileInfo.fattrib & AM_DIR)) {
			PRT_ERROR("ERROR: this is not a directory, use rm");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FileInfo.fattrib & AM_RDO) {
			PRT_ERROR("ERROR: the directory is read-only");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (FR_OK != f_unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_umnt(int argc, char *argv[])
{
char Drive[3];
int  RetVal;										/* Return value									*/
int  VolNmb;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("umnt  : Unmount a drive");
		return(0);
	}

	if (argc != 2) {								/* Need he volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: umnt device                Unmount a file system device number");
		RetVal = 1;
	}

	VolNmb = strtol(argv[1], NULL, 10);
	if ((VolNmb < 0)
	||  (VolNmb >= _VOLUMES)) {
	  #if ((_VOLUMES) == 1)
		PRT_ERROR("ERROR: volume number must be 0 (only one file system available)");
	  #else
		printf("ERROR: volume number must be between 0 and %d\n", (_VOLUMES)-1);
	  #endif
		RetVal = 1;
	}

	if (RetVal == 0) {
		Drive[0] = argv[1][0];
		Drive[1] = ':';
		Drive[2] = '\0';
		if (FR_OK != f_mount(NULL, Drive, 0)) {
			printf("ERROR: cannot unmount volume %s\n", argv[1]);
			RetVal = 1;
		}
	}

  #if ((VERBOSE) > 0)
	if (RetVal == 0) {
		puts("The volume is now unmounted");
	}
  #endif

	return(RetVal);
}

/* Needed for the interrupt call-back of GPIOs when target platform rerquites it					*/
/* ------------------------------------------------------------------------------------------------ */

void GPIOintHndl(int IOPin)
{
	return;
}

/* EOF */
