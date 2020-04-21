#include "motor/middle_level.h"
#include <stdio.h>



/*
 *  Transformation between the intertial frame and the local frame.
 *  It is a simple rotation matrix of argument (theta).
 */
void compute_local_velocities(double *xsi_inertial, double theta, double *xsi) {
    double X_dot = xsi_inertial[0];
    double Y_dot = xsi_inertial[1];
//    printf("X_dot : %f\t",X_dot);
//    printf("Y_dot : %f\t\n",Y_dot);

    xsi[0] = X_dot*cos(theta) - Y_dot*sin(theta);
    xsi[1] = X_dot*sin(theta) + Y_dot*cos(theta);
    xsi[2] = xsi_inertial[2];   // theta_dot is unchanged.
//    printf("xsi0 : %f\t",xsi[0]);
//    printf("xsi1 : %f\t\n",xsi[1]);
}

/*
 *  Computes the w_ref to apply to the motors, based on the local frame's
 *  velocities.
 *  Robot parameters are specified in the .h file, to be modified accordingly.
 */
void compute_motor_velocities(double *xsi, double *w) {
    w[1] = -(-xsi[0]*33.333333334 + xsi[1]*33.333333334 + xsi[2]*13.4667); //- du à la roue en sens inverse
    w[0] =  xsi[0]*33.333333334 + xsi[1]*33.333333334 - xsi[2]*13.4667;
    w[2] = -xsi[0]*33.333333334 + xsi[1]*33.333333334 - xsi[2]*13.4667;
    w[3] =  xsi[0]*33.333333334 + xsi[1]*33.333333334 + xsi[2]*13.4667;
}
