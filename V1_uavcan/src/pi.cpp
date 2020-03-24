#include "pi.hpp"


PIController::PIController() {
    this->kp = 3.248e-2;
    this->ki = 7.93e-2;
    this->alpha = 0.3;
    this->dt = 0.01;    // The motor board broadcast the speed 100x a second
    this->integral = 0;

    this->gearbox = 14;
    this->k_phi = 26.1e-3;
    this->i_limit = 7.1*0.78;
    this->v_limit = 24*0.95;
    this->saturation = 0;
}

double PIController::pi_process(double w_ref, double w_mes) {
    // Compute the error
    double error = (w_ref - w_mes) * this->gearbox;

    // Proportional action
    double output = error * this->kp;

    // Integral action + saturation
    this->integral += error*this->ki*this->dt + this->saturation*this->alpha;
    output += this->integral;
    this->saturation = -output;

    // Current limiter
    output = limit(output, this->i_limit);

    // Update saturation
    this->saturation += output;

    // Voltage limiter
    output += this->k_phi * w_mes * this->gearbox;
    output = limit(output, v_limit);
    return output;
}

double limit(double val, double limit) {
    if (val > limit) {
        return limit;
    } else if (val < -limit) {
        return -limit;
    }
    return val;
}
