/* fli_exposure.h
** $Header: /home/cjm/cvs/autoguider/ccd/fli/include/fli_exposure.h,v 1.1 2013-11-26 16:28:41 cjm Exp $
*/
#ifndef FLI_EXPOSURE_H
#define FLI_EXPOSURE_H
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "fli_general.h"

extern void FLI_Exposure_Initialise(void);
extern int FLI_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_time,
			       void *buffer,size_t buffer_length);
extern int FLI_Exposure_Bias(void *buffer,size_t buffer_length);
extern int FLI_Exposure_Abort(void);
extern struct timespec FLI_Exposure_Get_Exposure_Start_Time(void);
extern int FLI_Exposure_Loop_Pause_Length_Set(int ms);

/*
** $Log: not supported by cvs2svn $
*/
#endif
