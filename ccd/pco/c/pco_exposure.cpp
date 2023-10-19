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
#include "pco_setup.h"

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
 * @see ../../cdocs/ccd_exposure.html#CCD_EXPOSURE_STATUS
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
 * <li>We reset the Exposure_Data Abort flag.
 * <li>We call PCO_Setup_Get_Timestamp_Mode to retrieve the timestamp mode the camera was configured with in
 *     PCO_Setup_Startup.
 * <li>We call PCO_Command_Set_Timebase to set the camera timebase to microseconds.
 * <li>We convert the exposure length to microseconds using CCD_GENERAL_ONE_MILLISECOND_US.
 * <li>We set the exposure length by calling PCO_Command_Set_Delay_Exposure_Time.
 * <li>We set the shutter trigger mode to internal by calling PCO_Command_Set_Trigger_Mode.
 * <li>We ready the camera with the current configuration by calling PCO_Command_Arm_Camera.
 * <li>We update the grabber code with the current configuration by calling PCO_Command_Grabber_Post_Arm.
 * <li>If the timestamp mode is _not_ PCO_COMMAND_TIMESTAMP_MODE_BINARY, we generate an approximate
 *     exposure start time timestamp.
 * <li>We tell the camera to start recording data by calling PCO_Command_Set_Recording_State(TRUE).
 * <li>We update the Exposure Data Exposure Status to EXPOSE.
 * <li>We check whether the exposure has been aborted.
 * <li>We call PCO_Command_Grabber_Acquire_Image_Async_Wait to save an acquired image into the buffer.
 * <li>We check whether the exposure has been aborted.
 * <li>We set the Exposure Data Exposure Status to POST_READOUT.
 * <li>We get the camera image number from the image by calling PCO_Command_Get_Image_Number_From_Metadata.
 * <li>If the timestamp mode _is_ PCO_COMMAND_TIMESTAMP_MODE_BINARY, 
 *     we get the camera timestamp from the image by calling PCO_Command_Get_Timestamp_From_Metadata.
 * <li>We set the Exposure Data Start Timestamp to the retrieved camera timestamp.
 * <li>We tell the camera to stop recording data by calling PCO_Command_Set_Recording_State(FALSE).
 * <li>We set the Exposure Data Exposure Status to NONE and return.
 * </ul>
 * @param open_shutter A boolean, TRUE to open the shutter, FALSE to leave it closed (dark).
 * @param start_time The time to start the exposure. If both the fields in the <i>struct timespec</i> are zero,
 * 	the exposure can be started at any convenient time.
 * @param exposure_length The length of time to open the shutter for in milliseconds. This must be greater than zero.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in <b>pixels</b>.
 * @return Returns TRUE if the exposure succeeds and the data read out into the buffer, returns FALSE if an error
 *	occurs or the exposure is aborted.
 * @see #Exposure_Data
 * @see ../../cdocs/ccd_exposure.html#CCD_EXPOSURE_STATUS
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_US
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_NS
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 * @see pco_command.html#PCO_COMMAND_TIMEBASE
 * @see pco_command.html#PCO_COMMAND_TRIGGER_MODE
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE
 * @see pco_command.html#PCO_Command_Set_Timebase
 * @see pco_command.html#PCO_Command_Set_Delay_Exposure_Time
 * @see pco_command.html#PCO_Command_Set_Trigger_Mode
 * @see pco_command.html#PCO_Command_Arm_Camera
 * @see pco_command.html#PCO_Command_Grabber_Post_Arm
 * @see pco_command.html#PCO_Command_Set_Recording_State
 * @see pco_command.html#PCO_Command_Grabber_Acquire_Image_Async_Wait
 * @see pco_command.html#PCO_Command_Get_Image_Number_From_Metadata
 * @see pco_command.html#PCO_Command_Get_Timestamp_From_Metadata
 * @see pco_setup.html#PCO_Setup_Get_Timestamp_Mode
 */
int PCO_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_length,
			void *buffer,size_t buffer_length)
{
	enum PCO_COMMAND_TIMESTAMP_MODE timestamp_mode;
	struct timespec sleep_time,current_time,camera_timestamp;
	int exposure_length_us,camera_image_number;

#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_exposure.c","PCO_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,
			       "Started with open_shutter = %s, start time {%d,%d}, exposure length %d ms.",
			       open_shutter ? "TRUE" : "FALSE",start_time.tv_sec,start_time.tv_nsec,exposure_length);
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
	/* get the timestamp mode */
	timestamp_mode = PCO_Setup_Get_Timestamp_Mode();
	/* set exposure and delay timebase to microseconds */
	if(!PCO_Command_Set_Timebase(PCO_COMMAND_TIMEBASE_US,PCO_COMMAND_TIMEBASE_US))
		return FALSE;
	/* convert exposure length to microseconds. */
	exposure_length_us = (int)(exposure_length*((double)CCD_GENERAL_ONE_MILLISECOND_US));
	/* set exposure length in microseconds */
	if(!PCO_Command_Set_Delay_Exposure_Time(0,exposure_length_us))
		return FALSE;
	/* set the trigger mode to internal */
	if(!PCO_Command_Set_Trigger_Mode(PCO_COMMAND_TRIGGER_MODE_INTERNAL))
		return FALSE;
	/* get the camera ready with the new settings */
	if(!PCO_Command_Arm_Camera())
		return FALSE;
	/* update the grabber so thats ready */
	if(!PCO_Command_Grabber_Post_Arm())
		return FALSE;
	/* If we are not going to get an exposure start timestamp from the readout camera data,
	** get an approximate exposure start time here */
	if(timestamp_mode != PCO_COMMAND_TIMESTAMP_MODE_BINARY)
	{
		clock_gettime(CLOCK_REALTIME,&camera_timestamp);
	}
	/* start taking data */
	if(!PCO_Command_Set_Recording_State(TRUE))
		return FALSE;
	/* set exposure data to expose */
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_EXPOSE;
	/* check abort */
	if(Exposure_Data.Abort)
	{
		Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
		CCD_General_Error_Number = 1402;
		sprintf(CCD_General_Error_String,"PCO_Exposure_Expose: Aborted.");
		return FALSE;
	}
	/* get an acquired image buffer */
	if(!PCO_Command_Grabber_Acquire_Image_Async_Wait(buffer))
		return FALSE;
	/* check abort */
	if(Exposure_Data.Abort)
	{
		Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
		CCD_General_Error_Number = 1403;
		sprintf(CCD_General_Error_String,"PCO_Exposure_Expose: Aborted.");
		return FALSE;
	}
	/* set exposure data to post readout */
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_POST_READOUT;
	/* get camera image number */
	if(!PCO_Command_Get_Image_Number_From_Metadata(buffer,buffer_length,&camera_image_number))
		return FALSE;
	/* get camera timestamp from the readout data, if the camera is setup to produce this timestamp */
	if(timestamp_mode == PCO_COMMAND_TIMESTAMP_MODE_BINARY)
	{
		if(!PCO_Command_Get_Timestamp_From_Metadata(buffer,buffer_length,&camera_timestamp))
			return FALSE;
	}
	Exposure_Data.Exposure_Start_Time = camera_timestamp;
	/* stop recording data */
	if(!PCO_Command_Set_Recording_State(FALSE))
		return FALSE;
	/* reset the exposure status */
	Exposure_Data.Exposure_Status = CCD_EXPOSURE_STATUS_NONE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Expose",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Take a bias. We call PCO_Exposure_Expose with a start time of 0, 0 length exposure with shutter closed.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in <b>pixels</b>.
 * @return Returns TRUE on success, and FALSE if an error occurs or the exposure is aborted.
 * @see #PCO_Exposure_Expose
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
 * Abort an exposure. We set the Exposure Data Abort flag to TRUE, and call PCO_Command_Set_Recording_State to stop
 * recording frames.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see #PCO_Exposure_Expose
 * @see #Exposure_Data
 * @see pco_command.html#PCO_Command_Set_Recording_State
 */
int PCO_Exposure_Abort(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_exposure.c","PCO_Exposure_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	Exposure_Data.Abort = TRUE;
	/* stop the camera recording */
	if(!PCO_Command_Set_Recording_State(FALSE))
		return FALSE;
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
 * This value is currently unused in the current implementation of PCO_Exposure_Expose.
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

