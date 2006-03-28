/* andor_exposure.c
** Autoguider Andor CCD Library exposure routines
** $Header: /home/cjm/cvs/autoguider/ccd/andor/c/andor_exposure.c,v 1.2 2006-03-28 15:12:55 cjm Exp $
*/
/**
 * Exposure routines for the Andor autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
/* andor CCD library */
#include "atmcdLXd.h"

#include "ccd_exposure.h"
#include "ccd_general.h"
#include "andor_setup.h"
#include "andor_exposure.h"

/* data types */
/**
 * Structure used to hold local data to ccd_exposure.
 * <dl>
 * <dt>Exposure_Status</dt> <dd>Whether an operation is being performed to CLEAR, EXPOSE or READOUT the CCD.</dd>
 * <dt>Exposure_Length</dt> <dd>The last exposure length to be set (ms).</dd>
 * <dt>Abort</dt> <dd>Whether to abort an exposure.</dd>
 * <dt>Exposure_Start_Time</dt> <dd>The time stamp when an exposure was started.</dd>
 * </dl>
 * @see ccd_exposure.html#CCD_EXPOSURE_STATUS
 */
struct Exposure_Struct
{
	enum CCD_EXPOSURE_STATUS Exposure_Status;
	int Exposure_Length;
	int Abort;
	struct timespec Exposure_Start_Time;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: andor_exposure.c,v 1.2 2006-03-28 15:12:55 cjm Exp $";
/**
 * Data holding the current status of ccd_exposure.
 * @see #Exposure_Struct
 * @see #CCD_EXPOSURE_STATUS
 */
static struct Exposure_Struct Exposure_Data = 
{
	CCD_EXPOSURE_STATUS_NONE,
	0,FALSE,
	{0L,0L},
};

/* internal function declarations */
static int fexist(char *filename);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise Andor exposure code. Currently does nothing.
 */
void Andor_Exposure_Initialise(void)
{
	int i;
	/* do nothing? */
}

/**
 * Perform an exposure and save it into the specified buffer.
 * @param open_shutter A boolean, TRUE to open the shutter, FALSE to leav it closed (dark).
 * @param start_time The time to start the exposure. If both the fields in the <i>struct timespec</i> are zero,
 * 	the exposure can be started at any convenient time.
 * @param exposure_length The length of time to open the shutter for in milliseconds. This must be greater than zero.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in bytes.
 * @return Returns TRUE if the exposure succeeds and the data read out into the buffer, returns FALSE if an error
 *	occurs or the exposure is aborted.
 * @see andor_general.html#Andor_General_ErrorCode_To_String
 * @see andor_general.html#ANDOR_GENERAL_LOG_BIT_EXPOSURE
 * @see andor_setup.html#Andor_Setup_Get_Buffer_Length
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_NS
 */
int Andor_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_length,
			       void *buffer,size_t buffer_length)
{
	struct timespec sleep_time,current_time;
#ifndef _POSIX_TIMERS
	struct timeval gtod_current_time;
#endif
	unsigned int andor_retval;
	int exposure_status,acquisition_counter,done;

#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose started.");
#endif
	/* set shutter */
	if(open_shutter)
	{
		andor_retval = SetShutter(1,0,0,0);
		if(andor_retval != DRV_SUCCESS)
		{
			CCD_General_Error_Number = 1100;
			sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: SetShutter() failed %s(%u).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
			return FALSE;
		}
	}
	else
	{
		andor_retval = SetShutter(1,2,0,0);/* 2 means close */
		if(andor_retval != DRV_SUCCESS)
		{
			CCD_General_Error_Number = 1102;
			sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: SetShutter() failed %s(%u).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
			return FALSE;
		}
	}
	/* set exposure length */
	Exposure_Data.Exposure_Length = exposure_length;
	andor_retval = SetExposureTime(((float)exposure_length)/1000.0f);/* in seconds */
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1103;
		sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: SetExposureTime(%f) failed %s(%u).",
			(((float)exposure_length)/1000.0f),Andor_General_ErrorCode_To_String(andor_retval),
			andor_retval);
		return FALSE;
	}
	/* check buffer details */
	if(buffer == NULL)
	{
		CCD_General_Error_Number = 1104;
		sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: buffer was NULL.");
		return FALSE;
	}
	if(buffer_length < Andor_Setup_Get_Buffer_Length())
	{
		CCD_General_Error_Number = 1105;
		sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: buffer_length (%ld) was too small (%ld).",
			buffer_length,Andor_Setup_Get_Buffer_Length());
		return FALSE;

	}
	/* reset abort */
	Exposure_Data.Abort = FALSE;
	/* wait for start_time, if applicable */
	if(start_time.tv_sec > 0)
	{
		Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_WAIT_START;
		done = FALSE;
		while(done == FALSE)
		{
#ifdef _POSIX_TIMERS
			clock_gettime(CLOCK_REALTIME,&current_time);
#else
			gettimeofday(&gtod_current_time,NULL);
			current_time.tv_sec = gtod_current_time.tv_sec;
			current_time.tv_nsec = gtod_current_time.tv_usec*CCD_GLOBAL_ONE_MICROSECOND_NS;
#endif
#if ANDOR_DEBUG
			CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,
				       "Andor_Exposure_Expose():Waiting for exposure start time (%ld,%ld).",
				       current_time.tv_sec,start_time.tv_sec);
#endif
		/* if we've time, sleep for a second */
			if((start_time.tv_sec - current_time.tv_sec) > 0)
			{
				sleep_time.tv_sec = 1;
				sleep_time.tv_nsec = 0;
				nanosleep(&sleep_time,NULL);
			}
			else
			{
				/* sleep for remaining sub-second time (if it exists!) */
				sleep_time.tv_sec = start_time.tv_sec - current_time.tv_sec;
				sleep_time.tv_nsec = start_time.tv_nsec - current_time.tv_nsec;
				if((sleep_time.tv_sec == 0)&&(sleep_time.tv_nsec > 0))
					nanosleep(&sleep_time,NULL);
				/* exit the wait to start loop */
				done = TRUE;
			}
		/* check - have we been aborted? */
			if(Exposure_Data.Abort)
			{
				Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
				CCD_General_Error_Number = 1101;
				sprintf(CCD_General_Error_String,"Andor_Exposure_Expose:Aborted.");
				return FALSE;
			}
		}/* end while */
	}/* end if wait for start_time */
	/* start the exposure */
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_EXPOSE;
	andor_retval = StartAcquisition();
	if(andor_retval != DRV_SUCCESS)
	{
		Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
		CCD_General_Error_Number = 1106;
		sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: StartAcquisition() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* wait until acquisition complete */
	acquisition_counter = 0;
	do
	{
		/* sleep a (very small) bit */
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = CCD_GENERAL_ONE_MILLISECOND_NS;
		nanosleep(&sleep_time,NULL);
		/* get the status */
		andor_retval = GetStatus(&exposure_status);
		if(andor_retval != DRV_SUCCESS)
		{
			Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
			CCD_General_Error_Number = 1107;
			sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: GetStatus() failed %s(%u).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
			return FALSE;
		}
#if ANDOR_DEBUG
		if((acquisition_counter%100)==0)
		{
			CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose():"
					       "Current Acquisition Status after %d loops is %s(%u).",
					       acquisition_counter,Andor_General_ErrorCode_To_String(exposure_status),
					       exposure_status);
		}
#endif
		acquisition_counter++;
		/* check - have we been aborted? */
		if(Exposure_Data.Abort)
	        {
			CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose():"
					       "Abort detected, attempting Andor AbortAcquisition.");
			andor_retval = AbortAcquisition();
			CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose():"
					       "AbortAcquisition() return %u.",andor_retval);
			Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
			CCD_General_Error_Number = 1108;
			sprintf(CCD_General_Error_String,"Andor_Exposure_Expose:Aborted.");
			return FALSE;
		}
	}
	while(exposure_status==DRV_ACQUIRING);
#if ANDOR_DEBUG
	CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose():"
			       "Acquisition Status after %d loops is %s(%u).",acquisition_counter,
			       Andor_General_ErrorCode_To_String(exposure_status),exposure_status);
#endif
	/* diddly check exposure_status is correct (DRV_IDLE?) */
	/* get data */
#if ANDOR_DEBUG
	CCD_General_Log_Format(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose():"
			       "Calling GetAcquiredData16(%p,%ld).",buffer,buffer_length);
#endif
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_READOUT;
	andor_retval = GetAcquiredData16((unsigned short*)buffer,buffer_length);
	/*andor_retval = GetAcquiredData(imageData, width*height);*/
	if(andor_retval != DRV_SUCCESS)
	{
		Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
		CCD_General_Error_Number = 1109;
		sprintf(CCD_General_Error_String,"Andor_Exposure_Expose: GetAcquiredData16() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Expose finished.");
#endif
	return TRUE;
}

/**
 * Take a bias.
 * @return Returns TRUE on success, and FALSE if an error occurs or the exposure is aborted.
 * @see andor_general.html#Andor_General_ErrorCode_To_String
 * @see andor_general.html#ANDOR_GENERAL_LOG_BIT_EXPOSURE
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int Andor_Exposure_Bias(void *buffer,size_t buffer_length)
{
	struct timespec start_time = {0,0};
	int retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Bias started.");
#endif
	retval = Andor_Exposure_Expose(FALSE,start_time,0,buffer,buffer_length);
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Bias finished.");
#endif
	return retval;
}

/**
 * Abort an exposure
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see andor_general.html#ANDOR_GENERAL_LOG_BIT_EXPOSURE
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int Andor_Exposure_Abort(void)
{
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Abort started.");
#endif
	Exposure_Data.Abort = TRUE;
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_EXPOSURE,"Andor_Exposure_Abort finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/03/27 14:02:36  cjm
** Initial revision
**
*/
