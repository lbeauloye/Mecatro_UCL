/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_3_CORTEX_A9.c																	*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				Operation of the RTOS is shown on the serial port									*/
/*				Tasks info & configuration through the serial port									*/
/*				Serial port settings:																*/
/*							Baud Rate: 115200														*/
/*							Data bits:      8														*/
/*							Stop bits:      1														*/
/*							Parity:      none														*/
/*							Flow Ctrl:   none														*/
/*							Emulation: VT-100														*/
/*																									*/
/* INFO:																							*/
/*       How this demo works:																		*/
/*       6 Tasks are running, and also Adam & Eve (main) which is not displayed.					*/
/*       The RTOS timer tick interrupt callback handler goes through a state machine to control the	*/
/*       operations & triggers state changes of the 6 tasks.  The information is sent to the tasks	*/
/*       either by mailboxes or by events (see the defintion for USE_EVENTS).						*/
/*       Adam & Eve is periodically unblocked (semaphore) by the RTOS timertick callback.			*/
/*       Adam & Eve looks at the state opf all 6 tasks and send strings to print on stdout			*/
/*       through a mailbox.																			*/
/*       All 6 tasks monitor this mailbox and perform the printing on stdout.						*/
/*       Although a mutex should be used to provided exclusive access to stdout, interrupt			*/
/*       disabling & enabling is needed instead.  This is because if a mutex was used, the priority	*/
/*       inheritance would most likely kick in and change the priority of the printfing task.		*/
/*       That would create a weird display giving the impression tasks chenge priority for no		*/
/*       reasons.																					*/
/*       UART: Echo cannot be used because when ISRs are used, the echoed character can end up		*/
/*             being inserted in a cursor movement command string. That's the reason each char		*/
/*             obtained with GetKey() is sent out with putchar(). This can happen ebcause the		*/
/*             UART driver is not using mutex protection for exclusive access						*/
/*																									*/
/*       For a full description of what is displayed, see:											*/
/*                    www.code-time.com/demo3.html													*/
/*																									*/
/*       NOTE: This demo is very intensice for the kernel and ISR usage.							*/
/*             All cores are continuously polling (kernel call) to grab queued messages when avail.	*/
/*             The display is refreshed by Adam & Eve sending fresh new line all the time through	*/
/*             a mailbox.																			*/
/*             When the UART is set-up for ISR transfers display being refreshed non-stop triggers	*/
/*             lots of interrupts.																	*/
/*																									*/
/* Copyright (c) 2012-2018, Code-Time Technologies Inc. All rights reserved.						*/
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
/*	$Revision: 1.73 $																				*/
/*	$Date: 2018/08/06 12:29:32 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if (((OS_DEMO) != 3) && ((OS_DEMO) != -3) && ((OS_DEMO) == 4003) && ((OS_DEMO) == -4003))
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

/* ------------------------------------------------------------------------------------------------ */

#if  ((((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)													\
 ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5))
  #include "dw_uart.h"
#elif ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
  #include "cd_uart.h"
#elif (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000FEE6)
  #include "imx_uart.h"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define TSK_REFRESH_RATE	10						/* Display refresh rate: 1/TSK_REFRESH_RATE sec	*/
#define RTC_REFRESH_RATE	2						/* RTC refresh rate related to TSK_REFRESH_RATE	*/

#if ((OX_MAILBOX) == 0)
 #define USE_EVENTS			1						/* No mailboxes, then must use events			*/
#elif !defined(USE_EVENTS)
 #define USE_EVENTS			0						/* Events or mailboxes used to inform task 		*/
#endif

#ifndef USE_MAILBOX
  #define USE_MAILBOX		((USE_EVENTS) == 0)
#endif

#define OP_ZERO_CNT			1						/* Operation requested by the ISR state machine	*/
#define OP_LOCK_MTX			2						/* Must use bit-field in case exchanges are		*/
#define OP_UNLOCK_MTX		4						/* done through events							*/

#define TSK_HIGH			0						/* Index of the task info in TskDsc[]			*/
#define TSK_RRA				1
#define TSK_RRB				2
#define TSK_RRC				3
#if ((OX_N_CORE) <= 2)
 #define TSK_STRVA			4
 #define TSK_STRVB			5
#else
 #define TSK_RRD			4
 #define TSK_RRE			5
 #define TSK_RRF			6
 #define TSK_RRG			7
 #define TSK_STRVA			8
 #define TSK_STRVB			9
 #define TSK_STRVC			10
#endif

#define OP_ROUND_ROBIN		0						/* Command line configuration operation			*/
#define OP_STARVE_RUN		1
#define OP_STARVE_WAIT		2
#define OP_STARVE_PRIO		3
#define OP_PRIO				4

#ifndef MBX_DISP_SIZE
 #define MBX_DISP_SIZE		64						/* Depth of the mailbox for the display			*/
#endif

#ifndef TC_TSK_STACK
  #define TC_TSK_STACK		8192					/* Stack size for each task						*/
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer				*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer				*/

#if ((OX_N_CORE) <= 2)
 char Display[7][20] = { "00.0s  Px      ",
                         "00.0s  Px      ",
                         "00.0s  Px      ",
                         "00.0s  Px      ",
                         "00.0s  Px      ",
                         "00.0s  Px      ",
                         "00:00:00       "
                       };
 #define LAST_DISP		6							/* Number of lines / tasks to display			*/
 #define CURSOR_START()	printf("\033[15;0H")
 #define CURSOR_NEXT(x)	printf("\033[15;%02dH", (x)+15)
 #define CURSOR_BACK()	printf("\033[15;15H")
 #define CURSOR_ADD()	printf("\033[15;16H")
 #define LAST_MTX		4
#else
 char Display[12][20] = { "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00.0s  Px      ",
                          "00:00:00       "
                       };
 #define LAST_DISP		11
 #define CURSOR_START()	printf("\033[20;0H")
 #define CURSOR_NEXT(x)	printf("\033[20;%02dH", (x)+15)
 #define CURSOR_BACK()	printf("\033[20;15H")
 #define CURSOR_ADD()	printf("\033[20;16H")
 #define LAST_MTX		8
#endif

typedef struct {									/* Hold the descriptions for the task monitored	*/
	TSK_t *Task;									/* Task descriptor								*/
	int    Running;									/* If this is the running task (ignoring A&E)	*/
	int    WasReset;								/* To inform task to reset running time counter	*/
	int    CPUcnt;									/* Number of timer ticks task has been running	*/
	int    WillBlk;									/* If the task is next to block on the mutex	*/
	int    WillMtx;									/* If the task is next to lock the mutex		*/
  #if ((USE_MAILBOX) != 0)
	MBX_t *Mailbox;									/* Mailbox when ISR state machine uses mailboxes*/
  #endif
} TskDsc_t;

TskDsc_t TskDsc[LAST_DISP];

typedef struct {									/* Info sent to the task to update the display	*/
	char *Str;										/* String to print on the display				*/
	int  X;											/* X coordinate									*/
	int  Y;											/* Y coordinate									*/
} DispInfo_t;										/* It is sent to UpdateDisp()					*/

DispInfo_t DispInfo[MBX_DISP_SIZE];

volatile int g_AppRdy  = 0;							/* To inform RTOS timer callback when the		*/
volatile int g_CmdIdx;								/* Index where to write next character from kyb	*/
volatile int g_DispIdx = 0;							/* Next entry in DispInfo[] to fill				*/
													/* application is fully configured				*/
TSK_t *AdamEve;										/* Adam & Eve task descriptor					*/
MTX_t *Mutex;										/* Mutex used by the task to show inheritance	*/
#if ((USE_MAILBOX) != 0)
  MBX_t *MbxDisp;									/* Mailbox to send display updates to the tasks	*/
#else												/* No mailboxes, use a circulare buffer			*/
  volatile int g_DispRdIdx = 0;						/* Index used by Fct to grab the display test	*/
#endif
SEM_t *SemaAE;										/* Semaphore to awake Adam & Eve				*/

#if ((OX_STARVE_RUN_MAX) != 0)
  extern TSK_t *g_StrvList;							/* Imported from Abassi.c						*/
#else
  TSK_t *g_StrvList = (TSK_t *)NULL;
#endif

#if ((OX_N_CORE) >= 2)
  volatile int g_DispLock;							/* Mutual exclusion access to printing			*/
#endif

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void Fct(void);										/* A single function implements all the tasks	*/
void UpdateDisp(DispInfo_t *Disp);

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(void)
{
int   ccc;											/* Character read from the keyboard				*/
int   Done;											/* If command decoding is done					*/
int   Error;										/* If error during command decoding				*/
int   ii;											/* General purpose								*/
int   IsrState;
int   jj;											/* General purpose								*/
int   kk;											/* General purpose								*/
int   Op;											/* Operation requested by the command			*/
int   RdIdx;										/* Read index in the command line string		*/
char *String;										/* Display sting being created					*/
int   TaskIdx;										/* Index of the task specified on command line	*/
int   Toggle;										/* Toggle use to control refresh of RTC			*/

#if ((OX_N_CORE) >= 2)
  int CoreID;
  int GotLock;
#endif

static char Cmd[64];								/* Command line from keyboard					*/

/* ------------------------------------------------ */
/* Set buffering here: tasks inherit main's stdios	*/
/* when Newlib reent is used & stdio are shared		*/ 

	setvbuf(stdin,  NULL, _IONBF, 0);				/* By default, Newlib library flush the I/O		*/
	setvbuf(stdout, NULL, _IONBF, 0);				/* buffer when full or when new-line			*/
	setvbuf(stderr, NULL, _IONBF, 0);				/* Done before OSstart() to have all tasks with	*/
													/* the same stdio set-up						*/
/* ------------------------------------------------ */
/* Start Abassi	/ mAbassi							*/

	OSstart();										/* Start Abassi / mAbassi						*/

	SysCallInit();									/* Initialize the System Call layer				*/

	OSintOn();										/* Enable the interrupts						*/

	AdamEve = TSKmyID();							/* This demo needs to know when A&E is running	*/

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
	uart_init(UART_DEV, BAUDRATE, 8, 0, 10, UART_Q_SIZE_RX, UART_Q_SIZE_TX, UART_FILT_OUT_LF_CRLF);

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
/* Application set-up								*/
													/* Need to create all services before allowing	*/
  #if ((USE_MAILBOX) != 0)							/* any task to run								*/
	MbxDisp                  = MBXopen("Disp", MBX_DISP_SIZE);
  #endif
	Mutex                    = MTXopen("Mutex");
	SemaAE                   = SEMopen("Sem A&E"); 

	TskDsc[TSK_HIGH].Task    = TSKcreate("HI   ", 2, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRA].Task     = TSKcreate("RRA  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRB].Task     = TSKcreate("RRB  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRC].Task     = TSKcreate("RRC  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_STRVA].Task   = TSKcreate("STRVA", 7, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_STRVB].Task   = TSKcreate("STRVB", 9, TC_TSK_STACK, &Fct, 0);

  #if ((OX_N_CORE) > 2)
	TskDsc[TSK_RRD].Task     = TSKcreate("RRD  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRE].Task     = TSKcreate("RRE  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRF].Task     = TSKcreate("RRF  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_RRG].Task     = TSKcreate("RRG  ", 4, TC_TSK_STACK, &Fct, 0);
	TskDsc[TSK_STRVC].Task   = TSKcreate("STRVC", 9, TC_TSK_STACK, &Fct, 0);
   #if ((OX_PRIO_CHANGE) != 0)
	TSKsetPrio(TskDsc[TSK_STRVB].Task, 8);
   #endif
  #endif

  #if ((OX_ROUND_ROBIN) < 0)
   #if ((OX_N_CORE) <= 2)
	TSKsetRR(TskDsc[TSK_HIGH].Task,  4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRA].Task,   4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRB].Task,   6*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRC].Task,   8*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_STRVA].Task, 2*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_STRVB].Task, 2*OS_TICK_PER_SEC);
   #else
	TSKsetRR(TskDsc[TSK_HIGH].Task,  4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRA].Task,   4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRB].Task,   4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRC].Task,   4*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRD].Task,   5*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRE].Task,   6*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRF].Task,   7*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_RRG].Task,   8*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_STRVA].Task, 9*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_STRVB].Task, 7*OS_TICK_PER_SEC);
	TSKsetRR(TskDsc[TSK_STRVC].Task, 5*OS_TICK_PER_SEC);
   #endif
  #endif

  #if ((OX_STARVE_RUN_MAX) < 0)
	TSKsetStrvRunMax(TskDsc[TSK_HIGH].Task,    4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRA].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRB].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRC].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_STRVA].Task,   5*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_STRVB].Task,   5*OS_TICK_PER_SEC);
   #if ((OX_N_CORE) > 2)
	TSKsetStrvRunMax(TskDsc[TSK_RRD].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRE].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRF].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_RRG].Task,     4*OS_TICK_PER_SEC);
	TSKsetStrvRunMax(TskDsc[TSK_STRVC].Task,   5*OS_TICK_PER_SEC);
   #endif
  #endif

  #if ((OX_STARVE_WAIT_MAX) < 0)
	TSKsetStrvWaitMax(TskDsc[TSK_HIGH].Task,  (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRA].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRB].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRC].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_STRVA].Task, (25*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_STRVB].Task, (10*OS_TICK_PER_SEC)/10);
   #if ((OX_N_CORE) > 2)
 	TSKsetStrvWaitMax(TskDsc[TSK_RRD].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRE].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRF].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_RRG].Task,   (50*OS_TICK_PER_SEC)/10);
	TSKsetStrvWaitMax(TskDsc[TSK_STRVC].Task, (50*OS_TICK_PER_SEC)/10);
   #endif
  #endif

  #if ((OX_STARVE_PRIO) < 0)
	TSKsetStrvPrio(TskDsc[TSK_HIGH].Task,  3);
	TSKsetStrvPrio(TskDsc[TSK_RRA].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_RRB].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_RRC].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_STRVA].Task, 3);
	TSKsetStrvPrio(TskDsc[TSK_STRVB].Task, 3);
   #if ((OX_N_CORE) > 2)
	TSKsetStrvPrio(TskDsc[TSK_RRD].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_RRE].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_RRF].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_RRG].Task,   3);
	TSKsetStrvPrio(TskDsc[TSK_STRVC].Task, 3);
   #endif
  #endif

  #if ((OX_MP_TYPE) & 4U) && ((OX_N_CORE) != 1)
	TSKsetCore(TskDsc[TSK_HIGH].Task, 0);
	TSKsetCore(TskDsc[TSK_RRA].Task,  1);
   #if ((OX_N_CORE) > 2)
	TSKsetCore(TskDsc[TSK_RRB].Task,  2);
   #endif
   #if ((OX_N_CORE) > 3)
	TSKsetCore(TskDsc[TSK_RRC].Task,  3);
   #endif
  #endif

	for (ii=0 ; ii<sizeof(TskDsc)/sizeof(TskDsc[0]) ; ii++) {
		TSKsetArg(TskDsc[ii].Task, &TskDsc[ii]);
	  #if ((USE_MAILBOX) != 0)
		TskDsc[ii].Mailbox  = MBXopen(NULL, 32);
	  #endif
		TskDsc[ii].CPUcnt   = 0;
		TskDsc[ii].Running  = 0;
		TskDsc[ii].WasReset = 0;
		TskDsc[ii].WillBlk  = 0;
		TskDsc[ii].WillMtx  = 0;
		TSKresume(TskDsc[ii].Task);
	}

/* ------------------------------------------------ */
/* Here we are										*/

	g_AppRdy = 1;									/* Inform the timer callback everything is OK	*/

	printf("\033[H\033[J");
  #if defined(__ABASSI_H__)
	puts("\n    Abassi Demo 3\n");
  #else
	puts("\n     mAbassi Demo\n");
  #endif

  #if ((OX_N_CORE) <= 2)
	puts("High  00.0s  P2     ");
	puts("RR-A  00.0s  P4     ");
	puts("RR-B  00.0s  P4     ");
	puts("RR-C  00.0s  P4     ");
	puts("ST-A  00.0s  P7     ");
	puts("ST-B  00.0s  P9     ");
  #else
	puts("High  00.0s  P2     ");
	puts("RR-A  00.0s  P4     ");
	puts("RR-B  00.0s  P4     ");
	puts("RR-C  00.0s  P4     ");
	puts("RR-D  00.0s  P4     ");
	puts("RR-E  00.0s  P4     ");
	puts("RR-F  00.0s  P4     ");
	puts("RR-G  00.0s  P4     ");
	puts("ST-A  00.0s  P7     ");
	puts("ST-B  00.0s  P9     ");
	puts("ST-C  00.0s  P9     ");
  #endif

	g_CmdIdx = 0;
	Toggle   = 0;
  #if ((OX_N_CORE) >= 2)
	g_DispLock = 0;
  #endif

	CURSOR_START();
  #if defined(__ABASSI_H__)
	printf("Abassi Demo > ");						/* Prompt on Serial for the command line		*/
  #else
	printf("mAbassi Demo> ");						/* Prompt on Serial for the command line		*/
  #endif

	for (;;) {

		while(0 != SEMwait(SemaAE, -1));			/* while only useful in eval version			*/

		if (++Toggle >= RTC_REFRESH_RATE) { 		/* RTC is not always refreshed					*/
			Toggle = 0;
			String = &Display[LAST_DISP][0];		/* Display line to update						*/
													/* RTC display is up to 99:59:59 so				*/
			IsrState = OSintOff();					/* roll back 00:00:00 when max reached			*/
			if (G_OStimCnt >= (360000L*(long)OS_TICK_PER_SEC)) {
				G_OStimCnt -= (360000L*(long)OS_TICK_PER_SEC);
			}
			OSintBack(IsrState);
			jj = G_OStimCnt;

			kk = jj/(OS_TICK_PER_SEC*3600*10);		/* MS digit for the hours						*/
			String[0] = '0'+kk;
			jj -= kk*(OS_TICK_PER_SEC*3600*10);

			kk = jj/(OS_TICK_PER_SEC*3600);			/* LS digits for the hours						*/
			String[1] = '0'+kk;
			jj -= kk*(OS_TICK_PER_SEC*3600);

			kk = jj/(OS_TICK_PER_SEC*600);			/* MS digits for the minutes					*/
			String[3] = '0'+kk;
			jj -= kk*(OS_TICK_PER_SEC*600);

			kk = jj/(OS_TICK_PER_SEC*60);			/* LS digits for the minutes					*/
			String[4] = '0'+kk;
			jj -= kk*(OS_TICK_PER_SEC*60);

			kk = jj/(OS_TICK_PER_SEC*10);			/* MS digits for the seconds					*/
			String[6] = '0'+kk;
			jj -= kk*(OS_TICK_PER_SEC*10);

			kk = jj/(OS_TICK_PER_SEC);				/* LS digits for the seconds					*/
			String[7] = '0'+kk;

			DispInfo[g_DispIdx].Str = String;		/* Send the string to the running task			*/
			DispInfo[g_DispIdx].X   = (LAST_DISP)+5;
			DispInfo[g_DispIdx].Y   = 7;
		  #if ((USE_MAILBOX) != 0)
			MBXput(MbxDisp, (intptr_t)&DispInfo[g_DispIdx], 0);
		  #endif
			if (++g_DispIdx >= (sizeof(DispInfo)/sizeof(DispInfo[0]))) {
				g_DispIdx = 0;
			}
		}

		for (ii=0 ; ii<sizeof(TskDsc)/sizeof(TskDsc[0]) ; ii++) {	/* Update all tasks status		*/
			String = &Display[ii][0];				/* Display line to update						*/
			jj = (TskDsc[ii].CPUcnt*100)/(OS_TICK_PER_SEC);
			while (jj > 9999) {						/* Can't display more than 99.9 (is in /10s)	*/
				jj                -= 10000;			/* we'll roll over the CPU count				*/
				TskDsc[ii].CPUcnt -= 100
				                   * OS_TICK_PER_SEC;
			}

			kk = jj/1000;							/* Running time MS digit						*/
			String[0] = '0'+kk;
			jj = jj - (kk*1000);

			kk = jj/100;							/* Running time LS digit						*/
			String[1] = '0'+kk;
			jj = jj - (kk*100);

			kk = jj/10;								/* Running time fraction						*/
			String[3] = '0'+kk;

			if (TSKisSusp(TskDsc[ii].Task)) {		/* If the task is suspended, the priority		*/
				String[7] = 'S';					/* is replaced by "S "							*/
				String[8] = ' ';
			}
			else {									/* Regular priority field						*/
				String[7] = 'P';
				if (TskDsc[ii].Task->SuspRqst != 0) {
					String[8] = 'S';				/* When the task will be suspended, indicate so	*/
				}
				else {								/* Priority numerical value						*/
					String[8] = '0'+TSKgetPrio(TskDsc[ii].Task);
				}
			}
			if (TskDsc[ii].Running != 0) {			/* If this is the running task, indicate so		*/
			  #if ((OX_N_CORE) == 1) || (((OX_MP_TYPE)&6U) == 0)
				String[11] = 'R';					/* 'R' instead of Core #0 when single core		*/
			  #else
				String[11] = '0'+(TskDsc[ii].Running-1);
			  #endif
			}
			else {									/* Not a running task, nothing in this field	*/
				String[11] = ' ';
			}

			if (TskDsc[ii].Task == g_StrvList) {	/* If the task is under starvation protection	*/
				String[12] = 'A';					/* Show the "A"									*/
			}
			else {									/* Not under starvation protection, nothing to	*/
				String[12] = ' ';					/* shown in this field							*/
			}

			if (TskDsc[ii].Task == MTXowner(Mutex)) {
				String[13] = 'M';					/* If this task locks the mutex, show it		*/
			}
			else if (TskDsc[ii].Task->Blocker == Mutex) {
				String[13] = 'B';					/* If the task is blocked by the mutex, show it	*/
			}
			else if (TskDsc[ii].WillMtx  != 0) {
				String[13] = 'm';					/* If the task will lock the mutex, show it		*/
			}
			else if (TskDsc[ii].WillBlk  != 0) {
				String[13] = 'b';					/* if the task will block on the mutex, show it	*/
			}
			else {
				String[13] = ' ';					/* If the task not involved with the mutex, 	*/
			}										/* nothing to show in this field				*/

			String[14] = ' ';
		  #if ((OX_MTX_INVERSION) != 0)
			if ((TSKgetPrio(TskDsc[ii].Task) != TskDsc[ii].Task->OriPrio)) {
				if ((TskDsc[ii].Task == MTXowner(Mutex))
				&&  (Mutex->Blocked != (TSK_t *)NULL)) {
					if (TSKgetPrio(Mutex->Blocked) == TSKgetPrio(TskDsc[ii].Task)) {
						String[14] = 'I';
					}
				}
			}
		  #endif
			DispInfo[g_DispIdx].Str = String;
			DispInfo[g_DispIdx].X   = 4+ii;
			DispInfo[g_DispIdx].Y   = 7;
													/* Send the string to display to running task	*/
		  #if ((USE_MAILBOX) != 0)
			MBXput(MbxDisp, (intptr_t)&DispInfo[g_DispIdx], 0);
		  #endif
			if (++g_DispIdx >= (sizeof(DispInfo)/sizeof(DispInfo[0]))) {
				g_DispIdx = 0;
			}
		}

		while (0 != (ccc = GetKey())) {				/* Check for a new key pressed on the keyboard 	*/
		  #if ((OX_N_CORE) < 2)
			IsrState = OSintOff();					/* When A&E is printing, don't clash with other	*/
		  #else
			do {
				GotLock  = 0;						/* Assume I didn;t get the lock					*/
				IsrState = OSintOff();				/* When A&E is printing, don't clash with other	*/
				CoreID   = COREgetID()+1;			/* This is the value to use for my spinlock		*/
				SPINlock();							/* Get exclusive access							*/
				if (g_DispLock == 0) {				/* Only grab when the lock has been released	*/
					g_DispLock = CoreID;
					GotLock    = 1;					/* Yeap, got the lock							*/
				}
				SPINunlock();						/* Relinquish exclusive access					*/
				if (GotLock == 0) {
					OSintBack(IsrState);			/* Didn't get lock, re-enable ISRs for a bit	*/
				}
			} while (GotLock == 0);					/* Retry if didn;t get the lock					*/
		  #endif

			putchar(ccc);
			if (g_CmdIdx == 0) {					/* tasks										*/
				for (ii=0 ; ii<sizeof(Cmd) ; ii++) {
					putchar(' ');
				}
			}
			CURSOR_NEXT(g_CmdIdx);

			if (g_CmdIdx < (sizeof(Cmd)/sizeof(Cmd[0]))-2) {
				putchar(ccc);						/* A key was pressed , echo it					*/
			}										/* If command line buffer full, don't echo		*/

			if (ccc == '\r') {						/* Need to remap CR to '\n'						*/
				putchar('\n');						/* Kept in case uart driver filter not set		*/
			}
			else if (ccc == '\b') {					/* Backspace pressed, to erase previous key		*/
				if (g_CmdIdx != 0) {
					g_CmdIdx--;
					putchar(' ');
					putchar('\b');
				}
			}
			else {
				if (g_CmdIdx == 0) {				/* Add to the command line without overflow		*/
					CURSOR_ADD();
					for (ii=1 ; ii<sizeof(Cmd) ; ii++) {
						putchar(' ');
					}
					CURSOR_ADD();
				}
				if (g_CmdIdx < (sizeof(Cmd)/sizeof(Cmd[0]))-2) {
					Cmd[g_CmdIdx] = toupper(ccc);
					g_CmdIdx++;
				}
			}

			if ((ccc == '\r')						/* CR pressed, then process the command			*/
			||  (ccc == '\n')) {
				Cmd[g_CmdIdx] = '\0';
				TaskIdx = -1;
				RdIdx   = 0;
				while ((Cmd[RdIdx] == ' ')			/* Skip start of line white spaces				*/
				||     (Cmd[RdIdx] == ',')
				||     (Cmd[RdIdx] == '\t')) {
					RdIdx++;
				}

				Done  = 0;
				Error = 0;

				if (g_CmdIdx == RdIdx) {			/* When everything white space, nothing to do	*/
					Done = 1;
				}

				g_CmdIdx = 0;

				CURSOR_START();

				if ((0 == strncmp(&Cmd[RdIdx], "?", 1))
				||  (0 == strncmp(&Cmd[RdIdx], "INFO", 4))) {
					putchar('\n');
					putchar('\n');
													/* Has the lock and A&E cannot be pre-empted	*/
					for (ii=0 ; ii<sizeof(TskDsc)/sizeof(TskDsc[0]) ; ii++) {
					 #if ((OX_PERF_MON) == 0)
						printf("%s  -->  ",  TskDsc[ii].Task->TskName);
					  #if ((OX_ROUND_ROBIN) != 0)
						printf("RR %3d", TSKgetRR(TskDsc[ii].Task));
					  #endif
					  #if ((OX_STARVE_RUN_MAX) != 0)
					   #if ((OX_ROUND_ROBIN) == 0)
						printf("SP %1d", TSKgetStrvPrio(TskDsc[ii].Task));
					   #else
						printf("  |  SP %1d", TSKgetStrvPrio(TskDsc[ii].Task));
					   #endif
						printf("  |  SR %3d", TSKgetStrvRunMax(TskDsc[ii].Task));
						printf("  |  SW %3d", TSKgetStrvWaitMax(TskDsc[ii].Task));
					  #endif
					  #if ((OX_MTX_INVERSION) != 0)
						printf("  |  P %d      \n",  TskDsc[ii].Task->OriPrio);
					  #elif ((OX_ROUND_ROBIN) != 0)
						printf("  |  P %d      \n",  TskDsc[ii].Task->Prio);
					  #else
						printf("  P %d      \n",  TskDsc[ii].Task->Prio);
					  #endif
					 #else
						for (jj=0 ; jj<23 ; jj++) {
							putchar('-');
						}
						puts("--------------------------------------------------------------------");
						printf("******* %s  COUNTERS:   SEMBLK:%u / MUTEXBLK:%u ",
						       TskDsc[ii].Task->TskName,
						       (unsigned int)TskDsc[ii].Task->PMsemBlkCnt,
						       (unsigned int)TskDsc[ii].Task->PMmtxBlkCnt
						       );
						printf("/ STARVRUN:%u / INVERT:%u\n",
						       (unsigned int)TskDsc[ii].Task->PMstrvCnt,
						       (unsigned int)TskDsc[ii].Task->PMinvertCnt
						       );
					  #if ((OX_N_CORE) >= 2)
						printf("Core    %20lld",
						(long long int)TskDsc[ii].Task->PMcoreRun[0]);
						for (jj=1 ; jj<OX_N_CORE ; jj++) {
						  printf(" %20lld", (long long int)TskDsc[ii].Task->PMcoreRun[jj]);
						}
					  #endif
						printf("\n");
						printf("latent  %20lld %20lld %20lld %20lld\n",
						       (long long int)TskDsc[ii].Task->PMlatentLast,
						       (long long int)TskDsc[ii].Task->PMlatentMin,
						       (long long int)TskDsc[ii].Task->PMlatentMax,
						       (long long int)TskDsc[ii].Task->PMlatentAvg
						       );
						printf("alive   %20lld %20lld %20lld %20lld\n",
						       (long long int)TskDsc[ii].Task->PMaliveLast,
						       (long long int)TskDsc[ii].Task->PMaliveMin,
						       (long long int)TskDsc[ii].Task->PMaliveMax,
						       (long long int)TskDsc[ii].Task->PMaliveAvg
						       );
						printf("run     %20lld %20lld %20lld %20lld      %u\n",
						       (long long int)TskDsc[ii].Task->PMrunLast,
						       (long long int)TskDsc[ii].Task->PMrunMin,
						       (long long int)TskDsc[ii].Task->PMrunMax,
						       (long long int)TskDsc[ii].Task->PMrunAvg,
						        (unsigned int)TskDsc[ii].Task->PMblkCnt
						       );
						printf("preem   %20lld %20lld %20lld %20lld      %u\n",
						       (long long int)TskDsc[ii].Task->PMpreemLast,
						       (long long int)TskDsc[ii].Task->PMpreemMin,
						       (long long int)TskDsc[ii].Task->PMpreemMax,
						       (long long int)TskDsc[ii].Task->PMpreemAvg,
						        (unsigned int)TskDsc[ii].Task->PMpreemCnt
						       );
					  #endif
					}

					putchar('\n');
					Done = 1;
					CURSOR_BACK();

				}

				if (0 == strncmp(&Cmd[RdIdx], "HI", 2)) {
					TaskIdx = TSK_HIGH;
					RdIdx += 2;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRA", 3)) {
					TaskIdx = TSK_RRA;
					RdIdx += 3;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRB", 3)) {
					TaskIdx = TSK_RRB;
					RdIdx += 3;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRC", 3)) {
					TaskIdx = TSK_RRC;
					RdIdx += 3;
				}
			  #if ((OX_N_CORE) > 2)
				if (0 == strncmp(&Cmd[RdIdx], "RRD", 3)) {
					TaskIdx = TSK_RRD;
					RdIdx += 3;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRE", 3)) {
					TaskIdx = TSK_RRE;
					RdIdx += 3;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRF", 3)) {
					TaskIdx = TSK_RRF;
					RdIdx += 3;
				}
				if (0 == strncmp(&Cmd[RdIdx], "RRG", 3)) {
					TaskIdx = TSK_RRG;
					RdIdx += 3;
				}
			  #endif
				if (0 == strncmp(&Cmd[RdIdx], "STRVA", 5)) {
					TaskIdx = TSK_STRVA;
					RdIdx += 5;
				}
				if (0 == strncmp(&Cmd[RdIdx], "STRVB", 5)) {
					TaskIdx = TSK_STRVB;
					RdIdx += 5;
				}
			  #if ((OX_N_CORE) > 2)
				if (0 == strncmp(&Cmd[RdIdx], "STRVC", 5)) {
					TaskIdx = TSK_STRVC;
					RdIdx += 5;
				}
			  #endif
				if (Done == 0) {
					if (TaskIdx < 0) {
						CURSOR_BACK();				/* Set cursor after Abassi Demo >				*/
						ii = printf("Unrecognized task name");
						for( ; ii<sizeof(Cmd) ; ii++) {
							putchar(' ');
						}
						Error=1;
					}
				}

				Op = -1;							/* 2nd token if the service, check all			*/
				if ((Error == 0)
				&&  (Done == 0)) {
					while ((Cmd[RdIdx] == ' ')		/* Skip white spaces between 1st and 2nd token	*/
					||     (Cmd[RdIdx] == ',')
					||     (Cmd[RdIdx] == '\t')) {
						RdIdx++;
					}
					if (0 == strncmp(&Cmd[RdIdx], "RR", 2)) {
						Op = OP_ROUND_ROBIN;		/* Token names are re-mapped to operations #	*/
						RdIdx += 2;
					}
					if (0 == strncmp(&Cmd[RdIdx], "SR", 2)) {
						Op = OP_STARVE_RUN;
						RdIdx += 2;
					}
					if (0 == strncmp(&Cmd[RdIdx], "SP", 2)) {
						Op = OP_STARVE_PRIO;
						RdIdx += 2;
					}
					if (0 == strncmp(&Cmd[RdIdx], "SW", 2)) {
						Op = OP_STARVE_WAIT;
						RdIdx += 2;
					}
					if (0 == strncmp(&Cmd[RdIdx], "P", 1)) {
						Op = OP_PRIO;
						RdIdx += 1;
					}
				  #if ((OX_PERF_MON) != 0)
					if (0 == strncmp(&Cmd[RdIdx], "0", 1)) {
						PMrestart(TskDsc[TaskIdx].Task);
						Op     = 0;
						RdIdx += 1;
						Done   = 1;
					}
				  #endif
					if (Op < 0) {
						Error = 1;
						CURSOR_BACK();				/* Set cursor after Abassi Demo >				*/
						ii = printf("Unrecognized property        ");
						for( ; ii<sizeof(Cmd) ; ii++) {
							putchar(' ');
						}
					}
				}

				if ((Error == 0)
				&&  (Done == 0)) {
					while ((Cmd[RdIdx] == ' ')		/* Skip white spaces between 2nd and 3rd token	*/
					||     (Cmd[RdIdx] == ',')
					||     (Cmd[RdIdx] == '\t')) {
						RdIdx++;
					}
					ii = 0;
					if (Cmd[RdIdx] == '\0') {
						Error = 1;
						CURSOR_BACK();				/* Set cursor after Abassi Demo >				*/
						ii = printf("Invalid number");
						for( ; ii<sizeof(Cmd) ; ii++) {
							putchar(' ');
						}
					}
					do {							/* 3rd token is the service numerical value		*/
						if ((Cmd[RdIdx] >= '0')
						&&  (Cmd[RdIdx] <= '9')) {
							ii = ii*10;
							ii += Cmd[RdIdx] - '0';
							RdIdx ++;
						}
						else if ((Cmd[RdIdx] == '\0')
						     ||  (Cmd[RdIdx] == '\r')
						     ||  (Cmd[RdIdx] == '\n')) {
							Done = 1;
						}
						else {
							Error = 1;
							CURSOR_BACK();			/* Set cursor after Abassi Demo >				*/
							ii = printf("Invalid number");
							for( ; ii<sizeof(Cmd) ; ii++) {
								putchar(' ');
							}
						}
					} while ((Error == 0)
					  &&     (Done  == 0));

					if (Error == 0) {				/* When everything OK, set the numerical value	*/
						switch(Op) {				/* to the service in the task					*/
						case OP_ROUND_ROBIN:
						  #if ((OX_ROUND_ROBIN) < 0)
							TSKsetRR(TskDsc[TaskIdx].Task, ii);
						  #endif
							break;
						case OP_STARVE_PRIO:
						  #if ((OX_STARVE_PRIO) < 0)
							TSKsetStrvPrio(TskDsc[TaskIdx].Task, ii);
						  #endif
							break;
						case OP_STARVE_RUN:
						  #if ((OX_STARVE_RUN_MAX) < 0)
							TSKsetStrvRunMax(TskDsc[TaskIdx].Task, ii);
						  #endif
							break;
						case OP_STARVE_WAIT:
						  #if ((OX_STARVE_WAIT_MAX) < 0)
							TSKsetStrvWaitMax(TskDsc[TaskIdx].Task, ii);
						  #endif
							break;
						default:
						  #if ((OX_PRIO_CHANGE) != 0)
							TSKsetPrio(TskDsc[TaskIdx].Task, ii);
						  #endif
							break;
						}
					}
				}
				if (Error == 0) {
					CURSOR_START();					/* Set cursor at beginning of line				*/
				  #ifdef __ABASSI_H__
					printf("Abassi Demo > ");
				  #else
					printf("mAbassi Demo> ");
				  #endif
					for (ii=0 ; ii<sizeof(Cmd) ; ii++) {
						putchar(' ');
					}
					CURSOR_BACK();
				}
				else {
					CURSOR_NEXT(g_CmdIdx);
				}
			}
		  #if ((OX_N_CORE) >= 2)
			g_DispLock = 0;
		  #endif
			OSintBack(IsrState);
		}
	}
}

/* ------------------------------------------------------------------------------------------------ */
/*																									*/
/* Tasks function																					*/
/*																									*/
/* All tasks use the same function.																	*/
/* The Task Argument is the custom task description data structure									*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

void Fct(void)
{
DispInfo_t *Disp;									/* Information to send to UpdateDisp()			*/
int         ii;										/* General purpose								*/
intptr_t    Msg;									/* Message received from timer tick call back	*/
int         NoNewMsg;								/* If there are no new messages					*/
TskDsc_t   *MyInfo;									/* All the information about my task			*/
#if ((OX_N_CORE) >= 2)
  int CoreID;
  int GotLock;
#endif

  #if ((OX_LIB_REENT_PROTECT) != 0)					/* In case the re-entrance is set on per task	*/
	setvbuf(stdin,  NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
  #endif

	MyInfo = (TskDsc_t *)TSKgetArg();				/* Get the argument for this task				*/

	for ( ; ; ) {
	  #if ((USE_EVENTS) != 0)						/* Info from ISR state machine comes from 		*/
		NoNewMsg = EVTwait(0x0000, 0xFFFF, 0);		/* either an event flag or a mailbox			*/
		Msg      = EVTget();
	  #else
		NoNewMsg = MBXget(MyInfo->Mailbox, &Msg, 0);
	  #endif

		if (NoNewMsg == 0) {						/* When a valid sync trigger was received		*/
			if ((int)Msg & OP_ZERO_CNT) {			/* Request to zero the running time				*/
				MyInfo->CPUcnt   = 0;
				MyInfo->WasReset = 0;
			}

			if (((int)Msg & OP_LOCK_MTX)			/* Request from ISR to lock the mutex			*/
			&&  (TSKmyID() != MTXowner(Mutex))) {	/* Safety in case state machine has issues		*/
				MyInfo->WillBlk = 0;
				MyInfo->WillMtx = 0;
				MTXlock(Mutex, -1);
			}

			if (((int)Msg & OP_UNLOCK_MTX)			/* Request from ISR to unlock the mutex			*/
			&&  (TSKmyID() == MTXowner(Mutex))) {	/* Safety in case state machine has issues		*/
				MTXunlock(Mutex);
			}
		}

	  #if ((OX_N_CORE) < 2)	
		ii = OSintOff();							/* Will print, don't want to be pre-empted		*/
	  #else											/* Done outside if() to not have pending write	*/
		do {
			GotLock = 0;							/* Assume I didn;t get the lock					*/
			ii      = OSintOff();					/* When A&E is printing, don't clash with other	*/
			CoreID  = COREgetID()+1;				/* This is the value to use for my spinlock		*/
			SPINlock();								/* Get exclusive access							*/
			if (g_DispLock == 0) {					/* Only grab when the lock has been released	*/
				g_DispLock = CoreID;
				GotLock    = 1;						/* Yeap, got the lock							*/
			}
			SPINunlock();							/* Relinquish exclusive access					*/
			if (GotLock == 0) {
				OSintBack(ii);						/* Didn't get lock, re-enable ISRs for a bit	*/
			}
		} while (GotLock == 0);						/* Retry if didn;t get the lock					*/
	  #endif

	  #if ((USE_MAILBOX) != 0)
		if (0 ==  MBXget(MbxDisp, &Msg, 0)) {		/* Check if A&E has sent a string to display	*/
			Disp = (void *)Msg;
			UpdateDisp(Disp);
		}
	  #else
		if (g_DispIdx != g_DispRdIdx) {
			Disp = &DispInfo[g_DispRdIdx];
			if (++g_DispRdIdx >= (sizeof(DispInfo)/sizeof(DispInfo[0]))) {
				g_DispRdIdx = 0;
			}
			UpdateDisp(Disp);
		}

	  #endif
	  #if ((OX_N_CORE) >= 2)
		g_DispLock = 0;
	  #endif
		OSintBack(ii);
	}
}

/* ------------------------------------------------------------------------------------------------ */
/*																									*/
/* RTOS Timer call back function																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if ((USE_EVENTS) != 0)
	#define SEND_INFO(_Idx, _Info)	EVTset(TskDsc[_Idx].Task, _Info)
#else
	#define SEND_INFO(_Idx, _Info)	MBXput(TskDsc[_Idx].Mailbox, _Info, 0)
#endif

void TIMcallBack(void)
{
int    AErunning;
int    ii;
int    jj;
int    RunIdx[OX_N_CORE];
static int AEcnt     =  0;
static int MtxBlk    =  1;
static int MtxCnt    =  0;
static int MtxOwn    =  5;
static int MtxSelect =  5;
static int MtxState  =  0;
static int SuspCnt   =  0;

	if (g_AppRdy == 0) {							/* If the configuration not done, skip			*/
		return;
	}

	for (ii=0 ; ii<OX_N_CORE ; ii++) {				/* To remove "uninitialized warning"			*/
		RunIdx[ii] = 0;
	}

	AEcnt++;										/* Check if A&E is running as we can't update	*/
	AErunning = 0;									/* everything when it does						*/
	for (ii=0 ; ii<OX_N_CORE ; ii++) {
	  #ifdef __ABASSI_H__
		if (G_OStaskNow == AdamEve) {
	  #else
		if (G_OStaskNow[ii] == AdamEve) {
	  #endif
			AErunning = 1;
		}
	}
													/* Update the CPU of the tasks					*/
	if (AErunning == 0) {							/* When A&E is running, don't update			*/
		for (ii=0 ; ii<OX_N_CORE ; ii++) {			/* Invalidate running status of all tasks		*/
			RunIdx[ii] = -1;						/* RunIdx holds the running task indexes		*/
		}
		for (ii=0 ; ii<sizeof(TskDsc)/sizeof(TskDsc[0]) ; ii++) {
			TskDsc[ii].Running = 0;
			for (jj=0 ; jj<OX_N_CORE ; jj++) {		/* Check if is one of the current one running	*/
			  #ifdef __ABASSI_H__
				if (G_OStaskNow == TskDsc[ii].Task) {
			  #else
				if (G_OStaskNow[jj] == TskDsc[ii].Task) {
			  #endif
					TskDsc[ii].Running  = jj+1;		/* +1 to show the core #						*/
					RunIdx[jj] = ii;
				}
			}
			if (TskDsc[ii].Task->Blocker != (SEM_t *)NULL) {
				if (TskDsc[ii].WasReset == 0) {
					if (0 == SEND_INFO(ii, OP_ZERO_CNT)) {
						TskDsc[ii].WasReset = 1;
					}
				}
			}
		 #if ((OX_ROUND_ROBIN) != 0)
		  #if ((OX_ROUND_ROBIN) < 0)
			if (TskDsc[ii].Task->RRmax != 0U)
		  #endif
			 {
				if (TskDsc[ii].Task->RRcnt <= 1U) {
					if (TskDsc[ii].WasReset == 0) {
						if (0 == SEND_INFO(ii, OP_ZERO_CNT)) {
							TskDsc[ii].WasReset = 1;
						}
					}
				}
			}
		  #endif
		}
	}

	for (ii=0 ; ii<sizeof(TskDsc)/sizeof(TskDsc[0]) ; ii++) {	/* Excluding A&E & Core Idles		*/
		if (TskDsc[ii].Running != 0) {				/* It is supposed to be running, check if so	*/
			TskDsc[ii].CPUcnt++;					/* Can only display up to 99.9 s, roll over		*/
			if (TskDsc[ii].CPUcnt >= (100*OS_TICK_PER_SEC)) {
				SEND_INFO(ii, OP_ZERO_CNT);
			}
		}
	}

	if (AErunning == 0) {
		switch(MtxState) {
													/* -------------------------------------------- */
		case 0:										/* State #0 : new mutex owner					*/
			ii = MtxOwn;
			jj = MtxSelect;
			if (++MtxSelect >= sizeof(TskDsc)/sizeof(TskDsc[0])) {
				MtxSelect = 0;
			}
			if (MtxSelect == 2) {					/* RRB if not the first locker of the mutex		*/
				MtxSelect = 4;
			}

			MtxOwn = MtxSelect;
													/* Tell new owner to lock the mutex				*/
			if (0 != SEND_INFO(MtxOwn, OP_LOCK_MTX)) {
				MtxOwn    = ii;
				MtxSelect = jj;
			}
			else {
				if (++MtxBlk >= LAST_MTX) {			/* Select the task that will block				*/
					MtxBlk = 0;						/* Only Hi & RR can block on the mutex			*/
				}

				if (MtxBlk == MtxOwn) {				/* Get another task to block on the mutex		*/
					if (++MtxBlk >= LAST_MTX) {
						MtxBlk = 0;
					}
				}
				TskDsc[MtxOwn].WillMtx = 1;			/* Display it will lock the mutex soon			*/
				TskDsc[MtxBlk].WillBlk = 1;			/* And this will be blocked 					*/
				MtxState++;
			}
			break;
													/* -------------------------------------------- */
		case 1:										/* Waiting for the owner to lock the mutex		*/
			if (TskDsc[MtxOwn].Task == MTXowner(Mutex)) {
				MtxState++;
			}
			break;
													/* -------------------------------------------- */
		case 2:										/* OK, the mutex owner is locking the mutex		*/
			if (0 == SEND_INFO(MtxBlk, OP_LOCK_MTX)) {	/* Tell the task to be blocked to lock it	*/
				MtxState++;
			}
			break;
													/* -------------------------------------------- */
		case 3:										/* Make sure the task gets blocked				*/
		  #if (((OX_N_CORE) == 1) || ((OX_MP_TYPE & 1) == 0))/* To be blocked may not run			*/
			if (TskDsc[MtxOwn].Task->Prio == TskDsc[MtxBlk].Task->Prio) {	/* if non-packed		*/
				if (TSKisBlk(TskDsc[MtxBlk].Task)) {/* OK, the task is blocked on the mutex now		*/
					MtxState++;						/* This may take a while as it needs to run		*/
				}
			}
			else {									/* If to be suspended, must skip a few states	*/
				if (TskDsc[MtxOwn].Task->SuspRqst != 0) {	/* as != prio can't run at same time	*/
					TskDsc[MtxOwn].WillMtx = 0;
					TskDsc[MtxBlk].WillBlk   = 0;
					TskDsc[MtxBlk].WillMtx   = 1;
					MtxState++;
				}
			}
		  #elif ((OX_MP_TYPE) == 5)
			if ((TskDsc[MtxOwn].Task->TargetCore == TskDsc[MtxBlk].Task->TargetCore)
			&&  (TskDsc[MtxOwn].Task->TargetCore >= 0)) {
				if (TskDsc[MtxOwn].Task->SuspRqst != 0) {
					TskDsc[MtxOwn].WillMtx = 0;
					TskDsc[MtxBlk].WillBlk = 0;
					TskDsc[MtxBlk].WillMtx = 1;
					MtxState++;
				}
			}
			else {									/* If to be suspended, must skip a few states	*/
				if (TSKisBlk(TskDsc[MtxBlk].Task)) {/* OK, the task is blocked on the mutex now		*/
					MtxState++;						/* This may take a while as it needs to run		*/
				}
			}
		  #else
			if (TSKisBlk(TskDsc[MtxBlk].Task)) {	/* OK, the task is blocked on the mutex now		*/
				MtxState++;							/* This may take a while as it needs to run		*/
			}
		  #endif
			MtxCnt = 0;
			break;
													/* -------------------------------------------- */
		case 4:										/* Task is blocked on the mutex state			*/
			for (ii=0 ; ii<OX_N_CORE ; ii++) {
				if (MtxOwn == RunIdx[ii]) {			/* Make sure the owner is running long enough	*/
					jj = (MtxOwn == 0)				/* When HI task, is getting suspended			*/
					   ? (((15*OS_TICK_PER_SEC)+1) - TskDsc[0].CPUcnt)
					   :   ((5*OS_TICK_PER_SEC)+1);
					if (++MtxCnt > jj) {			/* Tell the owner to release the mutex			*/
						if (0 == SEND_INFO(MtxOwn, OP_UNLOCK_MTX)) {
							MtxState++;
						}
					}
				}
			}
			break;
													/* -------------------------------------------- */
		case 5:										/* Wait for owner to unlock the mutex state		*/
			if (TskDsc[MtxOwn].Task != MTXowner(Mutex)) {
				MtxOwn = MtxBlk;					/* The new owner is the one that was blocked	*/
				MtxState++;
			}
			break;
													/* -------------------------------------------- */
		case 6:										/* Wait for new owner to lock the mutex	state	*/
			if (TskDsc[MtxOwn].Task == MTXowner(Mutex)) {
				MtxState++;
				MtxCnt = 0;
			}
			break;
													/* -------------------------------------------- */
		case 7:										/* Task that was blocked mutex release state	*/
			for (ii=0 ; ii<OX_N_CORE ; ii++) {
				if (MtxOwn == RunIdx[ii]) {			/* Wait a while before telling to release		*/
					jj = (MtxOwn == 0)				/* When HI task, is getting suspended			*/
					   ? (((15*OS_TICK_PER_SEC)+1) - TskDsc[0].CPUcnt)
					   :   ((5*OS_TICK_PER_SEC)+1);
					if (++MtxCnt > jj) {			/* Tell the owner to release the mutex			*/
						if (0 == SEND_INFO(MtxOwn, OP_UNLOCK_MTX)) {
							MtxState++;
						}
					}
				}
			}
			break;
													/* -------------------------------------------- */
		default:									/* Wait for blocked to unlock state				*/
			if (TskDsc[MtxOwn].Task != MTXowner(Mutex)) {
				MtxState =  0;
				MtxOwn   = -1;
			}
			break;
  		}

		if ((AEcnt >= OS_TICK_PER_SEC/TSK_REFRESH_RATE)
		&&  (TSKisBlk(AdamEve))) {					/* Make sure A&E does not use all the CPU		*/
			SEMpost(SemaAE);						/* Tell A & E to refresh the display			*/
			AEcnt = 0;
		}
	}

	SuspCnt++;
	if (SuspCnt == 10*OS_TICK_PER_SEC+3) {			/* Suspend / resume the high priority			*/
		TSKsuspend(TskDsc[TSK_HIGH].Task);
	}
	else if (SuspCnt > 30*OS_TICK_PER_SEC+3) {
		TSKresume(TskDsc[TSK_HIGH].Task);
		SuspCnt = 0;
	}

	return;
}

/* ------------------------------------------------------------------------------------------------ */
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

void UpdateDisp(DispInfo_t *Disp)
{
int jj;

	if ((Disp->X == 0)								/* Cursor at top of screen						*/
	&&  (Disp->Y == 0)) {
//		printf("\033[H");
		putchar(0x1B);
		putchar('[');
		putchar('H');
	}
	else {
//		printf("\033[%d;%dH%s", Disp->X, Disp->Y, Disp->Str); 
		putchar(0x1B);
		putchar('[');
		if (Disp->X < 10) {
			putchar('0'+Disp->X);
		}
		else {
			putchar('0'+(Disp->X/10));
			putchar('0'+(Disp->X%10));
		}
		putchar(';');
		if (Disp->Y < 10) {
			putchar('0'+Disp->Y);
		}
		else {
			putchar('0'+(Disp->Y/10));
			putchar('0'+(Disp->Y%10));
		}
		putchar('H');

		for (jj=0 ; Disp->Str[jj] != '\0' ; jj++){
			putchar(Disp->Str[jj]);
		}
													/* Set cursor at the next typing location		*/
//		printf("\033[%02d;%02dH", LAST_DISP+9, g_CmdIdx+15);
		putchar(0x1B);
		putchar('[');
		putchar('0'+(((LAST_DISP)+9)/10));
		putchar('0'+(((LAST_DISP)+9)%10));
		putchar(';');
		jj = g_CmdIdx+15;
		if (jj < 10) {
			putchar('0'+jj);
		}
		else {
			putchar('0'+(jj/10));
			putchar('0'+(jj%10));
		}
		putchar('H');
	}

	return;
}

/* EOF */
