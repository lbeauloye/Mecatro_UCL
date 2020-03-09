#include <cstdio>
#include <stdlib.h> 
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "can.hpp"
#include "mid_level.hpp"
#include <signal.h>
#include <iostream>
#include <fstream>

// Garbage collection

void signal_callback_handler(int signum) {
   send_message("408", "1EFFFF");
    // Terminate program
   exit(signum);
}

int main(int argc, char *argv[])
{

    motor_card **motors =(motor_card**) calloc(4,sizeof(motor_card));
    motors[0] = new motor_card("408",2);
    motors[0]->init();
    motors[0]->ctrl_motor(1);
    //motors[0]->set_voltage(-100);
    signal(SIGINT, signal_callback_handler);

    double a, b, c, d;
    std::fstream myfile("../speed_robot.txt", std::ios_base::in);
    myfile >> a >> b >> c >> d;
    myfile.close();
    int cnt = 0;
    double speed = 0;
    int n = 30;
    while(true){
        
        usleep(10000);
        myfile.open("../speed_robot.txt", std::ios_base::in);
        myfile >> a >> b >> c >> d;
        //printf("Speed %f, %f, %f, %f\n", a, b, c, d);
        myfile.close();

        motors[0]->set_old_speed(b);
        cnt ++;

        if(cnt == n){
            speed = 20;
            printf("positif\n");
            motors[0]->set_voltage(100);
        } else if(cnt == 2*n){
            speed = -20;
            printf("négatif\n");
            //motors[0]->set_voltage(-100);
        } else if(cnt == 3*n){
            printf("zéro\n");
            speed = 0;
            cnt = 0;
            //motors[0]->set_voltage(0);
        }
        /*
        motors[0]->set_speed(speed);
        */

    }


}