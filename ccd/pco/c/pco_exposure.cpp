/* pco_exposure.cpp
** Autoguider PCO CMOS library exposure routines
*/
/**
 * Exposure routines for the PCO camera driver.
 * @author Chris Mottram
 * @version $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "pco_command.h"
#include "pco_exposure.h"

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
 * Initialise PCO exposure code. Currently does nothing.
 */
void PCO_Exposure_Initialise(void)
{
	int i;
	/* do nothing? */
}

/**
 * Perform an exposure and save it into the specified buffer.
 * <ul>
 * <li>We check the buffer is not NULL, and return an error if it is.
 * <li>We check the buffer length is not too short to hold the read out image.
 * <li>diddly
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
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_NS
 */
int PCO_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_length,
			void *buffer,size_t buffer_length)
{
	struct timespec sleep_time,current_time;
#ifndef _POSIX_TIMERS
	struct timeval gtod_current_time;
#endif
	
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","fpco_exposure.c","PCO_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* check buffer details */
	if(buffer == NULL)
	{
		CCD_General_Error_Number = 1400;
		sprintf(CCD_General_Error_String,"PCO_Exposure_Expose: buffer was NULL.");
		return FALSE;
	}
	/* reset abort */
	Exposure_Data.Abort = FALSE;
	/* diddly */
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Take a bias.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in <b>pixels</b>.
 * @return Returns TRUE on success, and FALSE if an error occurs or the exposure is aborted.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int PCO_Exposure_Bias(void *buffer,size_t buffer_length)
{
	struct timespec start_time = {0,0};
	int retval;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Bias",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	retval = PCO_Exposure_Expose(FALSE,start_time,0,buffer,buffer_length);
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Bias",LOG_VERBOSITY_INTERMEDIATE,NULL,"finished.");
#endif
	return retval;
}

/**
 * Abort an exposure.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see #PCO_Exposure_Expose
 * @see #Exposure_Data
 */
int PCO_Exposure_Abort(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	Exposure_Data.Abort = TRUE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"finished.");
#endif
	return TRUE;
}


/**
 * This routine gets the time stamp for the start of the exposure.
 * @return The time stamp for the start of the exposure.
 * @see #Exposure_Data
 */
struct timespec PCO_Exposure_Get_Exposure_Start_Time(void)
{
	return Exposure_Data.Exposure_Start_Time;
}

/**
 * Set how long to pause in the loop waiting for an exposure to complete in PCO_Exposure_Expose.
 * @param ms The length of time to sleep for, in milliseconds (between 1 and 999).
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see #PCO_Exposure_Expose
 * @see #Exposure_Data
 */
int PCO_Exposure_Loop_Pause_Length_Set(int ms)
{
	if((ms < 1) || (ms > 1000))
	{
		CCD_General_Error_Number = 1401;
		sprintf(CCD_General_Error_String,"PCO_Exposure_Loop_Pause_Length_Set: Milliseconds %d out of range.",
			ms);
		return FALSE;
	}
	Exposure_Data.Exposure_Loop_Pause_Length = ms;
	return TRUE;
}

