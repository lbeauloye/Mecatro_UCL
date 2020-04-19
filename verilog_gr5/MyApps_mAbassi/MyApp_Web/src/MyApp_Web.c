/* ------------------------------------------------------------------------------------------------ */

#define MYAPP

#include "MyApp_Web.h"
#include "mAbassi.h"          /* MUST include "SAL.H" and not uAbassi.h        */
#include "Platform.h"         /* Everything about the target platform is here  */
#include "HWinfo.h"           /* Everything about the target hardware is here  */

#include "ethernetif.h"
#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/def.h"
#include "lwip/opt.h"
#include "lwip/dhcp.h"
#include "tcp.h"
#include "tcpip.h"
#include "netif/etharp.h"
#include "lwipopts.h"

/* ------------------------------------------------------------------------------------------------ */
/* EMAC & lwIP initialization                                                                       */

void DoInitLwIP(void)
{
    int     CurrentTick;
    
    MY_BUTTON_INIT();
    MY_LED_INIT();
    
    NetAddr_Init();                                 /* Default static Addresses & Mask              */
    
    MTXLOCK_STDIO();
    puts("Initializing Ethernet I/F");
    MTXUNLOCK_STDIO();
    
    CurrentTick = G_OStimCnt;
    
    LwIP_Init();
    
    if ((G_OStimCnt-CurrentTick) > (30*OS_TICK_PER_SEC)) {
        MTXLOCK_STDIO();
        printf("The Ethernet I/F set-up seems to have taken a long time\n");
        printf("Is there a cable on the \"HPS Ethernet\" RJ-45 connector?\n\n");
        MTXUNLOCK_STDIO();
    }
    
    http_server_init();
    
    MTXLOCK_STDIO();
    puts("\nWaiting for Ethernet I/F & Webserver & file system to be ready\n");
    MTXUNLOCK_STDIO();
    TSKcreate("DHCP Client", DHCP_TASK_PRIO, 16384, LwIP_DHCP_task, 1);
    
    EVTwait(3, 0, -1);                                /* Wait for Ethernet & file system to be ready    */
    
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
}

/* ------------------------------------------------------------------------------------------------ */

void Task_FatFS(void)
{
    MTXLOCK_STDIO();
    DoInitFatFS();
    MTXUNLOCK_STDIO();
    
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
    printf("\n\nDE10-Nano - MyApp_Web\n\n");
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
        // toogle_hps_led();  This is disabled because the HPS LED is controlled by the Webserver Demo
        
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
        printf("Receive IRQ from Button %d and send message (Core = %d)\n", (int) alt_read_word(fpga_buttons), COREgetID());
        MTXUNLOCK_STDIO();
        
        MBXput(PrtMbx, PtrMsg, -1);     // -1 = Infinite blocking
    }
}

/* ------------------------------------------------------------------------------------------------ */

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
    
    // Clear the interruptmask of PIO core
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x0);
    
    // Enable the interruptmask and edge register of PIO core for new interrupt
    alt_write_word(fpga_buttons + PIOinterruptmask, 0x3);
    alt_write_word(fpga_buttons + PIOedgecapture, 0x3);
    
    PtrSem = SEMopen("MySemaphore");
    SEMpost(PtrSem);
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
