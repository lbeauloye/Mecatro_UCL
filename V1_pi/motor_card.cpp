#include "motor_card.hpp"
#include "can.hpp"


// TODO - brake for UCL card

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

void motor_card::set_voltage(int v){
    uint8_t volt = 128*v/100.0 +128;
    volt = volt >> 2;

    if(this->type == 0){

    } else if(this->type == 1){
        send_message(this->id, "25FF" + std::to_string(volt));
    } else if(this->type == 2){
        send_message(this->id, "26FF" + std::to_string(volt));
    }
  
  
}