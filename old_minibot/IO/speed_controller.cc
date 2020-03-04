
#include <stdio.h>
#include <stdlib.h>
#include "CtrlStruct.hh"
#include "speed_controller.hh"

double limit(double val, double limit){
	if(val > limit){
		return limit;
	}
	if(val < - limit){
		return - limit;
	}
	return val;
}

void run_speed_controller(CtrlStruct* theCtrlStruct, double* omega_ref){
	// We fetch the ref speed
	double omega_ref_l = omega_ref[L_ID]*14;
	double omega_ref_r = omega_ref[R_ID]*14;

	// Time to integrate
	const double deltaT = 0.04/60;

	// V limit
	const double limitV = 0.95*24;

	// I limit
	const double limitI = 7.1 * 0.78;

	// Get the speed
	double omega_l = theCtrlStruct->theCtrlIn->l_wheel_speed*14;
	double omega_r = theCtrlStruct->theCtrlIn->r_wheel_speed*14;

	printf("time %f, omega_ref_l: %f, omega_ref_r: %f\n", theCtrlStruct->theCtrlIn->t,omega_ref_l, omega_ref_r);
	printf("time %f, omega_l: %f, omega_r: %f\n", theCtrlStruct->theCtrlIn->t, omega_l, omega_r);

	// getting the error
	double error_w_l = omega_ref_l - omega_l;
	double error_w_r = omega_ref_r - omega_r ;

	// getting Vref

	// Propor. of the error
	double v_l = error_w_l*theCtrlStruct->theUserStruct->kp2;
	double v_r = error_w_r*theCtrlStruct->theUserStruct->kp2;

	// Integrate the error and removing alpha * saturation
	theCtrlStruct->theUserStruct->error_l += error_w_l*theCtrlStruct->theUserStruct->ki2 * deltaT + theCtrlStruct->theUserStruct->sat_l * theCtrlStruct->theUserStruct->alpha;
	theCtrlStruct->theUserStruct->error_r += error_w_r*theCtrlStruct->theUserStruct->ki2 * deltaT + theCtrlStruct->theUserStruct->sat_r * theCtrlStruct->theUserStruct->alpha;

	// adding the integral
	v_l += theCtrlStruct->theUserStruct->error_l;
	v_r += theCtrlStruct->theUserStruct->error_r;

	// To compute the saturation we remove the v before the limit
	theCtrlStruct->theUserStruct->sat_l = - v_l;
	theCtrlStruct->theUserStruct->sat_r = - v_r;

	// We limit in current and we had the back_emf
	v_l = limit(v_l, limitI) + theCtrlStruct->theUserStruct->kphi * omega_l;
	v_r = limit(v_r, limitI) + theCtrlStruct->theUserStruct->kphi * omega_r;

	// We had the to the saturation the v after the limit
	theCtrlStruct->theUserStruct->sat_l += v_l;
	theCtrlStruct->theUserStruct->sat_r += v_r;
	
	// Adding back emf and limit
	v_l = limit(v_l, limitV);
	v_r = limit(v_r, limitV);

	
	printf("time %f, sat_l: %f, sat_r: %f\n", theCtrlStruct->theCtrlIn->t,theCtrlStruct->theUserStruct->sat_l, theCtrlStruct->theUserStruct->sat_r);

	printf("time %f, v_l: %f, v_r: %f\n", theCtrlStruct->theCtrlIn->t, v_l, v_r);
	
	theCtrlStruct->theCtrlOut->wheel_commands[L_ID] = -100 * v_l / (limitV);
	theCtrlStruct->theCtrlOut->wheel_commands[R_ID] = -100 * v_r / (limitV);
	return;
}

void init_speed_controller(CtrlStruct* theCtrlStruct){

	// PI
	theCtrlStruct->theUserStruct->kp2 = 0.02;
	theCtrlStruct->theUserStruct->ki2 = 15.0;

	// kphi
	theCtrlStruct->theUserStruct->kphi = 0.00027;

	// speed
	theCtrlStruct->theCtrlIn->l_wheel_speed = 0;
	theCtrlStruct->theCtrlIn->r_wheel_speed = 0;

	// error
	theCtrlStruct->theUserStruct->error_l = 0;
	theCtrlStruct->theUserStruct->error_r = 0;

	// anti-windup
	theCtrlStruct->theUserStruct->sat_l = 0;
	theCtrlStruct->theUserStruct->sat_r = 0;
	theCtrlStruct->theUserStruct->alpha = 0.3;

	return;
}
