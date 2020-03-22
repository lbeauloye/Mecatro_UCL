/* ------------------------------------------------------------------------------------------------ */

#include "mAbassi.h"
#include "Platform.h"								/* Everything about the target processor is here*/
#include "HWinfo.h"									/* Everything about the tagret hardware is here	*/
#include "SysCall.h"								/* System Call layer stuff						*/

#include "alt_gpio.h"
#include "dw_ethernet.h"
#include "dw_i2c.h"
#include "dw_sdmmc.h"
#include "dw_uart.h"
#include "arm_pl330.h"

#include "ethernetif.h"

/* ------------------------------------------------------------------------------------------------ */
/* App variables																					*/

int G_UartDevIn  = UART_DEV;						/* Needed by the system call layer if used		*/
int G_UartDevOut = UART_DEV;						/* Needed by the system call layer if used		*/
int G_UartDevErr = UART_DEV;						/* Needed by the system call layer if used		*/

/* ------------------------------------------------------------------------------------------------ */
/* App functions																					*/

extern void Task_HPS_Led(void);
extern void Task_FPGA_Led(void);
extern void Task_FPGA_Button(void);
extern void Task_FatFS(void);

void DoInitLwIP(void);

/* ------------------------------------------------------------------------------------------------ */

int main(void)
{
    TSK_t *Task;
    int ii;

	setvbuf(stdin,  NULL, _IONBF, 0);				/* By default, Newlib library flush the I/O		*/
	setvbuf(stdout, NULL, _IONBF, 0);				/* buffer when full or when new-line			*/
	setvbuf(stderr, NULL, _IONBF, 0);

/* ------------------------------------------------ */
/* Start uAbassi									*/

	OSstart();										/* Start uAbassi								*/

	TSKsetCore(TSKmyID(), 0);						/* Make sure App_0 will always run on Core #0	*/

	SysCallInit();									/* Initialize the System Call layer				*/

	OSintOn();										/* Enable the interrupts						*/

/* ------------------------------------------------ */
/* SysTick Set-up									*/
													/* Timer tick needed for Sleep() & timeouts		*/
	OSisrInstall(OS_TICK_INT, &TIMtick);			/* Install the RTOS timer tick function			*/
	TIMERinit(((((OS_TICK_SRC_FREQ/OS_TICK_PRESCL)/100000)*(OX_TIMER_US))/(10)));
	GICenable(OS_TICK_INT, 128, 1);					/* Priority == mid / Edge trigger				*/

/* ------------------------------------------------ */
/* UART set-up										*/
/* 8 bits, No Parity, 1 Stop						*/
/* Cyclone V has 2 UART                             */

    uart_init(UART_DEV, BAUDRATE, 8, 0, 10, UART_Q_SIZE_RX, UART_Q_SIZE_TX, UART_FILT_OUT_LF_CRLF
              | UART_FILT_IN_CR_LF
              | UART_ECHO
              | UART_ECHO_BS_EXPAND
              | UART_FILT_EOL_CR
              | UART_FILT_EOF_CTRLD);

	OSisrInstall(UART0_INT, &UARTintHndl_0);
	GICenable(-UART0_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
    
	OSisrInstall(UART1_INT, &UARTintHndl_1);
	GICenable(-UART1_INT, 128, 0);					/* UART Must use level and not edge trigger		*/
    
/* ------------------------------------------------ */
/* DMA set-up                                       */
    
    ii = dma_init(0);
    if (ii != 0) {
        printf("ERROR : dma_init() returned %d\n", ii);
    }
    
    OSisrInstall(DMA0_INT, &DMAintHndl_0);
    GICenable(DMA0_INT, 128, 0);
    
    OSisrInstall(DMA1_INT, &DMAintHndl_1);
    GICenable(DMA1_INT, 128, 0);
    
    OSisrInstall(DMA2_INT, &DMAintHndl_2);
    GICenable(DMA2_INT, 128, 0);
    
    OSisrInstall(DMA3_INT, &DMAintHndl_3);
    GICenable(DMA3_INT, 128, 0);
    
    OSisrInstall(DMA4_INT, &DMAintHndl_4);
    GICenable(DMA4_INT, 128, 0);
    
    OSisrInstall(DMA5_INT, &DMAintHndl_5);
    GICenable(DMA5_INT, 128, 0);
    
    OSisrInstall(DMA6_INT, &DMAintHndl_6);
    GICenable(DMA6_INT, 128, 0);
    
    OSisrInstall(DMA7_INT, &DMAintHndl_7);
    GICenable(DMA7_INT, 128, 0);

/* ------------------------------------------------ */
/* SD/MMC interrupt handler                         */
    
    OSisrInstall(SDMMC0_INT, MMCintHndl_0);         /* The SD/MMC driver, mmc_init(), is not called    */
    GICenable(SDMMC0_INT, 128, 0);                  /* because FatFS deals with the initialization    */

/* ------------------------------------------------ */
/* Ethernet interrupt handler	                    */

	OSisrInstall(EMAC0_INT, Emac0_IRQHandler);		/* Install the Ethernet interrupt for EMAC #0	*/
	GICenable(EMAC0_INT, 128, 0);					/* Enable the EMAC #0 interrupts (Level)		*/

	OSisrInstall(EMAC1_INT, Emac1_IRQHandler);		/* Install the Ethernet interrupt for EMAC #1	*/
	GICenable(EMAC1_INT, 128, 0);					/* Enable the EMAC #1 interrupst (Level)		*/

/* ------------------------------------------------ */
/* I2C initialization								*/

    int  I2CdevToInit[5];                           /* To only init a device once                    */
    int  Speed[5];                                  /* Use smallest speed amongst same device        */
    int  Width[5];                                  /* Use smallest width amongst same device        */

	memset(&I2CdevToInit[0], 0, sizeof(I2CdevToInit));
	for (ii=0 ; ii<sizeof(Speed)/sizeof(Speed[0]) ; ii++) {
		Speed[ii] = 400000;
		Width[ii] = 10;
	}

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
    
/* ------------------------------------------------ */
/* Application set-up								*/

    Task = TSKcreate("Task HPS Led", 4, 8192, &Task_HPS_Led, 0);
    TSKsetCore(Task, 0);                            /* Create new task, will always run on core #1    */
    TSKresume(Task);                                /* when BMP (G_OS_MP_TYPE == 4 or 5)            */
    
	Task = TSKcreate("Task FPGA Led", 0, 8192, &Task_FPGA_Led, 0);
	TSKsetCore(Task, 1);							/* Create new task, will always run on core #1	*/
	TSKresume(Task);								/* when BMP (G_OS_MP_TYPE == 4 or 5)			*/

	Task = TSKcreate("Task FPGA Button", 0, 8192, &Task_FPGA_Button, 0);
	TSKsetCore(Task, 1);							/* Create new task, will always run on core #1	*/
	TSKresume(Task);								/* when BMP (G_OS_MP_TYPE == 4 or 5)			*/
  
    Task = TSKcreate("Task FatFS", 5, 16384, &Task_FatFS, 0);
    TSKsetCore(Task, 1);
    TSKresume(Task);
    
#if defined(USE_SHELL)
    TSKcreate("Shell", OX_PRIO_MIN, 16384, OSshell, 1);
    TSKsleep(OS_MS_TO_TICK(100));                   /* Make sure Shell Task initialization if over  */
#endif

	TSKsetCore(TSKmyID(), 0);						/* Make sure Task_HPS_Led always run on Core #0	*/
    
    DoInitLwIP();
    
    for( ;; ) {
        TSKselfSusp();
    }
}

/* ------------------------------------------------------------------------------------------------ */
/* Not used but needed to build uAbassi																*/

void TIMcallBack(void)
{
	return;
}

/* EOF */
