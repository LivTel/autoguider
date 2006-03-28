/* andor_exposure.h
** $Header: /home/cjm/cvs/autoguider/ccd/andor/include/andor_exposure.h,v 1.2 2006-03-28 15:12:56 cjm Exp $
*/
#ifndef ANDOR_EXPOSURE_H
#define ANDOR_EXPOSURE_H
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
#include "andor_general.h"

extern void Andor_Exposure_Initialise(void);
extern int Andor_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_time,
			       void *buffer,size_t buffer_length);
extern int Andor_Exposure_Bias(void *buffer,size_t buffer_length);
extern int Andor_Exposure_Abort(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/03/27 14:03:01  cjm
** Initial revision
**
*/
#endif
