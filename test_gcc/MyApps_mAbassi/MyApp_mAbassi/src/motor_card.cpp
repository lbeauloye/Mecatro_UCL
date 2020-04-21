#include "../inc/motor/motor_card.h"
#include "MyDriver_CAN.h"
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

void motor_card::ctrl_motor(int state){
	uint8_t  tx_Data[3];
	tx_Data[0] = 0x1E;
	tx_Data[2] = state? 0x00: 0xFF;
    if(this->type == 0){

    } else if(this->type == 1){
    	tx_Data[1] = 0x01;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME); //1E01" + state? "00": "FF");
    } else if(this->type == 2){
    	tx_Data[1] = 0x02;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    }
	//send_message(this->id,"1E3000");
}

void motor_card::set_voltage(double v){

    uint8_t volt = 128 + 128*limit(v, 99.0)/100.0;
    
    uint8_t command = volt >> 2;

    uint8_t  tx_Data[3];
    tx_Data[1] = 0xFF;
	tx_Data[2] = command;

    if(this->type == 0){
        
    } else if(this->type == 1){
    	//printf("volatge sent \n");
    	tx_Data[0] = 0x25;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    } else if(this->type == 2){
    	tx_Data[0] = 0x26;
    	CAN_sendMsg(this->id, tx_Data, 3,  MCP2515_TX_STD_FRAME);
    }
  
    if(volt >= 127 && volt <= 128){
    	this->set_brake(1);
    }
    else{
    	this->set_brake(0);
    }
  
}

void motor_card::set_brake(int state){

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
	this->speed_command = speed;
}


void motor_card::init(){

    // Remove break from motor
//    if(this->type == 0){
//        return;
//    } else if(this->type == 1){
//        //send_message(this->id, "1E1000");
//    } else if(this->type == 2){
//       // send_message(this->id, "1E2000");
//    }

    this->kp = 0.1;
    this->ki = 0.0;
    this->alpha = 0.0;

    this->khpi = 0.0234;
    this->deltaT = 0.04;
    this->limitI = 2.82 * 100;
    this->limitV = 0.97*24;
    this->gearbox = 5.8;

    this->error_integ = 0;
    this->saturation = 0;
}

void motor_card::set_old_speed(double speed){
    this->wheel_speed = speed; 
    //printf("Previous speed = %f \n",speed);
}

void motor_card::set_speed(){
    // Error on speed
	//printf("wheel speed : %f \n",this->wheel_speed);
	//printf("speed : %f \n",speed);

    double err_w = (this->speed_command - this->wheel_speed)* this->gearbox;

    // Kp
    double v = err_w * this->kp;
    // Ki - saturation
    this->error_integ += err_w * this->ki * this->deltaT + this->saturation * this->alpha;

    // Adding Ki
    v += this->error_integ;

    // Reset saturation
    this->saturation = - v;

    // Limit on current + adding back emf
    v = limit(v, this->limitI) + this->khpi * this->wheel_speed * this->gearbox; // ajout this->gearbox
    // Compute saturation
    this->saturation += v;

    // Limit on Voltage
    v = limit(v, this->limitV);

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

/*
string int_to_hex(int a){
    string str = "";
    switch(a/16){
        case 0:
            str += "0";
        break;
        case 1:
            str += "1";
        break;
        case 2:
            str += "2";
        break;
        case 3:
            str += "3";
        break;
        case 4:
            str += "4";
        break;
        case 5:
            str += "5";
        break;
        case 6:
            str += "6";
        break;
        case 7:
            str += "7";
        break;
        case 8:
            str += "8";
        break;
        case 9:
            str += "9";
        break;
        case 10:
            str += "A";
        break;
        case 11:
            str += "B";
        break;
        case 12:
            str += "C";
        break;
        case 13:
            str += "D";
        break;
        case 14:
            str += "E";
        break;
        case 15:
            str += "F";
        break;
    }
    switch(a % 16){
        case 0:
            str += "0";
        break;
        case 1:
            str += "1";
        break;
        case 2:
            str += "2";
        break;
        case 3:
            str += "3";
        break;
        case 4:
            str += "4";
        break;
        case 5:
            str += "5";
        break;
        case 6:
            str += "6";
        break;
        case 7:
            str += "7";
        break;
        case 8:
            str += "8";
        break;
        case 9:
            str += "9";
        break;
        case 10:
            str += "A";
        break;
        case 11:
            str += "B";
        break;
        case 12:
            str += "C";
        break;
        case 13:
            str += "D";
        break;
        case 14:
            str += "E";
        break;
        case 15:
            str += "F";
        break;
    }
    return str;
}
*/
