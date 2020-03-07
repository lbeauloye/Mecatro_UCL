#include "motor_driver.hpp"


/*
 * Class constructor
 * id: CAN bus node id
 * direction: 1 or -1
 * correction: calibration/correction factor
 */
Motor::Motor(uavcan::INode& node, int id, int direction, float correction) {
    node_id = id;
    control_mode = NO_CONTROL;
    direction_factor = direction;
    correction_factor = correction;

    // Initialize the publishers
    int res;

    velocity_pub = new uavcan::Publisher<motor::control::Velocity>(node);
    res = velocity_pub->init();
    if (res < 0) {
        printf("Failed to init the velocity publisher.\n");
        exit(EXIT_FAILURE);
    }
    velocity_setpoint.node_id = node_id;
    velocity_setpoint.velocity = 0.0;

    voltage_pub = new uavcan::Publisher<motor::control::Voltage>(node);
    res = voltage_pub->init();
    if (res < 0) {
        printf("Failed to init the voltage controller.\n");
        exit(EXIT_FAILURE);
    }
    voltage_setpoint.node_id = node_id;
    voltage_setpoint.voltage = 0.0;
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

/*
 *  Feedback handler for the motors
 *  Handles velocity & position data streams
 *  NOTE: the stream/motor_pos parameter must be non-zero for a stream to exist.
 */
 static void callback(const uavcan::ReceivedDataStructure<motor::feedback::MotorPosition>& msg) {

     // We get the source of the message
     int id = msg.getSrcNodeID().get();

     // Print data
     printf("Motor no.%d velocity: %f\n", id, msg.velocity);

     // We save the data
     // FILE *fp = fopen("speed_data.txt", "a");    // Add an "ID" distinction
     // fprintf(fp, "%f, %f\n", msg.velocity, msg.position);
 }

void feedback_handler_init(uavcan::INode& node) {
    int res;

    // Velocity feedback
    static uavcan::Subscriber<motor::feedback::MotorPosition> position_sub(node);
    res = position_sub.start(callback);
    if (res < 0) {
        printf("Failed to init motor feedback !\n");
        exit(EXIT_FAILURE);
    }
}
