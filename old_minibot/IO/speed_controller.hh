#ifndef _SPEED_CONTROLLER_HH_
#define _SPEED_CONTROLLER_HH_

void run_speed_controller(CtrlStruct* theCtrlStruct, double* omega_ref);
void init_speed_controller(CtrlStruct* theCtrlStruct);

#endif