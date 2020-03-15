#ifndef _MIDDLE_LEVEL_HPP_
#define _MIDDLE_LEVEL_HPP_

#include <math.h>


/*
    Middle level controller for an omnidirectionnal robot.
    Wheels are disposed as follows:
            [1]-----[0]             ^ Y_r
             |       |              |
             |       |  LENGTH      |
             |       |              |
            [2]-----[3]             . ------> X_r
               WIDTH
    The coefficients were computed using a MATLAB script, using a certain paper's
    framework (not using Ronsse's, as some weird stuff was happening).
    To make it fit to your own robot, you should normally only modify the L and
    W variables, definied as shown on the schematics drawn here.
    The script is copied here:
    L = 8.134e-2;
    W = 19.925e-2;

    a = L/2;
    b = W/2;
    l = sqrt(a*a + b*b);
    r = 3e-2;

    % Alpha angles
    a0 = pi/2 - atan(a/b);
    a1 = -a0;
    a3 = pi/2 + atan(a/b);
    a2 = -a3;

    % Beta angles
    b0 = -pi/2;
    b1 = -pi/2;
    b2 = -pi/2;
    b3 = -pi/2;

    % Gamma angles
    g0 = -pi/4;
    g1 = pi/4;
    g2 = -pi/4;
    g3 = pi/4;

    % Compute the motor velocitites...
    M = [[sin(b0 - g0), cos(b0 - g0), l*sin(b0 - g0 - a0)] / sin(g0);
         [sin(b1 - g1), cos(b1 - g1), l*sin(b1 - g1 - a1)] / sin(g1);
         [sin(b2 - g2), cos(b2 - g2), l*sin(b2 - g2 - a2)] / sin(g2);
         [sin(b3 - g3), cos(b3 - g3), l*sin(b3 - g3 - a3)] / sin(g3)] ./ -r;

    % Display results
    clc;
    disp("Forward speed")   % Forward = y_dot != 0
    disp(M * [.0, .1, 0]')
    disp("Lateral speed")   % Lateral = x_dot != 0
    disp(M * [.1, .0, 0]')
 */
void compute_local_velocities(double *xsi_interial, double theta, double *xsi);
void compute_motor_velocities(double *xsi, double *w);

#endif
