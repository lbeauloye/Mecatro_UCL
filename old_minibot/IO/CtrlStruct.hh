#include "ctrl_io.h"
typedef struct UserStruct{
	// Proportionnal
	double kp2;

	// PI
	double ki2;

	// Back emf;
	double kphi;

	// Error
	double error_l;
	double error_r;

	// anti windup
	double alpha;
	double sat_l;
	double sat_r;
} UserStruct;

typedef struct CtrlStruct{
	UserStruct *theUserStruct;  ///< user defined CtrlStruct
	CtrlIn *theCtrlIn;   ///< controller inputs
	CtrlOut *theCtrlOut; ///< controller outputs
} CtrlStruct;

int size_UserStruct();
