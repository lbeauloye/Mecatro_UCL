#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <stdlib.h>
#include <uavcan/uavcan.hpp>

#include "motor_driver.hpp"
#include "middle_level.hpp"


// Platform-dependant functions to initialize the CAN node
extern uavcan::ICanDriver& getCanDriver();
extern uavcan::ISystemClock& getSystemClock();

constexpr unsigned NodeMemoryPoolSize = 16384;
const uavcan::NodeID node_id = 1;


// Custom data types
// #include <motor/config/EnableMotor.hpp>

int main() {
    int res;

    // Initialize the node
    uavcan::Node<NodeMemoryPoolSize> node(getCanDriver(), getSystemClock());
    node.setNodeID(node_id);
    node.setName("node.master");

    // Start the node
    res = node.start();
    if (res < 0) {
        printf("Failed to start the node.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the motor
    Motor motor_back_left = Motor(node, 122, 1, 1);
    Motor motor_back_right = Motor(node, 83, -1, 1);
    Motor motor_front_right = Motor(node, 82, -1, 1);
    Motor motor_front_left = Motor(node, 121, 1, 1);
    double w_ref[4];
    double xsi[3] = {.0, .1, .0};
    compute_motor_velocities(xsi, w_ref);
    for (int i = 0; i < 4; i++) {
        printf("Wheel number %d: w = %f [rad/s]\n", i, w_ref[i]);
    }

    // Initialize timer
    int t = 0;

    // Velocity Subscriber
    static uavcan::Subscriber<motor::feedback::MotorPosition> position_sub(node);
    res = position_sub.start(
        [&](const uavcan::ReceivedDataStructure<motor::feedback::MotorPosition>& msg) {
            // Get message info
            int id = msg.getSrcNodeID().get();
            double w_mes = msg.velocity;
            t += 1;

            // Fetch velocity ref
            // double w_ref = 0;
            // if (t > 15e2 * 4) {
            //     w_ref = 0;
            // } else if (t > 10e2 * 4) {
            //     w_ref = -3;
            // } else {
            //     w_ref = 3;
            // }

            // Command the motors
            double voltage;

            switch (id) {
                case 82:    // FRONT RIGHT (-1)
                    voltage = motor_front_right.ctrl->pi_process(-w_ref[0], w_mes);
                    motor_front_right.set_voltage(voltage);
                    motor_front_right.send_command();
                    break;

                case 83:    // BACK RIGHT (-1)
                    voltage = motor_back_right.ctrl->pi_process(-w_ref[3], w_mes);
                    motor_back_right.set_voltage(voltage);
                    motor_back_right.send_command();
                    break;

                case 121:   // FRONT LEFT (1)
                    voltage = motor_front_left.ctrl->pi_process(w_ref[1], w_mes);
                    motor_front_left.set_voltage(voltage);
                    motor_front_left.send_command();
                    break;

                case 122:   // BACK LEFT (1)
                    voltage = motor_back_left.ctrl->pi_process(w_ref[2], w_mes);
                    motor_back_left.set_voltage(voltage);
                    motor_back_left.send_command();
                    break;
            }

            // Print data
            // printf("Motor no.%d velocity: %f\n", id, w_mes);
        }
    );
    if (res < 0) {
        printf("Failed to init motor feedback !\n");
        exit(EXIT_FAILURE);
    }

    // Initialization finished !
    node.setModeOperational();


    // Infinite loop
    while(true) {
        // If there is nothing to do...
        res = node.spin(uavcan::MonotonicDuration::fromMSec(100));
        if (res < 0) {
            printf("Spin fail.\n");
            exit(EXIT_FAILURE);
        }
    }
}
