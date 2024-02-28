/* pco_setup.cpp
** Autoguider PCO CMOS library setup routines
*/
/**
 * Setup routines for the PCO camera driver.
 * @author Chris Mottram
 * @version $Id$
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_general.h"
#include "pco_command.h"
#include "pco_setup.h"
/*
 * CCD_Config calls produce -Wwrite-strings warnings when compiled, so turn off this warning for
 * this source file.
 */
#pragma GCC diagnostic ignored "-Wwrite-strings"

/* hash defines */
/**
 * The maximum length of enumerated value strings in Setup_Data.
 * The longest SensorReadoutMode value is 25 characters long.
 * The longest SimplePreAmpGainControl value is 40 characters long.
 */
#define SETUP_ENUM_VALUE_STRING_LENGTH (64)

/* data types */
/**
 * Data type holding local data to pco_setup. This consists of the following:
 * <dl>
 * <dt>Camera_Board</dt> <dd>The board parameter passed to Open_Cam, to determine which camera to connect to.</dd>
 * <dt>Camera_Setup_Flag</dt> <dd>The camera setup flag to use, when configuring how the shuttering/readout/reset on the
 *                                camera is configured.</dd>
 * <dt>Camera_Timestamp_Mode</dt> <dd>The camera timestamp mode, one of off or binary. This determines whether the 
 *                                camera stores the actual exposure start time in the first 14 pixels of the image, or whether
 *                                we guess the exposure start time from when we command the camera to take the image.</dd>
 * <dt>Horizontal_Binning</dt> <dd>The readout horizontal binning, stored as an integer. Can be one of 1,2,3,4,8. </dd>
 * <dt>Vertical_Binning</dt> <dd>The readout vertical binning, stored as an integer. Can be one of 1,2,3,4,8. </dd>
 * <dt>Serial_Number</dt> <dd>An integer containing the serial number retrieved from the camera head
 *                            Retrieved from the camera library during PCO_Setup_Startup.</dd>
 * <dt>Pixel_Width</dt> <dd>A double storing the pixel width in micrometers. Setup from the sensor type 
 *                          during CCD_Setup_Startup.</dd>
 * <dt>Pixel_Height</dt> <dd>A double storing the pixel height in micrometers. Setup from the sensor type 
 *                          during CCD_Setup_Startup.</dd>
 * <dt>Sensor_Width</dt> <dd>An integer storing the sensor width in pixels retrieved from the camera during 
 *                       CCD_Setup_Startup.</dd>
 * <dt>Sensor_Height</dt> <dd>An integer storing the sensor height in pixels retrieved from the camera during 
 *                       CCD_Setup_Startup.</dd>
 * <dt>Start_X</dt> <dd>Start X position pixel of the imaging window (inclusive).</dd>
 * <dt>Start_Y</dt> <dd>Start Y position pixel of the imaging window (inclusive).</dd>
 * <dt>End_X</dt> <dd>End X position pixel of the imaging window (inclusive).</dd>
 * <dt>End_Y</dt> <dd>End Y position pixel of the imaging window (inclusive).</dd>
 * </dl>
 * @see ccd_command.html#PCO_COMMAND_SETUP_FLAG
 */
struct Setup_Struct
{
	int Camera_Board;
	enum PCO_COMMAND_SETUP_FLAG Camera_Setup_Flag;
	enum PCO_COMMAND_TIMESTAMP_MODE Camera_Timestamp_Mode;
	int Horizontal_Binning;
	int Vertical_Binning;
	int Serial_Number;
	double Pixel_Width;
	double Pixel_Height;
	int Sensor_Width;
	int Sensor_Height;
	int Start_X;
	int Start_Y;
	int End_X;
	int End_Y;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The instance of Setup_Struct that contains local data for this module. This is initialised as follows:
 * <dl>
 * <dt>Camera_Board</dt> <dd>0</dd>
 * <dt>Camera_Setup_Flag</dt> <dd>PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER</dd>
 * <dt>Camera_Timestamp_Mode</dt> <dd>PCO_COMMAND_TIMESTAMP_MODE_BINARY</dd>
 * <dt>Horizontal_Binning</dt> <dd>1</dd>
 * <dt>Vertical_Binning</dt> <dd>1</dd>
 * <dt>Serial_Number</dt> <dd>-1</dd>
 * <dt>Pixel_Width</dt> <dd>0.0</dd>
 * <dt>Pixel_Height</dt> <dd>0.0</dd>
 * <dt>Sensor_Width</dt> <dd>0</dd>
 * <dt>Sensor_Height</dt> <dd>0</dd>
 * <dt>Start_X</dt> <dd>0</dd>
 * <dt>Start_Y</dt> <dd>0</dd>
 * <dt>End_X</dt> <dd>0</dd>
 * <dt>End_Y</dt> <dd>0</dd>
 * </dl>
 * @see pco_command.html#PCO_COMMAND_SETUP_FLAG
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE
 */
static struct Setup_Struct Setup_Data = 
{
	0,PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER,PCO_COMMAND_TIMESTAMP_MODE_BINARY,1,1,-1,0.0,0.0,0,0,0,0,0,0
};

/* internal functions */

/* --------------------------------------------------------
** External Functions
** -------------------------------------------------------- */
/**
 * Do the initial setup for a PCO camera.
 * <ul>
 * <li>We retrieve the configured board number from the config file by caling CCD_Config_Get_Integer with the keyword
 *     "ccd.pco.setup.board_number" and save it to Setup_Data.Camera_Board.
 * <li>We retrieve the shutter mode from the config file by caling CCD_Config_Get_String with the keyword
 *     "ccd.pco.setup.shutter_mode" and save it to Setup_Data.Camera_Setup_Flag, 
 *     after converting the returned string to a PCO_COMMAND_SETUP_FLAG.
 * <li>We retrieve the timestamp mode from the config file by caling CCD_Config_Get_String with the keyword
 *     "ccd.pco.setup.timestamp_mode" and save it to Setup_Data.Camera_Timestamp_Mode, 
 *     after converting the returned string to a PCO_COMMAND_TIMESTAMP_MODE.
 * <li>We retrieve the status led configuration from the config file by caling CCD_Config_Get_String with the keyword
 *     "ccd.pco.setup.status_led" and convert it to a boolean.
 * <li>We initialise the libraries used using PCO_Command_Initialise_Camera.
 * <li>We open a connection to the PCO camera using PCO_Command_Open, using the retrieved board number. 
 * <li>We set the camera shutter readout/reset mode, 
 *     by calling PCO_Command_Set_Camera_Setup with the previously configured Setup_Data.Camera_Setup_Flag as a parameter.
 * <li>We reboot the camera head, to make the camera setup change take effect, by calling PCO_Command_Reboot_Camera.
 * <li>We close the open connection to the camera head by calling PCO_Command_Close_Camera.
 * <li>We delete the camera and logger object reference create in PCO_Command_Initialise_Camera 
 *      by calling PCO_Command_Finalise_Camera.
 * <li>We sleep for 10 seconds whilst the camera reboots.
 * <li>We re-initialise the libraries used using PCO_Command_Initialise_Camera.
 * <li>We re-open a connection to the PCO camera using PCO_Command_Open, using the retrieved board number. 
 * <li>We create a grabber reference by calling PCO_Command_Initialise_Grabber.
 * <li>We set the PCO camera to use the current time by calling PCO_Command_Set_Camera_To_Current_Time.
 * <li>We stop any ongoing image acquisitions by calling PCO_Command_Set_Recording_State(FALSE).
 * <li>We reset the camera to a known state by calling PCO_Command_Reset_Settings.
 * <li>We set the camera timestamps using PCO_Command_Set_Timestamp_Mode to the previously configured
 *     Setup_Data.Camera_Timestamp_Mode.
 * <li>We set the camera exposure and delay timebase to microseconds using 
 *     PCO_Command_Set_Timebase(PCO_COMMAND_TIMEBASE_US,PCO_COMMAND_TIMEBASE_US).
 * <li>We set an initial delay and exposure time by calling PCO_Command_Set_Delay_Exposure_Time(0,50);
 * <li>We call PCO_Command_Description_Get_Num_ADCs to get the number of ADCs supported by this camera.
 * <li>If the returned ADC count is greater than one, we call PCO_Command_Set_ADC_Operation(2) to use the extra ADC.
 * <li>We call PCO_Command_Set_Bit_Alignment(0x0001) to set the returned data to be LSB.
 * <li>We call PCO_Command_Set_Noise_Filter_Mode to set noise reduction to off.
 * <li>We call PCO_Command_Set_HW_LED_Signal to set the PCO camera status LED to either on or off (depending on the
 *     previously loaded configuration).
 * <li>We call PCO_Command_Description_Get_Max_Horizontal_Size to get Setup_Data.Sensor_Width from the camera
 *     description.
 * <li>We call PCO_Command_Description_Get_Max_Vertical_Size to get Setup_Data.Sensor_Height from the camera
 *     description.
 * <li>We call PCO_Command_Get_Camera_Type to get the camera's serial number from it's head.
 * <li>We call PCO_Command_Description_Get_Sensor_Type to get the camera's sensor type number from it's description.
 * <li>We call PCO_Command_Arm_Camera to update the cameras internal state to take account of the new settings.
 * <li>We call PCO_Command_Grabber_Post_Arm to update the grabber's state to match the camera's state.
 * <ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #PCO_SETUP_KEYWORD_ROOT
 * @see #Setup_Data
 * @see pco_command.html#PCO_COMMAND_SETUP_FLAG
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE
 * @see pco_command.html#PCO_COMMAND_TIMEBASE
 * @see pco_command.html#PCO_Command_Initialise_Camera
 * @see pco_command.html#PCO_Command_Open
 * @see ccd_command.html#PCO_Command_Set_Camera_Setup
 * @see ccd_command.html#PCO_Command_Reboot_Camera
 * @see ccd_command.html#PCO_Command_Close_Camera
 * @see ccd_command.html#PCO_Command_Finalise_Camera
 * @see pco_command.html#PCO_Command_Set_Camera_To_Current_Time
 * @see pco_command.html#PCO_Command_Set_Recording_State
 * @see pco_command.html#PCO_Command_Reset_Settings
 * @see pco_command.html#PCO_Command_Set_Timestamp_Mode
 * @see pco_command.html#PCO_Command_Set_HW_LED_Signal
 * @see pco_command.html#PCO_Command_Set_Timebase
 * @see pco_command.html#PCO_Command_Set_Delay_Exposure_Time
 * @see pco_command.html#PCO_Command_Description_Get_Num_ADCs
 * @see pco_command.html#PCO_Command_Set_ADC_Operation
 * @see pco_command.html#PCO_Command_Set_Bit_Alignment
 * @see pco_command.html#PCO_Command_Set_Noise_Filter_Mode
 * @see pco_command.html#PCO_Command_Description_Get_Max_Horizontal_Size
 * @see pco_command.html#PCO_Command_Description_Get_Max_Vertical_Size
 * @see pco_command.html#PCO_Command_Get_Camera_Type
 * @see pco_command.html#PCO_Command_Description_Get_Sensor_Type
 * @see pco_command.html#PCO_Command_Arm_Camera
 * @see pco_command.html#PCO_Command_Grabber_Post_Arm
 * @see ../../cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Setup_Startup(void)
{
	char *shutter_mode_string = NULL;
	char *timestamp_mode_string = NULL;
	char *status_led_string = NULL;
	int adc_count,camera_type,sensor_type,sensor_subtype,led_onoff;
	
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	/* get the board number to use in PCO_Command_Open */
	if(!CCD_Config_Get_Integer(PCO_SETUP_KEYWORD_ROOT"board_number",&(Setup_Data.Camera_Board)))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file PCO board number:%d.",Setup_Data.Camera_Board);
#endif
	/* get the shutter mode / setup flag to configure the camera with */
	if(!CCD_Config_Get_String(PCO_SETUP_KEYWORD_ROOT"shutter_mode",&shutter_mode_string))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file shutter mode :%s.",shutter_mode_string);
#endif
	if(strcmp(shutter_mode_string,"ROLLING_SHUTTER") == 0)
		Setup_Data.Camera_Setup_Flag = PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER;
	else if(strcmp(shutter_mode_string,"GLOBAL_SHUTTER") == 0)
		Setup_Data.Camera_Setup_Flag = PCO_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER;
	else if(strcmp(shutter_mode_string,"GLOBAL_RESET") == 0)
		Setup_Data.Camera_Setup_Flag = PCO_COMMAND_SETUP_FLAG_GLOBAL_RESET;
	else
	{
		CCD_General_Error_Number = 1305;
		sprintf(CCD_General_Error_String,"PCO_Setup_Startup: Unknown shutter mode string : %s.",shutter_mode_string);
		if(shutter_mode_string != NULL)
			free(shutter_mode_string);
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Shutter mode : %s creates setup flag %d.",shutter_mode_string,Setup_Data.Camera_Setup_Flag);
#endif
	if(shutter_mode_string != NULL)
		free(shutter_mode_string);
	/* get the timestamp mode */
	if(!CCD_Config_Get_String(PCO_SETUP_KEYWORD_ROOT"timestamp_mode",&timestamp_mode_string))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file timestamp mode :%s.",timestamp_mode_string);
#endif
	if(strcmp(timestamp_mode_string,"OFF") == 0)
		Setup_Data.Camera_Timestamp_Mode = PCO_COMMAND_TIMESTAMP_MODE_OFF;
	else if(strcmp(timestamp_mode_string,"BINARY") == 0)
		Setup_Data.Camera_Timestamp_Mode = PCO_COMMAND_TIMESTAMP_MODE_BINARY;
	else
	{
		CCD_General_Error_Number = 1308;
		sprintf(CCD_General_Error_String,"PCO_Setup_Startup: Unknown timestamp mode string : %s.",timestamp_mode_string);
		if(timestamp_mode_string != NULL)
			free(timestamp_mode_string);
		return FALSE;
	}	
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Timestamp mode : %s creates setup flag %d.",timestamp_mode_string,
			       Setup_Data.Camera_Timestamp_Mode);
#endif
	if(timestamp_mode_string != NULL)
		free(timestamp_mode_string);
	/* get the status led configuration */
	if(!CCD_Config_Get_String(PCO_SETUP_KEYWORD_ROOT"status_led",&status_led_string))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file status led :%s.",status_led_string);
#endif
	if(strcmp(status_led_string,"OFF") == 0)
		led_onoff = FALSE;
	else if(strcmp(status_led_string,"ON") == 0)
		led_onoff = TRUE;
	else
	{
		CCD_General_Error_Number = 1309;
		sprintf(CCD_General_Error_String,"PCO_Setup_Startup: Unknown status led string : %s.",
			status_led_string);
		if(status_led_string != NULL)
			free(status_led_string);
		return FALSE;
	}	
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Status LED : %s creates status led boolean %d.",status_led_string,led_onoff);
#endif
	if(status_led_string != NULL)
		free(status_led_string);
	/* initialise the PCO libraries (first time) */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Initialising PCO camera libraries.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Initialise_Camera())
		return FALSE;
	/* open a connection to the CCD camera - first time */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Opening a connection to a PCO camera with board number %d.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Open(Setup_Data.Camera_Board))
		return FALSE;
	/* stop any ongoing exposures */
	if(!PCO_Command_Set_Recording_State(FALSE))
		return FALSE;
	/* set the camera shutter readout/reset mode */
	if(!PCO_Command_Set_Camera_Setup(Setup_Data.Camera_Setup_Flag))
		return FALSE;
	/* reboot the camera head, to make the camera setup change take effect */
	if(!PCO_Command_Reboot_Camera())
		return FALSE;
	/* close camera connection after reboot */
	if(!PCO_Command_Close_Camera())
		return FALSE;
	/* delete camera and logger object reference before recreating */
	if(!PCO_Command_Finalise_Camera())
		return FALSE;
	/* wait 10 seconds before attempting to re-connect. See MA_PCOSDK_V127.pdf,  Section 2.4.8, PCO_SetCameraSetup, P52 */
#if LOGGING > 0
	CCD_General_Log_Format(LOG_VERBOSITY_TERSE,"PCO_Setup_Startup: Sleeping for 10 seconds whilst the camera reboots.");
#endif /* LOGGING */
	sleep(10);
	/* initialise the PCO libraries (second time) */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Initialising PCO camera libraries.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Initialise_Camera())
		return FALSE;
	/* open a connection to the CCD camera - second time */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Opening a connection to a PCO camera with board number %d.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Open(Setup_Data.Camera_Board))
		return FALSE;
	/* initialise grabber reference */
	if(!PCO_Command_Initialise_Grabber())
		return FALSE;
	/* initial configuration of the camera */
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,"Initialising PCO camera.");
#endif
	/* set camera to use current time */
	if(!PCO_Command_Set_Camera_To_Current_Time())
		return FALSE;
	/* stop any ongoing exposures */
	if(!PCO_Command_Set_Recording_State(FALSE))
		return FALSE;
	/* reset camera to a known state */
	if(!PCO_Command_Reset_Settings())
		return FALSE;
	/* set what timestamp data to include in the read out image */
	if(!PCO_Command_Set_Timestamp_Mode(Setup_Data.Camera_Timestamp_Mode))
		return FALSE;
	/* set exposure and delay timebase to microseconds */
	if(!PCO_Command_Set_Timebase(PCO_COMMAND_TIMEBASE_US,PCO_COMMAND_TIMEBASE_US))
		return FALSE;
	/* set an initial delay and exposure time */
	if(!PCO_Command_Set_Delay_Exposure_Time(0,50))
		return FALSE;
	/* get the number of adc's supported by the camera */
	if(!PCO_Command_Description_Get_Num_ADCs(&adc_count))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Camera has %d ADCs.",adc_count);
#endif
	/* we must use only 1 ADC, otherwise the horizontal ROI must be symetrical. This is slower though */
	if(!PCO_Command_Set_ADC_Operation(1))
		return FALSE;
	if(!PCO_Command_Set_Bit_Alignment(0x0001)) /* 0x001 = LSB */
		return FALSE;
	if(!PCO_Command_Set_Noise_Filter_Mode(0x0000)) /* 0x0000 = off */
		return FALSE;
	if(!PCO_Command_Set_HW_LED_Signal(led_onoff))
		return FALSE;
	/* get and store some data from the description, for use later */
	if(!PCO_Command_Description_Get_Max_Horizontal_Size(&Setup_Data.Sensor_Width))
		return FALSE;
	if(!PCO_Command_Description_Get_Max_Vertical_Size(&Setup_Data.Sensor_Height))
		return FALSE;
	/* get and store camera serial number */
	if(!PCO_Command_Get_Camera_Type(&camera_type,&(Setup_Data.Serial_Number)))
		return FALSE;
	/* get the camera sensor type and subtype */
	if(!PCO_Command_Description_Get_Sensor_Type(&sensor_type,&sensor_subtype))
		return FALSE;
	/* based on sensor type, figure out pixel sizes - PCO library cannot do this directly */
	switch(sensor_type)
	{
		case 0x2002: /* sCMOS CIS1042_V1_FI_BW, as present in out pco.edge 4.2 */
			/* according to the PCO Edge manual MA_PCOEDGE_V225.pdf, P27 */
			Setup_Data.Pixel_Width = 6.5;
			Setup_Data.Pixel_Height = 6.5;
			break;
		default:
			CCD_General_Error_Number = 1300;
			sprintf(CCD_General_Error_String,"PCO_Setup_Startup: Unknown sensor type 0x%x : "
				"unable to set pixel size.",sensor_type);
			return FALSE;
	}
	/* prepare camera for taking data */
	if(!PCO_Command_Arm_Camera())
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Shutdown the connection to the PCO camera.
 * <ul>
 * <li>We close connection to the PCO Grabber using PCO_Command_Close_Grabber.
 * <li>We finalise the PCO grabber object used using PCO_Command_Finalise_Grabber.
 * <li>We close connection to the PCO camera using PCO_Command_Close_Camera.
 * <li>We finalise the PCO camera and logger objects used using PCO_Command_Finalise_Camera.
 * <ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see pco_command.html#PCO_Command_Close_Grabber
 * @see pco_command.html#PCO_Command_Finalise_Grabber
 * @see pco_command.html#PCO_Command_Close_Camera
 * @see pco_command.html#PCO_Command_Finalise_Camera
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int PCO_Setup_Shutdown(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Shutdown",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	/* close the open connection to the PCO grabber */
	if(!PCO_Command_Close_Grabber())
		return FALSE;
	/* Free up the PCO Grabber objects */
	if(!PCO_Command_Finalise_Grabber())
		return FALSE;
	/* close the open Camera connection  */
	if(!PCO_Command_Close_Camera())
		return FALSE;
	/* Free up the PCO Camera and Logger objects */
	if(!PCO_Command_Finalise_Camera())
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Shutdown",LOG_VERBOSITY_INTERMEDIATE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Check the dimensions (particularily the window dimensions) are valid for the PCO camera. PCO_SetROI is very
 * picky about valid window positions and sizes, and will silenty modify the actual window, which then
 * causes dark subtraction, flat fielding and detected object position problems. 
 * @param ncols The address of an integer, 
 *              on entry to the function containing the number of unbinned image columns (X).
 * @param nrows The address of an integer, 
 *              on entry to the function containing the number of unbinned image rows (Y).
 * @param hbin The address of an integer, on entry to the function containing the binning in X.
 * @param vbin The address of an integer, on entry to the function containing the binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A pointer to a structure containing window data. 
 *        These dimensions are inclusive, and in binned pixels.
 *        The window data is modified by the routine so the window bounds are legal for the PCO camera.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_command.html#PCO_Command_Description_Get_ROI_Horizontal_Step_Size
 * @see ccd_command.html#PCO_Command_Description_Get_ROI_Vertical_Step_Size
 */
int PCO_Setup_Dimensions_Check(int *ncols,int *nrows,int *hbin,int *vbin,
			       int window_flags,struct CCD_Setup_Window_Struct *window)
{
	int roi_hss,roi_vss,offset_sx,offset_sy,offset_wsx,offset_wsy,wsx,wsy;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	if(window == NULL)
	{
		CCD_General_Error_Number = 1307;
		sprintf(CCD_General_Error_String,"PCO_Setup_Dimensions_Check: window was NULL.");
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Input window (sx=%d,sy=%d,ex=%d,ey=%d).",window->X_Start,window->Y_Start,
			       window->X_End,window->Y_End);
#endif
	/* According to the PCO edge manual, MA_PCOEDGE_V225.pdf, Section 7.3 (P25), ROI table,
	** pco.edge 4.2 USB 3.0 has a ROI horizontal step (roi_hss) of 4, and a ROI vertical step (roi_vss) of 1, 
	** with no vertical symmetry.
	** Note only the x start position has to lie on a  (roi_hss) 4 pixel boundary.
	** Note the first pixel is 1, so the valid (roi_hss) 4 pixel boundaries are a not obvious, i.e.
	** 1,5,9,13 etc...
	*/
	/* for the generic case, we can get the ROI Horizontal Step Size and the ROI Vertical Step Size
	** from the camera description */
	if(!PCO_Command_Description_Get_ROI_Horizontal_Step_Size(&roi_hss))
		return FALSE;
	if(!PCO_Command_Description_Get_ROI_Vertical_Step_Size(&roi_vss))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "roi_hss = %d, roi_vss = %d.",roi_hss,roi_vss);
#endif
	/* X_Start */
	offset_sx = (window->X_Start-1)%roi_hss;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "offset_sx = %d.",offset_sx);
#endif
	if(offset_sx > 0)
	{
		window->X_Start -= offset_sx;
		window->X_End -= offset_sx;
#ifdef PCO_DEBUG
		CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,
				       NULL,"New window sx = %d, New window ex = %d.",window->X_Start,window->X_End);
#endif
	}/* end if offset_sx > 0 */
	
	/* check the end x position is also aligned correctly. We can change the window size here,
	** as the main autoguider code resizes the guide buffer after each CCD_Setup_Dimensions call. Hopefully
	** the field (full-frame) dimensions will always be sane.
	** Here the window has to be a width that is a whole number of (roi_hss) 4 pixels, 
	** but the start and end positions are inclusive pixels. Note this guarantees that the end pixel will not be on a 
	** (roi_hss) 4 pixel boundary! 
	*/
	/* X_End */
	wsx = (window->X_End-window->X_Start)+1; /* inclusive pixels */
	offset_wsx = wsx%roi_hss;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "offset_wsx = %d.",offset_wsx);
#endif
	if(offset_wsx > 0)
	{
		window->X_End -= offset_wsx;
#ifdef PCO_DEBUG
		CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,
				       NULL,"New window ex = %d.",window->X_End);
#endif
	}
	/* Y_Start */
	offset_sy = (window->Y_Start-1)%roi_vss;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "offset_sy = %d.",offset_sy);
#endif
	if(offset_sy > 0)
	{
		window->Y_Start -= offset_sy;
		window->Y_End -= offset_sy;
#ifdef PCO_DEBUG
		CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,
				       NULL,"New window sy = %d, New window ey = %d.",window->Y_Start,window->Y_End);
#endif
	}/* end if offset_sy > 0 */
	
	/* check the end y position is also aligned correctly. We can change the window size here,
	** as the main autoguider code resizes the guide buffer after each CCD_Setup_Dimensions call. Hopefully
	** the field (full-frame) dimensions will always be sane.
	** Here the window has to be a height that is a whole number of roi_vss pixels, 
	** but the start and end positions are inclusive pixels. Note this guarantees that the end pixel will not be on a 
	** roi_vss pixel boundary! 
	*/
	/* Y_End */
	wsy = (window->Y_End-window->Y_Start)+1; /* inclusive pixels */
	offset_wsy = wsy%roi_vss;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "offset_wsy = %d.",offset_wsy);
#endif
	if(offset_wsy > 0)
	{
		window->Y_End -= offset_wsy;
#ifdef PCO_DEBUG
		CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_VERY_VERBOSE,
				       NULL,"New window ey = %d.",window->Y_End);
#endif
	}
	/* finish */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Finished window (sx=%d,sy=%d,ex=%d,ey=%d).",window->X_Start,window->Y_Start,
			       window->X_End,window->Y_End);
#endif
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions_Check",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Finished.");
#endif
	return TRUE;
}

/**
 * Setup binning and other per exposure configuration.
 * <ul>
 * <li>We use PCO_SETUP_BINNING_IS_VALID to check the binning parameter is a supported binning.
 * <li>We store the binning in Setup_Data.Binning.
 * <li>We call PCO_Command_Set_Binning to set the binning.
 * <li>We call PCO_Command_Set_ROI to set the region of interest to match the binning,
 *     with the end positions computed from Setup_Data.Sensor_Width / Setup_Data.Sensor_Height.
 * <li>We call PCO_Command_Arm_Camera to update the camera's internal settings to use the new binning.
 * <li>We call PCO_Command_Grabber_Post_Arm to update the grabber's internal settings to use the new binning.
 * <li>We call PCO_Command_Get_ROI to see if the camera's ROI matches what we just set.
 * <li> We call PCO_Command_Grabber_Get_Actual_Size to see what actual size the PCO camera thinks the window will be.
 * <li>We check the PCO camera is going to use the same window size as we have passed it, 
 *     and return an error if this is not the case.
 * </ul>
 * @param ncols Number of unbinned image columns (X).
 * @param nrows Number of unbinned image rows (Y).
 * @param hbin Binning in X.
 * @param vbin Binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A structure containing window data. The window is meant to be inclusive, i.e. it goes from
 *        window.X_Start to window.X_End (with both pixels being included) and the width of the window is
 *        (window.X_End-window.X_Start)+1.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #PCO_SETUP_BINNING_IS_VALID
 * @see #Setup_Data
 * @see pco_command.html#PCO_Command_Set_Binning
 * @see pco_command.html#PCO_Command_Set_ROI
 * @see pco_command.html#PCO_Command_Arm_Camera
 * @see pco_command.html#PCO_Command_Grabber_Post_Arm
 * @see pco_command.html#PCO_Command_Get_ROI
 * @see pco_command.html#PCO_Command_Grabber_Get_Actual_Size
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 */
int PCO_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
			 int window_flags,struct CCD_Setup_Window_Struct window)
{
	int actual_roi_sx,actual_roi_sy,actual_roi_ex,actual_roi_ey;
	int actual_w,actual_h,actual_bp;
	
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Started with ncols=%d, nrows=%d, hbin=%d, vbin=%d, window_flags=%d, "
			       "window={xstart=%d,ystart=%d,xend=%d,yend=%d}.",ncols,nrows,hbin,vbin,window_flags,
			       window.X_Start,window.Y_Start,window.X_End,window.Y_End);
#endif
	if(!PCO_SETUP_BINNING_IS_VALID(hbin))
	{
		CCD_General_Error_Number = 1301;
		sprintf(CCD_General_Error_String,"PCO_Setup_Dimensions: Horizontal binning %d not valid.",hbin);
		return FALSE;
	}
	if(!PCO_SETUP_BINNING_IS_VALID(vbin))
	{
		CCD_General_Error_Number = 1302;
		sprintf(CCD_General_Error_String,"PCO_Setup_Dimensions: Vertical binning %d not valid.",hbin);
		return FALSE;
	}
	/* save the binning for later retrieval */
	Setup_Data.Horizontal_Binning = hbin;
	Setup_Data.Vertical_Binning = vbin;
	/* set the actual binning */
	if(!PCO_Command_Set_Binning(hbin,vbin))
		return FALSE;
	if(window_flags > 0)
	{
		/* Set_ROI takes binned pixels, the passed in window also has binned pixels */ 
		Setup_Data.Start_X = window.X_Start;
		Setup_Data.Start_Y = window.Y_Start;
		Setup_Data.End_X = window.X_End; /* the window pixel dimensions are inclusive */
		Setup_Data.End_Y = window.Y_End; /* the window pixel dimensions are inclusive */
	}
	else
	{
		/* set the ROI to the binned pixel area to read out */
		Setup_Data.Start_X = 1;
		Setup_Data.Start_Y = 1;
		Setup_Data.End_X = Setup_Data.Sensor_Width/hbin;
		Setup_Data.End_Y = Setup_Data.Sensor_Height/vbin;
	}
	if(!PCO_Command_Set_ROI(Setup_Data.Start_X,Setup_Data.Start_Y,Setup_Data.End_X,Setup_Data.End_Y))
		return FALSE;
	/* get camera to update it's internal settings */
	if(!PCO_Command_Arm_Camera())
		return FALSE;
	/* get grabber to update to the new binning */
	if(!PCO_Command_Grabber_Post_Arm())
		return FALSE;
	/* get the actual ROI and compare it with what we set it to */
	if(!PCO_Command_Get_ROI(&actual_roi_sx,&actual_roi_sy,&actual_roi_ex,&actual_roi_ey))
		return FALSE;
	if((Setup_Data.Start_X != actual_roi_sx)||(Setup_Data.Start_Y != actual_roi_sy)||
	   (Setup_Data.End_X != actual_roi_ex)||(Setup_Data.End_Y != actual_roi_ey))
	{
		CCD_General_Error_Number = 1306;
		sprintf(CCD_General_Error_String,
			"PCO_Setup_Dimensions: Returned ROI (%d,%d,%d,%d) does not match "
			"configured ROI (%d,%d,%d,%d).",
			actual_roi_sx,actual_roi_sy,actual_roi_ex,actual_roi_ey,
			Setup_Data.Start_X,Setup_Data.Start_Y,Setup_Data.End_X,Setup_Data.End_Y);
		return FALSE;
	}
	/* what actual dimensions does the PCO library think we should be using? */
	if(!PCO_Command_Grabber_Get_Actual_Size(&actual_w,&actual_h,&actual_bp))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Actual image width/height returned by grabber (w=%d,h=%d,bp=%d).",
			       actual_w,actual_h,actual_bp);
#endif
	/* Check ROI dimensions and actual size returned by the PCO camera API match.
	** The camera can increase the window size (ours does if the horizontal window width is not divisible by 4).
	** This would crash the autoguider software (the buffers are not big enough) so return an error is this has been
	** allowed to happen */
	if(actual_w != PCO_Setup_Get_NCols())
	{
		CCD_General_Error_Number = 1303;
		sprintf(CCD_General_Error_String,
			"PCO_Setup_Dimensions: Returned actual width does not match window (%d vs %d).",
			actual_w,PCO_Setup_Get_NCols());
		return FALSE;
	}
	if(actual_h != PCO_Setup_Get_NRows())
	{
		CCD_General_Error_Number = 1304;
		sprintf(CCD_General_Error_String,
			"PCO_Setup_Dimensions: Returned actual height does not match window (%d vs %d).",
			actual_h,PCO_Setup_Get_NRows());
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Dimensions",LOG_VERBOSITY_INTERMEDIATE,NULL,"Finished.");
#endif
	return TRUE;
}

void PCO_Setup_Abort(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Abort",LOG_VERBOSITY_INTERMEDIATE,NULL,"Finished.");
#endif
}

/**
 * Return the number of binned pixels in X in the current image.
 * @return The number of binned pixels in X in the current image.
 * @see Setup_Data
 */
int PCO_Setup_Get_NCols(void)
{
	return (Setup_Data.End_X-Setup_Data.Start_X)+1;
}

/**
 * Return the number of binned pixels in Y in the current image.
 * @return The number of binned pixels in Y in the current image.
 * @see Setup_Data
 */
int PCO_Setup_Get_NRows(void)
{
	return (Setup_Data.End_Y-Setup_Data.Start_Y)+1;
}

/**
 * Return the camera timestamp mode configured using PCO_Command_Set_Timestamp_Mode during PCO_Setup_Startup.
 * @return The configured PCO timestamp mode.
 * @see #Setup_Data
 * @see #PCO_Setup_Startup
 * @see #PCO_COMMAND_TIMESTAMP_MODE
 */
enum PCO_COMMAND_TIMESTAMP_MODE PCO_Setup_Get_Timestamp_Mode(void)
{
	return Setup_Data.Camera_Timestamp_Mode;
}
