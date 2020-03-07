using namespace std;
#include <string>



class motor_card{
    public:
    motor_card(string id, int type);
    ~motor_card();
    void set_voltage(int volt);
    void ctrl_motor(int state);
    void set_speed(int speed);


    private:
    string id;
    // 0 - CVRA
    // 1 - UCL1 for the first motor
    // 2 - UCL2 for the second motor
    int type;
};