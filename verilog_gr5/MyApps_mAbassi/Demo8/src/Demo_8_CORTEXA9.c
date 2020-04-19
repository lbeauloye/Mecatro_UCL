/* ------------------------------------------------------------------------------------------------ */
/* FILE :		Demo_8_CORTEXA9.c																	*/
/*																									*/
/* CONTENTS :																						*/
/*				Demo for the Altera's Arria V, Cyclone V, and Arria 10 boards						*/
/*				Show the use of the I2C driver: LCD display and Real Time Clock 					*/
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
/*	$Revision: 1.49 $																				*/
/*	$Date: 2019/01/10 18:07:15 $																	*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if (((OS_DEMO) != 8) && ((OS_DEMO) != -8))
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

#if (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AAC5)
  #include "arm_pl330.h"
  #include "dw_i2c.h"
  #include "dw_uart.h"
#elif (((OS_PLATFORM) & 0x00FFFFFF) == 0x0000AA10)
  #include "arm_pl330.h"
  #include "dw_i2c.h"
  #include "dw_uart.h"
#elif ((((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)													\
  ||   (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007753))
  #include "cd_i2c.h"
  #include "cd_uart.h"
#endif

/* ------------------------------------------------------------------------------------------------ */
/* App definitions																					*/

#define NIBBLING(x)	( (((x)/10)<<4)      + ((x)%10) )
#define UNNIBBLE(x) (((((x)>>4)&0xF)*10) + ((x)&0xF))

/* ------------------------------------------------------------------------------------------------ */
/* Apps variables																					*/

int G_UartDevIn  = UART_DEV;						/* Required by SysCall							*/
int G_UartDevOut = UART_DEV;
int G_UartDevErr = UART_DEV;

#if ((I2C_USE_RTC) != 0)
static const char *RTCprompt[] = {
				"Second     (0-59) : ",
				"Minute     (0-59) : ",
				"Hour       (0-23) : ",
				"Date       (1-31) : ",
				"Month      (1-12) : ",
				"Year  (2000-2099) : "};
static const int RTCvalid[][2] = {
				{    0,   59},
				{    0,   59},
				{    0,   23},
				{    1,   31},
				{    1,   12},
				{ 2000, 2099}};

static const char DayOfWeek[7][4]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char MonthNames[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif

#ifdef DEMO_XTRA_VARS								/* Platform specific variables					*/
  DEMO_XTRA_VARS
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Apps functions																					*/

void CleanRead  (char *Str);
int  EEPROMtest (void);
int  GetKey     (void);								/* Non-blocking keyboard input					*/
int  LCDputs    (char msg[][128]);
int  LT2497read (int Address, int32_t *Data);
int  MAX1619init(void);
int  MAX1619read(double *TlocalC, double *TremoteC);
int  RTCrd      (int *Date);
int  RTCwrt     (int *Date);
int  XL345init  (void);
int  XL345read  (int *Xaix, int *Yaxis, int *Zaxis);

/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------------------------------ */

int main(void)
{
int  Err;
int  ii; 	                                        /* General purpose								*/
int  Speed[5];										/* Use smallest speed amongst same device		*/
int  Width[5];										/* Use smallest width amongst same device		*/
int  I2CdevToInit[5];								/* To only init a device once					*/
int  Key;											/* Key pressed for the command					*/
char Options[64];									/* When un-recognized key pressed, avail keys	*/
#if ((I2C_USE_ADC) != 0)
 #if (((I2C_ADC_TYPE)&0xFFFFFF) == 0x2497)
  int32_t Data[8];
 #endif
#endif
#if ((I2C_USE_RTC) != 0)
 int  Date[7];										/* Date : YYYY / MM / DD / HH / MM / SS / DAY	*/
#endif
#if ((I2C_USE_LCD) != 0)
 char Msg[2][128];									/* Message to write on the LCD					*/
#endif
#if ((I2C_USE_MAX1619) != 0)
 double TlocalC;
 double TremoteC;
#endif
#if ((I2C_USE_XL345) != 0)
 int Xaxis;
 int Yaxis;
 int Zaxis;
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

	OSintOn();										/* Enable the interrupts   						*/

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

	puts("\n\nmAbassi Demo #8\n\n");

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
/* I2C initialization								*/

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

  #if ((I2C_USE_EEPROM) != 0)
	if (Speed[I2C_EEPROM_DEVICE] > (I2C_EEPROM_SPEED)) {
		Speed[I2C_EEPROM_DEVICE] = (I2C_EEPROM_SPEED);
	}
	if (Width[I2C_EEPROM_DEVICE] > I2C_EEPROM_WIDTH) {
		Width[I2C_EEPROM_DEVICE] = I2C_EEPROM_WIDTH;
	}
	I2CdevToInit[I2C_EEPROM_DEVICE] = 1;
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

  #if ((I2C_USE_RTC) != 0)
	if (Speed[I2C_RTC_DEVICE] > I2C_RTC_SPEED) {
		Speed[I2C_RTC_DEVICE] = I2C_RTC_SPEED;
	}
	if (Width[I2C_RTC_DEVICE] > I2C_RTC_WIDTH) {
		Width[I2C_RTC_DEVICE] = I2C_RTC_WIDTH;
	}
	I2CdevToInit[I2C_RTC_DEVICE] = 1;
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

  #if ((I2C_USE_XL345) != 0)
	if (Speed[I2C_XL345_DEVICE] > I2C_XL345_SPEED) {
		Speed[I2C_XL345_DEVICE] = I2C_XL345_SPEED;
	}
	if (Width[I2C_XL345_DEVICE] > I2C_XL345_WIDTH) {
		Width[I2C_XL345_DEVICE] = I2C_XL345_WIDTH;
	}
	I2CdevToInit[I2C_XL345_DEVICE] = 1;
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

/* ------------------------------------------------ */
/* DE0-nano / DE10-nano:							*/
/* GPIO selects SPI / I2C on LT connector			*/

  #if ((OS_PLATFORM) == 0x0100AAC5)

	DE0_SELECT_LT_I2C();

  #endif

/* ------------------------------------------------ */
/* Application set-up								*/

  #if ((I2C_USE_MAX1619) != 0)
	ii = MAX1619init();
	if (ii != 0) {
		puts("ERROR: cannot initialize the temperature sensor");
	}
  #endif

  #if ((I2C_USE_XL345) != 0)
	ii = XL345init();
	if (ii != 0) {
		puts("ERROR: cannot initialize ADXL345 accelerometer");
	}
  #endif

/* ------------------------------------------------ */
/* Prompt / key pressed check / processing loop		*/

	for (;;) {
		Options[0] = '\0';
		puts("\n");
	  #if ((I2C_USE_ADC) != 0)
		puts("a : read the ADC(s)");
		strcat(&Options[0], "'a' ");
	  #endif
	  #if ((I2C_USE_RTC) != 0)
		puts("d : read the Real Time Clock (RTC)");
		strcat(&Options[0], "'d' ");
	  #endif
	  #if ((I2C_USE_EEPROM) != 0)
		printf("e : EEPROM (#%d) access\n", I2C_EEPROM_TYPE);
	   #if (((OS_PLATFORM) == 0x0000AAC5)															\
		||  ((OS_PLATFORM) == 0x0100AAC5))
		printf("    Accessed on the LT connector\n");
	   #endif
		strcat(&Options[0], "'e' ");
	  #endif
	  #if ((I2C_USE_XL345) != 0)
		puts("g : read the accelerometer (G-Sensor)");
		strcat(&Options[0], "'g' ");
	  #endif
	  #if ((I2C_USE_LCD) != 0)
		puts("l : send a text to the LCD");
		strcat(&Options[0], "'l' ");
	  #endif
	  #if ((I2C_USE_RTC) != 0)
		puts("s : set the Real Time Clock (RTC)");
		strcat(&Options[0], "'s' ");
	  #endif
	  #if ((I2C_USE_MAX1619) != 0)
		puts("t : read the temperature sensor");
		strcat(&Options[0], "'t' ");
	  #endif
	  #if ((I2C_TEST_AVAIL) != 0)
		for (ii=0 ; ii<sizeof(I2CdevToInit)/sizeof(I2CdevToInit[0]) ; ii++) {
			if (I2CdevToInit[ii] != 0) {
		    	printf("%d : test the I2C bus #%d with square waves\n", ii, ii);
				strcat(&Options[0], "'");
				Err = strlen(Options);
				Options[Err]   = '0'+ii;
				Options[Err+1] = '\'';
				Options[Err+2] = ' ';
				Options[Err+3] = '\0';
			}
		}
	  #endif
		fputs("\nCommand ? ", stdout);
		do {
			Key = GetKey();
		} while (Key == 0);

		putchar('\n');
		putchar('\n');

		switch(Key) {
	  #if ((I2C_USE_ADC) != 0)
	   #if (((I2C_ADC_TYPE)&0xFFFFFF) == 0x2497)
		case 'a':
		case 'A':
		  #if (((I2C_ADC_TYPE)&0xFF000000) == 0x02000000)
			LT2497read(I2C_ADC_ADDR_0, &Data[0]);
			printf("PCI:      %6.1lfmA\n", 1000.0*0.15*(((double)Data[0])/4194304.0)/0.01);
			printf("FMA:      %6.1lfmA\n", 1000.0*0.15*(((double)Data[1])/4194304.0)/0.01);
			printf("FMB:      %6.1lfmA\n", 1000.0*0.15*(((double)Data[2])/4194304.0)/0.01);
			printf("FMAVAD:   %6.1lfmA\n", 1000.0*0.15*(((double)Data[3])/4194304.0)/0.001);
			printf("FMBVAD:   %6.1lfmA\n", 1000.0*0.15*(((double)Data[4])/4194304.0)/0.001);

			LT2497read(I2C_ADC_ADDR_1, &Data[0]);
			printf("2.5V:     %6.1lfmA\n", 1000.0*0.15*(((double)Data[0])/4194304.0)/0.001);
			printf("HLVDD:    %6.1lfmA\n", 1000.0*0.15*(((double)Data[1])/4194304.0)/0.001);
			printf("HLVDDQ:   %6.1lfmA\n", 1000.0*0.15*(((double)Data[2])/4194304.0)/0.001);
			printf("HPSVDD:   %6.1lfmA\n", 1000.0*0.15*(((double)Data[3])/4194304.0)/0.001);
			printf("1.8V:     %6.1lfmA\n", 1000.0*0.15*(((double)Data[4])/4194304.0)/0.00025);
			printf("1.0V:     %6.1lfmA\n", 1000.0*0.15*(((double)Data[5])/4194304.0)/0.00025);
			printf("0.95V:    %6.1lfmA\n", 1000.0*0.15*(((double)Data[6])/4194304.0)/0.001);
			printf("HPS0.95V: %6.1lfmA\n", 1000.0*0.15*(((double)Data[7])/4194304.0)/0.001);
		  #endif
			break;
	   #endif
	  #endif

  	  #if ((I2C_USE_EEPROM) != 0)
		case 'e':
		case 'E':
			EEPROMtest();
			break;
	  #endif

	  #if ((I2C_USE_XL345) != 0)
		case 'g':
		case 'G':
			printf("Press any key to stop the reading\n");
			do {
				ii = 0;
				do {
					Err = XL345read(&Xaxis, &Yaxis, &Zaxis);
					if (Err != 0) {
						puts("Error reading the I2C");
						ii++;
					}
				} while((Err != 0)
				  &&    (ii < 3));
				if (Err == 0) {
					printf("\r X=%5dmg     Y=%5dmg     Z=%5dmg    ", Xaxis, Yaxis, Zaxis);
				}
				Key = GetKey();
			} while (Key == 0);
			printf("\n");
			break;
	  #endif

	  #if ((I2C_USE_LCD) != 0)
		case 'l':
		case 'L':
			printf("Text to send to the LCD (line 1): ");
			gets(&Msg[0][0]);
			CleanRead(&Msg[0][0]);
			printf("Text to send to the LCD (line 2): ");
			gets(&Msg[1][0]);
			CleanRead(&Msg[1][0]);
			ii = 0;
			do {
				Err = LCDputs(Msg);
				if (Err != 0) {
					puts("Error writing on the I2C");
					ii++;
				}
			} while((Err != 0)
			  &&    (ii < 3));
			if (ii >= 3) {
				puts("Aborting the write");
			}
			break;
	  #endif
	  #if ((I2C_USE_RTC) != 0)
		case 'd':
		case 'D':
			ii = 0;
			do {
				Err = RTCrd(&Date[0]);
				if (Err != 0) {
					puts("Error reading from the I2C");
					ii++;
				}
			} while((Err != 0)
			  &&    (ii < 3));
			if (ii >= 3) {
				puts("Aborting the read");
			}
			else {
				printf("%s, %02d-%s-%04d  %02d:%02d:%02d\n", &DayOfWeek[Date[6]][0], Date[3], 
			            &MonthNames[Date[4]][0], Date[5], Date[2], Date[1], Date[0]);
			}
			break;
		case 's':
		case 'S':
			for (ii=5 ; ii>=0 ; ii--) {
				printf(RTCprompt[ii]);
				scanf("%d", &Date[ii]);
				if ((Date[ii] < RTCvalid[ii][0])
				||  (Date[ii] > RTCvalid[ii][1])) {
					printf("Value out of range : %d\n", Date[ii]);
					ii++;
				}
			}
			ii = 0;
			do {
				Err = RTCwrt(&Date[0]);
				if (Err != 0) {
					puts("Error writing on the I2C");
					ii++;
				}
			} while((Err != 0)
			  &&    (ii < 3));
			if (ii >= 3) {
				puts("Aborting the write");
			}
			break;
	  #endif
	  #if ((I2C_USE_MAX1619) != 0)
		case 't':
		case 'T':
			ii = 0;
			do {
				Err = MAX1619read(&TlocalC, &TremoteC);
				if (Err != 0) {
					puts("Error reading the I2C");
					ii++;
				}
			} while((Err != 0)
			  &&    (ii < 3));
			if (ii >= 3) {
				puts("Aborting the reading");
			}
			else {
				printf("Board: %3.1lfC\nFPGA:  %3.1lfC\n", TlocalC, TremoteC);
			}
			break;
	  #endif
	  #if ((I2C_TEST_AVAIL) != 0)
		case '0':
			if (I2CdevToInit[0] != 0) {
				puts("The test will last 1 minute");
				i2c_test(0);
			}
			break;
		case '1':
			if (I2CdevToInit[1] != 0) {
				puts("The test will last 1 minute");
				i2c_test(1);
			}
			break;
		case '2':
			if (I2CdevToInit[2] != 0) {
				puts("The test will last 1 minute");
				i2c_test(2);
			}
			break;
		case '3':
			if (I2CdevToInit[3] != 0) {
				puts("The test will last 1 minute");
				i2c_test(3);
			}
			break;
		case '4':
			if (I2CdevToInit[4] != 0) {
				puts("The test will last 1 minute");
				i2c_test(4);
			}
			break;
	  #endif
		default:
			printf("Please use %s\n", &Options[0]);
			break;
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
#if ((I2C_USE_LCD) != 0)

int LCDputs(char msg[][128])
{
int  ii;											/* General purpose								*/
int  jj;											/* General purpose								*/
int  RetVal;										/* Return value : bus error						*/
static const char LCDclr[]     = {0xFE, 0x51};		/* Clear display command						*/
static const char LCDlin[2][3] = {{0xFE, 0x45, 0x00},	/* Set cursor at beginning of line 1		*/
                                  {0xFE, 0x45, 0x40}};	/* Set cursor at beginning of line 2		*/

  #if ((I2C_LCD_TYPE) == 2163)
	RetVal = 0;
	RetVal |= i2c_send(I2C_LCD_DEVICE, I2C_LCD_ADDR, &LCDclr[0], 2);/* Clear the LCD				*/
	TSKsleep((OS_MS_TO_TICK(2)>2)?1+OS_MS_TO_TICK(2):2);	/* LCD very slow between transactions	*/
	for (ii=0 ; ii<2 ; ii++) {						/* Send both lines								*/
		jj = strlen(&msg[ii][0]);					/* Check if the string is empty					*/
		if (jj > 0) {								/* Non-empty string								*/
			RetVal |= i2c_send(I2C_LCD_DEVICE, I2C_LCD_ADDR, &LCDlin[ii][0], 3);/* Go to line		*/
			TSKsleep((OS_MS_TO_TICK(2)>2)?1+OS_MS_TO_TICK(2):2);
	    	RetVal |= i2c_send(I2C_LCD_DEVICE, I2C_LCD_ADDR, &msg[ii][0], jj);	/* String to display*/
			TSKsleep((OS_MS_TO_TICK(2)>2)?1+OS_MS_TO_TICK(2):2);
		}
	}
  #endif

	return(RetVal);
}
#endif

/* ------------------------------------------------------------------------------------------------ */
#if ((I2C_USE_ADC) != 0)
 #if (((I2C_ADC_TYPE)&0xFFFFFF) == 0x2497)

int LT2497read(int Address, int32_t *Data)
{
char Buffer[3];
int ii;
int RetVal;

	RetVal = 0;
	for (ii=0 ; ii<8 ; ii++) {
		Buffer[0] = 0xA0							/* 1 / 0 / EN=1 / SGL=0 / ODD=0 				*/
		          |  (char)(0x07 & ii);				/* A2 / A1 / A0									*/
		RetVal    |= i2c_send(I2C_ADC_DEVICE, Address, &Buffer[0], 1);

		TSKsleep(OS_MS_TO_TICK(175));				/* ADC takes around 150 ms to convert			*/
													/* Perform a Start / Read / Start conversion	*/
		RetVal   |= i2c_send_recv(I2C_ADC_DEVICE, Address, &Buffer[0], 1, &Buffer[0], 3);

		Data[ii]  = (((unsigned int)(Buffer[0]))<<16)
		          | (((unsigned int)(Buffer[1]))<<8)
		          |  ((unsigned int)(Buffer[2]));
		if (Data[ii] == 0x00C00000) {				/* Max +ve is > max 2's complement positive		*/
			Data[ii] -= 1;							/* Print it a single bit less positive			*/
		}
		Data[ii] ^= 0x00800000;						/* Sign is inverse of 2's complement			*/
		Data[ii] <<= 8;								/* Sign extend from 24 bits to 32 bits			*/
		Data[ii] >>= 8;
	}
	return(RetVal);
}
 #endif
#endif

/* ------------------------------------------------------------------------------------------------ */
/* NOTE:																							*/
/*		Microchip's 24LCXX EEPROMs need special handling when perfroming a write, either single		*/
/*		byte of a page write.  The EEPROMs don't ACK the last byte (before the stop) until the		*/
/*		write operation is completed.  The I2C standard indicates the slave should force the clock	*/
/*		low until it is ready, but that's not how Microchip's EEPROMs operate. All write operations	*/
/*		on these EEPROM report an error using i2c_send(), the error being a timeout to receive the	*/
/*		ACK.  One way to perfrom a write is to 1st do the write, ignoring the error reported by		*/
/*		i2c_send() and then loop a few time reading the written location and make sure the data		*/
/*		has been correctly written.  If after a few read iterations the data is not correct, the	*/
/*		process can be redone: write then loop reading.												*/
/*																									*/
/* ------------------------------------------------------------------------------------------------ */

#if ((I2C_USE_EEPROM) != 0)

int EEPROMtest(void)
{
int  Err;											/* Error tracking								*/
char Buf[300];										/* Work buffer									*/
char BufA[300];										/* Work buffer									*/
int  ii;											/* General purpose								*/
int  jj;											/* General purpose								*/
int  kk;											/* General purpose								*/
int  mm;											/* General purpose								*/
int  Naddr;											/* Number of address byte to use with the part	*/
int  Page;											/* Page size of the part						*/
int  Size;											/* Size to work with 128 for 2K mem, else 256	*/
int  Skip;											/* If skipping the write						*/

#if (((I2C_EEPROM_TYPE) == 2401) || ((I2C_EEPROM_TYPE) == 2402) || ((I2C_EEPROM_TYPE) == 24025))
  Naddr = 1;
#else
  Naddr = 2;
#endif

#if ((I2C_EEPROM_TYPE) == 2401)
  Page = 8;
  Size = 128;
#else
  Page = 16;
  Size = 256;
#endif

	printf("Reading blocks of 1 to %d bytes starting at address 0x0000\n", Size);

	Err  = 0;										/* -------------------------------------------- */
	for (kk=1 ; (kk<=Size) && (Err==0) ; kk++) {	/* Read 1 to Size bytes to check I2C recv caps	*/

		printf("Reading %3d bytes\r", kk);

		memset(&Buf[2], 0x55, sizeof(Buf)-2);
		Buf[0] = 0x00;								/* Set the address to 0							*/
		Buf[1] = 0x00;

	  #if (((OS_PLATFORM) & 0x00FFFFFF) == 0x00007020)	/* Zynq can only Xfer a max of 255			*/
		Err = i2c_send_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR,  &Buf[0], Naddr,
		                                                         &Buf[0], (kk>128) ? 128 : kk);
		if ((kk > 128) && (Err == 0)) {
			Err = i2c_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[128], kk-128);
		}
	  #else
		if ((kk & 1) == 0) {						/* Alternate between send_recv & send+recv		*/
			Err = i2c_send_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[0], Naddr, &Buf[0], kk);
		}
		else {
			Err = i2c_send(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[0], Naddr);
			Err = i2c_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[0], kk);
		}
	  #endif

		if (Err == 0) {								/* Check if previous data read is identical to	*/
			jj = 0;									/* the one from this read burst					*/
			for (ii=0 ; (ii<(kk-1)) && (Err==0) ; ii++) {
				if (BufA[ii] != Buf[ii]) {
					jj++;
				}
			}
			if (jj != 0) {
				printf("\nRetrying: %d Read mismatch from reading %d and %d bytes\n", jj, kk-1, kk);
				kk--;
				BufA[kk-1] = Buf[kk-1];				/* Copy in case 1st read is the wrong one		*/
			}
			else {
				memmove(&BufA[0], &Buf[0], sizeof(BufA));
			}
		}
		else {
			printf("\nI2C reported error %d for %d bytes\n", Err, kk);
		}
		if (kk == Size) {
			printf("\n");
		}

		TSKsleep(5);								/* Doing back to back transactions seems to be	*/
	}												/* causing lots of transfer errors with EEPROM	*/

	if (Err != 0) {
		printf("\nError sending/receiving\n");
	}

	mm   = 1;
	Skip = 1;
	if (Err == 0) {
		for (ii=0 ; ii<Size ; ii++) {				/* Decide if writing 0x00 -> 0xFF or if			*/
			if (Buf[ii] != ii) {					/* writing 0xFF -> 0x00							*/
				mm = 0;
			}
		}

		printf("\n\nThe first %d bytes in the EEPROM are:\n\n", Size);

		for(ii=0 ; ii<Size ; ii++ ){				/* Display what has been read					*/
			printf("%02X", Buf[ii]);
			printf("%c", ((ii&15) == 15) ? '\n' : ' ');
		}

		Skip = 0;									/* On socFPGA, skip on-board writing			*/
		if ((I2C_EEPROM_TYPE) == 2432) {
			if (((OS_PLATFORM) == 0x0000AAC5)		/* EEPROM on Cyclone V socFPGA board			*/
			||  ((OS_PLATFORM) == 0x4000AAC5)) {	/* EEPROM on Arria V socFPGA board				*/
				puts("\n\n **** Skipping write for socFPGA on-board EEPROM");
				Skip = 1;							/* Only reading it as used for EtherCAT IP	*/
			}
		}
	}

	if ((Err  == 0)									/* -------------------------------------------- */
	&&  (Skip == 0)) {								/* Write 1 to Page bytes to check I2C send caps	*/
		if (mm == 0) {
			printf("\nWriting values 0x00 -> 0x%02X\n", Size-1);
		}
		else {
			printf("\nWriting values 0xFF -> 0x%02X\n", 256-Size);
		}

		for (ii=0 ; (ii<Size) && (Err==0) ; ii+=Page) {	/* Set-up the data to send on the I2C bus	*/
			Buf[0]       = 0x00;					/* Set address: handle 1 or 2 address bytes		*/
			Buf[Naddr-1] = ii;
			for (jj=0 ; jj<Page ; jj++) {			/* Page size of data to write					*/
				Buf[jj+Naddr] =  (mm==0)			/* Select if data is 0x00-0xFF or 0xFF->0x00	*/
				              ?  (ii+jj)
				              : ~(ii+jj);
			}										/* Here we use i2c_send() & i2c_recv() to write	*/
													/* i2c_send_recv() was used when reading		*/
			for (kk=1 ; (kk<=Page) && (Err==0) ; kk++) {	/* Perform  write of 1 to Page bytes	*/

				printf("Writing 0x%02X at address 0x%02X (write of %2d bytes) ", Buf[Naddr], ii, kk);
													/* Must ignore the error (see note above)		*/
				i2c_send(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[0], Naddr+kk);

				TSKsleep(10);						/* Give time for the EEPROM to get programmed	*/

				for(jj=0 ; jj<1000 ; jj++) {		/* Try to read to know when done				*/
					Err = i2c_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &BufA[1], 1);
					if (Err == 0) {
						printf(" OK at try #%d          \r", jj);
						break;
					}
				}

				if (jj >= 1000) {
					printf(" *** failed ***             \n");
				}
			}
		}

		printf("\r%d bytes written in the EEPROM                                       \n\n", Size);

		memset(&BufA[0], 0xAA, sizeof(BufA));		/* 1 shot read of what was written				*/
		for (ii=0 ; (ii<Size) && (Err==0) ; ii+=16) {
			Buf[0]       = 0x00;
			Buf[Naddr-1] = ii;

			Err = i2c_send_recv(I2C_EEPROM_DEVICE, I2C_EEPROM_ADDR, &Buf[0], Naddr, &BufA[ii], 16);
			if (Err != 0) {
				printf("send/recv error\n");
			}
		}

		for (ii=0 ; (ii<Size) && (Err==0) ; ii++ ) {/* Display what has been read					*/
			printf("%02X", BufA[ii]);
			printf("%c", ((ii&15) == 15) ? '\n' : ' ');
		}	
	}

	if (Err != 0) {
		printf("Error value reported %d", Err);
	}
	printf("\n");

	return(Err);
}
#endif

/* ------------------------------------------------------------------------------------------------ */
#if ((I2C_USE_MAX1619) != 0)

int MAX1619init(void)
{
char Buffer[32];									/* Holds the data read/written from/to			*/
int  RetVal;										/* Return value : bus error						*/

	RetVal = 0;
	Buffer[0] = 0x09;								/* Write configuration byte (WCA) reg #0x09		*/
	Buffer[1] = 0x80;								/* Mask Alert interrupt							*/
	RetVal   |= i2c_send(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 2);

	Buffer[0] = 0x0A;								/* Write conversion rate byte (WCRW) reg #0x0A	*/
	Buffer[1] = 0x07;								/* Maximum conversion rate: 8 Hz				*/
	RetVal   |= i2c_send(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 2);

	Buffer[0] = 0x0E;								/* Write remote T low  (WCLN) reg #0x0E			*/
	Buffer[1] = 0x7F;								/* Set to lowest: not interested with Alters	*/
	RetVal   |= i2c_send(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 2);

	Buffer[0] = 0x0D;								/* Write remote T high (WCHA) reg #0x0D			*/
	Buffer[1] = 0x80;								/* Set to highest: not interested with Alters	*/
	RetVal   |= i2c_send(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 2);

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int MAX1619read(double *TlocalC, double *TremoteC)
{
char Buffer[32];									/* Holds the data read/written from/to			*/
int  RetVal;										/* Return value : bus error						*/
													/* Note MAC1619 can only read a single byte		*/
	RetVal = 0;

	Buffer[0] = 0x00;								/* Read local temperature (RLTS)				*/
	RetVal |= i2c_send_recv(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 1, &Buffer[0], 1);
	*TlocalC = (double)Buffer[0];

	Buffer[0] = 0x01;								/* Read remote temperature (RRTE)				*/
	RetVal |= i2c_send_recv(I2C_MAX1619_DEVICE, I2C_MAX1619_ADDR, &Buffer[0], 1, &Buffer[0], 1);
	*TremoteC = (double)Buffer[0];

	return(RetVal);
}
#endif

/* ------------------------------------------------------------------------------------------------ */
#if ((I2C_USE_RTC) != 0)

int RTCrd(int *Date)
{
char Buffer[32];									/* Holds the data read from the RTC				*/
int  RetVal;										/* Return value : bus error						*/

	RetVal = -1;
  #if ((I2C_RTC_TYPE) == 1339)
	Buffer[0] = 0;
	RetVal = i2c_send_recv(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 1, &Buffer[0], 7);
//	i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 1);
//	i2c_recv(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 7);

	Date[0] = (int)UNNIBBLE(Buffer[0]);				/* Seconds										*/
	Date[1] = (int)UNNIBBLE(Buffer[1]);				/* Minutes										*/
	Date[2] = (int)UNNIBBLE(Buffer[2]);				/* Hours										*/
	Date[6] = (int)UNNIBBLE(Buffer[3])-1;			/* Day of the Week (RTC is 1 -> 7)				*/
	Date[3] = (int)UNNIBBLE(Buffer[4]);				/* Date											*/
	Date[4] = (int)UNNIBBLE(Buffer[5])-1;			/* Month (RTC is 1 -> 12)						*/
	Date[5] = (int)UNNIBBLE(Buffer[6])+2000;		/* Year (RTC is 00 -> 99)						*/

  #elif ((I2C_RTC_TYPE) == 4182)
	Buffer[0]  = 0x0C;								/* Clear HT bit: is set when back from Battery	*/
	RetVal     = i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 1);
	RetVal    |= i2c_recv(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[1], 1);
	Buffer[1] &= 0xBF;
	RetVal    |= i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 2);

	Buffer[0] = 0x1;								/* Request to start reading from second reg		*/
	RetVal   |= i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 1);
	RetVal   |= i2c_recv(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 7);

	Buffer[0] &= 0x7F;								/* Remove the controls bits						*/
	Buffer[2] &= 0x3F;								/* Remove the Century bits						*/

	Date[0] = (int)UNNIBBLE(Buffer[0]);				/* Seconds										*/
	Date[1] = (int)UNNIBBLE(Buffer[1]);				/* Minutes										*/
	Date[2] = (int)UNNIBBLE(Buffer[2]);				/* Hours										*/
	Date[6] = (int)UNNIBBLE(Buffer[3])-1;			/* Day of the Week (RTC is 1 -> 7)				*/
	Date[3] = (int)UNNIBBLE(Buffer[4]);				/* Date											*/
	Date[4] = (int)UNNIBBLE(Buffer[5])-1;			/* Month (RTC is 1 -> 12)						*/
	Date[5] = (int)UNNIBBLE(Buffer[6])+2000;		/* Year (RTC is 00 -> 99)						*/
  #endif

	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int RTCwrt(int *Date)
{
char Buffer[32];									/* Holds the data to send to the RTC			*/
int  RetVal;										/* Return value : bus error						*/
int  d;
int  dw;
int  m;
int  y;

	RetVal = -1;

	y  = Date[5];									/* Compute what day of the week this is			*/
	m  = Date[4];
	d  = Date[3];
	d += (m < 3)
	   ? y--
	   : y-2;
	dw = ((23*m/9+d+4+y/4-y/100+y/400)%7);

	memset(&Buffer[0], 0, sizeof(Buffer));

  #if ((I2C_RTC_TYPE) == 1339)
	Buffer[1] = (char)NIBBLING(Date[0]);			/* Seconds										*/
	Buffer[2] = (char)NIBBLING(Date[1]);			/* Minutes										*/
	Buffer[3] = (char)NIBBLING(Date[2]);			/* Hours										*/
	Buffer[4] = (char)(dw+1);						/* Day of the Week (RTC is 1 -> 7)				*/
	Buffer[5] = (char)NIBBLING(Date[3]);			/* Date											*/
	Buffer[6] = (char)NIBBLING(Date[4]);			/* Month										*/
	Buffer[7] = (char)NIBBLING(Date[5]-2000);		/* RTC year 00 is 2000 (range 00 -> 99)			*/

	Buffer[0] = 0;									/* Address to start writing to					*/
	RetVal    = i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 8);

  #elif ((I2C_RTC_TYPE) == 4182)
	Buffer[1] = (char)NIBBLING(Date[0]);			/* Seconds										*/
	Buffer[2] = (char)NIBBLING(Date[1]);			/* Minutes										*/
	Buffer[3] = (char)NIBBLING(Date[2]);			/* Hours										*/
	Buffer[4] = (char)(dw+1);						/* Day of the Week (RTC is 1 -> 7)				*/
	Buffer[5] = (char)NIBBLING(Date[3]);			/* Date											*/
	Buffer[6] = (char)NIBBLING(Date[4]);			/* Month										*/
	Buffer[7] = (char)NIBBLING(Date[5]-2000);		/* RTC year 00 is 2000 (range 00 -> 99)			*/

	Buffer[0] = 0x01;								/* Address to start writing to					*/
	RetVal    = i2c_send(I2C_RTC_DEVICE, I2C_RTC_ADDR, &Buffer[0], 31);

  #endif

	return(RetVal);
}
#endif

/* ------------------------------------------------------------------------------------------------ */
#if ((I2C_USE_XL345) != 0)

int XL345init(void)
{
char Buffer[2];
int  RetVal;

	Buffer[0]  = XL345_REG_DATA_FORMAT;
	Buffer[1]  = (XL345_RANGE_2G)					/* +- 2G range, 10 bit resolution				*/
	           | (XL345_FULL_RESOLUTION);			/* This delivers results of 4mg per bit			*/
	RetVal     = i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

	Buffer[0]  = XL345_REG_BW_RATE;
	Buffer[1]  = XL345_RATE_12_5;					/* Conversion rate is 12.5 Hz (once every 80ms)	*/
	RetVal    |= i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

	Buffer[0]  = XL345_REG_INT_ENABLE;
	Buffer[1]  = XL345_DATAREADY;					/* Enable the interrupt for data ready			*/
	RetVal    |= i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

	Buffer[0]  = XL345_REG_POWER_CTL;
	Buffer[1]  = XL345_STANDBY;						/* Stop the measurements						*/
	RetVal    |= i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

 	return(RetVal);
}

/* ------------------------------------------------------------------------------------------------ */

int XL345read(int *Xaxis, int *Yaxis, int *Zaxis)
{
char Buffer[7];
int  RetVal;

	Buffer[0]  = XL345_REG_POWER_CTL;
	Buffer[1]  = XL345_MEASURE;						/* Start the measurements						*/
	RetVal     = i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

	TSKsleep(OS_MS_TO_TICK(100));					/* Wait for the measurement to be done			*/

	do {											/* Check to make sure is done					*/
		Buffer[0] = XL345_REG_INT_SOURCE;
		RetVal = i2c_send_recv(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 1, &Buffer[0], 1);
	} while (((XL345_DATAREADY & (int)Buffer[0]) == 0)
	  &&      (RetVal == 0));

	Buffer[0] = 0x32;								/* Read the data; *4 because 0.004g/bit			*/
	RetVal |= i2c_send_recv(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 1, &Buffer[0], 6);

	if (RetVal == 0) {
		*Xaxis = 4*((((int)(signed char)Buffer[1])<<8) + (0xFF & ((int)Buffer[0])));
		*Yaxis = 4*((((int)(signed char)Buffer[3])<<8) + (0xFF & ((int)Buffer[2])));
		*Zaxis = 4*((((int)(signed char)Buffer[5])<<8) + (0xFF & ((int)Buffer[4])));
	}

	Buffer[0]  = XL345_REG_POWER_CTL;
	Buffer[1]  = XL345_STANDBY;						/* Stop the measurements						*/
	RetVal |= i2c_send(I2C_XL345_DEVICE, I2C_XL345_ADDR, &Buffer[0], 2);

	return(RetVal);
}
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Needed for the interrupt call-back of GPIOs when target platform rerquires it					*/
/* ------------------------------------------------------------------------------------------------ */

void GPIOintHndl(int IOPin)
{
	return;
}

/* EOF */

