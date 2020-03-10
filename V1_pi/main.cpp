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
   send_message("708", "1EFFFF");
    // Terminate program
   exit(signum);
}

int main(int argc, char *argv[])
{

    motor_card **motors =(motor_card**) calloc(4,sizeof(motor_card));
    motors[0] = new motor_card("708",1);
    motors[0]->init();
    motors[0]->ctrl_motor(1);
    //motors[0]->set_voltage(-100);
    signal(SIGINT, signal_callback_handler);


    double a, b, c, d, kp = 0.06337, ki = 0.622;
    std::fstream myfile("../speed_robot.txt", std::ios_base::in);
    myfile >> a >> b >> c >> d;
    myfile.close();
    int cnt = 0;
    double speed = 0;
    int n = 60;


    while(true){
        
        usleep(10000);
        myfile.open("../speed_robot.txt", std::ios_base::in);
        myfile >> a >> b >> c >> d;
        myfile.close();

        myfile.open("../pid.txt", std::ios_base::in);
        myfile >> kp >> ki;
        myfile.close();

        motors[0]->set_kp(kp);
        motors[0]->set_ki(ki);

        //printf("%f %f\n", kp, ki);

        motors[0]->set_old_speed(d);
        cnt ++;

        if(cnt == n){
            speed = 20;
        } else if(cnt == 2*n){
            speed = -20;
        } else if(cnt == 3*n){
            speed = 0;
            cnt = 0;
        }
        
        motors[0]->set_speed(speed);
        //motors[0]->set_voltage(100);
        system("rm -rf ../data_robot.txt");
        std::fstream mydata("../data_robot.txt", std::ios_base::out);
        mydata.seekp(0);
        mydata << speed << " " << d;
        mydata.close();
    }


}