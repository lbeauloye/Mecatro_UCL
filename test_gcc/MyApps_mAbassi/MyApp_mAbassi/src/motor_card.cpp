#include "../inc/motor/motor_card.h"
#include "MyDriver_CAN.h"
#include <math.h>
//using namespace std;

motor_card::motor_card(uint32_t id, int type){
    this->id = id;
    this->type = type;
}


motor_card::~motor_card(){

}

void motor_card::set_kp(double kp){
    this->kp = kp;
}

void motor_card::set_ki(double ki){
    this->ki = ki;
}

// Send message to motor to control it
void motor_card::ctrl_motor(int state){
	uint8_t  tx_Data[3];
	tx_Data[0] = 0x1E;
	tx_Data[2] = state? 0x00: 0x01;
	tx_Data[1] = 0x30;
	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
}

void motor_card::set_voltage(double v){
	// Set voltage on the motor through CAN
    uint8_t volt = 128 + 128*limit(v, 99.0)/100.0;
    uint8_t command = volt >> 2;

    uint8_t  tx_Data[3];
    tx_Data[1] = 0xFF;
	tx_Data[2] = command;

    if(this->type == 0){
        
    } else if(this->type == 1){
    	tx_Data[0] = 0x25;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    	tx_Data[0] = 0x21;
    	tx_Data[2] = (volt & 0b11) + 0b10000000;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    	tx_Data[0] = 0x23;
		tx_Data[2] = 0x3F;
		CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);

    } else if(this->type == 2){
    	tx_Data[0] = 0x26;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    	tx_Data[0] = 0x22;
		tx_Data[2] = (volt & 0b11) + 0b10000000;
		CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
		tx_Data[0] = 0x24;
		tx_Data[2] = 0x3F;
		CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    }
  
    if((volt >= 127 && volt <= 128)){
    	this->set_brake(1);
    }
    else{
    	this->set_brake(0);
    }
  
}

void motor_card::set_brake(int state){
	// Set brake on the motors through CAN
	uint8_t  tx_Data[3];
	tx_Data[0] = 0x1E;
	tx_Data[2] = state? 0xFF: 0x00;
	if(this->type == 0){

	} else if(this->type == 1){
		tx_Data[1] = 0x10;
		CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME); //1E01" + state? "00": "FF");
	} else if(this->type == 2){
		tx_Data[1] = 0x20;
		CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
	}
}


void motor_card::set_command(double speed){
	// Set the speed command
	this->speed_command = speed;
}


void motor_card::init(){
	// Parameters initialization
    this->kp = 0.024;
    this->ki = 0.8;
    this->alpha = 0.0;

    this->khpi = 0.0234;
    this->deltaT = 0.04;
    this->limitI = 1.16;
    this->limitV = 0.97*12.0;
    this->gearbox = 5.8;

    this->error_integ = 0;
    this->saturation = 0;
    this->R = 2.32;
}

void motor_card::set_old_speed(double speed){
	// Set wheel speed
	this->wheel_speed = speed;
}

int motor_card::get_speed_command(){
	// Return speed command
	return (int)this->speed_command;
}

double motor_card::get_speed_command_double(){
	// Return speed command as double
	return this->speed_command;
}

void motor_card::set_deltaT(double deltaT){
	// Set time step
	this->deltaT=deltaT;
}
void motor_card::set_speed(){
    // Error on speed
	printf("speed : %f \n",this->speed_command);

    double err_w = (this->speed_command - this->wheel_speed)* this->gearbox;

    // Kp
    double v = err_w * this->kp;

    // Adding Ki
    v += this->error_integ * this->ki;// * this->deltaT ;//this->error_integ;

    // Limit on current + adding back emf
    v = v + this->khpi * this->wheel_speed*this->gearbox; // ajout this->gearbox     limit(v, this->limitI)

    // Compute saturation
    double V_such_as_Imax_l = 12.0; //this->R*this->limitI + this->khpi*this->speed_command*this->gearbox;

    if(my_limiter(v, V_such_as_Imax_l)){
        printf("saturated %f and limit is %f \n", v, V_such_as_Imax_l);
        this->error_integ = 0.0;
    }
     else{
    	 this->error_integ += err_w * this->deltaT;
     }

    printf("Voltage sent %f \n", v);
    this->set_voltage(100 * v / this->limitV);
}

double limit(double val, double limit){
	if(val > limit){
        printf("saturated %f and limit is %f \n", val, limit);
		return limit;
	}
	if(val < - limit){
		return -limit;
	}
	return val;
}

int my_limiter(double &value_to_limit, double max_value) {
	double min_value = -max_value;
	if (value_to_limit < min_value) {
		value_to_limit = min_value;
		return 1;
	}
	if (value_to_limit > max_value) {
		value_to_limit = max_value;
		return 1;
	}
	return 0;
}
