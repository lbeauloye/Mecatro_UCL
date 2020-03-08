#include <cstdio>
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "can.hpp"
#include "mid_level.hpp"

// Garbage collection

int main(int argc, char *argv[])
{

    motor_card **motors =(motor_card**) calloc(4,sizeof(motor_card));
    motors[0] = new motor_card("408",1);
    motors[1] = new motor_card("408",2);
    motors[2] = new motor_card("408",1);
    motors[3] = new motor_card("708",2);
    mid_level_ctrl *ctrl = new mid_level_ctrl(10,10,3,0,motors);
    ctrl->set_wheel_speed(5,0,0,0);
}