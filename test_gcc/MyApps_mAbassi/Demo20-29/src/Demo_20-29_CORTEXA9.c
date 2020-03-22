/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_20-29_CORTEXA9.c																*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				System Call functions for mAbassi & FatFS /FullFAT / ueFAT							*/
/*					- supports SD/MMC, QSPI & Memeoy Drive											*/
/*				Demo #20 - SysCall with FatFS														*/
/*				Demo #21 - SysCall with FullFAT														*/
/*				Demo #22 - SysCal with usFAT														*/
/*				Demo #29 - SysCall with multi-FS (all of the above)									*/
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
/*	$Revision: 1.30 $																				*/
/*	$Date: 2019/01/10 18:07:15 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if ((((OS_DEMO) < 20) || ((OS_DEMO) > 29)) && (((OS_DEMO) > -20) || ((OS_DEMO) < -29)))
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

#if  ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)													\
 ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5))
  #include "arm_pl330.h"
  #include "dw_i2c.h"
  #include "cd_qspi.h"
  #include "dw_sdmmc.h"
  #include "dw_spi.h"
  #include "dw_uart.h"
 #if ((OS_PLATFORM) == 0x0100AAC5)					/* DE0-nano / DE10-nano use GPIO 40 to select	*/
  #include "alt_gpio.h"								/* I2C or SPI 									*/
 #endif
#elif ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
  #include "cd_i2c.h"
  #include "xlx_lqspi.h"
  #include "xlx_sdmmc.h"
  #include "cd_spi.h"
  #include "cd_uart.h"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Xtra defs and checks 																			*/

#ifndef DEMO_USE_RTC
 #if ((I2C_USE_RTC) != 0)
  #define DEMO_USE_RTC	1
 #else
  #define DEMO_USE_RTC	0
 #endif
#endif

#ifndef OS_SYS_CALL
  #error "OS_SYS_CALL must be defined and set to non-zero to use the system call layer"
#else
  #if ((OS_SYS_CALL) == 0)
	#error "OS_SYS_CALL must be set to non-zero to use the system call layer"
  #endif
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#ifndef SYS_CALL_DEV_TTY
  #error "Must define SYS_CALL_DEV_TTY and set to non-zero"
#endif
#ifndef SYS_CALL_TTY_EOF
  #error "Must define the stdin EOF char with SYS_CALL_TTY_EOF"
#endif

#ifndef VERBOSE
  #define VERBOSE			0						/* != 0 print more information					*/
#endif
#if ((VERBOSE) != 0)
  #define PRT_ERROR(x) puts(x)
#else
  #define PRT_ERROR(x) puts(x)
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer				*/

typedef struct {
	char *Name;
	int (* FctPtr)(int argc, char *argv[]);
} Cmd_t;
													/* Align on cache lines if cached transfers		*/
static char g_Buffer[16384] __attribute__ ((aligned (OX_CACHE_LSIZE)));	/* Buffer used for all  I/O	*/

static TSK_t *g_AllTask[3];
static SEM_t *g_AllSem[3];

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void CleanRead(char *Str);
void TermTask(void);								/* Terminal task								*/

int cmd_cat(int argc, char *argv[]);
int cmd_cd(int argc, char *argv[]);
int cmd_chmod(int argc, char *argv[]);
int cmd_cp(int argc, char *argv[]);
int cmd_du(int argc, char *argv[]);
int cmd_errno(int argc, char *argv[]);
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
int cmd_tst(int argc, char *argv[]);
int cmd_umnt(int argc, char *argv[]);
Cmd_t CommandLst[] = {
	{ "cat",	&cmd_cat	},
	{ "cd",		&cmd_cd		},
	{ "chmod",	&cmd_chmod	},
	{ "cp",		&cmd_cp		},
	{ "du",		&cmd_du		},
	{ "errno",	&cmd_errno	},
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
	{ "tst",	&cmd_tst	},
	{ "umnt",	&cmd_umnt	},
	{ "?",      &cmd_help	}
};

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(int argc, char *argv[]) 
{
int  ii;											/* General purpose								*/
const char *PtrC;

#if ((DEMO_USE_RTC) && ((I2C_USE_RTC) != 0) && ((I2C_IN_USE)!= 0))
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
/* QSPI interrupt handler							*/

 #if (defined(DEMO_USE_QSPI) && ((DEMO_USE_QSPI) != 0) && defined(QSPI_DEV))
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
/* SPI interrupt handler							*/

 #if (defined(DEMO_USE_SPI) && ((DEMO_USE_SPI) != 0) && defined(DEV_SPI) && defined(SYS_CALL_DEV_SPI))
  #if ((SYS_CALL_DEV_SPI) != 0)
   #if (0U == ((SPI_LIST_DEVICE) & (1U << (SPI_DEV))))
	#error "Selected SPI device is not in SPI_LIST_DEVICE"
   #endif

   #if (((SPI_LIST_DEVICE) & 0x01U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI0_INT, &SPIintHndl_0);
	GICenable(SPI0_INT, 128, 0);					/* Must be level triggered and not edge			*/
   #endif

   #if (((SPI_LIST_DEVICE) & 0x02U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI1_INT, &SPIintHndl_1);
	GICenable(SPI1_INT, 128, 0);					/* Must be level triggered and not edge			*/
   #endif

   #if (((SPI_LIST_DEVICE) & 0x04U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI2_INT, &SPIintHndl_2);
	GICenable(SPI2_INT, 128, 0);					/* Must be level triggered and not edge			*/
   #endif

   #if (((SPI_LIST_DEVICE) & 0x08U) != 0U)			/* Install the SPI interrupt handler			*/
	OSisrInstall(SPI3_INT, &SPIintHndl_3);
	GICenable(SPI3_INT, 128, 0);					/* Must be level triggered and not edge			*/
   #endif

  #if ((OS_PLATFORM) == 0x0100AAC5)					/* DE0-nano: select SPI on LTC connector		*/
	DE0_SELECT_LT_SPI();
  #endif

  #endif
 #endif

/* ------------------------------------------------ */
/* I2C initialization								*/

 #if (((DEMO_USE_RTC) && ((I2C_USE_RTC) != 0) && ((I2C_IN_USE)!= 0)) && defined(I2C_DEV))

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
/* App set-up										*/

	g_AllSem[0] = SEMopen("Sem #0");
	g_AllSem[1] = SEMopen("Sem #1");
	g_AllSem[2] = SEMopen("Sem #2");

	SEMpost(g_AllSem[0]);

	g_AllTask[0] = TSKmyID();
	g_AllTask[1] = TSKcreate("Term #1", 0, 8192, &TermTask, 0);
	g_AllTask[2] = TSKcreate("Term #2", 0, 8192, &TermTask, 0);
	for (ii=0 ; ii<3 ; ii++) {
		TSKsetArg(g_AllTask[ii], (void *)(uintptr_t)ii);
	}
	TSKresume(g_AllTask[1]);
	TSKresume(g_AllTask[2]);

/* ------------------------------------------------ */
/* Processing loop									*/

  #if (((OS_DEMO) == 20) || ((OS_DEMO) == -20))		/* Welcome banner								*/
	puts("\nTiny Shell for FatFS\n");				
  #elif (((OS_DEMO) == 21) || ((OS_DEMO) == -21))
	puts("\nTiny Shell for FullFAT\n");
  #elif (((OS_DEMO) == 22) || ((OS_DEMO) == -22))
	puts("\nTiny Shell for ueFAT\n");
  #elif (((OS_DEMO) == 29) || ((OS_DEMO) == -29))
	printf("\nTiny Shell for multi-FS with ");
   #ifdef SYS_CALL_MULTI_FATFS
    #if ((SYS_CALL_MULTI_FATFS) != 0)
	printf("- FatFS ");
    #endif
   #endif
   #ifdef SYS_CALL_MULTI_FULLFAT
    #if ((SYS_CALL_MULTI_FULLFAT) != 0)
	printf("- FullFat ");
    #endif
   #endif
   #ifdef SYS_CALL_MULTI_UEFAT
    #if ((SYS_CALL_MULTI_UEFAT) != 0)
	printf("- ueFat ");
    #endif
   #endif
	puts("\n");
  #endif
	puts("Available commands\n\n");

	cmd_help(100, NULL);

	printf("\nThe EOF character for 'cat' is CTRL-%c\n", ((SYS_CALL_TTY_EOF)-1)+'A');
	puts("The device must first be mounted using the command \"mnt\"");
	puts("      example : \"mnt 0 /dsk0\"\n");

	puts("\nAvailable drives / devices:");
	for (ii=0 ; ii<10 ; ii++) {
		PtrC = media_info(ii);
		if (PtrC != (char *)NULL) {
			printf("    Drive %d is %s\n", ii, PtrC);
		}
	}
	putchar('\n');

	TermTask();

	for(;;);
}

/* ------------------------------------------------------------------------------------------------ */

void TermTask(void)
{
int  Arg_C;											/* Number of tokens on the command line			*/
int  ii;											/* General purpose								*/
int  IsStr1;										/* Toggle to decode quoted ' strings			*/
int  IsStr2;										/* Toggle to decode quoted " strings			*/
int  jj;											/* General purpose								*/
char PrevC;											/* Previous character during lexical analysis	*/

char *Arg_V[10];									/* Individual tokens from the command line		*/
char  MyDir[SYS_CALL_MAX_PATH+1];
char  CmdLine[256];									/* Command line typed by the user				*/

	ii = (int)(intptr_t)TSKgetArg();
	SEMwait(g_AllSem[ii], -1);
	printf("NOW IN TASK #%d\n", ii);

	for (;;) {										/* Infinite loop								*/
		if (NULL != getcwd(&MyDir[0], sizeof(MyDir))) {
			printf("%s > ", MyDir);					/* Prompt										*/
		}
		else {
			printf("> ");
		}

		memset(&CmdLine[0], 0, sizeof(CmdLine));	/* Can't use gets(): TTY_EOF needed in cmd_cat	*/
		for (ii=0 ; ii<sizeof(CmdLine)-1 ; ii++) {	/* Get the command from the user				*/
			read(0, &CmdLine[ii], 1);
			if (CmdLine[ii] == '\n') {
				CmdLine[ii] = '\0';
				break;
			}
		}

		CleanRead(&CmdLine[0]);						/* Replace tab by space and handle backspaces	*/

		ii = CmdLine[0]-'0';
		if ((1 == strlen(&CmdLine[0]))
		&&  (isdigit((int)CmdLine[0]))
		&&  (ii < sizeof(g_AllSem)/sizeof(g_AllSem[0]))) {
			SEMpost(g_AllSem[ii]);
			SEMwait(g_AllSem[(int)(intptr_t)TSKgetArg()], -1);
			printf("BACK IN TASK #%d\n", (int)(intptr_t)TSKgetArg());
		}
		else {
			ii     = 0;
			IsStr1 = 0;
			IsStr2 = 0;
			jj     = 0;
			PrevC  = ' ';
			while (CmdLine[ii] != '\0') {			/* Multi-space elimination						*/
				if (CmdLine[ii] == '\'') {			/* Toggle beginning / end of string				*/
					IsStr1 = !IsStr1;
				}
				if (CmdLine[ii] == '\"') {			/* Toggle beginning / end of string				*/
					IsStr2 = !IsStr2;
				}

				while ((PrevC == ' ')
				&&     (CmdLine[ii] == ' ')
				&&     (IsStr1 == 0)				/* Strings are left untouched					*/
				&&     (IsStr2 == 0)) {
					ii++;
				}
				PrevC = CmdLine[ii];
				CmdLine[jj++] = CmdLine[ii++];
			}
			CmdLine[jj] = '\0';

			Arg_C  = 0;								/* Split the command line into tokens			*/
			ii     = 0;
			IsStr1 = 0;
			IsStr2 = 0;
			PrevC  = '\0';
			while (CmdLine[ii] != '\0') {
				if (CmdLine[ii] == '\'') {			/* Properly handle strings enclosed with '		*/
					if (IsStr1 == 0) {
						Arg_V[Arg_C++] = &CmdLine[++ii];
					}
					else {
						CmdLine[ii++] = '\0';
					}
					IsStr1 = !IsStr1;
				}
				else if (CmdLine[ii] == '\"') {		/* Properly handle strings enclosed with "		*/
					if (IsStr2 == 0) {
						Arg_V[Arg_C++] = &CmdLine[++ii];
					}
					else {
						CmdLine[ii++] = '\0';
					}
					IsStr2 = !IsStr2;
				}
				if ((IsStr1 == 0)					/* Not in as string and is a white space		*/
				&&  (IsStr2 == 0)
				&&  (CmdLine[ii] == ' ')) {
					CmdLine[ii] = '\0';				/* This is the end of the token					*/
				}
						
				if ((PrevC == '\0')					/* Start of a new token							*/
				&&  (IsStr1 == 0)
				&&  (IsStr2 == 0)) {
					Arg_V[Arg_C++] = &CmdLine[ii];
				}

				PrevC = CmdLine[ii];
				ii++;
			}

			if (IsStr1 != 0) {						/* Check for incomplete strings					*/
				puts("ERROR: Missing end quote \'");
				Arg_C = 0;
			}
			if (IsStr2 != 0) {
				puts("ERROR: Missing end quote \"");
				Arg_C = 0;
			}

			if (Arg_C != 0) {						/* It is not an empty line						*/
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
int   Fd;											/* File descriptor with open()					*/
FILE *Fdsc;											/* File descriptor with fopen()					*/
char *Fname;										/* Name of the file to cat						*/
int   ii;											/* General purpose								*/
int   IsEOF;										/* If eod of file is reached					*/
int   IsWrite;										/* If writing to the file						*/
int   Nrd;											/* Number of bytes read							*/
int   RetVal;										/* Return value									*/

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
		if (IsWrite == 0) {							/* Dumping a file to stdout						*/
			Fd = open(Fname, O_RDONLY, 0777);
			if (Fd < 0 ) {
				printf("ERROR: cannot open file %s\n", Fname);
				RetVal = 1;
			}
			else {
				do {								/* Dump the file contents to stdio until EOF	*/
					Nrd = read(Fd, &g_Buffer[0], sizeof(g_Buffer));
					if (Nrd < 0) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Nrd    = 0;
					}
					for (ii=0 ; ii<Nrd ; ii++) {
						putchar(g_Buffer[ii]);
					}
				} while (Nrd == sizeof(g_Buffer));
				if (0 != close(Fd)) {
					PRT_ERROR("ERROR: closing the file");
				}
			}
		}
		else {										/* Writing to a file							*/
			if (RetVal == 0) {						/* Using fopen() etc and not open() becauee		*/
				Fdsc = fopen(Fname, "w");			/* each call to write() does the physical write	*/
				if (Fdsc == NULL) {					/* This takes time and could wear some devices	*/
					printf("ERROR: cannot open file %s\n", Fname);
					RetVal = 1;
				}
				else {
					IsEOF = 0;						/* Make sure it is buffered						*/
					setvbuf(Fdsc, &g_Buffer[0], _IOFBF, sizeof(g_Buffer));
					do {
						ii = getchar();
						if (ii == EOF) {
							IsEOF = 1;
						}
						else {
							if (ii == (char)((SYS_CALL_TTY_EOF) & 0x7FFFFFFF)) {
								IsEOF = 1;
							}
							else {
								fputc(ii, Fdsc);
							}
						}
					}
					while (IsEOF == 0);
					if (0 != fclose(Fdsc)) {
						PRT_ERROR("ERROR: closing the file");
					}
				}
			}
		}
	}

	if (RetVal == 0) {
		putchar('\n');
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
		if (0 != chdir(argv[1])) {
			PRT_ERROR("ERROR: cannot change directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_chmod(int argc, char *argv[])
{
int  RetVal;										/* Return value									*/
mode_t NewMode;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("chmod : Change a file / directory access modes");
		return(0);
	}

	if (argc != 3) {								/* Need a mode + directory name or a file name	*/
		RetVal = 1;
	}

	NewMode = 0;
	if (0 == strcmp(argv[1], "-w")) {
		NewMode = S_IRUSR|S_IRGRP|S_IROTH;
	}
	else if (0 == strcmp(argv[1], "+w")) {
		NewMode = S_IRUSR|S_IRGRP|S_IROTH
		        | S_IWUSR|S_IWGRP|S_IWOTH;
	}
	else {
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: chmod -w file_name        Change a file / directory to read-only");
		puts("       chmod +w file_name        Change a file / directory to read-write");
		RetVal = 1;
	}

	if (RetVal == 0) {
       if (0 != chmod(argv[2], NewMode)) {
			PRT_ERROR("ERROR: cannot chmod the file");
		  #if (((OS_DEMO) == 22) || ((OS_DEMO) == -22))
			puts("       chmod() always returns an error with ueFAT");
			puts("       The error is expected (See SysCall_ueFAT.c for more info");
		  #endif

			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_cp(int argc, char *argv[])
{
int   FdDst;										/* Destination file descriptor					*/
int   FdSrc;										/* Source file descriptor						*/
int   Nrd;											/* Number of bytes read							*/
int   Nwrt;											/* Number of bytes written						*/
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
		FdSrc = open(argv[1], O_RDONLY, 0777);
		if (FdSrc < 0) {
			PRT_ERROR("ERROR: cannot open src file");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		FdDst = open(argv[2], O_RDWR|O_CREAT, 0777);
		if (FdDst < 0) {
			PRT_ERROR("ERROR: cannot open dst file");
			RetVal = 1;
			close(FdSrc);
		}
	}

	if (RetVal == 0) {
		do {
			Nrd = read(FdSrc, &g_Buffer[0], sizeof(g_Buffer));
			if (Nrd < 0) {
				PRT_ERROR("ERROR: problems reading the file");
				RetVal = 1;
				Nrd    = 0;
				Nwrt   = 0;
			}
			Nwrt = write(FdDst, &g_Buffer[0], Nrd);
			if (Nwrt < 0) {
				PRT_ERROR("ERROR: problems writing the file");
				RetVal = 1;
				Nrd    = 0;
			}
			if (Nrd != Nwrt) {
				PRT_ERROR("ERROR: problems writing the file");
				Nrd = 0;
			}
		} while (Nrd == sizeof(g_Buffer));

		close(FdSrc);
		close(FdDst);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_du(int argc, char *argv[])
{
int All;											/* If dumping all file systems					*/
int Drv;
int ErrNoBack;										/* Could be trying a lot of drive				*/
int ii;												/* General purpose								*/
int RetVal;											/* Return value									*/
int StatRet;
struct statfs FSstat;								/* Statistics returned by statfs()				*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("du    : Show disk usage");
		return(0);
	}

	All = (argc == 1) ? 1 : 0;

	if (argc > 2) 	{								/* 1 or 2 arguments only						*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: du                         Show the disk usage");
		RetVal = 1;
	}

	if (RetVal == 0) {
		for (ii=0 ; ii<10 ; ii++) {					/* Check the first 10 file systems				*/
			if (All == 0) {							/* Dumping the specified file system			*/
				StatRet =  statfs(argv[1], &FSstat);
				ii      = 100000;
				Drv     = 0;
			}
			else {									/* Dumping all file systems						*/
				ErrNoBack = errno;
				StatRet   = fstatfs(ii, &FSstat);
				errno     = ErrNoBack;
				Drv       = ii;
			}

			if (StatRet == 0) {
				printf("Device    : \'%s\'\n", &(FSstat.f_mntfromname[0]));
				printf("Mount on  : \'%s\'\n", &(FSstat.f_mntonname[0]));
				printf("FS type   : %s\n", &(FSstat.f_fstypename[0]));
				printf("Access    : %s\n", (FSstat.f_flags & MNT_RDONLY) ? "RO" : "R/W");
				printf("Disk size :%12llu bytes\n", (unsigned long long int)
				                                    (FSstat.f_bsize * FSstat.f_blocks));
				printf("Disk free :%12llu bytes\n", (unsigned long long int)
				                                    (FSstat.f_bsize * FSstat.f_bfree));
				printf("Disk used :%12llu bytes\n", (unsigned long long int)
				                                   (FSstat.f_bsize*(FSstat.f_blocks-FSstat.f_bfree)));
				printf("Sector    :%12llu bytes\n", (unsigned long long int)FSstat.f_bsize);

			  #if (((OS_PLATFORM) & 0x00FFFFFF) == 0x000007020)
				if (0 == strncmp("SDMMC", media_info(Drv), 5)) {
					puts("\nOn ZYNQ, the two next fields are invalid for the SD-MMC\n");
				}
			  #endif

				printf("DEV size  :%12llu bytes\n", (unsigned long long)media_size(Drv));
				printf("DEV blocks:%12u bytes\n", (unsigned int)media_blksize(Drv));
				printf("\n");
			}
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_errno(int argc, char *argv[])
{
int RetVal;

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("errno : read or reset errno");
		return(0);
	}

	if (argc > 2) 	{								/* 1 or 2 arguments only						*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: errno                      Show the current value of errno");
		puts("usage: errno 0                    Reset errno to 0");
		RetVal = 1;
	}

	if ((RetVal == 0)
	&&  (argc == 1)) {
		printf("Errno value %d\n", errno);
	}

	if ((RetVal == 0)
	&&  (argc == 2)) {
		if (0 == strcmp(argv[1], "0")) {
			errno = 0;
		}
		else {
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_fmt(int argc, char *argv[])
{
const char *Ftype;									/* File system type								*/
int         RetVal;									/* Return value									*/

  #ifdef FILE_SYSTEM_TYPE							/* File system specified, use it				*/
	Ftype = FILE_SYSTEM_TYPE;						/* File system not specified, try cmd line		*/
  #else
	if (argc == 3) {
		if (0 == strcasecmp("FAT16", argv[2])) {
			Ftype = FS_TYPE_NAME_FAT16;
		}
		else if (0 == strcasecmp("exFAT", argv[2])) {
			Ftype = FS_TYPE_NAME_EXFAT;
		}
		else if (0 == strcasecmp("FAT32", argv[2])) {
			Ftype = FS_TYPE_NAME_FAT32;
		}
		else {
			Ftype = FS_TYPE_NAME_AUTO;
		}
		argc = 2;
	}
	else {
		Ftype = FS_TYPE_NAME_AUTO;
	}
  #endif

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("fmt   : Format a drive");
		puts("        fmt # [FAT16|FAT32|exFAT]");
	  #if (((OS_DEMO) == 29) || ((OS_DEMO) == -29))
		puts("        Can also specify the FS stack e.g. fmt #:FatFS FAT32");
	  #endif
		return(0);
	}

	if ((argc < 2)									/* Need a volume name to format					*/
	||  (argc > 3)) {
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: fmt device                 Format the device");
		RetVal = 1;
	}

	if (RetVal == 0) {
	  #if ((VERBOSE) > 0) 
		puts("Formatting started");
	  #endif
	  #if ((((OS_DEMO) == 21) || ((OS_DEMO) == -21))												\
	   ||  (((OS_DEMO) == 22) || ((OS_DEMO) == -22)))
		puts("This will take a while due to the full device erasing");
	  #endif
		if (0 != mkfs(Ftype, argv[1])) {
			PRT_ERROR("ERROR: format of the drive failed");
		  #if ((OS_DEMO) == 21) 					/* FullFAT always return error for format		*/
			puts("       mkfs() always returns an error with FullFAT");
			puts("       The error is expected (see SysCall_FullFAT.c for more info)");
		  #endif
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
DIR_t         *Dinfo;
struct dirent *DirFile;
struct stat    Finfo;
char           Fname[SYS_CALL_MAX_PATH+1];
char           MyDir[SYS_CALL_MAX_PATH+1];
struct tm     *Time;
int            RetVal;								/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("ls    : List the current directory contents");
		return(0);
	}

	if (argc > 2) {									/* No argument or directory						*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: ls                         List the directory contents");
		RetVal = 1;
	}

	if (RetVal == 0) {								/* Refresh the current directory path			*/
		if (argc == 1) {
			if (NULL == getcwd(&MyDir[0], sizeof(MyDir))) {
				PRT_ERROR("ERROR: cannot obtain current directory");
				RetVal = 1;
			}
		}
		else {
			strcpy(&MyDir[0], argv[1]);
		}
	}

	if (RetVal == 0) {								/* Open the dir to see if it's there			*/
		if (NULL == (Dinfo = opendir(&MyDir[0]))) {
			PRT_ERROR("ERROR: cannot open directory");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {								/* Valid directory, read each entries and print	*/
		errno = 0;
		while(NULL != (DirFile = readdir(Dinfo))) {
			strcpy(&Fname[0], &MyDir[0]);
			strcat(&Fname[0], "/");
			strcat(&Fname[0], &(DirFile->d_name[0]));

			stat(&Fname[0], &Finfo);
			putchar(((Finfo.st_mode & S_IFMT) == S_IFLNK) ? 'l' :
			        ((Finfo.st_mode & S_IFMT) == S_IFDIR) ? 'd':' ');
			putchar((Finfo.st_mode & S_IRUSR) ? 'r' : '-');
			putchar((Finfo.st_mode & S_IWUSR) ? 'w' : '-');
			putchar((Finfo.st_mode & S_IXUSR) ? 'x' : '-');

			if ((Finfo.st_mode & S_IFLNK) == S_IFLNK) {
				printf(" (%-9s mnt point)           - ", media_info(DirFile->d_drv));
			}
			else {
				Time = localtime(&Finfo.st_mtime);
				if (Time != NULL) {
					printf(" (%04d.%02d.%02d %02d:%02d:%02d) ", Time->tm_year + 1900,
					                                            Time->tm_mon,
					                                            Time->tm_mday,
					                                            Time->tm_hour,
					                                            Time->tm_min,
					                                            Time->tm_sec);
				}
				printf(" %10lu ", Finfo.st_size);
			}
			puts(DirFile->d_name);
		}
		closedir(Dinfo);							/* Done, close the directory					*/
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
		if (0 != mkdir(argv[1], 0777)) {
			PRT_ERROR("ERROR: cannot create directory");
			RetVal = 1;
		}
	}
	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_mnt(int argc, char *argv[])
{
char  *Cptr;										/* TO validate strtol()							*/
int    Drv;											/* Drive number									*/
DIR_t *MyDir;
int    RetVal;										/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("mnt   : Mount a drive to a mount point e.g. mnt 0 /");
	  #if (((OS_DEMO) == 29) || ((OS_DEMO) == -29))
		puts("        Can also specify the FS stack e.g. mnt 0:FatFS /");
	  #endif
		return(0);
	}

	if (argc != 3) {								/* Need the volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: mnt device MntPoint        Mount a file system device number");
	}

	if (RetVal == 0) {
		if (argv[2][0] != '/') {
			printf("ERROR: the mount point must always start with /\n");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		Drv = strtol(argv[1], &Cptr, 10);
	  #if (((OS_DEMO) == -29) || ((OS_DEMO) == 29))
		if ((*Cptr != '\0')
		&&  (*Cptr != ':')) {
	  #else
		if (*Cptr != '\0') {
	  #endif
			printf("ERROR: invalid number for drive # (%s)\n", argv[1]);
			RetVal = 1;				
		}
		else if (media_info(Drv) == NULL) {
			printf("ERROR: drive # %s out of range\n", argv[1]);
			RetVal = 1;				
		}
	}

	if (RetVal == 0) {
		if (0 != mount(FS_TYPE_NAME_AUTO, argv[2], 0, argv[1])) {
			printf("ERROR: cannot mount volume %s on %s\n", argv[1], argv[2]);
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		MyDir =  opendir(argv[2]);
		if (MyDir == NULL) {
			printf("ERROR: cannot read / dir on volume %s\n", argv[1]);
			RetVal = 1;
		}
		else {
			closedir(MyDir);
		}
	}


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
		if (0 != rename(argv[1], argv[2])) {
			RetVal = 1;
		}
	}
	if (RetVal != 0) {
		PRT_ERROR("ERROR: renaming the file");
		RetVal = 1;
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_pwd(int argc, char *argv[])
{
char MyDir[SYS_CALL_MAX_PATH+1];
int  RetVal;										/* Return value									*/


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
		if (NULL == getcwd(&MyDir[0], sizeof(MyDir))) {
			PRT_ERROR("ERROR: cannot get directory information");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		printf("Current directory: %s\n", &MyDir[0]);
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_perf(int argc, char *argv[])
{
int    BlkSize;
char  *Buffer;
char  *Cptr;
char   Data;
int    Fd;
int    Left;
int    Nrd;
int    Nwrt;
int    RetVal;										/* Return value									*/
int    Size;
int    StartTime;
double Time;
static const char Fname[] = "__PERF__";


	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("perf  : Throughput measurements");
		return(0);
	}

	if ((argc < 3)									/* Need the size of the transfers				*/
	||  (argc > 4)) {
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: perf                       Measure the read and write transfer rates");
		puts("       perf Nbytes BlkSize [Data] Nbytes : file size to write then read");
		puts("                                  BlkSize: block size to use");
		puts("                                  Data   : byte use to fill");
		RetVal = 1;
	}

	if (RetVal == 0) {
		Size = (int)strtoul(argv[1], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			Size *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			Size *= 1024*1024;
		}
		BlkSize = (int)strtoul(argv[2], &Cptr, 10);
		if ((*Cptr=='k')
		||  (*Cptr=='K')) {
			BlkSize *= 1024;
		}
		if ((*Cptr=='m')
		||  (*Cptr=='M')) {
			BlkSize *= 1024*1024;
		}

		Data = 0x55;
		if (argc == 4) {
			Data = (int)strtoul(argv[3], &Cptr, 0);
		}

		Buffer  = &g_Buffer[0];
		if (BlkSize > sizeof(g_Buffer)) {
			Buffer = malloc(BlkSize);
			if (Buffer == (char *)NULL) {
				puts("ERROR: cannot allocate memory");
				RetVal = 1;
			}

		}
		memset(&Buffer[0], Data, BlkSize);
		TSKsleep(2);								/* Do this to make sure the test always same	*/
		if (RetVal == 0) {
			printf("%d bytes file using R/W block size of %d bytes\n", Size, BlkSize);
			Fd = open(Fname, O_RDWR|O_CREAT, 0777);
			if (Fd < 0) {
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
					Nrd = write(Fd, &Buffer[0], Nwrt);
					if (Nrd < 0) {
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

				close(Fd);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					if (Time != 0) {
						printf("[%7.3lfs] Write rate %9.3lf kB/s\n",
						       Time, ((double)Size/1000.0)/Time);
					}
					else {
						printf("[%7.3lfs] Write rate --------- kB/s (Less than 1 timer tick)\n",
						       Time);
					}
				}
			}
			Fd = open(Fname, O_RDONLY, 0777);
			if ((RetVal == 0)
			&&  (Fd < 0)) {
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
					Nrd = read(Fd, &Buffer[0], BlkSize);
					if (Nrd < 0) {
						PRT_ERROR("ERROR: problems reading the file");
						RetVal = 1;
						Left   = 0U;
					}
				} while (Left != 0U);
				StartTime = G_OStimCnt
				          - StartTime;

				close(Fd);

				Time = ((double)StartTime)/(1000000.0/OX_TIMER_US);
				if (RetVal == 0) {
					if (Time != 0) {
						printf("[%7.3lfs] Read rate  %9.3lf kB/s\n",
						       Time, ((double)Size/1000.0)/Time);
					}
					else {
						printf("[%7.3lfs] Read rate  --------- kB/s (Less than 1 timer tick)\n",
						       Time);
					}
				}
			}
		}
		if ((BlkSize > sizeof(g_Buffer))
		&&  (Buffer != (char *)NULL)) {
			free(Buffer);
		}
	}

	if (RetVal == 0) {
		if (0 != unlink(Fname)) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rm(int argc, char *argv[])
{
int RetVal;											/* Return value									*/
struct stat Finfo;

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
		RetVal = stat(argv[1], &Finfo);
		if (RetVal != 0) {
			PRT_ERROR("ERROR: Can't get stats of the file");
		}
	}

	if (RetVal == 0) {
		if (Finfo.st_mode & S_IFDIR) {
			PRT_ERROR("ERROR: this is a directory, use rmdir");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (0 != unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the file");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_rmdir(int argc, char *argv[])
{
int RetVal;											/* Return value									*/
struct stat Stat;

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
		RetVal = stat(argv[1], &Stat);
		if (RetVal != 0) {
			PRT_ERROR("ERROR: Can't get stats of the file");
		}
	}

	if (RetVal == 0) {
		if (0 == (Stat.st_mode & S_IFDIR)) {
			PRT_ERROR("ERROR: this is not a directory, use rm");
			RetVal = 1;
		}
	}

	if (RetVal == 0) {
		if (0 != unlink(argv[1])) {
			PRT_ERROR("ERROR: cannot remove the directory");
			RetVal = 1;
		}
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_tst(int argc, char *argv[])
{
int fd;
int fd2;
int ii;
int RetVal;											/* Return value									*/
struct stat Stat;
char buf[32];
DIR_t *RootDir;
struct dirent *DirEntry;
int DirPos;
char Fname[10][32];
#ifdef SYS_CALL_DEV_SPI
 #if ((SYS_CALL_DEV_SPI) != 0)
  int Bufi[32];
 #endif
#endif

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("tst   : general test");
		return(0);
	}

	if (argc != 1) {								/* Need the directory name to delete	   		*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: tst");
		RetVal = 1;
	}
	if (RetVal == 0) {
		puts("Test #0 - file R/W & seek");
		fd = open("/tst_file", O_RDWR|O_CREAT|O_TRUNC, 0777);
		if (fd < 0) {
			PRT_ERROR("ERROR: opening test file (1)");
			RetVal = 1;
		}
		else {
			for (ii=0 ; ii<sizeof(buf) ; ii++) {
				buf[ii] = (char)ii;
			}
			if (sizeof(buf) != write(fd, &buf[0], sizeof(buf))) {
				PRT_ERROR("ERROR: writing test file (1)");
				RetVal = 1;
			}
			if (9 != lseek(fd, 9, SEEK_SET)) {
				PRT_ERROR("ERROR: seeking test file (1)");
				RetVal = 1;
			}
			if (1 != read(fd, &buf[0], 1)) {
				PRT_ERROR("ERROR: reading test file (1)");
				RetVal = 1;
			}
			else {
				if (buf[0] != 9) {
					PRT_ERROR("ERROR: seeking test file (2)");
					RetVal = 1;
				}
			}
			if (20 != lseek(fd, 10, SEEK_CUR)) {
				PRT_ERROR("ERROR: seeking test file (3)");
				RetVal = 1;
			}
			if (1 != read(fd, &buf[0], 1)) {
				PRT_ERROR("ERROR: reading test file (2)");
				RetVal = 1;
			}
			else {
				if (buf[0] != 20) {
					PRT_ERROR("ERROR: seeking test file (4)");
					RetVal = 1;
				}
			}
			if (sizeof(buf) != lseek(fd, 0, SEEK_END)) {
				PRT_ERROR("ERROR: seeking test file (5)");
				RetVal = 1;
			}
			for (ii=0 ; ii<sizeof(buf) ; ii++) {
				buf[ii] = ii+sizeof(buf);
			}
			if (sizeof(buf) != write(fd, &buf[0], sizeof(buf))) {
				PRT_ERROR("ERROR: writing test file (2)");
				RetVal = 1;
			}
			else {
				if (0 != lseek(fd, 0, SEEK_SET)) {
					PRT_ERROR("ERROR: seeking test file (6)");
					RetVal = 1;
				}
				else {
					if (sizeof(buf) != read(fd, &buf[0], sizeof(buf))) {
						PRT_ERROR("ERROR: reading test file (3)");
						RetVal = 1;
					}
					else {
						for (ii=0 ; ii<sizeof(buf) ; ii++) {
							if (buf[ii] != ii) {
								PRT_ERROR("ERROR: seeking test file (7)");
								RetVal = 1;
								break;
							}
						}
					}
					if (sizeof(buf) != read(fd, &buf[0], sizeof(buf))) {
						PRT_ERROR("ERROR: reading test file (3)");
						RetVal = 1;
					}
					else {
						for (ii=0 ; ii<sizeof(buf) ; ii++) {
							if (buf[ii] != ii+sizeof(buf)) {
								PRT_ERROR("ERROR: seeking test file (7)");
								RetVal = 1;
								break;
							}
						}
					}
				}
			}
			if (0 != close(fd)) {
				PRT_ERROR("ERROR: closing file (1)");
				RetVal = 1;
			}
		}

		if (RetVal == 0) {
			puts("Test #1 - file opened twice");
			fd  = open("/tst_file", O_RDONLY, 0777);
			fd2 = open("/tst_file", O_RDONLY, 0777);
			if (fd2 < 0) {
				PRT_ERROR("          INFO: cannot have a file RO open more than once (10)");
			}
			else {
				if (0 != close(fd2)) {
					PRT_ERROR("ERROR: closing file (10)");
					RetVal = 1;
				}
			}
			if (0 != close(fd)) {
				PRT_ERROR("ERROR: closing file (10)");
				RetVal = 1;
			}
		}
		if (RetVal == 0) {
			puts("Test #2 - dup()");
		  #if ((OS_DEMO == 22) || ((OS_DEMO) == -22))
			puts("          skipping");
		  #else
			fd  = open("/tst_file", O_RDONLY, 0777);
			fd2 = dup(fd);
			if (fd2 < 0) {
				PRT_ERROR("ERROR: dup (20)");
				RetVal = 1;
			}
			else {
				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing file (20)");
					RetVal = 1;
				}
				fd = fd2;
			}
			if (sizeof(buf) != read(fd, &buf[0], sizeof(buf))) {
				PRT_ERROR("ERROR: reading test file (20)");
				RetVal = 1;
			}
			else {
				for (ii=0 ; ii<sizeof(buf) ; ii++) {
					if (buf[ii] != (char)ii) {
						PRT_ERROR("ERROR: reading test file (21)");
						RetVal = 1;
						break;
					}
				}
			}
			if (0 != close(fd)) {
				PRT_ERROR("ERROR: closing file (21)");
				RetVal = 1;
			}
		  #endif
		}

		if (RetVal == 0) {
			puts("Test #3 - open exclusive of existing refusal");
			fd = open("/tst_file", O_RDWR|O_CREAT|O_EXCL, 0777);
			if (fd >= 0) {
				PRT_ERROR("ERROR: opening test file (30)");
				RetVal = 1;
				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing file (30)");
				}
			}
		}

		if (RetVal == 0) {
			puts("Test #4 - unlink()");
			if (0 != unlink("/tst_file")) {
				PRT_ERROR("ERROR: deleting test file (40)");
				RetVal = 1;
			}
		}

		if (RetVal == 0) {
			puts("Test #5 - fstat()");
			fd = open("/tst_file", O_RDWR|O_CREAT|O_TRUNC, 0777);
			if (fd < 0) {
				PRT_ERROR("ERROR: opening test file (50)");
				RetVal = 1;
			}
			else {
				for (ii=0 ; ii<sizeof(buf) ; ii++) {
					buf[ii] = (char)ii;
				}
				if (sizeof(buf) != write(fd, &buf[0], sizeof(buf))) {
					PRT_ERROR("ERROR: writing test file (50)");
					RetVal = 1;
				}
				if (0 != fstat(fd, &Stat)) {
					PRT_ERROR("ERROR: getting file stat (50)");
					RetVal = 1;
				}
				else {
					if ((Stat.st_mode & 0777) != 0777) {
						PRT_ERROR("ERROR: wrong access mode (50)");
						RetVal = 1;
					}
				}
				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing file (50)");
					RetVal = 1;
				}
			}
		}

		if (RetVal == 0) {
			puts("Test #6 - chmod");
		  #if ((OS_DEMO == 22) || ((OS_DEMO) == -22))
			puts("          skipping");
		  #else
			chmod("/tst_file", 0555);
			if (0 != stat("/tst_file", &Stat)) {
				PRT_ERROR("ERROR: getting file stat (60)");
				RetVal = 1;
			}
			else {
				if ((Stat.st_mode & 0777) != 0555) {
					printf("%0o\n", (int)Stat.st_mode);
					PRT_ERROR("ERROR: wrong access mode (60)");
					RetVal = 1;
				}
			}
		  #endif
		}

		if (RetVal == 0) {
			puts("Test #7 - unlink of RO file refusal");
		  #if ((OS_DEMO == 22) || ((OS_DEMO) == -22))
			puts("          skipping");
		  #else
			if (0 == unlink("/tst_file")) {
				PRT_ERROR("ERROR: R/O file was unlinked (70)");
				RetVal = 1;
			}
		  #endif
		}

		if (RetVal == 0) {
			puts("Test #8 - unlink of RO file set back to R/W");
			chmod("/tst_file", 0777);
			if (0 != unlink("/tst_file")) {
				PRT_ERROR("ERROR: deleting file (80)");
				RetVal = 1;
			}
			if (0 == stat("/tst_file", &Stat)) {
				PRT_ERROR("ERROR: deleting file (81)");
				RetVal = 1;
			}
		}

		if (RetVal == 0) {
			puts("Test #9 - dir operations");
			fd = open("/tst1", O_RDWR|O_CREAT|O_TRUNC, 0777);
			if (fd < 0) {
				PRT_ERROR("ERROR: opening test file (90)");
				RetVal = 1;
			}
			else {
				for (ii=0 ; ii<sizeof(buf) ; ii++) {
					buf[ii] = (char)ii;
				}
				if (sizeof(buf) != write(fd, &buf[0], sizeof(buf))) {
					PRT_ERROR("ERROR: writing test file (90)");
					RetVal = 1;
				}
				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing file (90)");
					RetVal = 1;
				}

				argv[1] = "tst1";
				argv[2] = "tst2";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst3";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst4";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst5";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst6";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst7";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst8";
				cmd_cp(3, &argv[0]);
				argv[2] = "tst9";
				cmd_cp(3, &argv[0]);

				RootDir = opendir("/");
				if (RootDir == NULL) {
					PRT_ERROR("ERROR: opening dir (90)");
					RetVal = 1;
				}
				else {
					DirPos = 0;
					for (ii=0 ; ii<5 ; ii++) {
						if (ii == 3) {
							DirPos = telldir(RootDir);
						}
						DirEntry = readdir(RootDir);
						if (DirEntry == NULL) {
							PRT_ERROR("ERROR: reading dir (90)");
							RetVal = 1;
						}
						else {
							strcpy(&Fname[ii][0], &DirEntry->d_name[0]);
						}
					}

					rewinddir(RootDir);
					for (ii=0 ; ii<5 ; ii++) {
						DirEntry = readdir(RootDir);
						if (DirEntry == NULL) {
							PRT_ERROR("ERROR: reading dir (91)");
							RetVal = 1;
						}
						else {
							if (0 != strcmp(&Fname[ii][0], &DirEntry->d_name[0])) {
								PRT_ERROR("ERROR: rewinding dir (90)");
								RetVal = 1;
							}
						}
					}

					rewinddir(RootDir);
					seekdir(RootDir, DirPos);
					DirEntry = readdir(RootDir);
					if (DirEntry != NULL) {
						if (0 != strcmp(&Fname[3][0], &DirEntry->d_name[0])) {
							PRT_ERROR("ERROR: seeking dir (90)");
							RetVal = 1;
						}
					}
					readdir(RootDir);
					readdir(RootDir);
					seekdir(RootDir, DirPos);
					DirEntry = readdir(RootDir);
					if (DirEntry != NULL) {
						if (0 != strcmp(&Fname[3][0], &DirEntry->d_name[0])) {
							PRT_ERROR("ERROR: seeking dir (91)");
							RetVal = 1;
						}
					}
				}
				unlink("tst1");
				unlink("tst2");
				unlink("tst3");
				unlink("tst4");
				unlink("tst5");
				unlink("tst6");
				unlink("tst7");
				unlink("tst8");
				unlink("tst9");
			}
		}

	  #ifdef SYS_CALL_DEV_I2C
	   #if (((SYS_CALL_DEV_I2C) != 0) && ((I2C_RTC_DEVICE) != 0))
		if (RetVal == 0) {
			sprintf(&buf[0], "/dev/i2c%d", I2C_RTC_DEVICE);
			printf("\nTesting %s:\nshould see date/time printed\n", &buf[0]);
			fd = open(&buf[0], O_RDWR, 0777);
			if (fd < 0) {
				PRT_ERROR("ERROR: opening /dev/i2c");
				RetVal = 1;
			}
			else {
				buf[0] = (char)0;
				buf[1] = (char)I2C_RTC_ADDR;
				buf[2] = (char)0;
				if (1 != write(fd, &buf[0], 1)) {
					PRT_ERROR("ERROR: writing I2C");
					RetVal = 1;
				}

				memset(&buf[0], 0, sizeof(buf));
				buf[0] = (char)0;
				buf[1] = (char)I2C_RTC_ADDR;
				if (7 != read(fd, &buf[0], 7)) {
					PRT_ERROR("ERROR: writing I2C");
					RetVal = 1;
				}				
				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing I2C /dev");
					RetVal = 1;
				}
				printf("20%02x.%x.%x %x:%02x:%02x\n\n",buf[6],buf[5],buf[4],buf[2],buf[1],buf[0]); 
			}
		}
	   #endif
	  #endif

	  #ifdef SYS_CALL_DEV_TTY
	   #if ((SYS_CALL_DEV_TTY) != 0)
		if (RetVal == 0) {
			sprintf(&buf[0], "/dev/tty%d", UART_DEV);
			printf("\nTesting %s:\nshould see \"0123456789\" printed\n", &buf[0]);

			fd = open(&buf[0], O_RDWR, 0777);
			if (fd < 0) {
				PRT_ERROR("ERROR: opening /dev/tty");
				RetVal = 1;
			}
			else {
				if (11 != write(fd, "0123456789\n", 11)) {
					PRT_ERROR("ERROR: writing TTY");
					RetVal = 1;
				}
				puts("\ntype 10 char, they will be echoed after the 10th\n");
				ii = read(fd, &buf[0], 10);
				if (ii < 0) {
					PRT_ERROR("ERROR: reading TTY");
					RetVal = 1;
				}
				else if (ii != 10) {
					PRT_ERROR("ERROR: EOF reading TTY");
				}
				buf[10] = '\0';
				printf("\nChars read \"%s\"\n", buf);

				if (0 != close(fd)) {
					PRT_ERROR("ERROR: closing TTY /dev");
					RetVal = 1;
				}
			}
		}
	   #endif
	  #endif

	  #ifdef SYS_CALL_DEV_SPI
	   #if ((SYS_CALL_DEV_SPI) != 0)
		if (RetVal == 0) {

			sprintf(&buf[0], "/dev/spi%d%d", SPI_DEV, SPI_SLV);
			printf("\nTesting %s\n", &buf[0]);

			fd = open(&buf[0], O_RDWR, 0777);
			if (fd < 0) {
				PRT_ERROR("ERROR: opening /dev/spi");
				RetVal = 1;
			}
			if (RetVal == 0) {
				Bufi[0] = 50 * 1000000;
				Bufi[1] = 8;
				Bufi[2] = SPI_PROTO_SPI
				        | SPI_CLK_CPOL1
				        | SPI_CLK_CPHA1
				        | SPI_ALIGN_RIGHT
				        | SPI_TX_RX_EEPROM
				        | SPI_MASTER
				        | SPI_XFER_ISR
				        | SPI_EOT_ISR;
				RetVal = devctl(fd, &Bufi[0]);
				if (RetVal != 0) {
					PRT_ERROR("ERROR: initializing /dev/spi");
				}
			}
			if (RetVal == 0) {
				while(0 != GetKey());
				puts("Press any key to stop the test");
				buf[1] = 0;
				buf[2] = 0;
				while(0 == GetKey()) {
					buf[0] = 0x30;
					buf[2] += 0x10;
					if (buf[2] == 0) {
						buf[1]++;
					}
					if (3 != write(fd, &buf[0], 3)) {
						PRT_ERROR("ERROR: writing SPI");
						RetVal = 1;
						break;
					}
				}
				printf("\n");
			}
		}
	   #endif
	  #endif
	}

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int cmd_umnt(int argc, char *argv[])
{
int RetVal;											/* Return value									*/

	RetVal = 0;										/* Assume everything is OK						*/
	if (argc < 0) {									/* Special value to print usage					*/
		puts("umnt  : Unmount a mount point");
		return(0);
	}

	if (argc != 2) {								/* Need he volume and the mount point			*/
		RetVal = 1;
	}

	if (RetVal != 0) {								/* Print usage in case of error					*/
		puts("usage: umnt device                Unmount a mount point in the file system");
		RetVal = 1;
	}

	if (RetVal == 0) {
		if (0 != unmount(argv[1], 0)) {
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

/* ------------------------------------------------------------------------------------------------ */
/* Needed for the interrupt call-back of GPIOs														*/
/* ------------------------------------------------------------------------------------------------ */

void GPIOintHndl(int IOPin)
{
	return;
}

/* EOF */
