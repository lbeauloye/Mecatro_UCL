
#include "can.hpp"
#include "mid_level.hpp"
#include <iostream>
#include <fstream>
#include <math.h>

// TODO - brake for UCL card

mid_level_ctrl::mid_level_ctrl(double length, double width, double r_wheel, double gamma, motor_card* mc[4]){
    this->length = length;
    this->width = width;
    this->gamma = gamma;
    this->r_wheel = r_wheel;
    this->wheel = mc;
}

mid_level_ctrl::~mid_level_ctrl(){
    free(this->wheel[0]);
    free(this->wheel[1]);
    free(this->wheel[2]);
    free(this->wheel[3]);
    free(this->wheel);
}




void mid_level_ctrl::set_wheel_speed(double theta_dot, double y_dot, double x_dot, double theta){
    
    double a = this->length/2;
    double b = this->width/2;
    double l = sqrt(a*a + b*b);
    double r = this->r_wheel;
    double gamma = this->gamma;
    double alpha1 = atan(a/b);
    double alpha2 = M_PI-atan(a/b);
    double alpha3 = atan(a/b)-M_PI;
    double alpha4 = -atan(a/b);
    
    double beta1 = alpha1;
    double beta2 = alpha2;
    double beta3 = alpha3;
    double beta4 = alpha4;

    std::fstream myfile("../speed_robot.txt", std::ios_base::in);
    double FL, RL, FR, RR;
    myfile >> FL >> RL >> FR >> RR;
    myfile.close();
    
    this->wheel[0]->set_old_speed(FL);
    this->wheel[0]->set_speed(x_dot*((cos(theta)*sin(alpha1 + beta1 + gamma))/(r*cos(gamma)) + (sin(theta)*cos(alpha1 + beta1 + gamma))/(r*cos(gamma))) - y_dot*((cos(theta)*cos(alpha1 + beta1 + gamma))/(r*cos(gamma)) - (sin(theta)*sin(alpha1 + beta1 + gamma))/(r*cos(gamma))) - (l*theta_dot*cos(beta1 + gamma))/(r*cos(gamma))) ;
    
    this->wheel[1]->set_old_speed(RL);
    this->wheel[1]->set_speed( x_dot*((cos(theta)*sin(alpha3 + beta3 + gamma))/(r*cos(gamma)) + (sin(theta)*cos(alpha3 + beta3 + gamma))/(r*cos(gamma))) - y_dot*((cos(theta)*cos(alpha3 + beta3 + gamma))/(r*cos(gamma)) - (sin(theta)*sin(alpha3 + beta3 + gamma))/(r*cos(gamma))) - (l*theta_dot*cos(beta3 + gamma))/(r*cos(gamma))) ;
    
    this->wheel[2]->set_old_speed(FR);
    this->wheel[2]->set_speed( x_dot*((cos(alpha2 + beta2 - gamma)*sin(theta))/(r*cos(gamma)) + (sin(alpha2 + beta2 - gamma)*cos(theta))/(r*cos(gamma))) - y_dot*((cos(alpha2 + beta2 - gamma)*cos(theta))/(r*cos(gamma)) - (sin(alpha2 + beta2 - gamma)*sin(theta))/(r*cos(gamma))) - (l*theta_dot*cos(beta2 - gamma))/(r*cos(gamma)));

    this->wheel[3]->set_old_speed(RR);
    this->wheel[3]->set_speed( x_dot*((cos(alpha4 + beta4 - gamma)*sin(theta))/(r*cos(gamma)) + (sin(alpha4 + beta4 - gamma)*cos(theta))/(r*cos(gamma))) - y_dot*((cos(alpha4 + beta4 - gamma)*cos(theta))/(r*cos(gamma)) - (sin(alpha4 + beta4 - gamma)*sin(theta))/(r*cos(gamma))) - (l*theta_dot*cos(beta4 - gamma))/(r*cos(gamma)));
}
