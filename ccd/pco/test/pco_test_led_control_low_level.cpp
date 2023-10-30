/* pco_test_led_control_low_level.cpp
 * $Id$
 * Test turning the rear camera status LEDs on and off.
 */
/**
 * Test turning the rear camera status LEDs on and off.
 * @author $Author: cjm $
 * @version $Revision$
 */
#include <iostream>
#include <stdio.h>
#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_usb.h"

/* internal variables */
/**
 * Camera control object.
 */
CPco_com *Camera = NULL;
/**
 * Camera grabber control object.
 */
CPco_grab_usb* Grabber = NULL;
/**
 * PCO library logger.
 */
CPco_Log* PCO_Logger = NULL;
/**
 * Which camera board to talk to.
 */
int Camera_Board = 0;
/**
 * Timeout for the camera grabber.
 */
int Grabber_Timeout = 40000;
/**
 * Whether to turn the camera status LEDs on or off.  From Cpco_com_func_2.h:
 * <ul>
 * <li>0x00000000 = [OFF]
 * <li>0xFFFFFFFF = [ON]
 * </ul>
 */
DWORD LED_OnOff = 0x00000000;

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);

/* -----------------------------------------------------------------------------
**      External routines
** ----------------------------------------------------------------------------- */
/**
 * Main program.
 * <ul>
 * <li>We parse the command line arguments.
 * <li>We create the Camera object.
 * <li>We create the PCO_Logger object.
 * <li>We set the PCO_Logger log bits.
 * <li>We set the Camera object's logger.
 * <li>We open a connection to the camera specified by Camera_Board.
 * <li>We set the camera's HW LED state by calling the Camera object's PCO_SetHWLEDSignal method with LED_OnOff 
 *     as a parameter.
 * <li>We close the camera and then delete the Camera object.
 * <li>We delete the PCO_Logger object.
 * </ul>
 * @see #Parse_Arguments
 * @see #Camera
 * @see #Camera_Board
 * @see #PCO_Logger
 * @see #LED_OnOff
 */
int main(int argc, char *argv[])
{
	DWORD pco_err;
	WORD camtype;
	DWORD serialnumber;
	int retval;
		
	std::cout << "pco_test_led_control_low_level:started.\n";
	/* parse arguments */
	std::cout << "Parsing Arguments.\n";
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* PCO_Command_Initialise_Camera */
	std::cout << "Create Camera object.\n";
	Camera = new CPco_com_usb();
	if(Camera == NULL)
	{
		std::cout << "Creating camera failed.\n";
		return 1;
	}
	std::cout << "Create Logger object.\n";
	PCO_Logger = new CPco_Log("pco_camera_grab.log");
	if(PCO_Logger == NULL)
	{
		std::cout << "Creating loggerfailed .\n";
		return 1;
	}
	std::cout << "Initialise Logger bits.\n";
 	PCO_Logger->set_logbits(0x3);
	std::cout << "Set camera logger.\n";
	Camera->SetLog(PCO_Logger);
	/* PCO_Command_Open */
	std::cout << "Open Camera.\n";
	pco_err = Camera->Open_Cam(Camera_Board);
	if(pco_err != PCO_NOERROR)
	{
		std::cout << "Open Camera failed.\n";
		return 1;
	}
	// Try getting camera type here
	//std::cout << "Get Camera Type.\n";
	//pco_err = Camera->PCO_GetCameraType(&camtype,&serialnumber);
	//if(pco_err!=PCO_NOERROR)
	//{
	//	std::cout << "Get Camera Type failed.\n";
	//	Camera->Close_Cam();
	//	delete Camera;
	//	return 1;
	//}
	/* PCO_Command_Initialise_Grabber*/
	//std::cout << "Initialising Grabber.\n";
	//Grabber = new CPco_grab_usb((CPco_com_usb*)(Camera));
	//if(Grabber == NULL)
	//{
	//	std::cout << "Initialising Grabber failed.\n";
	//	return 1;
	//}
	//std::cout << "Set Grabber logger.\n";
	//Grabber->SetLog(PCO_Logger);
	//std::cout << "Open Grabber for board " << Camera_Board << ".\n";
	//pco_err = Grabber->Open_Grabber(Camera_Board);
	//if(pco_err != PCO_NOERROR)
	//{
	//	std::cout << "Opening Grabber failed.\n";
	//	return 1;
	//}
	//std::cout << "Setting Grabber Timeout to " << Grabber_Timeout << ".\n";
	//Grabber->Set_Grabber_Timeout(Grabber_Timeout);
	// get camera descriptor
	//std::cout << "Get Camera descriptior.\n";
	//Camera->PCO_GetCameraDescriptor(&description);
	// Do we need to stop recording before closing the camera?
	//std::cout << "Set Camera recording state to 0.\n";
	//Camera->PCO_SetRecordingState(0);
	// Arm camera
	//std::cout << "Arming camera.\n";
	//pco_err = Camera->PCO_ArmCamera();
	//if(pco_err != PCO_NOERROR)
	//{
	//	std::cout << "Arming camera failed.\n";
	//	return 1;
	//}
	//std::cout << "Grabber Post Arm.\n";
	//pco_err = Grabber->PostArm();
	//if(pco_err != PCO_NOERROR)
	//{
	//	std::cout << "Grabber Post Arm failed.\n";
	//	return 1;
	//}
	// Set camera LED
	std::cout << "Setting HW LED signal to " << LED_OnOff <<  ".\n";
	pco_err = Camera->PCO_SetHWLEDSignal(LED_OnOff);
	if(pco_err!=PCO_NOERROR)
	{
		std::cout << "Set HW LED signal failed.\n";
		Camera->Close_Cam();
		delete Camera;
		return 1;
	}
	// Do we need to sleep here for a bit to see whether the change worked?
	
	/* PCO_Command_Finalise*/
	//if(Grabber != NULL)
	//{
	//	std::cout << "Close Grabber.\n";
	//	Grabber->Close_Grabber();
	//	std::cout << "Delete Grabber.\n";
	//	delete Grabber;
	//}
	/* PCO_Command_Close */
	std::cout << "Close Camera.\n";
	Camera->Close_Cam();
	if(Camera != NULL)
	{
		std::cout << "Delete Camera.\n";
		delete Camera;
	}
	if(PCO_Logger != NULL)
	{
		std::cout << "Delete Logger.\n";
		delete PCO_Logger;
	}
	std::cout << "pco_test_led_control_low_level:finished.\n";
	return 0;
}

/* -----------------------------------------------------------------------------
**      Internal routines
** ----------------------------------------------------------------------------- */
/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"PCO Test LED control:Help.\n");
	fprintf(stdout,"pco_test_led_control_low_level -on|-off [-camera_board <board number>][-help]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\t-help prints out this message and stops the program.\n");
	fprintf(stdout,"\t-camera_board select which camera to control, the default is 0.\n");
	fprintf(stdout,"\t-on turns the Camera HW status LEDs on.\n");
	fprintf(stdout,"\t-off turns the Camera HW status LEDs off. This is the default\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Camera_Board
 * @see #LED_OnOff
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-camera_board")==0)||(strcmp(argv[i],"-c")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Camera_Board);
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing camera board %s failed.\n",argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:Camera_Board requires an integer number.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
		}
		else if(strcmp(argv[i],"-off")==0)
			LED_OnOff = 0x00000000;
		else if(strcmp(argv[i],"-on")==0)
			LED_OnOff = 0xFFFFFFFF;
		else
		{
			fprintf(stderr,"Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}
	}
	return TRUE;
}

