/* ------------------------------------------------------------------------------------------------ */

#include "mAbassi.h"
#include "Platform.h"								/* Everything about the target processor is here*/
#include "HWinfo.h"									/* Everything about the tagret hardware is here	*/
#include "SysCall.h"								/* System Call layer stuff						*/

#include "arm_pl330.h"
#include "dw_i2c.h"
#include "cd_qspi.h"
#include "dw_sdmmc.h"
#include "dw_spi.h"
#include "dw_uart.h"
#include "alt_gpio.h"

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
extern void Task_MTL1(void);
extern void Task_DisplayFile(void);
extern void Task_FatFS(void);

/* ------------------------------------------------------------------------------------------------ */

int main(void)
{
TSK_t *     Task;

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

   #if (0U == ((UART_LIST_DEVICE) & (1U << (UART_DEV))))
	#error "Selected UART device is not in UART_LIST_DEVICE"
   #endif

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
    
    int ii = dma_init(0);
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
/* SD/MMC interrupt handler                            */
    
    OSisrInstall(SDMMC0_INT, MMCintHndl_0);            /* The SD/MMC driver, mmc_init(), is not called    */
    GICenable(SDMMC0_INT, 128, 0);                    /* because FatFS deals with the initialization    */
   
/* ------------------------------------------------ */
/* Application set-up								*/
    
	Task = TSKcreate("App FPGA Led", 3, 8192, &Task_FPGA_Led, 0);
	TSKsetCore(Task, 1);							/* Create new task, will always run on core #1	*/
	TSKresume(Task);								/* when BMP (G_OS_MP_TYPE == 4 or 5)			*/

	Task = TSKcreate("App FPGA Button", 3, 8192, &Task_FPGA_Button, 0);
	TSKsetCore(Task, 1);							/* Create new task, will always run on core #1	*/
	TSKresume(Task);								/* when BMP (G_OS_MP_TYPE == 4 or 5)			*/
    
    Task = TSKcreate("App MTL1", 2, 16384, &Task_MTL1, 0);
    TSKsetCore(Task, 1);
    TSKresume(Task);
    
    Task = TSKcreate("App Display File", 4, 8192, &Task_DisplayFile, 0);
    TSKresume(Task);
    
    Task = TSKcreate("Task FatFS", 4, 16384, &Task_FatFS, 0);
    TSKresume(Task);
    
#if defined(USE_SHELL)
    TSKcreate("Shell", OX_PRIO_MIN, 16384, OSshell, 1);
    TSKsleep(OS_MS_TO_TICK(100));                   /* Make sure Shell Task initialization if over  */
#endif

	TSKsetCore(TSKmyID(), 0);						/* Make sure Task_HPS_Led always run on Core #0	*/

	Task_HPS_Led();									/* Go into core #0 Task_HPS_Led code			*/

	for(;;);										/* Should never come back here					*/
}

/* ------------------------------------------------------------------------------------------------ */
/* Not used but needed to build uAbassi																*/

void TIMcallBack(void)
{
	return;
}

/* EOF */

