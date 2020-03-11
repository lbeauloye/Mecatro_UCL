#include "middle_level.hpp"


/*
 *  Transformation between the intertial frame and the local frame.
 *  It is a simple rotation matrix of argument (theta).
 */
void compute_local_velocities(double *xsi_inertial, double theta, double *xsi) {
    double X_dot = xsi_inertial[0];
    double Y_dot = xsi_inertial[1];

    xsi[0] = X_dot*cos(theta) - Y_dot*sin(theta);
    xsi[1] = X_dot*sin(theta) + Y_dot*sin(theta);
    xsi[2] = xsi_inertial[2];   // theta_dot is unchanged.
}

/*
 *  Computes the w_ref to apply to the motors, based on the local frame's
 *  velocities.
 *  Robot parameters are specified in the .h file, to be modified accordingly.
 */
void compute_motor_velocities(double *xsi, double *w) {
    double x_dot = xsi[0];
    double y_dot = xsi[1];
    double theta_dot = xsi[2];

    double a = LENGTH/2.0;
    double b = WIDTH/2.0;
    double l = sqrt(a*a + b*b);

    // Alpha angles
    double a0 = atan(a/b);
    double a1 = M_PI - a0;
    double a2 = M_PI + a0;
    double a3 = - a0;

    // Beta angles
    double b0 = a0;
    double b1 = a1;
    double b2 = a2;
    double b3 = a3;

    // Motor velocities
    w[0] = (x_dot*sin(a0 + b0 + G) - y_dot*cos(a0 + b0 + G) \
                - l*theta_dot*cos(b0 + G)) / (R*cos(G));

    w[1] = - (x_dot*sin(a1 + b1 - G) - y_dot*cos(a1 + b1 - G) \
                - l*theta_dot*cos(b1 - G)) / (R*cos(G));

    w[2] = (x_dot*sin(a2 + b2 + G) - y_dot*cos(a2 + b2 + G) \
                - l*theta_dot*cos(b2 + G)) / (R*cos(G));

    w[3] = - (x_dot*sin(a3 + b3 - G) - y_dot*cos(a3 + b3 - G) \
                - l*theta_dot*cos(b3 - G)) / (R*cos(G));
}
