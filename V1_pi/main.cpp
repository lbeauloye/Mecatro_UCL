#include <cstdio>
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "can.hpp"
#include "mid_level.hpp"
#include <signal.h>
#include "math.h"


// Garbage collection

void signal_callback_handler(int signum) {
   send_message("408", "1EFFFF");
   send_message("508", "1EFFFF");
    // Terminate program
   exit(signum);
}


int main(int argc, char *argv[])
{
    signal(SIGINT, signal_callback_handler);
    motor_card **motors =(motor_card**) calloc(4,sizeof(motor_card));
    motors[0] = new motor_card("508",2);
    motors[1] = new motor_card("508",1);
    motors[2] = new motor_card("408",1);
    motors[3] = new motor_card("408",2);
    motors[0]->init();
    motors[1]->init();
    motors[2]->init();
    motors[3]->init();
    //motors[0]->set_voltage(50);
    mid_level_ctrl *ctrl = new mid_level_ctrl(0.13,0.24,0.03,3.1415926537/4,motors);
    
    int cnt = 0;
    
    while(cnt < 200){
        printf("avancer\n");
        ctrl->set_wheel_speed(sin(cnt*2*M_PI/50.0)*7,0.7,0,0);
        usleep(10000);
        cnt ++;
    }

    cnt = 0;

    while(cnt < 35){
        ctrl->set_wheel_speed(0,0,0.3,0);
        usleep(10000);
        cnt ++;
    }

    cnt = 0;

    while(cnt < 80){
        ctrl->set_wheel_speed(0,-1.1,0,0);
        usleep(10000);
        cnt ++;
    }

    cnt = 0;

    while(cnt < 80){
        ctrl->set_wheel_speed(-8,0,0,0);
        usleep(10000);
        cnt ++;
    }


    /*
    ctrl->set_wheel_speed(0,0,0,0);
    usleep(10000);

    while(cnt < 60){
        printf("translater\n");
        ctrl->set_wheel_speed(15,0,0,0);
        usleep(10000);
        cnt ++;
    }
    */

    ctrl->set_wheel_speed(0,0,0,0);
    usleep(10000);
    /*
    while(cnt < 90){
        printf("tourner\n");
        ctrl->set_wheel_speed(0,0,0.4,0);
        usleep(10000);
        cnt ++;
    }*/
    
   send_message("408", "1EFFFF");
   send_message("508", "1EFFFF");
}