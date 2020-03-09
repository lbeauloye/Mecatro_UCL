#ifndef _PI_HPP_
#define _PI_HPP_


class PIController {
    // PI Parameters
    double kp;
    double ki;
    double alpha;
    double integral;
    double dt;

    // Motor Parameters
    double gearbox;
    double k_phi;
    double i_limit;
    double v_limit;
    double saturation;

public:
    PIController();
    double pi_process(double w_ref, double w_mes);
};

double limit(double val, double limit);

#endif
