/* pco_command.cpp
** Autoguider PCO CMOS library
*/
/**
 * Command wrapper around the PCO SDK library.
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
#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_usb.h"
/*
 *  PCO_errt_w.h produces loads of -Wwrite-strings warnings when compiled, so turn off this warning for
 * this source file.
 */
#pragma GCC diagnostic ignored "-Wwrite-strings"
/**
 * Define PCO_ERRT_H_CREATE_OBJECT before including PCO_errt_w.h to enable the PCO_GetErrorText
 * function to be defined.
 */
#define PCO_ERRT_H_CREATE_OBJECT
/**
 * It is required by PCO_errt_w.h to define a sprintf_s function. PCO_errt_w.h defines one in the header
 * file for Microsoft C, here we declare sprintf_s as snprintf which should work for Linux.
 */
#define sprintf_s snprintf
#include "PCO_errt_w.h"
#include "ccd_general.h"
#include "pco_command.h"


/* check CCD_COMMAND_SETUP_FLAG enums match PCO_EDGE_SETUP #defines.
** We create our own enum so we don't have to include PCO SDK headers outside pco_command.cpp, for better
** code separation from the SDK */
/* THESE TESTS DON'T WORK, probably because one in an enumeration and one is an int.
#if CCD_COMMAND_SETUP_FLAG_ROLLING_SHUTTER != PCO_EDGE_SETUP_ROLLING_SHUTTER
#error "CCD_COMMAND_SETUP_FLAG_ROLLING_SHUTTER does not match PCO_EDGE_SETUP_ROLLING_SHUTTER declaration"
#endif
#if CCD_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER != PCO_EDGE_SETUP_GLOBAL_SHUTTER
#error "CCD_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER does not match PCO_EDGE_SETUP_GLOBAL_SHUTTER declaration"
#endif
#if CCD_COMMAND_SETUP_FLAG_GLOBAL_RESET != PCO_EDGE_SETUP_GLOBAL_RESET
#error "CCD_COMMAND_SETUP_FLAG_GLOBAL_RESET does not match PCO_EDGE_SETUP_GLOBAL_RESET declaration"
#endif
*/

/* data types */
/**
 * Data type holding local data to pco_command. This consists of the following:
 * <dl>
 * <dt>Camera</dt> <dd>The instance of CPco_com used to communicate with the PCO camera. </dd>
 * <dt>Grabber</dt> <dd>The instance of CPco_grab_usb used to grab images from the PCO camera. </dd>
 * <dt>PCO_Logger</dt> <dd>The instance of CPco_Log used to receive logging from the PCO library.</dd>
 * <dt>Camera_Board</dt> <dd>The board number passed to Open_Cam.</dd>
 * <dt>Grabber_Timeout</dt> <dd>The timeout for grabbing images, in milliseconds.</dd>
 * <dt>Description</dt> <dd>The camera description returned from PCO_GetCameraDescriptor.</dd>
 * </dl>
 */
struct Command_Struct
{
	CPco_com *Camera;
	CPco_grab_usb* Grabber;
	CPco_Log* PCO_Logger;
	int Camera_Board;
	int Grabber_Timeout;
	SC2_Camera_Description_Response Description;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The instance of Command_Struct that contains local data for this module. This is initialised as follows:
 * <dl>
 * <dt>Camera</dt> <dd>NULL</dd>
 * <dt>Grabber</dt> <dd>NULL</dd>
 * <dt>PCO_Logger</dt> <dd>NULL</dd>
 * <dt>Camera_Board</dt> <dd>0</dd>
 * <dt>Grabber_Timeout</dt> <dd>40000</dd> (The PCO Edge's maximum exposure length is 20s).
 * <dt>Description</dt> <dd>{}</dd>
 * </dl>
 * @see #Command_Struct
 */
static struct Command_Struct Command_Data = 
{
	NULL,NULL,NULL,0,40000,
};

/**
 * A buffer to store the error string generated by a call to Command_PCO_Get_Error_Text.
 * @see #CCD_GENERAL_ERROR_STRING_LENGTH
 */
static char Command_PCO_Error_String[CCD_GENERAL_ERROR_STRING_LENGTH] = "";

/* internal functions */
static char *Command_PCO_Get_Error_Text(DWORD pco_err);
static int Command_BCD_To_Decimal(unsigned char x);

/* --------------------------------------------------------
** External Functions
** -------------------------------------------------------- */
/**
 * Initialise the PCO library Camera reference. 
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int PCO_Command_Initialise_Camera(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Camera",LOG_VERBOSITY_TERSE,NULL,"Started.");
#endif
	CCD_General_Error_Number = 0;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Camera",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"Creating CPco_com_usb instance.");
#endif
	Command_Data.Camera = new CPco_com_usb();
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1100;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Initialise_Camera:Creating CPco_com_usb instance failed.");
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Camera",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"Creating CPco_Log instance.");
#endif 
	Command_Data.PCO_Logger = new CPco_Log("pco_camera_grab.log");
	if(Command_Data.PCO_Logger == NULL)
	{
		CCD_General_Error_Number =1101;
		sprintf(CCD_General_Error_String,"PCO_Command_Initialise_Camera:Creating CPco_Log instance failed.");
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Camera",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"Initialising CPco_Log instance.");
#endif
 	Command_Data.PCO_Logger->set_logbits(0x3);
	Command_Data.Camera->SetLog(Command_Data.PCO_Logger);
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Camera",LOG_VERBOSITY_TERSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Finalise (finish using) the CCD library. 
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int PCO_Command_Finalise(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Finalise",LOG_VERBOSITY_TERSE,NULL,"Started.");
#endif
	if(Command_Data.Grabber != NULL)
	{
#ifdef PCO_DEBUG
		CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Finalise",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				"Deleting Grabber object.");
#endif
		delete Command_Data.Grabber;
	}
	Command_Data.Grabber = NULL;
	if(Command_Data.Camera != NULL)
	{
#ifdef PCO_DEBUG
		CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Finalise",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				"Deleting Camera object.");
#endif
		delete Command_Data.Camera;
	}
	Command_Data.Camera = NULL;
	if(Command_Data.PCO_Logger != NULL)
	{
#ifdef PCO_DEBUG
		CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Finalise",LOG_VERBOSITY_VERY_VERBOSE,NULL,
				"Deleting PCO_Logger object");
#endif
		delete Command_Data.PCO_Logger;
	}
	Command_Data.PCO_Logger = NULL;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Finalise",LOG_VERBOSITY_TERSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Open a connection to the PCO camera and get a camera handle.
 * <ul>
 * <li>We check the Camera CPco_com_usb instance has been created.
 * <li>We set Command_Data.Camera_Board to the board parameter.
 * <li>We call the Camera's Open_Cam method with the board parameter to open a connection to the board.
 * <li>We get the camera's description by calling PCO_GetCameraDescriptor and store it in Command_Data.Description.
 * </ul>
 * @param board Which camera to connect to.
 * @return The routine returns TRUE on success and FALSE if it fails.
 * @see #Command_Data
 * @see #Command_PCO_Get_Error_Text
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_SECOND_MS
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_MILLISECOND_NS
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_ONE_SECOND_NS
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Open(int board)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Open",LOG_VERBOSITY_TERSE,NULL,
			       "Started for board %d.",board);
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1102;
		sprintf(CCD_General_Error_String,"PCO_Command_Open:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	Command_Data.Camera_Board = board;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Open",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Calling Open_Cam(%d).",Command_Data.Camera_Board);
#endif
	pco_err = Command_Data.Camera->Open_Cam(Command_Data.Camera_Board);
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1103;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Open:Camera Open_Cam(board=%d) failed with PCO error code 0x%x (%s).",
			Command_Data.Camera_Board,pco_err,Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Open",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "Getting camera description.");
#endif
	pco_err = Command_Data.Camera->PCO_GetCameraDescriptor(&(Command_Data.Description));
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1104;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Open:Camera PCO_GetCameraDescriptor failed with PCO error code 0x%x (%s).",
			pco_err,Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Open",LOG_VERBOSITY_TERSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Close an open connection to the camera.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Close(void)
{
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Close",LOG_VERBOSITY_TERSE,NULL,"Started.");
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1105;
		sprintf(CCD_General_Error_String,"PCO_Command_Close:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	Command_Data.Camera->Close_Cam();
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Close",LOG_VERBOSITY_TERSE,NULL,"Finished.");	
#endif
	return TRUE;
}

/**
 * Routine to initialise the PCO Grabber object reference, which handles the downloading of image data.
 * This needs to be initialised after the PCO camera object has been initialised (PCO_Command_Initialise_Camera)
 *  and opened  (PCO_Command_Open).
 * <ul>
 * <li>We construct an instance of CPco_grab_usb attached to the opened camera and assign it to Command_Data.Grabber.
 * <li>We set the Grabber's log instance to Command_Data.PCO_Logger.
 * <li>We open a connection to the grabber by calling the Grabber's Open_Grabber method with the board parameter.
 * <li>We set the Grabber's timeout to Command_Data.Grabber_Timeout.
 * </ul>
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see #PCO_Command_Initialise_Camera
 * @see #PCO_Command_Open
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Initialise_Grabber(void)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Grabber",LOG_VERBOSITY_TERSE,NULL,"Started.");
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1106;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Initialise_Grabber:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	/* create grabber for opened camera */
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Initialise_Grabber",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"Creating Grabber for camera.");
#endif
	Command_Data.Grabber = new CPco_grab_usb((CPco_com_usb*)(Command_Data.Camera));
	if(Command_Data.Grabber == NULL)
	{
		CCD_General_Error_Number = 1107;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Initialise_Grabber:Creating CPco_grab_usb instance failed.");
		return FALSE;
	}
	Command_Data.Grabber->SetLog(Command_Data.PCO_Logger);
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Initialise_Grabber",LOG_VERBOSITY_INTERMEDIATE,
			       NULL,"Opening Grabber with board ID %d.",Command_Data.Camera_Board);
#endif
	pco_err = Command_Data.Grabber->Open_Grabber(Command_Data.Camera_Board);
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1108;
		sprintf(CCD_General_Error_String,
		 "PCO_Command_Initialise_Grabber:Grabber Open_Grabber(board=%d) failed with PCO error code 0x%x (%s).",
			Command_Data.Camera_Board,pco_err,Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
	Command_Data.Grabber->Set_Grabber_Timeout(Command_Data.Grabber_Timeout);
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Initialise_Grabber",LOG_VERBOSITY_TERSE,NULL,
			       "Finished.");
#endif
	return TRUE;
}

/**
 * Setup the camera. This changes some settings (notably shutter readout mode) which then requires a camera reboot
 * (and associated re-connect/open) to cause the camera head to pick up the new settings.
 * @param setup_flag The flag to setup. These are currently the shutter mode, set as follows:
 *        <ul>
 *        <li>0x00000001 = PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER = PCO_EDGE_SETUP_ROLLING_SHUTTER = Rolling Shutter
 *        <li>0x00000002 = PCO_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER  = PCO_EDGE_SETUP_GLOBAL_SHUTTER  = Global Shutter
 *        <li>0x00000004 = PCO_COMMAND_SETUP_FLAG_GLOBAL_RESET    = PCO_EDGE_SETUP_GLOBAL_RESET    = Global Reset 
 *        </ul>
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #PCO_COMMAND_SETUP_FLAG
 * @see #Command_PCO_Get_Error_Text
 * @see #Command_Data
 * @see #PCO_Command_Reboot_Camera
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Set_Camera_Setup(enum PCO_COMMAND_SETUP_FLAG setup_flag)
{
	DWORD setup_flag_dword;
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Set_Camera_Setup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Started with setup flag 0x%x.",setup_flag);
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1109;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Set_Camera_Setup:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	/* To set the current shutter mode input index setup_id must be set to 0. 
	** The new shutter mode should be set in setup_flag[0]. */
	setup_flag_dword = setup_flag;
	pco_err = Command_Data.Camera->PCO_SetCameraSetup(0,&setup_flag_dword,1);
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1110;
		sprintf(CCD_General_Error_String,"PCO_Command_Set_Camera_Setup:"
			"Camera PCO_SetCameraSetup(0,0x%x,1) failed with PCO error code 0x%x (%s).",setup_flag_dword,
			pco_err,Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Set_Camera_Setup",LOG_VERBOSITY_VERBOSE,NULL,"Finished.");
#endif
	return TRUE;
}
	
/**
 * This function reboots the PCO camera head. The function returns as soon as the reboot process has started. 
 * After calling this function the camera handle should be closed using PCO_Command_Close. 
 * The reboot can take 6-10 seconds.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_PCO_Get_Error_Text
 * @see #Command_Data
 * @see #PCO_Command_Close
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Reboot_Camera(void)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Reboot_Camera",LOG_VERBOSITY_VERBOSE,NULL,"Started.");
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1111;
		sprintf(CCD_General_Error_String,"PCO_Command_Reboot_Camera:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	pco_err = Command_Data.Camera->PCO_RebootCamera();
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1112;
		sprintf(CCD_General_Error_String,"PCO_Command_Reboot_Camera:"
			"Camera PCO_RebootCamera failed with PCO error code 0x%x (%s).",pco_err,
			Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Reboot_Camera",LOG_VERBOSITY_VERBOSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Prepare the camera to start taking data. All previous settings are validated and the internal settings of the camera
 * updated.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_PCO_Get_Error_Text
 * @see #Command_Data
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Arm_Camera(void)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Arm_Camera",LOG_VERBOSITY_VERBOSE,NULL,"Started.");
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1113;
		sprintf(CCD_General_Error_String,"PCO_Command_Arm_Camera:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	pco_err = Command_Data.Camera->PCO_ArmCamera();
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1114;
		sprintf(CCD_General_Error_String,"PCO_Command_Arm_Camera:"
			"Camera PCO_ArmCamera failed with PCO error code 0x%x (%s).",pco_err,
			Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Arm_Camera",LOG_VERBOSITY_VERBOSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Prepare the camera to start taking data. All previous settings are validated and the internal settings of the camera
 * updated.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see #Command_PCO_Get_Error_Text
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Grabber_Post_Arm(void)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Grabber_Post_Arm",LOG_VERBOSITY_VERBOSE,NULL,"Started.");
#endif
	if(Command_Data.Grabber == NULL)
	{
		CCD_General_Error_Number = 1115;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Grabber_Post_Arm:Grabber CPco_grab_usb instance not created.");
		return FALSE;
	}
	pco_err = Command_Data.Grabber->PostArm();
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1116;
		sprintf(CCD_General_Error_String,"PCO_Command_Grabber_Post_Arm:"
			"Grabber PostArm failed with PCO error code 0x%x (%s).",pco_err,
			Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Grabber_Post_Arm",LOG_VERBOSITY_VERBOSE,NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Set the camera's time to the current time.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see #Command_PCO_Get_Error_Text
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Set_Camera_To_Current_Time(void)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Set_Camera_To_Current_Time",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"Started.");
#endif
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1117;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Set_Camera_To_Current_Time:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	pco_err = Command_Data.Camera->PCO_SetCameraToCurrentTime();
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1118;
		sprintf(CCD_General_Error_String,"PCO_Command_Set_Camera_To_Current_Time:"
			"Camera PCO_SetCameraToCurrentTime failed with PCO error code 0x%x (%s).",pco_err,
			Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Set_Camera_To_Current_Time",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"Finished.");
#endif
	return TRUE;
}

/**
 * Set the camera's recording state to either TRUE (1) or FALSE (0). This allows the camera to start
 * collecting data (exposures).
 * @param rec_state An integer/boolean, set to TRUE (1) to start recording data and FALSE (0) to stop recording data.
 * @return The routine returns TRUE on success and FALSE if an error occurs.
 * @see #Command_Data
 * @see #Command_PCO_Get_Error_Text
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Command_Set_Recording_State(int rec_state)
{
	DWORD pco_err;

#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_command.cpp","PCO_Command_Set_Recording_State",LOG_VERBOSITY_INTERMEDIATE,
			       NULL,"PCO_Command_Set_Recording_State(%d): Started.",rec_state);
#endif
	if(!CCD_GENERAL_IS_BOOLEAN(rec_state))
	{
		CCD_General_Error_Number = 1119;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Set_Recording_State:Illegal value for rec_state parameter (%d).",rec_state);
		return FALSE;
	}
	if(Command_Data.Camera == NULL)
	{
		CCD_General_Error_Number = 1120;
		sprintf(CCD_General_Error_String,
			"PCO_Command_Set_Recording_State:Camera CPco_com_usb instance not created.");
		return FALSE;
	}
	pco_err = Command_Data.Camera->PCO_SetRecordingState(rec_state);
	if(pco_err != PCO_NOERROR)
	{
		CCD_General_Error_Number = 1121;
		sprintf(CCD_General_Error_String,"PCO_Command_Set_Recording_State:"
			"Camera PCO_SetRecordingState(%d) failed with PCO error code 0x%x (%s).",
			rec_state,pco_err,Command_PCO_Get_Error_Text(pco_err));
		return FALSE;
	}
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_command.cpp","PCO_Command_Set_Recording_State",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			"Finished.");
#endif
	return TRUE;
}
	
/* =======================================
**  internal functions 
** ======================================= */
/**
 * This function is a wrapper to the PCO_GetErrorText routine imported from the PCO_errt_w.h.
 * This allows us to produce a descriptive string for the specified PCO error code.
 * @param pco_err The PCO error code to provide a descriptive string for.
 * @return A pointer to a string containing the descriptive string for the PCO error code. The
 *         string pointer to is always Command_PCO_Error_String.
 * @see #CCD_GENERAL_ERROR_STRING_LENGTH
 * @see #Command_PCO_Error_String
 */
static char *Command_PCO_Get_Error_Text(DWORD pco_err)
{
	PCO_GetErrorText(pco_err,Command_PCO_Error_String,CCD_GENERAL_ERROR_STRING_LENGTH);
	return Command_PCO_Error_String;
}

/**
 * Routine to convert the BCD (binary coded decimal) number to a normal integer.
 * @param x An unisgned char containing the BCD number (0..100). This is normally passed in a WORD, and we take
 *        the lower byte which contains the encoded number.
 * @return The decoded integer (0..100).
 */
static int Command_BCD_To_Decimal(unsigned char x)
{
    return x - 6 * (x >> 4);
}
