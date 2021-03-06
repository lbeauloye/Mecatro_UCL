#ifndef _TEST_H_
#define _TEST_H_

#include <stdbool.h>
#include <stdint.h>

#include "socal/alt_gpio.h"
#include "socal/hps.h"
#include "socal/socal.h"

#include "alt_generalpurpose_io.h"
#include "alt_interrupt.h"
#include "hps_0.h"      /* MUST include "SAL.H" and not uAbassi.h        */
#include "Platform.h"         /* Everything about the target platform is here  */
#include "HWinfo.h"           /* Everything about the target hardware is here  */

#ifdef __cplusplus
extern "C"
{
#endif

int Test_func(int a);

#ifdef __cplusplus
}
#endif


#endif