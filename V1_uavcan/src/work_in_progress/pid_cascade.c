#include "pid_cascade.h"
#include "math.h"
#include <filter/basic.h>

float periodic_error(float err)
{
    err = fmodf(err, 2 * M_PI);
    if (err > M_PI) {
        return err - 2 * M_PI;
    }
    if (err < -M_PI) {
        return err + 2 * M_PI;
    }
    return err;
}

void pid_cascade_control(struct pid_cascade_s* ctrl, float transmission)
{
    // position control
    // We cannot do position control anymore, since the position is given in
    // terms of output shaft position instead of the motor's shaft position.
    // Our controller (at UCLouvain) works for the motor's shaft position/velocity.

    // velocity control
    float vel_ctrl_current;
    if (ctrl->setpts.velocity_control_enabled) {
        float velocity_setpt = ctrl->setpts.velocity_setpt;
        ctrl->velocity_setpoint = velocity_setpt;
        ctrl->velocity_error = (velocity_setpt - ctrl->velocity) / transmission;
            // We want the motor's shaft velocity.
        vel_ctrl_current = pid_process(&ctrl->velocity_pid, ctrl->velocity_error);
        ctrl->velocity_ctrl_out = vel_ctrl_current;
    } else {
        pid_reset_integral(&ctrl->velocity_pid);
        vel_ctrl_current = 0;
    }

    // Current control
    float current_setpt = filter_limit_sym(current_setpt, ctrl->current_limit);
    ctrl->current_setpoint = current_setpt;
    ctrl->current_error = ctrl->current - current_setpt;
    ctrl->motor_voltage = pid_process(&ctrl->current_pid, ctrl->current_error);

    // Back-emf compensation
    ctrl->motor_voltage += ctrl->motor_current_constant * ctrl->velocity / transmission;
        // We want the motor's shaft velocity.

    // In the program, everything is expressed in terms of output shaft's speed.
    // However, our controller is designed to work with the motor's shaft speed,
    // therefore we must do the necessary computations to make it work.
}
