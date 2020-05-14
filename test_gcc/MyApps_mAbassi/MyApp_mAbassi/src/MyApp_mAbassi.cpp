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

#include <math.h>
#include <unistd.h>


#define OPP 0 //Set to 1 when there is an opponent to detect



void Task_HPS_Led(void)
{
	// Creation of the objects motors
	motors[1] = new motor_card(0x708,1);//FR
	motors[3] = new motor_card(0x708,2);//RR
	motors[2] = new motor_card(0x408,1);//RL
	motors[0] = new motor_card(0x408,2);//FL

	flag = 0;

	// Initialization of Mailbox  and semaphore
    MBX_t    *PrtMbx;
    intptr_t PtrMsg;
    SEM_t    *PtrSem = SEMopen("SemSetup");
    
    //Initialization of the motors
    for(int i = 0; i<4 ; i++)
        	motors[i]->init();
    setup_hps_gpio();               // This is the Adam&Eve Task and we have first to setup everything
    setup_Interrupt();
    SEMpost(PtrSem);
    

    MTXLOCK_STDIO();
    printf("\n\nDE10-Nano - MyApp_mAbassi\n\n");
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

void Task_FPGA_Led(void) // Task where we could perform a certain action
{
    SEM_t    *PtrSem;
	PtrSem = SEMopen("MySemaphoreTarget");

	for( ;; )
	{
		SEMwait(PtrSem, -1);            // -1 = Infinite blocking
		//Start if semaphore activated in the High Level Task
		SEMreset(PtrSem);
		MTXLOCK_STDIO();
		printf("Action On Target here\n");  // The Keys O and 1 seem to be inverted somewhere...
		path_plan->recompute=1; // Path needs to be recomputed
		MTXUNLOCK_STDIO();
        TSKsleep(OS_MS_TO_TICK(250));
	}
}

/* ------------------------------------------------------------------------------------------------ */

void Task_HIGH_LEVEL(void)
{
	double theta;
	// Semaphore initialization
	SEM_t    *PtrSem;
	PtrSem = SEMopen("MySemaphoreTarget");
    double dif_x, dif_y; // difference between position and target node
    int targets[3][2] = {{5,4},{5,9},{6,9}}; // list of target's index

    speed_x = 0.0;
    speed_y = 0.0;
    // Memory allocation for the path planning
    path_plan = (PathPlanning*) malloc(sizeof(PathPlanning));

    double xsi_in[3]; // input of middle level
	double speed[4]; // speed of the wheels
	double xsi[3]; // output of the middle level
	double b = 0.4*1.5; // Speed of the robot in m/s

	MTXLOCK_STDIO();
	path_plan->IMAX = 199;
	path_plan->JMAX = 299;

	path_plan->nbrx_nodes = 8; // Map is 80cm wide, so 1 node every 10 cm
	path_plan->nbry_nodes = 17;// Map is 170cm long, so 1 node every 10 cm

	// Memory allocation for the grid of nodes to follow
	path_plan->Grid = (Node **)malloc(path_plan->nbrx_nodes * sizeof(Node *));
	for(int i = 0; i< path_plan->nbrx_nodes; i++){
		path_plan->Grid[i] = (Node *)malloc(path_plan->nbry_nodes * sizeof(Node));
	}
	path_plan->last_t = 0.0;
	path_plan->path_found = 0;

	// Fill the list of the targets for the path
	for(int i = 0; i < 3; i++){
		path_plan->target_list[i][0] = targets[i][0];
		path_plan->target_list[i][1] = targets[i][1];
	}

	// Number of target visited
	path_plan->target_cnt = 0;

	path_plan->recompute = 1;

	mapping(path_plan);

	//Opponent detection/initialization
	#if OPP
		path_plan->opp_index = (int**)  malloc(sizeof(int *) * 9);
	    for(int i = 0; i<9; i++){
	        path_plan->opp_index[i] = (int *)malloc(2 * sizeof(int));
	    }
	    // Position of the opponent seen by the Lidar
	    int opp_x = get_neg(alt_read_word(fpga_adv_x));
	    int opp_y = get_neg(alt_read_word(fpga_adv_y));

	    addOpponent(path_plan, opp_x, opp_y); // Add opponent on the map
	    path_plan->move_opp = 0;
	#endif
	MTXUNLOCK_STDIO();

	int counter1=0;
	for( ;; )
	{
		printf("path_plan->recompute : %d\n",path_plan->recompute);

		if(path_plan->recompute == 1){
			MTXLOCK_STDIO();
			// Initial position of the robot on the map
			int i_start = 4 + (int) round(get_neg(alt_read_word(fpga_x_pos))/10.0);
			int j_start = 8 + (int) round(get_neg(alt_read_word(fpga_y_pos))/10.0);

			// Goal position of the robot on the map
			int i_goal = path_plan->target_list[path_plan->target_cnt][0];//4 + (int)floor((0.1)/0.1);
			int j_goal = path_plan->target_list[path_plan->target_cnt][1];//8 + (int)floor((0.1)/0.1);

			// Start and goal initialization of the robot for the A* algorithm
			path_plan->start = &(path_plan->Grid[i_start][j_start]);
			path_plan->goal = &(path_plan->Grid[i_goal][j_goal]);

			// Computation of the path
			List *path = Astarsearch(path_plan);
			if(path){
				// If path found, modification of some parameters
				View(path);
				path_plan->path_found = 1;
				path_plan->path = path;
				path_plan->current = Pop(&path_plan->path);
				path_plan->current = Pop(&path_plan->path);
				path_plan->start_path = 1;
			}
			else{
				// If path not found, modification of path_found parameter
				path_plan->path_found = 0;
			}

			printf("path found : %d \n", path_plan->path_found);
			path_plan->recompute = 0;
			MTXUNLOCK_STDIO();
		}

		if(path_plan->nbr_nodes_path > 0){
			// If not on goal, update position and pop part of the list
				dif_x = round(get_neg(alt_read_word(fpga_x_pos))/10.0) - path_plan->current->position_x*10;
				dif_y = round(get_neg(alt_read_word(fpga_y_pos))/10.0) - path_plan->current->position_y*10;
		    	alt_write_word(fpga_to_pi, (path_plan->current->i <<8) + path_plan->current->j);
				printf("dif_x : %f, dif_y : %f\n",dif_x,dif_y);
				printf("pos_x : %f, pos_y : %f\n",path_plan->current->position_x*10,path_plan->current->position_y*10);
		        if(fabs(dif_x) <= 0.1 && fabs(dif_y) <= 0.1){
		            // if opponent on the path -> recompute
		            // if last node
		            if(path_plan->nbr_nodes_path == 1){
		                path_plan->nbr_nodes_path -= 1;
		            }
		            // if too far from position -> recompute
		            else{
		                path_plan->current = Pop(&path_plan->path);
		                path_plan->nbr_nodes_path -= 1;
		            }
		        }

		        else if(fabs(dif_x) >= 2 || fabs(dif_y) >= 2){ // change tolerance here if recompute too much
		        	path_plan->recompute = 1;
		        }

		        // Evaluation of the speed in the x and y direction in function of the difference between the actual and target position
		        if(dif_x>0.1){
					speed_x = -b;
				}
				else if(dif_x<-0.1){
					speed_x = b;
				}
				else{
					speed_x = 0.0;
				}
		        if(dif_y>0.1){
					speed_y = -b;
				}
				else if(dif_y<-0.1){
					speed_y = b;
				}
				else{
					speed_y = 0.0;
				}

		    }
		    else{
		    	// If on goal : STOP
		        printf("On goal \n");
		        speed_x = 0.0;
		        speed_y = 0.0;
		    }
		if(speed_x != 0.0 && speed_y != 0.0){
			// Reduce the diagonal speed
			speed_x = speed_x/2;
			speed_y = speed_y/2;
		}
		// Set the input of the Middle-Level
		xsi_in[0] = speed_x;
		xsi_in[1] = speed_y;
		xsi_in[2] = 0.0;

		compute_local_velocities(xsi_in, 0.0 , xsi); // Middle-level
		compute_motor_velocities(xsi, speed);		 // Middle-level
		for(int i = 0; i<4 ; i++){
			// Set the speed command on each wheel
			printf("speed [%d]: %f\t",i,speed[i]);
			motors[i]->set_command(speed[i]);
		}


		#if OPP
			// Update of the position of the opponent
			if(fabs(opp_x-get_neg(alt_read_word(fpga_adv_x)))>=10){ //temporary condition TO CHANGE when opp moves
				if(flag==0){
			 opp_x = get_neg(alt_read_word(fpga_adv_x));
			 opp_y = get_neg(alt_read_word(fpga_adv_y));
			 moveOpponent(path_plan, opp_x, opp_y);
			 flag=1;
				}
			 if(isNewObstacle(path_plan->path))
			 	 path_plan->recompute = 1;
			path_plan->move_opp = 0;
			}
		#endif
	}
}

/* ------------------------------------------------------------------------------------------------ */

void spi_CallbackInterrupt (uint32_t icciar, void *context)
{
    // Interrupt from the SPI
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

void actions_CallbackInterrupt (uint32_t icciar, void *context)
{
    printf("actions : %d \n",alt_read_word(fpga_actions));

    // Clear the interruptmask of PIO core
    alt_write_word(fpga_actions + PIOinterruptmask, 0x0);

    // Enable the interruptmask and edge register of PIO core for new interrupt
    alt_write_word(fpga_actions + PIOinterruptmask, 0xFF);
    alt_write_word(fpga_actions + PIOedgecapture, 0xFF);

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


void Task_LOW_LEVEL(void)
{

    int i;

    SEMwait(SEMopen("SemSetup"), -1);

    DE0_SELECT_LT_SPI();

    CAN_init();

    // Initialization of input and output for Middle-Level
    double xsi_in[3] = {0.0, 0.0, 0.0};
	double speed[4];
	double xsi[3];
	compute_local_velocities(xsi_in, 0.0 , xsi); // Middle-level
	compute_motor_velocities(xsi, speed);	 	 // Middle-level
	for(i = 0; i<4 ; i++){
		// Set speed command for each wheel
	    motors[i]->set_command(speed[i]);
	}

	// Initialization of the motor card
    motors[1]->ctrl_motor(0);
    motors[0]->ctrl_motor(0);
    motors[1]->ctrl_motor(1);
    motors[0]->ctrl_motor(1);

    // Motor Brake
    motors[0]->set_brake(0);
    motors[1]->set_brake(0);
    motors[2]->set_brake(0);
    motors[3]->set_brake(0);

    // Gains
    double kp = 0.03;//0.03;//128;//128; //0.035; //0.005; //0.08
    double ki = 0.1;//0.1;//53;//53; //0.01 //0.3

    // Set gains for each motor
    motors[0]->set_kp(kp);
    motors[0]->set_ki(ki);
    motors[1]->set_kp(kp);
    motors[1]->set_ki(ki);
    motors[2]->set_kp(kp);
    motors[2]->set_ki(ki);
    motors[3]->set_kp(kp);
    motors[3]->set_ki(ki);

    // Time settings
    double time;
    double Tick0 = G_OStimCnt;
    double Tick1 = G_OStimCnt;
    double Tick2 = G_OStimCnt;
    double Tick3 = G_OStimCnt;
    counter = 0;
    for( ;; )
    {
    	printf("speed_0 : %f,\tspeed_1 : %f,\tspeed_2 : %f,\tspeed_3 : %f,\t \n", get_speed(0),get_speed(1),get_speed(2),get_speed(3));
    	printf("speed_x : %f, \t speed_y : %f\t\n", speed_x, speed_y);

    	// Motor 0 : Set previous speed, calculate and set time for the integral and set speed
    	motors[0]->set_old_speed(get_speed(0));
		time = (OS_TIMER_US*(G_OStimCnt-Tick0))/1000000;
		motors[0]->set_deltaT(time);
		motors[0]->set_speed();
		Tick0 = G_OStimCnt;

    	// Motor 1 : Set previous speed, calculate and set time for the integral and set speed
    	motors[1]->set_old_speed(-get_speed(1));
    	time = (OS_TIMER_US*(G_OStimCnt-Tick1))/1000000;
    	motors[1]->set_deltaT(time);
    	motors[1]->set_speed();
    	Tick1 = G_OStimCnt;

    	// Motor 2 : Set previous speed, calculate and set time for the integral and set speed
    	motors[2]->set_old_speed(get_speed(2));
		time = (OS_TIMER_US*(G_OStimCnt-Tick2))/1000000;
		motors[2]->set_deltaT(time);
		motors[2]->set_speed();
		Tick2 = G_OStimCnt;

    	// Motor 3 : Set previous speed, calculate and set time for the integral and set speed
		motors[3]->set_old_speed(get_speed(3));
		time = (OS_TIMER_US*(G_OStimCnt-Tick3))/1000000;
		motors[3]->set_deltaT(time);
		motors[3]->set_speed();
		Tick3 = G_OStimCnt;

		path_plan->move_opp = 1;

    	TSKsleep(OS_MS_TO_TICK(10));
    }

}


// Input : Number of tics from the odometer in 1/2k sec
// Output : Speed of the wheel
double get_speed(int choice){
	uint32_t PIO;
	double pio_double;
	double speed;
	double ratio = 5.8 * 4;
	double T = 1.0/2000;
	double max = 2048;
	switch (choice) {
		case 0:
			PIO = alt_read_word(fpga_pio_0);
			break;
		case 1:
			PIO = alt_read_word(fpga_pio_1);
			break;
		case 2:
			PIO = alt_read_word(fpga_pio_2);
			break;
		case 3:
			PIO = alt_read_word(fpga_pio_3);
			break;
	}
	if(PIO/4294967296.0>=0.5){
		pio_double = -(4294967296.0 - PIO);
	} else {
		pio_double = PIO;
	}
	speed = 2*M_PI*pio_double/(max*T*ratio);
	return speed;
}


// Input : One's complement integer
// Output : integer
int get_neg(int uns){
	int negat;
	if(uns/65536.0>=0.5){
		negat = -(65536 - uns);
	} else {
		negat = uns;
	}
	return negat;
}
