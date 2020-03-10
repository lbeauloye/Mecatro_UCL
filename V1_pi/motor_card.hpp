using namespace std;
#include <string>

double limit(double val, double limit);
string int_to_hex(int a);

class motor_card{
    public:
    motor_card(string id, int type);
    ~motor_card();
    void init();
    void set_voltage(double volt);
    void ctrl_motor(int state);
    void set_speed(double speed);
    void set_old_speed(double speed);
    void set_kp(double kp);
    void set_ki(double ki);


    private:
    string id;
    // 0 - CVRA
    // 1 - UCL1 for the first motor
    // 2 - UCL2 for the second motor
    int type;

    // For UCL card
    // low-level controller
    double wheel_speed;
    double error_integ;
    double saturation;
    double alpha;
    double kp;
    double ki;
    double gearbox;
    double khpi;
    double deltaT;
    double limitV;
    double limitI;
};