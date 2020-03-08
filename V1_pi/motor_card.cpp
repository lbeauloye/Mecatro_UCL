#include "motor_card.hpp"
#include "can.hpp"

motor_card::motor_card(string id, int type){
    this->id = id;
    this->type = type;
}

motor_card::~motor_card(){

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

    uint8_t volt = 128*limit(v, 100)/100.0 +128;
    volt = volt >> 2;

    if(this->type == 0){

    } else if(this->type == 1){
        send_message(this->id, "25FF" + std::to_string(volt));
    } else if(this->type == 2){
        send_message(this->id, "26FF" + std::to_string(volt));
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

    this->kp = 0.02;
    this->ki = 15.0;
    this->alpha = 0.3;

    this->khpi = 0.00027;
    this->deltaT = 0.04;
    this->limitI = 7.1*0.78;
    this->limitV = 0.97*24;
    this->gearbox = 14;

    this->error_integ = 0;
    this->saturation = 0;
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
    v = limit(v, this->limitI) + this->khpi * this->wheel_speed*this->gearbox;

    // Compute saturation
    this->saturation += v;

    // Limit on Voltage
    v = limit(v, this->limitV);

    this->set_speed(100 * v / this->limitV);

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
