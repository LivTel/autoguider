/* pco_temperature.h */

#ifndef PCO_TEMPERATURE_H
#define PCO_TEMPERATURE_H

#include "ccd_temperature.h"

/*  the following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/* functions */
extern int PCO_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status);
extern int PCO_Temperature_Set(double target_temperature);
extern int PCO_Temperature_Cooler_On(void);
extern int PCO_Temperature_Cooler_Off(void);

	
#ifdef __cplusplus
}
#endif

#endif
