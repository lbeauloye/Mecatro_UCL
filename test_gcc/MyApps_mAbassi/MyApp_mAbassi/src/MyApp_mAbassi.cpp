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


#define OPP 0

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
	motors[1] = new motor_card(0x708,1);//FR
	motors[3] = new motor_card(0x708,2);//RR
	motors[2] = new motor_card(0x408,1);//RL
	motors[0] = new motor_card(0x408,2);//FL

	flag = 0;
	base_opp_x = get_neg(alt_read_word(fpga_adv_x));
	base_opp_y = get_neg(alt_read_word(fpga_adv_y));


    MBX_t    *PrtMbx;
    intptr_t PtrMsg;
    SEM_t    *PtrSem = SEMopen("SemSetup");
    
    for(int i = 0; i<4 ; i++)
        	motors[i]->init();
    setup_hps_gpio();               // This is the Adam&Eve Task and we have first to setup everything
    setup_Interrupt();
    SEMpost(PtrSem);
    

    MTXLOCK_STDIO();
    printf("\n\nDE10-Nano - MyApp_mAbassi\n\n");
    printf("Task_HPS_Led running on core #%d\n\n", COREgetID());

//    printf("salut salut %d\n",Test_func(5));


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
    SEM_t    *PtrSem;
	PtrSem = SEMopen("MySemaphoreTarget");

	for( ;; )
	{

//		printf("x_pos : %d,\t y_pos : %d,\t theta : %d\n",get_neg(alt_read_word(fpga_x_pos)),get_neg(alt_read_word(fpga_y_pos)),get_neg(alt_read_word(fpga_theta)));
//
//		printf("adv_x : %d,\t adv_y : %d \n",get_neg(alt_read_word(fpga_adv_x)),get_neg(alt_read_word(fpga_adv_y)));

		SEMwait(PtrSem, -1);            // -1 = Infinite blocking
		SEMreset(PtrSem);
		MTXLOCK_STDIO();
		printf("Action On Target here\n");  // The Keys O and 1 seem to be inverted somewhere...
		path_plan->recompute=1;
		MTXUNLOCK_STDIO();
        TSKsleep(OS_MS_TO_TICK(250));
	}
}


void Task_HIGH_LEVEL(void)
{
    //uint32_t leds_mask;
	double theta;
	SEM_t    *PtrSem;
	PtrSem = SEMopen("MySemaphoreTarget");

    //alt_write_word(fpga_x_pos, 80);
    //alt_write_word(fpga_y_pos, 10);
    double dif_x, dif_y;

//    //CHANGES HERE
    int targets[3][2] = {{5,4},{5,9},{6,9}}; //list of target's index

    speed_x = 0.0;
    speed_y = 0.0;
    path_plan = (PathPlanning*) malloc(sizeof(PathPlanning));

    double xsi_in[3];
	double speed[4];
	double xsi[3];
	double b = 0.4*1.5;

	MTXLOCK_STDIO();
	path_plan->IMAX = 199;
	path_plan->JMAX = 299;

	path_plan->nbrx_nodes = 8;
	path_plan->nbry_nodes = 17;

	path_plan->Grid = (Node **)malloc(path_plan->nbrx_nodes * sizeof(Node *));
	for(int i = 0; i< path_plan->nbrx_nodes; i++){
		path_plan->Grid[i] = (Node *)malloc(path_plan->nbry_nodes * sizeof(Node));
	}
	path_plan->last_t = 0.0;
	path_plan->path_found = 0;

	//CHANGES HERE
	for(int i = 0; i < 3; i++){
		path_plan->target_list[i][0] = targets[i][0];
		path_plan->target_list[i][1] = targets[i][1];
	}
	//nbr of target visited
	path_plan->target_cnt = 0;

	path_plan->recompute = 1;

	mapping(path_plan);

	//CHANGES HERE FOR OPPONENT
	#if OPP
		path_plan->opp_index = (int**)  malloc(sizeof(int *) * 9);
	    for(int i = 0; i<9; i++){
	        path_plan->opp_index[i] = (int *)malloc(2 * sizeof(int));
	    }
	    int opp_x = get_neg(alt_read_word(fpga_adv_x));
	    int opp_y = get_neg(alt_read_word(fpga_adv_y));

	    addOpponent(path_plan, opp_x, opp_y);
	    path_plan->move_opp = 0;
	#endif
//
////	printf("value : %d\n",path_plan->Grid[4][8].i);
	MTXUNLOCK_STDIO();

	int counter1=0;
	for( ;; )
	{
		//printf("x_pos : %d,\t y_pos : %d,\t theta : %d\n",alt_read_word(fpga_x_pos),alt_read_word(fpga_y_pos),alt_read_word(fpga_theta));

		printf("path_plan->recompute : %d\n",path_plan->recompute);

		if(path_plan->recompute == 1){
			MTXLOCK_STDIO();
			int i_start = 4 + (int) round(get_neg(alt_read_word(fpga_x_pos))/10.0);
			int j_start = 8 + (int) round(get_neg(alt_read_word(fpga_y_pos))/10.0);

			//CHANGES HERE
			int i_goal = path_plan->target_list[path_plan->target_cnt][0];//4 + (int)floor((0.1)/0.1);
			int j_goal = path_plan->target_list[path_plan->target_cnt][1];//8 + (int)floor((0.1)/0.1);

		    printf("i_start : %d,\tj_start : %d,\ti_goal : %d,\tj_goal : %d\n",i_start,j_start,i_goal,j_goal);

			path_plan->start = &(path_plan->Grid[i_start][j_start]);
			path_plan->goal = &(path_plan->Grid[i_goal][j_goal]);

			List *path = Astarsearch(path_plan);
			if(path){
				View(path);
				path_plan->path_found = 1;
				path_plan->path = path;
				path_plan->current = Pop(&path_plan->path);
				path_plan->current = Pop(&path_plan->path);
				path_plan->start_path = 1;

//				Node *start_path = Pop(&path);
//				printf("%d, %d \n", start_path->i, start_path->j);
//				while (path){
//					Node *node = Pop(&path);
//					printf("%d, %d \n", node->i, node->j);
//				}
				//path_plan->last_t = inputs->t;
			}
			else{
				path_plan->path_found = 0;
			}

			printf("path found : %d \n", path_plan->path_found);
			path_plan->recompute = 0;

//			for(int i = 0; i< path_plan->nbrx_nodes; i++){
//				        free(path_plan->Grid[i]);
//				    }
//				    free(path_plan->Grid);
//					free(path_plan);
			MTXUNLOCK_STDIO();
		}
		printf("x_pos : %f,\t y_pos : %f\n",get_neg(alt_read_word(fpga_x_pos))/10.0,get_neg(alt_read_word(fpga_y_pos))/10.0);

		if(path_plan->nbr_nodes_path > 0){
				dif_x = round(get_neg(alt_read_word(fpga_x_pos))/10.0) - path_plan->current->position_x*10;
				dif_y = round(get_neg(alt_read_word(fpga_y_pos))/10.0) - path_plan->current->position_y*10;
		    	alt_write_word(fpga_to_pi, (path_plan->current->i <<8) + path_plan->current->j);
		    	printf("%d\n",(path_plan->current->i <<8) + path_plan->current->j);

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

		        //CHANGES HERE
		        else if(fabs(dif_x) >= 2 || fabs(dif_y) >= 2){ //change tolerance here if recompute too muchh
		        	path_plan->recompute = 1;
		        	printf("PPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPPP\n");
		        }

//		        theta = atan2(dif_y,dif_x);

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
		        printf("On goal \n");


		        speed_x = 0.0;
		        speed_y = 0.0;


		        //CHANGES HERE
//		        if(path_plan->target_cnt < 2){
////		        	path_plan->recompute = 1;
//		        	path_plan->target_cnt++;
//		        	printf("Target changed \n");
//		        	SEMpost(PtrSem);
//		        }
		    }
		if(speed_x != 0.0 && speed_y != 0.0){
			speed_x = speed_x/2;
			speed_y = speed_y/2;
		}
//		if(counter1==3000){
//			xsi_in[0] = b;//speed_x;
//			xsi_in[1] = 0.0;//speed_y;
//			xsi_in[2] = 0.0;
//		}
//		else{
//		printf("v_x : %f, v_y: %f\n",speed_x, speed_y);
		xsi_in[0] = -b/sqrt(2);//speed_x;
		xsi_in[1] = -b/sqrt(2);//speed_y;
		xsi_in[2] = 0.0;
//		counter1++;
//		}

		compute_local_velocities(xsi_in, 0.0 , xsi);
		compute_motor_velocities(xsi, speed);
		for(int i = 0; i<4 ; i++){
			printf("speed [%d]: %f\t",i,speed[i]);
			motors[i]->set_command(speed[i]);
		}


		//CHANGES HERE
		#if OPP
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

//    // IRQ from Key0 and Key1
//	OSisrInstall(GPT_ACTIONS_IRQ, (void *) &actions_CallbackInterrupt);
//	GICenable(GPT_ACTIONS_IRQ, 128, 1);
//
//	// Enable interruptmask and edgecapture of PIO core for actions
//	alt_write_word(fpga_actions + PIOinterruptmask, 0xFF);
//	alt_write_word(fpga_actions + PIOedgecapture, 0xFF);

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

    double xsi_in[3] = {0.0, 0.0, 0.0};
	double speed[4];
	double xsi[3];
	compute_local_velocities(xsi_in, 0.0 , xsi);
	compute_motor_velocities(xsi, speed);
	for(i = 0; i<4 ; i++){
//		printf("speed [%d]: %f\t",i,speed[i]);
	    motors[i]->set_command(speed[i]);
	}
//	printf("\n");

    motors[1]->ctrl_motor(0);
    motors[0]->ctrl_motor(0);

    motors[1]->ctrl_motor(1);
    motors[0]->ctrl_motor(1);
    motors[0]->set_brake(0);
    motors[1]->set_brake(0);
    motors[2]->set_brake(0);
    motors[3]->set_brake(0);


    double kp = 0.03;//0.03;//128;//128; //0.035; //0.005; //0.08
    double ki = 0.1;//0.1;//53;//53; //0.01 //0.3

    motors[0]->set_kp(kp);
    motors[0]->set_ki(ki);
    motors[1]->set_kp(kp);
    motors[1]->set_ki(ki);
    motors[2]->set_kp(kp);
    motors[2]->set_ki(ki);
    motors[3]->set_kp(kp);
    motors[3]->set_ki(ki);
    double time;
    double Tick0 = G_OStimCnt;
    double Tick1 = G_OStimCnt;
    double Tick2 = G_OStimCnt;
    double Tick3 = G_OStimCnt;
    counter = 0;
    double a = 5;
    for( ;; )
    {
    	printf("speed_0 : %f,\tspeed_1 : %f,\tspeed_2 : %f,\tspeed_3 : %f,\t \n", get_speed(0),get_speed(1),get_speed(2),get_speed(3));
//    	alt_write_word(fpga_to_pi, motors[3]->get_speed_command());
    	printf("%f \t %f \n",motors[1]->get_speed_command_double(),get_speed(1));

//    	printf("SPEED : %d \n",motors[3]->get_speed_command());
    	printf("speed_x : %f, \t speed_y : %f\t\n", speed_x, speed_y);
//    	for(int j = 0; j<4 ; j++){
//    	//		printf("speed [%d]: %f\t",i,speed[i]);
//    		if (counter == 200){
//        		motors[j]->set_command(motors[j]->get_speed_command()+a);
//    		}
//    		else if(counter == 300){
//    			motors[j]->set_command(motors[j]->get_speed_command()-a);
//    			if(j==3){
//					counter = 0;
//				}
//    		}
////    		else if (counter == 350){
////				motors[j]->set_command(motors[j]->get_speed_command()-a/4);
////    			if(j==3){
////					counter = 0;
////				}
////    		}
//    	}
    	//counter++;
//		printf("counter : %d\n", counter);


    	motors[0]->set_old_speed(get_speed(0));
//    	printf("G_OS : %d\n",G_OStimCnt);
//    	printf("OS_MS_TO_TICK : %d \n",OS_MS_TO_TICK(4));
//    	printf("OS_TIMER : %f \n", OS_TIMER_US*(G_OStimCnt-Tick0));
		time = (OS_TIMER_US*(G_OStimCnt-Tick0))/1000000;
		motors[0]->set_deltaT(time);
//		motors[0]->set_voltage(20);
		motors[0]->set_speed();
		printf("time : %f \n",time);
		Tick0 = G_OStimCnt;
//		TSKsleep(OS_\MS_TO_TICK(4));

    	motors[1]->set_old_speed(-get_speed(1));
    	time = (OS_TIMER_US*(G_OStimCnt-Tick1))/1000000;
    	motors[1]->set_deltaT(time);
//    	motors[1]->set_voltage(20);
    	motors[1]->set_speed();
    	Tick1 = G_OStimCnt;
//		TSKsleep(OS_MS_TO_TICK(4));


    	motors[2]->set_old_speed(get_speed(2));
		time = (OS_TIMER_US*(G_OStimCnt-Tick2))/1000000;
		motors[2]->set_deltaT(time);
//		motors[2]->set_voltage(20);
		motors[2]->set_speed();
		Tick2 = G_OStimCnt;
//		TSKsleep(OS_MS_TO_TICK(4));

		motors[3]->set_old_speed(get_speed(3));
		time = (OS_TIMER_US*(G_OStimCnt-Tick3))/1000000;
		motors[3]->set_deltaT(time);
//		motors[3]->set_voltage(20);
		motors[3]->set_speed();
		Tick3 = G_OStimCnt;

		//CHANGES HERE
		path_plan->move_opp = 1;

    	TSKsleep(OS_MS_TO_TICK(10));


    }

}

void Task_MID_LEVEL(void){


	double xsi_in[3] = {1.0, 0.0, 0.0};
	double speed[4];
	double xsi[3];
	int i;


	for( ;; )
	{
		compute_local_velocities(xsi_in, 0.0 , xsi);
		compute_motor_velocities(xsi, speed);
		for(i = 0; i<4 ; i++){
		   motors[i]->set_command(speed[i]);
		}
		TSKsleep(OS_MS_TO_TICK(100));
	}

}

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


    tx_Identifier = 0x708;//0xabc;
	tx_Length     = 3;//8;
	tx_FrameType  = MCP2515_TX_STD_FRAME;
	tx_Data[0] = 0x1E;
	tx_Data[1] = 0x40;
	tx_Data[2] = 0x40;
	CAN_sendMsg(tx_Identifier, tx_Data, tx_Length, tx_FrameType);





    for( ;; )
    {
    	tx_Identifier = 0x408;//0xabc;
		tx_Length     = 3;//8;
		tx_FrameType  = MCP2515_TX_STD_FRAME;
		tx_Data[0] = 0x1E;
		tx_Data[1] = 0x40;
		tx_Data[2] = 0x40;
		CAN_sendMsg(tx_Identifier, tx_Data, tx_Length, tx_FrameType);
    	TSKsleep(OS_SEC_TO_TICK(1));
    	tx_Identifier = 0x408;//0xabc;
		tx_Length     = 3;//8;
		tx_FrameType  = MCP2515_TX_STD_FRAME;
		tx_Data[0] = 0x1E;
		tx_Data[1] = 0x40;
		tx_Data[2] = 0x00;
		CAN_sendMsg(tx_Identifier, tx_Data, tx_Length, tx_FrameType);
    	TSKsleep(OS_SEC_TO_TICK(1));

    }

}


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
		pio_double = -(4294967296.0 - PIO); // LE PLUS 1 EST NECESSAIRE ICI???
	} else {
		pio_double = PIO;
	}
	speed = 2*M_PI*pio_double/(max*T*ratio);
	return speed;
}

int get_neg(int uns){
	int negat;
	if(uns/65536.0>=0.5){
		negat = -(65536 - uns);
	} else {
		negat = uns;
	}
	return negat;
}
