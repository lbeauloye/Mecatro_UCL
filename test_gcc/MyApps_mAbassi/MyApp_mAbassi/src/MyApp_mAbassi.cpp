/* ------------------------------------------------------------------------------------------------ */

#define MYAPP

#include "MyDriver_CAN.h"
#include "MyApp_mAbassi.h"

#include "mAbassi.h"          /* MUST include "SAL.H" and not uAbassi.h        */
#include "Platform.h"         /* Everything about the target platform is here  */
#include "HWinfo.h"           /* Everything about the target hardware is here  */
#include "alt_gpio.h"

#include "test.h"
#include "libcanard/canard.h"
#include "o1heap/o1heap.h"

/*
static void* memAllocate(CanardInstance* const ins, const size_t amount)
{
    (void) ins;
    return o1heapAllocate(malloc, amount);
}

static void memFree(CanardInstance* const ins, void* const pointer)
{
    (void) ins;
    o1heapFree(free, pointer);
}
*/
/* ------------------------------------------------------------------------------------------------ */


void Task_HPS_Led(void)
{
    MBX_t    *PrtMbx;
    intptr_t PtrMsg;
    SEM_t    *PtrSem = SEMopen("SemSetup");
    
    setup_hps_gpio();               // This is the Adam&Eve Task and we have first to setup everything
    setup_Interrupt();
    SEMpost(PtrSem);
    
    MTXLOCK_STDIO();
    printf("\n\nDE10-Nano - MyApp_mAbassi\n\n");
    printf("Task_HPS_Led running on core #%d\n\n", COREgetID());

    printf("salut salut %d\n",Test_func(5));
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

void Task_CAN(void)
{
    uint32_t tx_Identifier, rx_Identifier;
    uint8_t  tx_Data[8],    rx_Data[8];
    uint8_t  tx_Length,     rx_Length;
    uint8_t  tx_FrameType;
    
    int i;
    

    SEMwait(SEMopen("SemSetup"), -1);

    DE0_SELECT_LT_SPI();

    CAN_init();

    CAN_debug();

    CanardInstance ins = canardInit(&malloc, &free);
    ins.mtu_bytes = CANARD_MTU_CAN_CLASSIC;  // Defaults to 64 (CAN FD); here we select Classic CAN.
    ins.node_id   = 42;                      // Defaults to anonymous; can be set up later at any point.


    tx_Identifier = 0xabc;
    tx_Length     = 8;
    tx_FrameType  = MCP2515_TX_STD_FRAME;
    
    for(i=0; i<tx_Length; i++)
        tx_Data[i] = i;

    MTXLOCK_STDIO();
    printf("youpitou maboi\n");
    MTXUNLOCK_STDIO();

    CAN_sendMsg(tx_Identifier, tx_Data, tx_Length, tx_FrameType);

    static uint8_t my_message_transfer_id;  // Must be static or heap-allocated to retain state between calls.
    const CanardTransfer transfer = {
        .timestamp_usec = 0,      // Zero if transmission deadline is not limited.
        .priority       = CanardPriorityNominal,
        .transfer_kind  = CanardTransferKindMessage,
        .port_id        = 1234,                       // This is the subject-ID.
        .remote_node_id = CANARD_NODE_ID_UNSET,       // Messages cannot be unicast, so use UNSET.
        .transfer_id    = my_message_transfer_id,
        .payload_size   = 47,
        .payload        = "\x2D\x00" "Sancho, it strikes me thou art in great fear.",
    };
    ++my_message_transfer_id;  // The transfer-ID shall be incremented after every transmission on this subject.
    int32_t result = canardTxPush(&ins, &transfer);
    if (result < 0)
    {
        // An error has occurred: either an argument is invalid or we've ran out of memory.
        // It is possible to statically prove that an out-of-memory will never occur for a given application if the
        // heap is sized correctly; for background, refer to the Robson's Proof and the documentation for O1Heap.
        abort();
    }
    
    for( ;; )
    {
        for (const CanardFrame* txf = NULL; (txf = canardTxPeek(&ins)) != NULL;)  // Look at the top of the TX queue.
        {
            CAN_sendMsg(txf->extended_can_id, txf->payload, txf->payload_size, tx_FrameType);
            canardTxPop(&ins);                         // Remove the frame from the queue after it's transmitted.
            ins.memory_free(&ins, (CanardFrame*)txf);  // Deallocate the dynamic memory afterwards.
        }
    }
}
