#include "motor_driver.hpp"

/*
 * Class constructor
 * id: CAN bus node id
 * direction: 1 or -1
 * correction: calibration/correction factor
 */
Motor::Motor(uavcan::INode& node, int id, int direction, float correction) {
    this->node_id = id;
    this->control_mode = NO_CONTROL;
    this->direction_factor = direction;
    this->correction_factor = correction;

    // Initialize the publishers
    int res;

    this->velocity_pub = new uavcan::Publisher<motor::control::Velocity>(node);
    res = velocity_pub->init();
    if (res < 0) {
        printf("Failed to init the velocity publisher.\n");
        exit(EXIT_FAILURE);
    }
    this->velocity_setpoint.node_id = node_id;
    this->velocity_setpoint.velocity = 0.0;

    this->voltage_pub = new uavcan::Publisher<motor::control::Voltage>(node);
    res = voltage_pub->init();
    if (res < 0) {
        printf("Failed to init the voltage controller.\n");
        exit(EXIT_FAILURE);
    }
    this->voltage_setpoint.node_id = node_id;
    this->voltage_setpoint.voltage = 0.0;

    // PI controller for this motor
    this->ctrl = new PIController();
}

/*
 *  Change the velocity setpoint.
 */
void Motor::set_velocity(double velocity) {
    control_mode = VELOCITY_CONTROL;
    velocity_setpoint.velocity = velocity * direction_factor * correction_factor;
}

/*
 * Change the voltage setpoint.
 */
void Motor::set_voltage(double voltage) {
    control_mode = VOLTAGE_CONTROL;
    voltage_setpoint.voltage = voltage * direction_factor;
}

/*
 *  Send the command to the motor
 */
void Motor::send_command() {
    switch(control_mode) {
        case NO_CONTROL:
            ;   // Do nothing...
            break;

        case VOLTAGE_CONTROL:
            voltage_pub->broadcast(voltage_setpoint);
            break;

        case VELOCITY_CONTROL:
            velocity_pub->broadcast(velocity_setpoint);
            break;

        default:
            printf("Unknown control mode\n");
            // Do nothing...
    }
}
