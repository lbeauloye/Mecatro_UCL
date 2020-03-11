#ifndef _MIDDLE_LEVEL_HPP_
#define _MIDDLE_LEVEL_HPP_

#include <math.h>


/*
 *      FRONT
 *
 *    width
 * <----------->
 * -------------  ^
 * |1|       |0|  |   l
 *   |       |    |   e
 *   |       |    |   n
 *   |       |    |   g
 * |2|       |3|  |   t
 * -------------  |   h
 *
 *     BACK
 */
#define WIDTH   0.19925
#define LENGTH  0.08134
#define G       (M_PI/4)    // Gamma angle
#define R       0.03        // Wheel radius

void compute_local_velocities(double *xsi_interial, double theta, double *xsi);
void compute_motor_velocities(double *xsi, double *w);

#endif
