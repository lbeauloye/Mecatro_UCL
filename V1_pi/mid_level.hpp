using namespace std;
#include <string>
#include "motor_card.hpp"


class mid_level_ctrl{
    public:
    //Constructor
    mid_level_ctrl(double length, double width, double r_wheel, double gamma, motor_card* mc[4]);
    //Destructor
    ~mid_level_ctrl();
    void set_wheel_speed(double x_dot, double y_dot, double theta_dot, double theta);


    private:
    
    /*
        width
     <----------->
     -------------  ^
     |0|       |2|  |   l
       |       |    |   e
       |       |    |   n
       |       |    |   g
     |1|       |3|  |   t
     -------------  |   h
     
     */
    double length;
    double width;
    double gamma;
    double r_wheel;
    motor_card** wheel;
};
