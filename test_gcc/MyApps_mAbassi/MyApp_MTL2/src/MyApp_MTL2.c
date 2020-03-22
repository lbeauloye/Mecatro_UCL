/* ------------------------------------------------------------------------------------------------ */

#define MYAPP

#include "MyApp_MTL2.h"

#include "mAbassi.h"          /* MUST include "SAL.H" and not uAbassi.h        */
#include "Platform.h"         /* Everything about the target platform is here  */
#include "HWinfo.h"           /* Everything about the target hardware is here  */
#include "SysCall.h"          /* System Call layer stuff                        */

#include "arm_pl330.h"
#include "dw_i2c.h"
#include "cd_qspi.h"
#include "dw_sdmmc.h"
#include "dw_spi.h"
#include "dw_uart.h"
#include "alt_gpio.h"

#include "gui.h"

MTC2_INFO *myTouch;
VIP_FRAME_READER *myReader;
uint32_t  *myFrameBuffer;

int cmd_mnt(int argc, char *argv[]);

/* ------------------------------------------------------------------------------------------------ */

void Task_MTL2(void)
{
    myFrameBuffer = (uint32_t  *) 0x20000000;
    
    TSKsleep(OS_MS_TO_TICK(500));
    
    // Initialize global timer
    assert(ALT_E_SUCCESS == alt_globaltmr_init());
    assert(ALT_E_SUCCESS == alt_globaltmr_start());
    
    MTXLOCK_STDIO();
    printf("Starting MTL2 initialization\n");

    oc_i2c_init(fpga_i2c);
    myTouch = MTC2_Init(fpga_i2c, fpga_mtc2, LCD_TOUCH_INT_IRQ);
    
    // Enable IRQ for SPI & MTL
    
    OSisrInstall(GPT_SPI_IRQ, (void *) &spi_CallbackInterrupt);
    GICenable(GPT_SPI_IRQ, 128, 1);
    OSisrInstall(GPT_MTC2_IRQ, (void *) &mtc2_CallbackInterrupt);
    GICenable(GPT_MTC2_IRQ, 128, 1);
    
    // Enable interruptmask and edgecapture of PIO core for mtc2 irq
    alt_write_word(fpga_mtc2 + PIOinterruptmask, 0x3);
    alt_write_word(fpga_mtc2 + PIOedgecapture, 0x3);
    
    printf("\nMTL2 initialization completed\n");
    MTXUNLOCK_STDIO();
    
    for( ;; )
    {
        GUI(myTouch);
        TSKsleep(OS_MS_TO_TICK(5));
    }
}

// Used by i2C_core.c

void delay_us(uint32_t us) {
    uint64_t start_time = alt_globaltmr_get64();
    uint32_t timer_prescaler = alt_globaltmr_prescaler_get() + 1;
    uint64_t end_time;
    alt_freq_t timer_clock;
    
    assert(ALT_E_SUCCESS == alt_clk_freq_get(ALT_CLK_MPU_PERIPH, &timer_clock));
    end_time = start_time + us * ((timer_clock / timer_prescaler) / ALT_MICROSECS_IN_A_SEC);

    while(alt_globaltmr_get64() < end_time);
}

/* ------------------------------------------------------------------------------------------------ */

/* Align on cache lines if cached transfers */
static char g_Buffer[9600] __attribute__ ((aligned (OX_CACHE_LSIZE)));

void Task_DisplayFile(void)
{
    SEM_t    *PtrSem;
    int       FdSrc;
    int       Nrd;
    int       i;
    
    static const char theFileName[] = "MTL_Image.dat";
    
    PtrSem = SEMopen("MySem_DisplayFile");
    
    for( ;; )
    {
        SEMwait(PtrSem, -1);    // -1 = Infinite blocking
        SEMreset(PtrSem);
        
        FdSrc = open(theFileName, O_RDONLY, 0777);
        if (FdSrc >= 0) {
            myFrameBuffer = (uint32_t  *) 0x20000000;
            do {
                Nrd = read(FdSrc, &g_Buffer[0], sizeof(g_Buffer));
                i=0;
                while(i < Nrd) {
                    *myFrameBuffer++ = (g_Buffer[i] << 16) + (g_Buffer[i+1] << 8) + g_Buffer[i+2];
                    i += 3;
                }
            } while (Nrd >= sizeof(g_Buffer));
            close(FdSrc);
            myFrameBuffer = (uint32_t  *) 0x20000000;
        }
    }
}

/* ------------------------------------------------------------------------------------------------ */

void spi_CallbackInterrupt (uint32_t icciar, void *context)
{
    uint32_t rxdata = alt_read_word(fpga_spi + SPI_RXDATA);
    
    // Do something
    // printf("INFO: IRQ from SPI : %x (status = %x)\r\n", rxdata, status);
    alt_write_word(fpga_spi + SPI_TXDATA, 0x113377FF);
    
    *myFrameBuffer++ = rxdata;
    if (myFrameBuffer >= (uint32_t  *) (0x20000000 + (800 * 480 * 4)))
        myFrameBuffer = (uint32_t  *) 0x20000000;
    
    // Clear the status of SPI core
    alt_write_word(fpga_spi + SPI_STATUS, 0x00);
}

/* ------------------------------------------------------------------------------------------------ */

void mtc2_CallbackInterrupt (uint32_t icciar, void *context)
{
    //printf("INFO: IRQ from MTC2\r\n");
    
    // Clear the interruptmask of PIO core
    alt_write_word(fpga_mtc2 + PIOinterruptmask, 0x0);
    
    mtc2_QueryData(myTouch);
    
    // Enable the interruptmask and edge register of PIO core for new interrupt
    alt_write_word(fpga_mtc2 + PIOinterruptmask, 0x3);
    alt_write_word(fpga_mtc2 + PIOedgecapture, 0x3);
}

/* ------------------------------------------------------------------------------------------------ */

void Task_FatFS(void)
{
    // Mount drive 0 to a mount point
    char *argv[2];
    argv[1] = "0";    
    cmd_mnt(2, argv);
    
    for( ;; )
    {
        DoProcessFatFS();
    }
    
}

/* ------------------------------------------------------------------------------------------------ */

void Task_HPS_Led(void)
{
    MBX_t    *PrtMbx;
    intptr_t PtrMsg;
    
    setup_hps_gpio();               // This is the Adam&Eve Task and we have first to setup everything
    setup_Interrupt();
    
    MTXLOCK_STDIO();
    printf("\n\nDE10-Nano - MyApp_MTL2\n\n");
    printf("Task_HPS_Led running on core #%d\n\n", COREgetID());
    MTXUNLOCK_STDIO();
    
    PrtMbx = MBXopen("MyMailbox", 128);

	for( ;; )
	{
        if (MBXget(PrtMbx, &PtrMsg, 0) == 0) {  // 0 = Never blocks
            MTXLOCK_STDIO();
            printf("Receive message (Core = %d)\n", COREgetID());
            MTXUNLOCK_STDIO();
        }
        toogle_hps_led();
        
        TSKsleep(OS_MS_TO_TICK(500));
	}
}

/* ------------------------------------------------------------------------------------------------ */

void Task_FPGA_Led(void)
{
    uint32_t leds_mask;
    
    alt_write_word(fpga_leds, 0x01);

	for( ;; )
	{
        leds_mask = alt_read_word(fpga_leds);
        if (leds_mask != (0x01 << (LED_PIO_DATA_WIDTH - 1))) {
            // rotate leds
            leds_mask <<= 1;
        } else {
            // reset leds
            leds_mask = 0x1;
        }
        alt_write_word(fpga_leds, leds_mask);
        
        TSKsleep(OS_MS_TO_TICK(250));
	}
}

/* ------------------------------------------------------------------------------------------------ */

void Task_FPGA_Button(void)
{
    MBX_t    *PrtMbx;
    intptr_t  PtrMsg = (intptr_t) NULL;
    SEM_t    *PtrSem;
    
    PrtMbx = MBXopen("MyMailbox", 128);
    PtrSem = SEMopen("MySemaphore");
    
    for( ;; )
    {
        SEMwait(PtrSem, -1);            // -1 = Infinite blocking
        SEMreset(PtrSem);
        MTXLOCK_STDIO();
        printf("Receive IRQ from Button %d and send message (Core = %d)\n", (int) alt_read_word(fpga_buttons) - 1, COREgetID());  // The Keys O and 1 seem to be inverted somewhere...
        MTXUNLOCK_STDIO();
        
        MBXput(PrtMbx, PtrMsg, -1);     // -1 = Infinite blocking
    }
}

/* ------------------------------------------------------------------------------------------------ */

void button_CallbackInterrupt (uint32_t icciar, void *context)
{
    SEM_t    *PtrSem;
    
    // Clear the interruptmask of PIO core
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x0);
    
    // Enable the interruptmask and edge register of PIO core for new interrupt
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x3);
    alt_write_word(fpga_buttons + PIOedgecapture, 0x3);
    
    PtrSem = SEMopen("MySemaphore");
    SEMpost(PtrSem);

    if (alt_read_word(fpga_buttons) == 1) {
        PtrSem = SEMopen("MySem_DisplayFile");
        SEMpost(PtrSem);
    }
}

/* ------------------------------------------------------------------------------------------------ */

void setup_Interrupt( void )
{
    // IRQ from Key0 and Key1
    OSisrInstall(GPT_BUTTON_IRQ, (void *) &button_CallbackInterrupt);
    GICenable(GPT_BUTTON_IRQ, 128, 1);
    
    // Enable interruptmask and edgecapture of PIO core for buttons 0 and 1
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x3);
    alt_write_word(fpga_buttons + PIOedgecapture, 0x3);
    
    // IRQ from SPI slave connected to the RaspberryPI
    OSisrInstall(GPT_SPI_IRQ, (void *) &spi_CallbackInterrupt);
    GICenable(GPT_SPI_IRQ, 128, 1);
    
    // Initialize TXDATA to something (for testing purpose)
    alt_write_word(fpga_spi + SPI_TXDATA, 0x0103070F);
    alt_write_word(fpga_spi + SPI_EOP_VALUE, 0x55AA55AA);
    // Enable interrupt
    alt_write_word(fpga_spi + SPI_CONTROL, SPI_CONTROL_IRRDY + SPI_CONTROL_IE);
}

/* ------------------------------------------------------------------------------------------------ */

void setup_hps_gpio()
{
    uint32_t hps_gpio_config_len = 2;
    ALT_GPIO_CONFIG_RECORD_t hps_gpio_config[] = {
        {HPS_LED_IDX  , ALT_GPIO_PIN_OUTPUT, 0, 0, ALT_GPIO_PIN_DEBOUNCE, ALT_GPIO_PIN_DATAZERO},
        {HPS_KEY_N_IDX, ALT_GPIO_PIN_INPUT , 0, 0, ALT_GPIO_PIN_DEBOUNCE, ALT_GPIO_PIN_DATAZERO}
    };
    
    assert(ALT_E_SUCCESS == alt_gpio_init());
    assert(ALT_E_SUCCESS == alt_gpio_group_config(hps_gpio_config, hps_gpio_config_len));
}

/* ------------------------------------------------------------------------------------------------ */

void toogle_hps_led()
{
    uint32_t hps_led_value = alt_read_word(ALT_GPIO1_SWPORTA_DR_ADDR);
    hps_led_value >>= HPS_LED_PORT_BIT;
    hps_led_value = !hps_led_value;
    hps_led_value <<= HPS_LED_PORT_BIT;
    alt_gpio_port_data_write(HPS_LED_PORT, HPS_LED_MASK, hps_led_value);
}

/* ------------------------------------------------------------------------------------------------ */
