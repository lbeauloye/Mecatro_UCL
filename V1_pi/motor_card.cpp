#include "motor_card.hpp"
#include "can.hpp"

motor_card::motor_card(string id, int type){
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
    if(this->type == 0){

    } else if(this->type == 1){
        send_message(this->id, "1E01" + state? "00": "FF");
    } else if(this->type == 2){
        send_message(this->id, "1E02" + state? "00": "FF");
    }
}

void motor_card::set_voltage(double v){

    uint8_t volt = 128 + 128*limit(v, 99.0)/100.0;
    
    volt = volt >> 2;

    if(this->type == 0){
        
    } else if(this->type == 1){
        send_message(this->id, "25FF" + int_to_hex(volt));
    } else if(this->type == 2){
        send_message(this->id, "26FF" + int_to_hex(volt));
    }
  
  
}

void motor_card::init(){

    // Remove break from motor
    if(this->type == 0){
        return;
    } else if(this->type == 1){
        send_message(this->id, "1E1000");
    } else if(this->type == 2){
        send_message(this->id, "1E2000");
    }

    this->kp = 0.06337;
    this->ki = 0.622;
    this->alpha = 0.0;

    this->khpi = 0.037;
    this->deltaT = 0.04;
    this->limitI = 5.84 *100;
    this->limitV = 0.97*24;
    this->gearbox = 14;

    this->error_integ = 0;
    this->saturation = 0;
}

void motor_card::set_old_speed(double speed){
    this->wheel_speed = speed;
}

void motor_card::set_speed(double speed){
    // Error on speed
    double err_w = (speed - this->wheel_speed)* this->gearbox;

    // Kp
    double v = err_w * this->kp;
    // Ki - saturation
    this->error_integ += err_w * this->ki * this->deltaT + this->saturation * this->alpha;

    // Adding Ki
    v += this->error_integ;

    // Reset saturation
    this->saturation = - v;

    // Limit on current + adding back emf
    v = limit(v, this->limitI) + this->khpi * this->wheel_speed;

    // Compute saturation
    this->saturation += v;

    // Limit on Voltage
    v = limit(v, this->limitV);

    this->set_voltage(100 * v / this->limitV);

}




double limit(double val, double limit){
	if(val > limit){
		return limit;
	}
	if(val < - limit){
		return - limit;
	}
	return val;
}

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