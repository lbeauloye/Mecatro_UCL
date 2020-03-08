
#include "can.hpp"
#include "mid_level.hpp"
#include <math.h>

// TODO - brake for UCL card

mid_level_ctrl::mid_level_ctrl(double length, double width, double r_wheel, double gamma){
    this->length = length;
    this->width = width;
    this->gamma = gamma;
    this->r_wheel = r_wheel;
    for(int i = 0; i<4; i++){
        this->wheel_speed[i] = 0.0;
    }
}


void mid_level_ctrl::get_wheel_speed(double x_dot, double y_dot, double theta_dot, double theta){
    
    double a = this->length/2;
    double b = this->width/2;
    double l = sqrt(a*a + b*b);
    double r_wheel = this->r_wheel;
    double gamma = this->gamma;
    double alpha0 = atan(a/b);
    double alpha1 = M_PI-atan(a/b);
    double alpha2 = atan(a/b)-M_PI;
    double alpha3 = -atan(a/b);
    
    double beta0 = alpha0;
    double beta1 = alpha1;
    double beta2 = alpha2;
    double beta3 = alpha3;
    
    this->wheel_speed[0] = (x_dot*cos(theta)*sin(alpha0 + beta0 + gamma) + sin(theta)*cos(alpha0 + beta0 + gamma)- y_dot*cos(theta)*cos(alpha0 + beta0 + gamma) - sin(theta)*sin(alpha0 + beta0 + gamma) - l*theta_dot*cos(beta0 + gamma))/(r_wheel*cos(gamma)) ;
    
    this->wheel_speed[1] = (x_dot*cos(alpha1 + beta1 - gamma)*sin(theta) + sin(alpha1 + beta1 - gamma)*cos(theta) - y_dot*cos(alpha1 + beta1 - gamma)*cos(theta) - sin(alpha1 + beta1 - gamma)*sin(theta) - l*theta_dot*cos(beta1 - gamma))/(r_wheel*cos(gamma));
    
    this->wheel_speed[2] = (x_dot*cos(theta)*sin(alpha2 + beta2 + gamma) + sin(theta)*cos(alpha2 + beta2 + gamma)- y_dot*cos(theta)*cos(alpha2 + beta2 + gamma) - sin(theta)*sin(alpha2 + beta2 + gamma) - l*theta_dot*cos(beta2 + gamma))/(r_wheel*cos(gamma)) ;
    
    this->wheel_speed[3] = (x_dot*cos(alpha3 + beta3 - gamma)*sin(theta) + sin(alpha3 + beta3 - gamma)*cos(theta) - y_dot*cos(alpha3 + beta1 - gamma)*cos(theta) - sin(alpha3 + beta3 - gamma)*sin(theta) - l*theta_dot*cos(beta3 - gamma))/(r_wheel*cos(gamma));
    

}
