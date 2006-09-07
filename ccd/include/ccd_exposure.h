/* ccd_exposure.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_exposure.h,v 1.2 2006-09-07 15:36:12 cjm Exp $
*/
#ifndef CCD_EXPOSURE_H
#define CCD_EXPOSURE_H
#ifndef _POSIX_SOURCE
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes
 * for time.
 */
#define _POSIX_C_SOURCE 199309L
#endif

#include <time.h>
#include "ccd_general.h"

/**
 * Return value from CCD_Exposure_Get_Status. 
 * <ul>
 * <li>CCD_EXPOSURE_STATUS_NONE means the library is not currently performing an exposure.
 * <li>CCD_EXPOSURE_STATUS_WAIT_START means the library is waiting for the correct moment to open the shutter.
 * <li>CCD_EXPOSURE_STATUS_CLEAR means the library is currently clearing the ccd.
 * <li>CCD_EXPOSURE_STATUS_EXPOSE means the library is currently performing an exposure.
 * <li>CCD_EXPOSURE_STATUS_PRE_READOUT means the library is currently exposing, but is about
 * 	to start reading out data from the ccd (so don't start any commands that won't work in readout). 
 * <li>CCD_EXPOSURE_STATUS_READOUT means the library is currently reading out data from the ccd.
 * <li>CCD_EXPOSURE_STATUS_POST_READOUT means the library has finished reading out, but is post processing
 * 	the data (byte swap/de-interlacing/saving to disk).
 * </ul>
 * @see #CCD_Exposure_Get_Status
 */
enum CCD_EXPOSURE_STATUS
{
	CCD_EXPOSURE_STATUS_NONE,CCD_EXPOSURE_STATUS_WAIT_START,CCD_EXPOSURE_STATUS_CLEAR,
	CCD_EXPOSURE_STATUS_EXPOSE,CCD_EXPOSURE_STATUS_PRE_READOUT,CCD_EXPOSURE_STATUS_READOUT,
	CCD_EXPOSURE_STATUS_POST_READOUT
};

/**
 * Macro to check whether the exposure status is a legal value.
 * @see #CCD_EXPOSURE_STATUS
 */
#define CCD_EXPOSURE_IS_STATUS(status)	(((status) == CCD_EXPOSURE_STATUS_NONE)|| \
        ((status) == CCD_EXPOSURE_STATUS_WAIT_START)|| \
	((status) == CCD_EXPOSURE_STATUS_CLEAR)||((status) == CCD_EXPOSURE_STATUS_EXPOSE)|| \
        ((status) == CCD_EXPOSURE_STATUS_READOUT)||((status) == CCD_EXPOSURE_STATUS_POST_READOUT))


extern void CCD_Exposure_Initialise(void);
extern int CCD_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_time,
			       void *buffer,size_t buffer_length);
extern int CCD_Exposure_Bias(void *buffer,size_t buffer_length);
extern int CCD_Exposure_Abort(void);
extern int CCD_Exposure_Get_Exposure_Start_Time(struct timespec *timespec);
extern int CCD_Exposure_Loop_Pause_Length_Set(int ms);
extern int CCD_Exposure_Save(char *filename,void *buffer,size_t buffer_length,int ncols,int nrows);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/04/28 14:26:43  cjm
** Initial revision
**
*/
#endif
