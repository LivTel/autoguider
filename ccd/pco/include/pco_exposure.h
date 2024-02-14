/* pco_exposure.h */

#ifndef PCO_EXPOSURE_H
#define PCO_EXPOSURE_H

#include <time.h>

/*  the following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/* functions */
extern void PCO_Exposure_Initialise(void);
extern int PCO_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_time,
			       void *buffer,size_t buffer_length);
extern int PCO_Exposure_Bias(void *buffer,size_t buffer_length);
extern int PCO_Exposure_Abort(void);
extern struct timespec PCO_Exposure_Get_Exposure_Start_Time(void);
extern int PCO_Exposure_Loop_Pause_Length_Set(int ms);

	
#ifdef __cplusplus
}
#endif

#endif
