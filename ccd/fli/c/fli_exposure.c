/* fli_exposure.c
** Autoguider FLI CCD Library exposure routines
** $Header: /home/cjm/cvs/autoguider/ccd/fli/c/fli_exposure.c,v 1.2 2014-01-31 17:35:24 cjm Exp $
*/
/**
 * Exposure routines for the FLI autoguider CCD library.
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
/* fli CCD library */
#include "libfli.h"
#include "log_udp.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_temperature.h"
#include "fli_setup.h"
#include "fli_temperature.h"
#include "fli_exposure.h"

/* hash defines */
/**
 * Number of seconds to wait after an exposure is meant to have finished, before we abort with a timeout signal.
 */
#define EXPOSURE_TIMEOUT_SECS     (30.0)
/* data types */
/**
 * Structure used to hold local data to ccd_exposure.
 * <dl>
 * <dt>Exposure_Status</dt> <dd>Whether an operation is being performed to CLEAR, EXPOSE or READOUT the CCD.</dd>
 * <dt>Exposure_Length</dt> <dd>The last exposure length to be set (ms).</dd>
 * <dt>Abort</dt> <dd>Whether to abort an exposure.</dd>
 * <dt>Exposure_Start_Time</dt> <dd>The time stamp when an exposure was started.</dd>
 * <dt>Exposure_Loop_Pause_Length</dt> <dd>An amount of time to pause/sleep, in milliseconds, each time
 *     round the loop whilst waiting for an exposure to be done.
 * </dl>
 * @see ccd_exposure.html#CCD_EXPOSURE_STATUS
 */
struct Exposure_Struct
{
	enum CCD_EXPOSURE_STATUS Exposure_Status;
	int Exposure_Length;
	int Abort;
	struct timespec Exposure_Start_Time;
	int Exposure_Loop_Pause_Length;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id";
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
	1
};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise FLI exposure code. Currently does nothing.
 */
void FLI_Exposure_Initialise(void)
{
	int i;
	/* do nothing? */
}

/**
 * Perform an exposure and save it into the specified buffer.
 * <ul>
 * <li>We check the buffer is not NULL, and return an error if it is.
 * <li>We check the buffer length is not too short to hold the read out image.
 * <li>We use FLISetFrameType to configure the camera for dark or exposure.
 * <li>We set the exposure length using FLISetExposureTime.
 * <li>If a start time is configured, we enter a loop waiting for the start time.
 * <li>We start an exposure by calling FLIExposeFrame.
 * <li>We enter a loop waiting for the exposure to finish:
 *     <ul>
 *     <li>We call FLIGetDeviceStatus to get the camera status.
 *     <li>We log the returned camera status.
 *     <li>We call FLIGetExposureStatus to get the remaining exposure length.
 *     <li>If the camera status has the FLI_CAMERA_DATA_READY bit set, we decide the camera is ready for readout.
 *     <li>We check whether the Exposure_Data.Abort abort flag has been set, and if so we call FLICancelExposure
 *         to abort the exposure.
 *     <li>If the difference between the current time stamp and the Exposure_Data.Exposure_Start_Time is greater
 *         than the Exposure_Data.Exposure_Length + EXPOSURE_TIMEOUT_SECS we call FLICancelExposure
 *         to abort the exposure.
 *     <li>We sleep for Exposure_Data.Exposure_Loop_Pause_Length milliseconds.
 *     </ul>
 * <li>We retrieve the number of binned pixels using FLI_Setup_Get_NCols and FLI_Setup_Get_NRows
 * <li>We loop over the number of binned rows and call FLIGrabRow for each row of data, to put it into
 *     the relevant part of the image buffer.
 * <li>We set the exposure status to NONE and return.
 * </ul>
 * @param open_shutter A boolean, TRUE to open the shutter, FALSE to leav it closed (dark).
 * @param start_time The time to start the exposure. If both the fields in the <i>struct timespec</i> are zero,
 * 	the exposure can be started at any convenient time.
 * @param exposure_length The length of time to open the shutter for in milliseconds. This must be greater than zero.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in <b>pixels</b>.
 * @return Returns TRUE if the exposure succeeds and the data read out into the buffer, returns FALSE if an error
 *	occurs or the exposure is aborted.
 * @see #EXPOSURE_TIMEOUT_SECS
 * @see fli_setup.html#FLI_Setup_Get_Buffer_Length
 * @see fli_setup.html#FLI_Setup_Get_Dev
 * @see fli_setup.html#FLI_Setup_Get_NCols
 * @see fli_setup.html#FLI_Setup_Get_NRows
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_NS
 */
int FLI_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_length,
			void *buffer,size_t buffer_length)
{
	struct timespec sleep_time,current_time;
#ifndef _POSIX_TIMERS
	struct timeval gtod_current_time;
#endif
	unsigned short *image_data = NULL;
	long fli_retval,camera_status,remaining_exposure_length;
	int done,binned_pixels_x,binned_pixels_y,y;

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* check buffer details */
	if(buffer == NULL)
	{
		CCD_General_Error_Number = 1200;
		sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: buffer was NULL.");
		return FALSE;
	}
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"Supplied Buffer Length %ld vs. Setup buffer length %ld.",buffer_length,FLI_Setup_Get_Buffer_Length());
#endif
	if(buffer_length < FLI_Setup_Get_Buffer_Length())
	{
		CCD_General_Error_Number = 1201;
		sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: buffer_length (%ld) was too small (%d).",
			buffer_length,FLI_Setup_Get_Buffer_Length());
		return FALSE;

	}
	/* reset abort */
	Exposure_Data.Abort = FALSE;
	/* set shutter */
	if(open_shutter)
	{
#ifdef FLI_DEBUG
		CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_VERBOSE,NULL,
				"Setting frame type to normal.");
#endif
		fli_retval = FLISetFrameType(FLI_Setup_Get_Dev(),FLI_FRAME_TYPE_NORMAL);
		if(fli_retval != 0)
		{
			CCD_General_Error_Number = 1202;
			sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: FLISetFrameType failed %s(%ld).",
				strerror((int)-fli_retval),fli_retval);
			return FALSE;
		}
	}
	else
	{
#ifdef FLI_DEBUG
		CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_VERBOSE,NULL,
				"Setting frame type to dark.");
#endif
		fli_retval = FLISetFrameType(FLI_Setup_Get_Dev(),FLI_FRAME_TYPE_DARK);
		if(fli_retval != 0)
		{
			CCD_General_Error_Number = 1203;
			sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: FLISetFrameType failed %s(%ld).",
				strerror((int)-fli_retval),fli_retval);
			return FALSE;
		}
	}
	/* set exposure length */
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_VERBOSE,NULL,
			       "Setting exposure length to %d ms.",exposure_length);
#endif
	Exposure_Data.Exposure_Length = exposure_length;
	fli_retval = FLISetExposureTime(FLI_Setup_Get_Dev(),(long)(Exposure_Data.Exposure_Length));
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1204;
		sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: FLISetExposureTime(%d) failed %s(%ld).",
			Exposure_Data.Exposure_Length,strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
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
#if FLI_DEBUG
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERBOSE,NULL,
					       "Waiting for exposure start time (%ld,%ld).",
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
				CCD_General_Error_Number = 1205;
				sprintf(CCD_General_Error_String,"FLI_Exposure_Expose:Aborted.");
				return FALSE;
			}
		}/* end while */
	}/* end if wait for start_time */
	/* start the exposure */
#ifdef _POSIX_TIMERS
	clock_gettime(CLOCK_REALTIME,&(Exposure_Data.Exposure_Start_Time));
#else
	gettimeofday(&gtod_current_time,NULL);
	Exposure_Data.Exposure_Start_Time.tv_sec = gtod_current_time.tv_sec;
	Exposure_Data.Exposure_Start_Time.tv_nsec = gtod_current_time.tv_usec*CCD_GLOBAL_ONE_MICROSECOND_NS;
#endif
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_EXPOSE;
	/* start exposure */
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"Starting exposure.");
#endif
	fli_retval = FLIExposeFrame(FLI_Setup_Get_Dev());
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1206;
		sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: FLIExposeFrame failed %s(%ld).",
			strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
	/* wait for exposure to finish */
	done = FALSE;
	while(done == FALSE)
	{
		/* get current camera status */
		camera_status = 0;
		fli_retval = FLIGetDeviceStatus(FLI_Setup_Get_Dev(), &camera_status);
		if(fli_retval == 0)
		{
			if(camera_status == FLI_CAMERA_STATUS_UNKNOWN)
			{
#ifdef FLI_DEBUG
				CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
						LOG_VERBOSITY_INTERMEDIATE,NULL,"Camera status is UNKNOWN.");
#endif
			}
			else
			{
				if((camera_status&FLI_CAMERA_STATUS_MASK) == FLI_CAMERA_STATUS_IDLE)
				{
#ifdef FLI_DEBUG
					CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
							LOG_VERBOSITY_INTERMEDIATE,NULL,"Camera status is IDLE.");
#endif
				}
				if((camera_status&FLI_CAMERA_STATUS_WAITING_FOR_TRIGGER) > 0)
				{
#ifdef FLI_DEBUG
					CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
							LOG_VERBOSITY_INTERMEDIATE,NULL,
							"Camera status is WAITING_FOR_TRIGGER.");
#endif
				}
				if((camera_status&FLI_CAMERA_STATUS_EXPOSING) > 0)
				{
#ifdef FLI_DEBUG
					CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
							LOG_VERBOSITY_INTERMEDIATE,NULL,"Camera status is EXPOSING.");
#endif
				}
				if((camera_status&FLI_CAMERA_STATUS_READING_CCD) > 0)
				{
#ifdef FLI_DEBUG
					CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
							LOG_VERBOSITY_INTERMEDIATE,NULL,
							"Camera status is READING_CCD.");
#endif
				}
				if((camera_status&FLI_CAMERA_DATA_READY) > 0)
				{
#ifdef FLI_DEBUG
					CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",
							LOG_VERBOSITY_INTERMEDIATE,NULL,"Camera has data ready.");
#endif
				}
			}
		}
		fli_retval = FLIGetExposureStatus(FLI_Setup_Get_Dev(),&remaining_exposure_length);
		if(fli_retval == 0)
		{
#ifdef FLI_DEBUG
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_INTERMEDIATE,NULL,
					       "Remaining exposure length = %ld ms.\n",remaining_exposure_length);
#endif
		}
		/*if(remaining_exposure_length == 0) This doesn't work for BIAS frames as it fires before
		** the data has been digitized, the bottom of the image has pixels with value 0 in it */
		if((camera_status & FLI_CAMERA_DATA_READY) != 0)
		{
		/*if(((camera_status == FLI_CAMERA_STATUS_UNKNOWN)&&(remaining_exposure == 0))||
		**  ((camera_status != FLI_CAMERA_STATUS_UNKNOWN)&&((camera_status & FLI_CAMERA_DATA_READY) != 0))*/
			done = TRUE;
		}
		/* check - have we been aborted? */
		if(Exposure_Data.Abort)
	        {
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERBOSE,NULL,
					       "Abort detected, attempting FLI FLICancelExposure.");
			fli_retval = FLICancelExposure(FLI_Setup_Get_Dev());
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERBOSE,NULL,
					       "FLICancelExposure() return %u.",fli_retval);
			Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
			CCD_General_Error_Number = 1207;
			sprintf(CCD_General_Error_String,"FLI_Exposure_Expose:Aborted.");
			return FALSE;
		}
		/* check for timeout */
#ifdef _POSIX_TIMERS
		clock_gettime(CLOCK_REALTIME,&current_time);
#else
		gettimeofday(&gtod_current_time,NULL);
		current_time.tv_sec = gtod_current_time.tv_sec;
		current_time.tv_nsec = gtod_current_time.tv_usec*CCD_GLOBAL_ONE_MICROSECOND_NS;
#endif
		if(fdifftime(current_time,Exposure_Data.Exposure_Start_Time) >
		   ((((double)Exposure_Data.Exposure_Length)/1000.0)+EXPOSURE_TIMEOUT_SECS))
		{
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERBOSE,NULL,
					       "Timeout detected, attempting FLI FLICancelExposure.");
			fli_retval = FLICancelExposure(FLI_Setup_Get_Dev());
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERBOSE,NULL,
					       "FLICancelExposure() return %u.",fli_retval);
			Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
			CCD_General_Error_Number = 1208;
			sprintf(CCD_General_Error_String,"FLI_Exposure_Expose:Timeout.");
			CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",
					       LOG_VERBOSITY_VERY_TERSE,NULL,"Timeout.");
			return FALSE;
		}
		/* sleep for a bit */
		if(done == FALSE)
		{
			/* sleep a (very small (configurable)) bit */
			sleep_time.tv_sec = 0;
			sleep_time.tv_nsec = Exposure_Data.Exposure_Loop_Pause_Length*CCD_GENERAL_ONE_MILLISECOND_NS;
			nanosleep(&sleep_time,NULL);
		}
	}/* end while */
	/* download the image data */
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_VERBOSE,NULL,
			       "Starting Readout.");
#endif
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_READOUT;
	binned_pixels_x = FLI_Setup_Get_NCols();
	binned_pixels_y = FLI_Setup_Get_NRows();
	image_data = (unsigned short *)buffer;
	for(y = 0;y < binned_pixels_y; y++)
	{
		fli_retval = FLIGrabRow(FLI_Setup_Get_Dev(),&image_data[y*binned_pixels_x],binned_pixels_x);
		if(fli_retval != 0)
		{
			CCD_General_Error_Number = 1209;
			sprintf(CCD_General_Error_String,"FLI_Exposure_Expose: FLIGrabRow failed %s(%ld).",
				strerror((int)-fli_retval),fli_retval);
			return FALSE;
		}
	}
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Take a bias.
 * @return Returns TRUE on success, and FALSE if an error occurs or the exposure is aborted.
 * @see #
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int FLI_Exposure_Bias(void *buffer,size_t buffer_length)
{
	struct timespec start_time = {0,0};
	int retval;

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Bias",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	retval = FLI_Exposure_Expose(FALSE,start_time,0,buffer,buffer_length);
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Bias",LOG_VERBOSITY_INTERMEDIATE,NULL,"finished.");
#endif
	return retval;
}

/**
 * Abort an exposure.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see #FLI_Exposure_Expose
 * @see #Exposure_Data
 */
int FLI_Exposure_Abort(void)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	Exposure_Data.Abort = TRUE;
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_exposure.c","FLI_Exposure_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * This routine gets the time stamp for the start of the exposure.
 * @return The time stamp for the start of the exposure.
 * @see #Exposure_Data
 */
struct timespec FLI_Exposure_Get_Exposure_Start_Time(void)
{
	return Exposure_Data.Exposure_Start_Time;
}

/**
 * Set how long to pause in the loop waiting for an exposure to complete in FLI_Exposure_Expose.
 * This also determines the length of time between calls to FLIGetDeviceStatus and FLIGetExposureStatus in that loop.
 * @param ms The length of time to sleep for, in milliseconds (between 1 and 999).
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see #FLI_Exposure_Expose
 * @see #Exposure_Data
 */
int FLI_Exposure_Loop_Pause_Length_Set(int ms)
{
	if((ms < 1) || (ms > 1000))
	{
		CCD_General_Error_Number = 1210;
		sprintf(CCD_General_Error_String,"FLI_Exposure_Loop_Pause_Length_Set: Milliseconds %d out of range.",
			ms);
		return FALSE;
	}
	Exposure_Data.Exposure_Loop_Pause_Length = ms;
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2013/11/26 16:28:36  cjm
** Initial revision
**
*/

