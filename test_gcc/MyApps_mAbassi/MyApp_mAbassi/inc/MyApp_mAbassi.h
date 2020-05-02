/* ------------------------------------------------------------------------------------------------ */
/* Include                                                                                         */

#ifndef MYAPP_H_
#define MYAPP_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "socal/alt_gpio.h"
#include "socal/hps.h"
#include "socal/socal.h"

#include "alt_generalpurpose_io.h"
#include "alt_interrupt.h"
#include "hps_0.h"
#include "motor/motor_card.h"
#include "motor/middle_level.h"

#include "motor/path_planning.h"

/* ------------------------------------------------------------------------------------------------ */
/* Define                                                                                           */



//#ifdef __cplusplus
//extern "C"
//{
//#endif

#define assert(e) ((e) ? (void)0 :(void) printf("Error in %s, Line : %d\r\n", __FILE__, __LINE__))

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |   HPS_LED   |  GPIO53  |   GPIO1[24]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_LED_IDX        (ALT_GPIO_1BIT_53)                      // GPIO53
#define HPS_LED_PORT       (alt_gpio_bit_to_pid(HPS_LED_IDX))      // ALT_GPIO_PORTB
#define HPS_LED_PORT_BIT   (alt_gpio_bit_to_port_pin(HPS_LED_IDX)) // 24 (from GPIO1[24])
#define HPS_LED_MASK       (1 << HPS_LED_PORT_BIT)

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |  HPS_KEY_N  |  GPIO54  |   GPIO1[25]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_KEY_N_IDX      (ALT_GPIO_1BIT_54)                        // GPIO54
#define HPS_KEY_N_PORT     (alt_gpio_bit_to_pid(HPS_KEY_N_IDX))      // ALT_GPIO_PORTB
#define HPS_KEY_N_PORT_BIT (alt_gpio_bit_to_port_pin(HPS_KEY_N_IDX)) // 25 (from GPIO1[25])
#define HPS_KEY_N_MASK     (1 << HPS_KEY_N_PORT_BIT)

// IRQ for the 2 buttons
#define GPT_BUTTON_IRQ   ALT_INT_INTERRUPT_F2S_FPGA_IRQ0 + BUTTON_PIO_IRQ
#define GPT_SPI_IRQ      ALT_INT_INTERRUPT_F2S_FPGA_IRQ0 + SPI_RASPBERRYPI_IRQ
#define GPT_ACTIONS_IRQ  ALT_INT_INTERRUPT_F2S_FPGA_IRQ0 + ACTIONS_PIO_IRQ


// PIO Registers
#define PIOdirection        1*4
#define PIOinterruptmask    2*4
#define PIOedgecapture      3*4
#define PIOoutset           4*4
#define PIOoutclear         5*4

// SPI Registers and Control
#define SPI_RXDATA     0
#define SPI_TXDATA     1*4
#define SPI_STATUS     2*4
#define SPI_CONTROL    3*4
#define SPI_EOP_VALUE  6*4

#define SPI_STATUS_EOP    0x200
#define SPI_STATUS_E      0x100
#define SPI_STATUS_RRDY   0x80
#define SPI_STATUS_TRDY   0x40
#define SPI_STATUS_TMT    0x20
#define SPI_STATUS_TOE    0x10
#define SPI_STATUS_ROE    0x08

#define SPI_CONTROL_IEOP  0x200
#define SPI_CONTROL_IE    0x100
#define SPI_CONTROL_IRRDY 0x80
#define SPI_CONTROL_ITRDY 0x40
#define SPI_CONTROL_ITOE  0x10
#define SPI_CONTROL_IROE  0x08

// To protect non- multithread-safe functions in the standard libraries
#define MTXLOCK_ALLOC()		MTXlock(G_OSmutex, -1)
#define MTXUNLOCK_ALLOC()	MTXunlock(G_OSmutex)
#define MTXLOCK_STDIO()		MTXlock(G_OSmutex, -1)
#define MTXUNLOCK_STDIO()	MTXunlock(G_OSmutex)

/* ------------------------------------------------------------------------------------------------ */
/* Global variables                                                                                 */

#ifdef MYAPP
    void *fpga_leds = ALT_LWFPGASLVS_ADDR + LED_PIO_BASE;
    void *fpga_buttons = ALT_LWFPGASLVS_ADDR + BUTTON_PIO_BASE;
    void *fpga_spi = ALT_LWFPGASLVS_ADDR + SPI_RASPBERRYPI_BASE;
    void *fpga_pio_0 = ALT_LWFPGASLVS_ADDR + PIO_0_BASE;
    void *fpga_pio_1 = ALT_LWFPGASLVS_ADDR + PIO_1_BASE;
    void *fpga_pio_2 = ALT_LWFPGASLVS_ADDR + PIO_2_BASE;
    void *fpga_pio_3 = ALT_LWFPGASLVS_ADDR + PIO_3_BASE;
    void *fpga_x_pos = ALT_LWFPGASLVS_ADDR + X_POS_BASE;
    void *fpga_y_pos = ALT_LWFPGASLVS_ADDR + Y_POS_BASE;
    void *fpga_theta = ALT_LWFPGASLVS_ADDR + THETA_BASE;
    void *fpga_actions = ALT_LWFPGASLVS_ADDR + ACTIONS_PIO_BASE;
    void *fpga_to_pi = ALT_LWFPGASLVS_ADDR + TO_PI_BASE;

#else
    extern void *fpga_leds;
    extern void *fpga_buttons;
    extern void *fpga_spi;
    extern void *fpga_pio_0;
    extern void *fpga_pio_1;
    extern void *fpga_pio_2;
    extern void *fpga_pio_3;
    extern void *fpga_x_pos;
    extern void *fpga_y_pos;
    extern void *fpga_theta;
    extern void *fpga_actions;
    extern void *fpga_to_pi;
#endif

/* ------------------------------------------------------------------------------------------------ */
/* Tasks & Functions                                                                                */

void Task_HPS_Led(void);
void Task_FPGA_Led(void);
void Task_FPGA_Button(void);
void Task_CAN(void);
void Task_LOW_LEVEL(void);
void Task_MID_LEVEL(void);
void Task_HIGH_LEVEL(void);

void spi_CallbackInterrupt (uint32_t icciar, void *context);
void button_CallbackInterrupt (uint32_t icciar, void *context);
void actions_CallbackInterrupt (uint32_t icciar, void *context);
void setup_Interrupt( void );
void setup_hps_gpio( void );
void toogle_hps_led( void );

double get_speed(int choice);

//#ifdef __cplusplus
//}
//#endif

static double speed_x, speed_y;
static int counter;


#ifdef __cplusplus

static motor_card **motors = (motor_card **) calloc(4, sizeof(motor_card));

static PathPlanning *path_plan;
//static motor_card motors [4];

#endif

#endif
