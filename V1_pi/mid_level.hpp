using namespace std;
#include <string>



class mid_level_ctrl{
    public:
    mid_level_ctrl(double length, double width, double r_wheel, double gamma);
    void get_wheel_speed(double x_dot, double y_dot, double theta_dot, double theta);


    private:
    
    /*
        width
     <----------->
     -------------  ^
     |1|       |0|  |   l
       |       |    |   e
       |       |    |   n
       |       |    |   g
     |2|       |3|  |   t
     -------------  |   h
     
     */
    double length;
    double width;
    double gamma;
    double r_wheel;
    double wheel_speed[4];
};
