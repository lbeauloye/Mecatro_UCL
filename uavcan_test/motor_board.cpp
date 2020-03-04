#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <uavcan/uavcan.hpp>

#include <uavcan/protocol/debug/LogMessage.hpp>      // For purposes of example; not actually necessary.

/*
 * The custom data types.
 * If a data type has a default Data Type ID, it will be registered automatically once included
 * (the registration will be done before main() from a static constructor).
 */
#include <motor/control/Voltage.hpp>
#include <motor/control/Velocity.hpp>
#include <motor/config/EnableMotor.hpp>

using motor::control::Voltage;
using motor::config::EnableMotor;

extern uavcan::ICanDriver& getCanDriver();
extern uavcan::ISystemClock& getSystemClock();

constexpr unsigned NodeMemoryPoolSize = 16384;

int main(int argc, const char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <node-id> <remote-node-id>" << std::endl;
        return 1;
    }

    const uavcan::NodeID self_node_id = std::stoi(argv[1]);
    const uavcan::NodeID remote_node_id = std::stoi(argv[2]);

    uavcan::Node<NodeMemoryPoolSize> node(getCanDriver(), getSystemClock());
    node.setNodeID(self_node_id);
    node.setName("motor.test");

    const int node_start_res = node.start();
    if (node_start_res < 0)
    {
        throw std::runtime_error("Failed to start the node; error: " + std::to_string(node_start_res));
    }

    uavcan::Publisher<motor::control::Voltage> pub_volt(node);
    const int kv_pub_init_res = pub_volt.init();
    if (kv_pub_init_res < 0)
    {
        throw std::runtime_error("Failed to start the publisher; error: " + std::to_string(kv_pub_init_res));
    }

    uavcan::Publisher<motor::control::Velocity> pub_vel(node);
    pub_vel.init();

    uavcan::ServiceClient<motor::config::EnableMotor> client_en_motor(node);

    client_en_motor.setCallback([](const uavcan::ServiceCallResult<motor::config::EnableMotor>& res)
        {
            std::cout << res << std::endl;
        });

    motor::control::Voltage volt_ins;  // Always zero initialized
    volt_ins.voltage = 8;
    volt_ins.node_id = 125;

    motor::control::Velocity vel_ins;
    vel_ins.node_id = 125;
    vel_ins.velocity = 10;
    
    motor::config::EnableMotor::Request en_motor = motor::config::EnableMotor::Request();
    en_motor.enable = true;


    node.setModeOperational();

    while (true)
    {
        const int spin_res = node.spin(uavcan::MonotonicDuration::fromMSec(100));
        if (spin_res < 0)
        {
            std::cerr << "Transient failure: " << spin_res << std::endl;
        }
         pub_vel.broadcast(vel_ins);
        // pub_volt.broadcast(volt_ins);
    }


}

