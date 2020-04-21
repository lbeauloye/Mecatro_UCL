#ifndef _MOTOR_CARD_H_
#define _MOTOR_CARD_H_

//#include <string>
//using namespace std;
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//#include "socal/alt_gpio.h"
//#include "socal/hps.h"
//#include "socal/socal.h"
//
//#include "alt_generalpurpose_io.h"
//#include "alt_interrupt.h"
//#include "hps_0.h"      /* MUST include "SAL.H" and not uAbassi.h        */
//#include "Platform.h"         /* Everything about the target platform is here  */
//#include "HWinfo.h"           /* Everything about the target hardware is here  */



#ifdef __cplusplus
extern "C"
{
#endif



#ifdef __cplusplus
}
#endif

double limit(double val, double limit);
//string int_to_hex(int a);

class motor_card{
    public:
    motor_card(uint32_t id, int type);
    ~motor_card();
    void init();
    void set_voltage(double volt);
    void ctrl_motor(int state);
    void set_speed();
    void set_command(double speed);
    void set_old_speed(double speed);
    void set_kp(double kp);
    void set_ki(double ki);
    void set_brake(int state);


    private:
    uint32_t id;
    // 0 - CVRA
    // 1 - UCL1 for the first motor
    // 2 - UCL2 for the second motor
    int type;

    // For UCL card
    // low-level controller
    double speed_command;
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

#endif
