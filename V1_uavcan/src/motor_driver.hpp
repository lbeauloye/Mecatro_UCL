#ifndef _MOTOR_DRIVER_HPP_
#define _MOTOR_DRIVER_HPP_

#include <stdio.h>
#include <stdlib.h>
#include <uavcan/uavcan.hpp>
#include <motor/control/Voltage.hpp>
#include <motor/control/Velocity.hpp>
#include <motor/feedback/MotorPosition.hpp>

typedef enum {NO_CONTROL, VELOCITY_CONTROL, VOLTAGE_CONTROL} ControlMode;


// Class definition
class Motor {
    int node_id;
    int control_mode;
    int direction_factor;
    float correction_factor;
    uavcan::Publisher<motor::control::Velocity> *velocity_pub;
    motor::control::Velocity velocity_setpoint;
    uavcan::Publisher<motor::control::Voltage> *voltage_pub;
    motor::control::Voltage voltage_setpoint;

public:
    Motor(uavcan::INode& node, int id, int direction, float correction);
    void set_velocity(double velocity);
    void set_voltage(double voltage);
    void send_command();
};

// Function declaration
void feedback_handler_init(uavcan::INode& node);

#endif
