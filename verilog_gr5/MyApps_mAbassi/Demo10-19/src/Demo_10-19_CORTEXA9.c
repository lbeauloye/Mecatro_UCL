/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_10-19_CORTEXA9.c																*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the ARM Cortex A9															*/
/*				Demo #10 - Webserver using netconn with memory base file system						*/
/*				Demo #11 - Webserver using BSD sockets with memory base file system					*/
/*				Demo #12 - Webserver using netconn with SD/MMC FatFS								*/
/*				Demo #13 - Webserver using BSD sockets with SD/MMC FatFS							*/
/*				Demo #14 - Webserver using netconn with SD/MMC + SysCall and FatFS					*/
/*				Demo #15 - Webserver using BSD sockets with SD/MMC + SysCall and FatFS				*/
/*				Demo #16 - Webserver using netconn with SD/MMC + SysCall and FullFat				*/
/*				Demo #17 - Webserver using BSD sockets with SD/MMC + SysCall and FullFat			*/
/*				Demo #18 - Webserver using netconn with SD/MMC + SysCall and ueFAT					*/
/*				Demo #19 - Webserver using BSD sockets with SD/MMC + SysCall and ueFAT				*/
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
/*	$Revision: 1.63 $																				*/
/*	$Date: 2019/01/10 18:07:14 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if ((((OS_DEMO) < 10) || ((OS_DEMO) > 19)) && (((OS_DEMO) > -10) || ((OS_DEMO) < -19)))
	#error "Wrong OS_DEMO value" 
#endif

#include "WebApp.h"

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define DHCP_TASK_PRIO   	(OX_PRIO_MIN-2)			/* Priority of the DHCP task					*/
#define LED_TASK_PRIO    	(OX_PRIO_MIN-1)			/* Priority of the LED flashing task			*/

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

#if ((OX_EVENTS) == 0)
  volatile int G_WebEvents;
#endif

int G_UartDevIn  = UART_DEV;						/* Required by SysCall							*/
int G_UartDevOut = UART_DEV;						/* Done even if SysCall not used				*/
int G_UartDevErr = UART_DEV;

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void TaskLedFlash(void);

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(int argc, char *argv[]) 
{
#ifndef USE_STATIC_IP
  char TimeMsg[] = "    DHCP in 5s.   ";			/* Message line to refresh every seconds		*/
  int  WaitTime;									/* Elapsed time waiting for key pressed			*/
#endif
int  ii;											/* General purpose								*/
int  LastWait;										/* Last elapse time (to update display)			*/
#if ((I2C_IN_USE) != 0)
int  I2CdevToInit[5];								/* To only init a device once					*/
int  Speed[5];										/* Use smallest speed amongst same device		*/
int  Width[5];										/* Use smallest width amongst same device		*/
#endif

  #if ((OX_EVENTS) == 0)
	G_WebEvents = 0;
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

	putchar('\n');
	putchar('\n');

/* ------------------------------------------------ */
/* SD/MMC interrupt handler							*/

 #if ((((OS_DEMO <= -12) || ((OS_DEMO) >= 12))) && defined(SDMMC_DEV))

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
 #endif

/* ------------------------------------------------ */
/* Attach and enable the Ethernet interrupt handler	*/

  #if (0U == ((ETH_LIST_DEVICE) & (1U << (EMAC_DEV))))
	#error "Selected EMAC device is not in ETH_LIST_DEVICE"
  #endif

  #if (((ETH_LIST_DEVICE) & 0x01U) != 0U)
	OSisrInstall(EMAC0_INT, Emac0_IRQHandler);		/* Install the Ethernet interrupt for EMAC #0	*/
	GICenable(EMAC0_INT, 128, 0);					/* Enable the EMAC #0 interrupts (Level)		*/
  #endif

  #if (((ETH_LIST_DEVICE) & 0x02U) != 0U)
	OSisrInstall(EMAC1_INT, Emac1_IRQHandler);		/* Install the Ethernet interrupt for EMAC #1	*/
	GICenable(EMAC1_INT, 128, 0);					/* Enable the EMAC #1 interrupst (Level)		*/
  #endif

  #if (((ETH_LIST_DEVICE) & 0x04U) != 0U)
	OSisrInstall(EMAC2_INT, Emac2_IRQHandler);		/* Install the Ethernet interrupt for EMAC #2	*/
	GICenable(EMAC2_INT, 128, 0);					/* Enable the EMAC #2 interrupts (Level)		*/
  #endif

  #if (((ETH_LIST_DEVICE) & 0x08U) != 0U)
	OSisrInstall(EMAC3_INT, Emac3_IRQHandler);		/* Install the Ethernet interrupt for EMAC #2	*/
	GICenable(EMAC3_INT, 128, 0);					/* Enable the EMAC #3 interrupts (Level)		*/
  #endif

/* ------------------------------------------------ */
/* I2C initialization								*/

 #if (((I2C_IN_USE) != 0) && defined(I2C_DEV))

  #if (0U == ((I2C_LIST_DEVICE) & (1U << (I2C_DEV))))
	#error "Selected I2C device is not in I2C_LIST_DEVICE"
  #endif

	memset(&I2CdevToInit[0], 0, sizeof(I2CdevToInit));
	for (ii=0 ; ii<sizeof(Speed)/sizeof(Speed[0]) ; ii++) {
		Speed[ii] = 400000;
		Width[ii] = 10;
	}

  #if ((I2C_USE_ADC) != 0)
	if (Speed[I2C_ADC_DEVICE] > I2C_ADC_SPEED) {
		Speed[I2C_ADC_DEVICE] = I2C_ADC_SPEED;
	}
	if (Width[I2C_ADC_DEVICE] > I2C_ADC_WIDTH) {
		Width[I2C_ADC_DEVICE] = I2C_ADC_WIDTH;
	}
	I2CdevToInit[I2C_ADC_DEVICE] = 1;
  #endif

  #if ((I2C_USE_LCD) != 0)
	if (Speed[I2C_LCD_DEVICE] > I2C_LCD_SPEED) {
		Speed[I2C_LCD_DEVICE] = I2C_LCD_SPEED;
	}
	if (Width[I2C_LCD_DEVICE] > I2C_LCD_WIDTH) {
		Width[I2C_LCD_DEVICE] = I2C_LCD_WIDTH;
	}
	I2CdevToInit[I2C_LCD_DEVICE] = 1;
  #endif

  #if ((I2C_USE_MAX1619) != 0)
	if (Speed[I2C_MAX1619_DEVICE] > I2C_MAX1619_SPEED) {
		Speed[I2C_MAX1619_DEVICE] = I2C_MAX1619_SPEED;
	}
	if (Width[I2C_MAX1619_DEVICE] > I2C_MAX1619_WIDTH) {
		Width[I2C_MAX1619_DEVICE] = I2C_MAX1619_WIDTH;
	}
	I2CdevToInit[I2C_MAX1619_DEVICE] = 1;
  #endif

  #if ((I2C_USE_RTC) != 0)
	if (Speed[I2C_RTC_DEVICE] > I2C_RTC_SPEED) {
		Speed[I2C_RTC_DEVICE] = I2C_RTC_SPEED;
	}
	if (Width[I2C_RTC_DEVICE] > I2C_RTC_WIDTH) {
		Width[I2C_RTC_DEVICE] = I2C_RTC_WIDTH;
	}
	I2CdevToInit[I2C_RTC_DEVICE] = 1;
  #endif

  #if ((I2C_USE_XL345) != 0)
	if (Speed[I2C_XL345_DEVICE] > I2C_XL345_SPEED) {
		Speed[I2C_XL345_DEVICE] = I2C_XL345_SPEED;
	}
	if (Width[I2C_XL345_DEVICE] > I2C_XL345_WIDTH) {
		Width[I2C_XL345_DEVICE] = I2C_XL345_WIDTH;
	}
	I2CdevToInit[I2C_XL345_DEVICE] = 1;
  #endif

  #if (((I2C_LIST_DEVICE) & 0x01U) != 0U)
	if (I2CdevToInit[0] != 0) {
		OSisrInstall(I2C0_INT, &I2CintHndl_0);		/* Install the I2C driver interrupt (Edge)		*/
		GICenable(I2C0_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x02U) != 0U)
	if (I2CdevToInit[1] != 0) {
		OSisrInstall(I2C1_INT, &I2CintHndl_1);		/* Install the I2C driver interrupt (Edge)		*/
		GICenable(I2C1_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x04U) != 0U)
	if (I2CdevToInit[2] != 0) {
		OSisrInstall(I2C2_INT, &I2CintHndl_2);		/* Install the I2C driver interrupt (Edge)		*/
		GICenable(I2C2_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x08U) != 0U)
	if (I2CdevToInit[3] != 0) {
		OSisrInstall(I2C3_INT, &I2CintHndl_3);		/* Install the I2C driver interrupt (Edge)		*/
		GICenable(I2C3_INT, 128, 1);
	}
  #endif

  #if (((I2C_LIST_DEVICE) & 0x10U) != 0U)
	if (I2CdevToInit[4] != 0) {
		OSisrInstall(I2C4_INT, &I2CintHndl_4);		/* Install the I2C driver interrupt (Edge)		*/
		GICenable(I2C4_INT, 128, 1);
	}
  #endif

	for (ii=0 ; ii<sizeof(I2CdevToInit)/sizeof(I2CdevToInit[0]) ; ii++) {
		if (I2CdevToInit[ii] != 0) {
		    i2c_init(ii, Width[ii], Speed[ii]);
		}
	}

 #endif

/* ------------------------------------------------ */
/* Push button / LCD / LEDs / Switches set-up		*/
/* Are do nothing if non-existent					*/

	MY_BUTTON_INIT();
 	MY_LCD_INIT();
	MY_LED_INIT();
	MY_SWITCH_INIT();

/* ------------------------------------------------ */
/* Allow user selection static or dynamic address	*/

	printf("\n\n");
  #if ((MY_LCD_WIDTH) < 20)
	MY_LCD_STDOUT(0, "  Abassi Demo       ");
  #else
	MY_LCD_STDOUT(0, "    Abassi Demo       ");
  #endif

  #if ((MY_LCD_NMB_LINE) >= 10)
	MY_LCD_STDOUT(2, "  lwIP  Webserver   ");
   #if (((OS_DEMO)%2) != 0)
	MY_LCD_STDOUT(3, "  with BSD sockets  ");
   #else
	MY_LCD_STDOUT(3, "    with netconn    ");
   #endif
   #if (((OS_DEMO) < 12) && ((OS_DEMO) > -12))
	MY_LCD_STDOUT(4, "   using memory FS  ");
   #else
	MY_LCD_STDOUT(4, "  using uSD FAT FS  ");
   #endif
   #if ((((OS_DEMO) == 14) || (OS_DEMO) == 15) || ((OS_DEMO) == -14) || ((OS_DEMO) == -15))
    MY_LCD_STDOUT(5, "+SysCall with FatFS ");
   #endif
   #if ((((OS_DEMO) == 16) || (OS_DEMO) == 17) || ((OS_DEMO) == -16) || ((OS_DEMO) == -17))
    MY_LCD_STDOUT(5, "+SysCall with FullFAT");
   #endif
   #if ((((OS_DEMO) == 18) || (OS_DEMO) == 19) || ((OS_DEMO) == -18) || ((OS_DEMO) == -19))
    MY_LCD_STDOUT(5, "+SysCall with ueFAT");
   #endif
   #ifndef USE_STATIC_IP
	MY_LCD_STDOUT(6, "Will use DHCP unless");
	MY_LCD_STDOUT(7, "key pressed on UART ");
   #ifdef MY_BUTTON_0
	MY_LCD_STDOUT(8, " switch / joystick  ");
   #endif
	MY_LCD_LINE(9, &TimeMsg[0]);
   #endif
  #else												/* Not enough lines to display on the LCD		*/
	putchar('\n');
	puts("  lwIP  Webserver   ");
   #if (((OS_DEMO)%2) != 0)
	puts("  with BSD sockets  ");
   #else
	puts("    with netconn    ");
   #endif
   #if (((OS_DEMO) < 12) && ((OS_DEMO) > -12))
	puts("   using memory FS  ");
   #else
	puts("  using uSD FAT FS    ");
   #endif
   #if ((((OS_DEMO) == 14) || (OS_DEMO) == 15) || ((OS_DEMO) == -14) || ((OS_DEMO) == -15))
    puts("+SysCall with FatFS   ");
   #endif
   #if ((((OS_DEMO) == 16) || (OS_DEMO) == 17) || ((OS_DEMO) == -16) || ((OS_DEMO) == -17))
    puts("+SysCall with FullFAT   ");
   #endif
   #if ((((OS_DEMO) == 18) || (OS_DEMO) == 19) || ((OS_DEMO) == -18) || ((OS_DEMO) == -19))
    puts("+SysCall with ueFAT   ");
   #endif
   #ifndef USE_STATIC_IP
	puts("\nWill use DHCP unless");
	puts("key pressed on UART ");
    #ifdef MY_BUTTON_0
	puts(" or button pressed  ");
    #endif
   #endif
  #endif

/* ------------------------------------------------ */
/* Default static Addresses & Mask					*/

	NetAddr_Init();

/* ------------------------------------------------ */
/* UART prompt and key pressed check				*/

  #ifdef USE_STATIC_IP
	ii            = 1;								/* To remove possible unused variable warning	*/
	G_IPnetStatic = ii;
  #else
	WaitTime = G_OStimCnt;
	LastWait = -1;
	do {
		ii = (1000*(G_OStimCnt-WaitTime))
		   / OS_TICK_PER_SEC;
		if (LastWait != (5999-ii)/1000) {
			LastWait = (5999-ii)/1000;
			TimeMsg[12] = '0' + LastWait;
		  #if ((MY_LCD_NMB_LINE) >= 10)
			MY_LCD_LINE(8, "key/sw is static");
		   #if ((MY_LCD_WIDTH) < 20)
			MY_LCD_LINE(9, &TimeMsg[2]);
		   #else
			MY_LCD_LINE(9, &TimeMsg[0]);
		   #endif
		  #else
			MY_LCD_LINE(0, "key/sw is static");
		   #if ((MY_LCD_WIDTH) < 20)
			MY_LCD_LINE(1, &TimeMsg[2]);
		   #else
			MY_LCD_LINE(1, &TimeMsg[0]);
		   #endif
		  #endif
			printf("\r%s", &TimeMsg[0]);
		}

		G_IPnetStatic = GetKey()					/* Check if user pressed a key					*/
		            #ifdef MY_BUTTON_0
		              | (0 == BUT_GET(MY_BUTTON_0))
		            #endif
		            #ifdef MY_BUTTON_1
		              | (0 == BUT_GET(MY_BUTTON_1))
		            #endif
		            #ifdef MY_BUTTON_2
		              | (0 == BUT_GET(MY_BUTTON_2))
		            #endif
		            #ifdef MY_BUTTON_3
		              | (0 == BUT_GET(MY_BUTTON_3))
		            #endif
		            #ifdef MY_BUTTON_4
		              | (0 == BUT_GET(MY_BUTTON_4))
		            #endif
		            #ifdef MY_BUTTON_5
		              | (0 == BUT_GET(MY_BUTTON_5))
		            #endif
		            #ifdef MY_BUTTON_6
		              | (0 == BUT_GET(MY_BUTTON_6))
		            #endif
		            #ifdef MY_BUTTON_7
		              | (0 == BUT_GET(MY_BUTTON_7))
		            #endif
		            #ifdef MY_BUTTON_8
		              | (0 == BUT_GET(MY_BUTTON_8))
		            #endif
		            #ifdef MY_BUTTON_9
		              | (0 == BUT_GET(MY_BUTTON_9))
		            #endif
		              ;
	} while((ii < 5000)								/* Check for 5 seconds							*/
	  &&    (G_IPnetStatic == 0));					/* Or until a key is pressed					*/

	printf("\r                             \n");	/* Erase the remaining time from UART screen	*/

	if (G_IPnetStatic != 0) {
		printf("Key / Button pressed, using static address\n\n");
	}

  #endif

/* ------------------------------------------------ */
/* EMAC & lwIP initialization						*/

	puts("Initializing Ethernet I/F");
  #if ((MY_LCD_NMB_LINE) >= 10)
	MY_LCD_CLRLINE(8);
	MY_LCD_CLRLINE(9);
	MY_LCD_LINE(8, "Waiting link up");
  #else
	MY_LCD_CLRLINE(0);
	MY_LCD_CLRLINE(1);
	MY_LCD_LINE(0, "Waiting link up");
  #endif

	LastWait = G_OStimCnt;

	LwIP_Init();

	if ((G_OStimCnt-LastWait) > (30*OS_TICK_PER_SEC)) {
		MTXLOCK_STDIO();
		printf("The Ethernet I/F set-up seems to have taken a long time\n");
		printf("Is there a cable on the \"HPS Ethernet\" RJ-45 connector?\n\n");
		MTXUNLOCK_STDIO();
	}

	http_server_init();

/* ------------------------------------------------ */
/* DHCP task creation								*/

	if (G_IPnetStatic == 0) {
		MTXLOCK_STDIO();
		puts("\nWaiting for Ethernet I/F & Webserver & file system to be ready\n");
		MTXUNLOCK_STDIO();
		TSKcreate("DHCP Client", DHCP_TASK_PRIO, 16384, LwIP_DHCP_task, 1);
	}
	else {
		MTXLOCK_STDIO();
		puts("\nWaiting for the Webserver & file system to be ready\n");
		MTXUNLOCK_STDIO();
	}

	TSKcreate("LED Flash", LED_TASK_PRIO, 8192, TaskLedFlash, 1);

  #if ((OX_EVENTS) != 0)
	EVTwait(3, 0, -1);								/* Wait for Ethernet & file system to be ready	*/
  #else												/* DHCP prints when ready with IP Address		*/
	while((G_WebEvents & 3) != 3) {
		TSKsleep(OS_MS_TO_TICK(100));
	}
  #endif

	MTXLOCK_STDIO();
	puts("The Webserver is ready");
	MTXUNLOCK_STDIO();

  #if ((TEST_RXPERF) != 0)
    extern void IperfRXinit(void);
	IperfRXinit();
  #elif ((TEST_TXPERF) != 0)
    extern void IperfTXinit(void);
	IperfTXinit();
  #endif

  #if defined(USE_SHELL)
	TSKcreate("Shell", OX_PRIO_MIN, 16384, OSshell, 1);
  #endif

	for( ;; ) {
		TSKselfSusp();
	}
}

/* ------------------------------------------------------------------------------------------------ */

void TaskLedFlash(void)
{
int Toggle;

	Toggle = 0;
	for( ;; ) {
		if (Toggle == 0) {
		  #ifdef MY_LED_FLASH
			MY_LED_ON(MY_LED_FLASH);
		  #endif
		}
		else {
		  #ifdef MY_LED_FLASH
			MY_LED_OFF(MY_LED_FLASH);
		  #endif
		}
		Toggle = !Toggle;
		TSKsleep(OS_MS_TO_TICK(1250));
	}
}

/* ------------------------------------------------------------------------------------------------ */

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
