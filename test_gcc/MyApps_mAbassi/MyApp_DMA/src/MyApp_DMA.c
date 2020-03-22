/* ------------------------------------------------------------------------------------------------ */

#define MYAPP

#include "MyApp_DMA.h"

#include "mAbassi.h"          /* MUST include "SAL.H" and not uAbassi.h        */
#include "Platform.h"         /* Everything about the target platform is here  */
#include "HWinfo.h"           /* Everything about the target hardware is here  */

#include "arm_acp.h"
#include "dw_spi.h"
#include "arm_pl330.h"

#define USE_ACP 0

/* ------------------------------------------------------------------------------------------------ */

void Task_DMA(void)
{
    uint32_t ACPrd;
    uint32_t ACPwrt;
    
    uint8_t  * DMA_Src;
    uint8_t  * DMA_Dst;
    uint32_t   DMA_OpMode[16];
    int        DMA_Size;
    int        DMA_Err;
    int        DMA_XferID;
    
    SEM_t    *PtrSem;
    
    int Tick, i, Err;
    
#if (USE_ACP == 1)
    ACPwrt = acp_enable(-1, 0, 0, 0);                /* Page 0 (0x00000000->0x3FFFFFFF) is set-up    */
    ACPrd  = acp_enable(-1, 0, 0, 1);                /* to use ACP for both read & write             */
#else
    ACPwrt = 0;
    ACPrd  = 0;
#endif
    
    PtrSem = SEMopen("MySem_DMA");
    
    for( ;; )
    {
        SEMwait(PtrSem, -1);    // -1 = Infinite blocking
        SEMreset(PtrSem);
        MTXLOCK_STDIO();
        printf("\nStarting DMA on core %d\n", COREgetID());
        
        memset((void *) DMA_Src, 0x55, DMA_Size);
        memset((void *) DMA_Dst, 0xAA, DMA_Size);
        
        Tick = G_OStimCnt;
        
        i=0;
        
        DMA_Src       = (uint8_t *) 0x30000000;
        DMA_Dst       = (uint8_t *) 0x20000000;
        DMA_Size      = 30000000;
        DMA_OpMode[i++] = DMA_CFG_EOT_ISR;
#if (USE_ACP == 1)
        DMA_OpMode[i++] = DMA_CFG_NOCACHE_SRC;
        DMA_OpMode[i++] = DMA_CFG_NOCACHE_DST;
#endif
        DMA_OpMode[i++] = 0;
        
        DMA_Err = dma_xfer(0,
                           (uint8_t *)(ACPwrt + (uintptr_t) DMA_Dst), 1, MEMORY_DMA_ID,
                           (uint8_t *)(ACPrd  + (uintptr_t) DMA_Src), 1, MEMORY_DMA_ID,
                           1, 1, DMA_Size,
                           DMA_OPEND_NONE, NULL, (intptr_t)0,
                           &DMA_OpMode[0], &DMA_XferID, OS_MS_TO_TICK(1000));
        
        Tick = G_OStimCnt - Tick;
        
        if (DMA_Err != 0) {
            printf("\ndma_xfer() reported the error #%d\n", DMA_Err);
        } else {
            Err = 0;
            for (i=0; i<DMA_Size; i++)
                if (*(DMA_Src+i) != *(DMA_Dst+i)) {
                    Err++;
                    if (Err < 5)
                        printf("Error in DMA transfert : %x instead of %x at %x (%x)\n", *(DMA_Dst+i), *(DMA_Src+i), (unsigned int) (DMA_Dst+i), i);
                }
            printf("DMA Test: Size: %d - Xfer: %d ms - %d MB/s \n\n", DMA_Size, (OS_TIMER_US*Tick)/1000, (Tick == 0) ? 0 : DMA_Size/((OS_TIMER_US*Tick)));
        }
        MTXUNLOCK_STDIO();
        
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
    printf("\n\nDE10-Nano - MyApp_DMA\n\n");
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

void Task_SPI_TEST(void)
{

    for( ;; )
    {

        alt_write_word(fpga_spi + SPI_TXDATA, 0x0011 );
        printf("wesh wesh\n");
        TSKsleep(OS_MS_TO_TICK(500));
    }
}

void spi_CallbackInterrupt (uint32_t icciar, void *context)
{
    // Do something
    MTXLOCK_STDIO();
    printf("INFO: IRQ from SPI : %08x (status = %x)\r\n",
           (unsigned int) alt_read_word(fpga_spi + SPI_RXDATA),
           (unsigned int) alt_read_word(fpga_spi + SPI_STATUS));
    MTXUNLOCK_STDIO();
    alt_write_word(fpga_spi + SPI_TXDATA, 0x113377FF);
    
    // Clear the status of SPI core
    alt_write_word(fpga_spi + SPI_STATUS, 0x00);
}

/* ------------------------------------------------------------------------------------------------ */

void button_CallbackInterrupt (uint32_t icciar, void *context)
{
    SEM_t    *PtrSem;
    uint32_t button;
    
    button = alt_read_word(fpga_buttons);
    
    // Clear the interruptmask of PIO core
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x0);
    
    // Enable the interruptmask and edge register of PIO core for new interrupt
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x3);
    alt_write_word(fpga_buttons + PIOedgecapture, 0x3);
    
    PtrSem = SEMopen("MySemaphore");
    SEMpost(PtrSem);
    
    if (button == 1) {
        PtrSem = SEMopen("MySem_DMA");
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

