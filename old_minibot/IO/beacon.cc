#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "beacon.hh"
#include "CtrlStruct.hh"
#include "ctrl_io.h"
#include "speed_controller.hh"
#include "COM/CAN/CAN.hh"
#include "COM/SPI/Specific/SPI_CAN.hh"
#include "COM/SPI/SPI.hh"
#include <wiringPiSPI.h>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <math.h>
using namespace std;

#define CAN_BR 125e3
#define L_ID 0
#define R_ID 1

void follow_beacon(){
    // init

    double kp = 1.0;
    double ki = 10.0;

    CAN *can;
	can = new CAN(CAN_BR);
	can->configure();

    UserStruct *ustruct;
    ustruct = new UserStruct();

    CtrlIn *Cin;
    Cin = new CtrlIn();

    CtrlOut *Cout;
    Cout = new CtrlOut();

    CtrlStruct *Cstruct;
    Cstruct = new CtrlStruct();
    Cstruct->theCtrlIn = Cin;
    Cstruct->theCtrlOut = Cout;
    Cstruct->theUserStruct = ustruct;

    can->ctrl_led(1);
    
    can->ctrl_motor(1);

    can->push_TowDC(11);

    init_speed_controller(Cstruct);

    double speed[2] = {10.0, 10.0};
    double *omega = speed;
    std::fstream myfile("/home/pi/Minibot/IO/data_robot.txt", std::ios_base::in);
    double a, b, c, d;
    myfile >> a >> b >> c >> d;
    myfile.close();
    double base_speed = 10.0;
    double dist = 100.0;
    double R = 4.05; // Rayon du beacon en [cm]
    double coef = 1.0;
    double angle = 30.0;
    double angle_ref = 0.0;

    while(true){

        // 100000 for 0.1 s
        
        usleep(10000);
        myfile.open("/home/pi/Minibot/IO/data_robot.txt", std::ios_base::in);
        printf("Speed %f, %f\n", a, b);
        myfile >> a >> b >> c >> d;
        if(d < c){
            angle = 360 -c + d;
        } else {
            angle  = d - c;
        }
        dist = R/sin(3.14159*angle/(180)) *1.4;
        angle = (d + angle/2);
        if(angle > 360.0){
            angle -= 360;
        }
        if(angle > 180){
            angle -= 360;
        }
        coef = 1.0;
        printf("%f %f\n", c, d);
        printf("Angle %f\n", angle/2);
        printf("Distance %f\n", dist);
        if(dist < 35.0){
            coef = 0.0;
        }

        speed[0] = (15+15*angle/30) * coef;
        speed[1] = (15-15*angle/30)* coef;
        Cin->r_wheel_speed = a;
        Cin->l_wheel_speed = b;
        myfile.close();
        run_speed_controller(Cstruct, speed);
        can->push_PropDC(Cout->wheel_commands[0], Cout->wheel_commands[1]);

    }
    can->ctrl_motor(0);
}

void follow_beacon_drunk(){
    double kp = 1.0;
    double ki = 10.0;

    CAN *can;
	can = new CAN(CAN_BR);
	can->configure();

    UserStruct *ustruct;
    ustruct = new UserStruct();

    CtrlIn *Cin;
    Cin = new CtrlIn();

    CtrlOut *Cout;
    Cout = new CtrlOut();

    CtrlStruct *Cstruct;
    Cstruct = new CtrlStruct();
    Cstruct->theCtrlIn = Cin;
    Cstruct->theCtrlOut = Cout;
    Cstruct->theUserStruct = ustruct;

    can->ctrl_led(1);
    
    can->ctrl_motor(1);

    can->push_TowDC(11);

    init_speed_controller(Cstruct);

    double speed[2] = {10.0, 10.0};
    double *omega = speed;
    std::fstream myfile("/home/pi/Minibot/IO/data_robot.txt", std::ios_base::in);
    float a, b, c, d;
    myfile >> a >> b >> c >> d;
    myfile.close();
    double base_speed = 10.0;
    double dist = 100.0;
    double R = 4.05; // Rayon du beacon en [cm]
    double coef = 1.0;
    double angle = 30.0;
    double angle_ref = 0.0;

    while(true){

        // 100000 for 0.1 s
        
        usleep(10000);
        myfile.open("/home/pi/Minibot/IO/data_robot.txt", std::ios_base::in);
        myfile >> a >> b >> c >> d;
        if(d < c){
            angle = 360 -c + d;
        } else {
            angle  = d - c;
        }
        dist = R/sin(3.14159*angle/(180)) *1.4;
        coef = 1.0;
        printf("%f %f", c, d);
        printf("Angle %f\n", angle/2);
        printf("Distance %f\n", dist);
        if(dist < 50.0 && dist > 35.0){
            coef = 0.0;
        } else if(dist < 35.0){
            coef = -1.0;
        }

        if((c >345 || c < 15) && (d> 345 || d < 15)){
            if((c >5 || c < 15)){
                speed[0] = base_speed * coef;
                speed[1] = base_speed * coef*0.8;
            } else if((d >345 || d < 355)){
                speed[0] = base_speed * coef*0.8;
                speed[1] = base_speed * coef;
            } else {
                speed[0] = base_speed * coef;
                speed[1] = base_speed * coef;
            }
        } else if ( c < 360 - d){
            speed[0] = base_speed;
            speed[1] = -base_speed;
        } else {
            speed[0] = -base_speed;
            speed[1] = base_speed;
        }

        Cin->r_wheel_speed = a;
        Cin->l_wheel_speed = b;
        myfile.close();
        run_speed_controller(Cstruct, speed);
        can->push_PropDC(Cout->wheel_commands[0], Cout->wheel_commands[1]);
    }
    can->ctrl_motor(0);
}