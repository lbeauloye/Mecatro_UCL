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

    // Velocity Subscriber
    feedback_handler_init(node);

    // Initialize motor
    Motor motor = Motor(node, 82, -1, 1);
    // motor.set_voltage(6.32);
    motor.set_velocity(100.0);

    // Enable Publisher
    // uavcan::ServiceClient<motor::config::EnableMotor> client_en_motor(node);
    // client_en_motor.setCallback([](const uavcan::ServiceCallResult<motor::config::EnableMotor>& res)
    // {
    //     std::cout << res << std::endl;
    // });
    // motor::config::EnableMotor::Request en_motor = motor::config::EnableMotor::Request();
    // en_motor.enable = true;

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

        // Send command
        motor.send_command();
    }
}
