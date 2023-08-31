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
 */
struct Setup_Struct
{
	int Camera_Board;
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
 */
static struct Setup_Struct Setup_Data = 
{
	0,1,1,-1,0.0,0.0,0,0,0,0,0,0
};

/* internal functions */

/* --------------------------------------------------------
** External Functions
** -------------------------------------------------------- */
/**
 * Do the initial setup for a PCO camera.
 * <ul>
 * <li>We initialise the libraries used using PCO_Command_Initialise_Camera.
 * <li>We retrieve the configured board number from the config file by caling CCD_Config_Get_Integer with the keyword
 *     "ccd.pco.setup.board_number" and save it to Setup_Data.Camera_Board.
 * <li>We open a connection to the PCO camera using PCO_Command_Open, using the retrieved board number. 
 * <li>We set the PCO camera to use the current time by calling PCO_Command_Set_Camera_To_Current_Time.
 * <li>We stop any ongoing image acquisitions by calling PCO_Command_Set_Recording_State(FALSE).
 * <li>We reset the camera to a known state by calling PCO_Command_Reset_Settings.
 * <li>We set the camera timestamps using PCO_Command_Set_Timestamp_Mode to PCO_COMMAND_TIMESTAMP_MODE_BINARY.
 * <li>We set the camera exposure and delay timebase to microseconds using 
 *     PCO_Command_Set_Timebase(PCO_COMMAND_TIMEBASE_US,PCO_COMMAND_TIMEBASE_US).
 * <li>We set an initial delay and exposure time by calling PCO_Command_Set_Delay_Exposure_Time(0,50);
 * <li>We call PCO_Command_Description_Get_Num_ADCs to get the number of ADCs supported by this camera.
 * <li>If the returned ADC count is greater than one, we call PCO_Command_Set_ADC_Operation(2) to use the extra ADC.
 * <li>We call PCO_Command_Set_Bit_Alignment(0x0001) to set the returned data to be LSB.
 * <li>We call PCO_Command_Set_Noise_Filter_Mode to set noise reduction to off.
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
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE_BINARY
 * @see pco_command.html#PCO_COMMAND_TIMEBASE
 * @see pco_command.html#PCO_Command_Initialise_Camera
 * @see pco_command.html#PCO_Command_Open
 * @see pco_command.html#PCO_Command_Set_Camera_To_Current_Time
 * @see pco_command.html#PCO_Command_Set_Recording_State
 * @see pco_command.html#PCO_Command_Reset_Settings
 * @see pco_command.html#PCO_Command_Set_Timestamp_Mode
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
	int adc_count,camera_type,sensor_type,sensor_subtype;
	
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	/* initialise the PCO libraries */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Initialising PCO camera libraries.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Initialise_Camera())
		return FALSE;
	/* get the board number to use in PCO_Command_Open */
	if(!CCD_Config_Get_Integer(PCO_SETUP_KEYWORD_ROOT"board_number",&(Setup_Data.Camera_Board)))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file PCO board number:%d.",Setup_Data.Camera_Board);
#endif
	/* open a connection to the CCD camera */
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_setup.c","PCO_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Opening a connection to a PCO camera with board number %d.",Setup_Data.Camera_Board);
#endif
	if(!PCO_Command_Open(Setup_Data.Camera_Board))
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
	if(!PCO_Command_Set_Timestamp_Mode(PCO_COMMAND_TIMESTAMP_MODE_BINARY))
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
				"unable to set pixel size.",0x2002);
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
 * Shutdown the connection to the CCD.
 * <ul>
 * <li>We close connection to the CCD camera using PCO_Command_Close.
 * <li>We finalise the libraries used using PCO_Command_Finalise.
 * <ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see pco_command.html#PCO_Command_Close
 * @see pco_command.html#PCO_Command_Finalise
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int PCO_Setup_Shutdown(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Shutdown",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
#endif
	/* close the open connection to the CCD camera */
	if(!PCO_Command_Close())
		return FALSE;
	/* shutdown the PCO library */
	if(!PCO_Command_Finalise())
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Shutdown",LOG_VERBOSITY_INTERMEDIATE,NULL,"Finished.");
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
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 */
int PCO_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
			 int window_flags,struct CCD_Setup_Window_Struct window)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_setup.c","PCO_Setup_Dimensions",LOG_VERBOSITY_INTERMEDIATE,NULL,"Started.");
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
		/* diddly looks like Set_ROI takes binned pixels, are the passed in window binned or not? */ 
		Setup_Data.Start_X = window.X_Start;
		Setup_Data.Start_Y = window.Y_Start;
		Setup_Data.End_X = window.X_End; /* diddly do we need to add 1 to be inclusive? */
		Setup_Data.End_Y = window.Y_End; /* diddly do we need to add 1 to be inclusive? */
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
	/* diddly where do we store the window parameters? */
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
