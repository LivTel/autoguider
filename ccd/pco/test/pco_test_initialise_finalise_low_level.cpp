/* pco_test_initialise_finalise.cpp
 * $Id$
 * Test the initialisation and finalisation of the PCO library.
 */
/**
 * Test the initialisation and finalisation of the PCO library.
 * @author $Author: cjm $
 * @version $Revision$
 */
#include <iostream>
#include <stdio.h>
#include "VersionNo.h"
#include "Cpco_com.h"
#include "Cpco_grab_usb.h"

/* internal variables */
CPco_com *Camera = NULL;
CPco_grab_usb* Grabber = NULL;
CPco_Log* PCO_Logger = NULL;
int Camera_Board = 0;
int Grabber_Timeout = 40000;

/* -----------------------------------------------------------------------------
**      External routines
** ----------------------------------------------------------------------------- */
/**
 * Main program.
 * <ul>
 * <li>We call Parse_Arguments to parse the command line, setup logging and config filenames etc...
 * <li>We set the CCD library log handler function (CCD_General_Set_Log_Handler_Function) 
 *     to stdout (CCD_General_Log_Handler_Stdout).
 * </ul>
 */
int main(int argc, char *argv[])
{
	//	SC2_Camera_Description_Response description;
	DWORD pco_err;
	WORD camtype;
	DWORD serialnumber;
	int retval;
		
	std::cout << "pco_test_initialise_finalise_low_level:started.\n";
/* parse arguments */
	/*
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
	*/
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
	std::cout << "Initialising Grabber.\n";
	Grabber = new CPco_grab_usb((CPco_com_usb*)(Camera));
	if(Grabber == NULL)
	{
		std::cout << "Initialising Grabber failed.\n";
		return 1;
	}
	std::cout << "Set Grabber logger.\n";
	Grabber->SetLog(PCO_Logger);
	std::cout << "Open Grabber for board " << Camera_Board << ".\n";
	pco_err = Grabber->Open_Grabber(Camera_Board);
	if(pco_err != PCO_NOERROR)
	{
		std::cout << "Opening Grabber failed.\n";
		return 1;
	}
	std::cout << "Setting Grabber Timeout to " << Grabber_Timeout << ".\n";
	Grabber->Set_Grabber_Timeout(Grabber_Timeout);
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

	/* PCO_Command_Finalise*/
	if(Grabber != NULL)
	{
		std::cout << "Close Grabber.\n";
		Grabber->Close_Grabber();
		std::cout << "Delete Grabber.\n";
		delete Grabber;
	}
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
	std::cout << "pco_test_initialise_finalise_low_level:finished.\n";
	return 0;
}
