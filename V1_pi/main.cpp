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
    send_message("408", "1EFFFF");
    // Terminate program
   exit(signum);
}

int main(int argc, char *argv[])
{

    motor_card **motors =(motor_card**) calloc(4,sizeof(motor_card));
    motors[0] = new motor_card("708",1);
    motors[1] = new motor_card("708",2);
    motors[2] = new motor_card("408",1);
    motors[3] = new motor_card("408",2);
    motors[0]->init();
    motors[1]->init();
    motors[2]->init();
    motors[3]->init();
    motors[0]->ctrl_motor(1);
    motors[1]->ctrl_motor(1);
    motors[2]->ctrl_motor(1);
    motors[3]->ctrl_motor(1);
    //motors[0]->set_voltage(-100);
    signal(SIGINT, signal_callback_handler);
	motors[0]->set_voltage(0); // set_voltage(voltage);
	motors[1]->set_voltage(0);
	motors[2]->set_voltage(0);
	motors[3]->set_voltage(0);

     //usleep(1000000);
    double a, b, c, d, kp, ki;// = 0.06337    = 0.622
    std::fstream myfile("../speed_robot.txt", std::ios_base::in);
    myfile >> a >> b >> c >> d;
    myfile.close();
    int cnt = 0;
    double speed = 0;
    int n = 60;
    double voltage = 0;
    string direction;

	printf(" init %f %f %f %f\n", a, b,c,d);
	
    motors[0]->set_old_speed(d);
    motors[1]->set_old_speed(c);
    motors[2]->set_old_speed(a);
    motors[3]->set_old_speed(b);

    while(true){
	
        //std::fstream myfile("../Voltage_speed.txt", std::ios_base::in);
    	//myfile >> voltage;
    	//myfile.close();
        
        myfile.open("../speed.txt", std::ios_base::in);
    	myfile >> speed;
    	myfile.close();
        
        myfile.open("../direction.txt", std::ios_base::in);
    	myfile >> direction;
    	myfile.close();

        usleep(10000);
	
        myfile.open("../speed_robot.txt", std::ios_base::in);
        myfile >> a >> b >> c >> d;
        myfile.close();

        myfile.open("../pid.txt", std::ios_base::in);
        myfile >> kp >> ki;
        myfile.close();

        motors[0]->set_kp(kp);
        motors[1]->set_kp(kp);
        motors[2]->set_kp(kp);
        motors[3]->set_kp(kp);
        motors[0]->set_ki(ki);
        motors[1]->set_ki(ki);
        motors[2]->set_ki(ki);
        motors[3]->set_ki(ki);

        //printf("%f %f\n", kp, ki);

        motors[0]->set_old_speed(d);
        motors[1]->set_old_speed(c);
        motors[2]->set_old_speed(a);
        motors[3]->set_old_speed(b);
        printf("%f %f %f %f\n", a, b,c,d);

        cnt ++;
        
        
       	//speed = 30;
        //if(cnt == 5*n){
        //    speed = 20;
        //} else if(cnt == 2*n){
        //    speed = -20;
        //} else if(cnt == 10*n){
        //    speed = 0;
        //    cnt = 0;
       // }
        
        if(direction=="F"){
            //motors[0]->set_speed(speed);
            motors[1]->set_speed(-speed);
            motors[2]->set_speed(-speed);
            motors[3]->set_speed(-speed/4);
        }
        if(direction=="B"){
            motors[0]->set_speed(-speed);
            motors[1]->set_speed(speed);
            motors[2]->set_speed(speed);
            motors[3]->set_speed(speed/4);
        }
        if(direction=="L"){
            motors[0]->set_speed(speed);
            motors[1]->set_speed(speed);
            motors[2]->set_speed(-speed);
            motors[3]->set_speed(speed/4);
        }
        if(direction=="R"){
            motors[0]->set_speed(-speed);
            motors[1]->set_speed(-speed);
            motors[2]->set_speed(speed);
            motors[3]->set_speed(-speed/4);
        }
        
        motors[0]->set_voltage(15);
        
     	//motors[1]->set_speed(-speed);
     	//motors[2]->set_speed(-speed);
     	//motors[3]->set_speed(-speed);
   	//motors[0]->set_voltage(0); // set_voltage(voltage);
	//motors[1]->set_voltage(0);
	//motors[2]->set_voltage(0);
	//motors[3]->set_voltage(0);
        system("rm -rf ../data_robot.txt");
        std::fstream mydata("../data_robot.txt", std::ios_base::out);
        mydata.seekp(0);
        mydata << speed << " " << d;//a;
        mydata.close();
    }


}
