#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <stdlib.h>
#include <uavcan/uavcan.hpp>

#include "motor_driver.hpp"


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
    Motor motor = Motor(node, 122, -1, 1);
    // motor.set_voltage(3);

    // Velocity Subscriber
    static uavcan::Subscriber<motor::feedback::MotorPosition> position_sub(node);
    res = position_sub.start(
        [&](const uavcan::ReceivedDataStructure<motor::feedback::MotorPosition>& msg) {
            // Get message info
            int id = msg.getSrcNodeID().get();
            double w_mes = msg.velocity;

            // Command the motor
            double voltage = motor.ctrl->pi_process(20, w_mes);
            // printf("Applied command: %f [V]\n", voltage);
            motor.set_voltage(voltage);
            motor.send_command();

            // Print data
            printf("Motor no.%d velocity: %f\n", id, w_mes);
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

        // motor.send_command();
    }
}
