/* fli_driver.c
** Autoguider FLI CCD camera library driver interface routines
** $Header: /home/cjm/cvs/autoguider/ccd/fli/c/fli_driver.c,v 1.1 2013-11-26 16:28:36 cjm Exp $
*/
/**
 * Driver interface routines for the FLI autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.1 $
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
#include "log_udp.h"
#include "ccd_general.h"
#include "fli_driver.h"
#include "fli_exposure.h"
#include "fli_general.h"
#include "fli_setup.h"
#include "fli_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: fli_driver.c,v 1.1 2013-11-26 16:28:36 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Fill in the driver function structure.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see fli_setup.html#FLI_Setup_Startup
 * @see fli_setup.html#FLI_Setup_Dimensions
 * @see fli_setup.html#FLI_Setup_Abort
 * @see fli_setup.html#FLI_Setup_Get_NCols
 * @see fli_setup.html#FLI_Setup_Get_NRows
 * @see fli_setup.html#FLI_Setup_Shutdown
 * @see fli_exposure.html#FLI_Exposure_Expose
 * @see fli_exposure.html#FLI_Exposure_Bias
 * @see fli_exposure.html#FLI_Exposure_Abort
 * @see fli_exposure.html#FLI_Exposure_Get_Exposure_Start_Time
 * @see fli_temperature.html#FLI_Temperature_Get
 * @see fli_temperature.html#FLI_Temperature_Set
 * @see fli_temperature.html#FLI_Temperature_Cooler_On
 * @see fli_temperature.html#FLI_Temperature_Cooler_Off
 */
int FLI_Driver_Register(struct CCD_Driver_Function_Struct *functions)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_driver.c","FLI_Driver_Register",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
#endif
	if(functions == NULL)
	{
		CCD_General_Error_Number = 1000;
		sprintf(CCD_General_Error_String,"FLI_Driver_Register: functions were NULL.");
		return FALSE;
	}
	/* setup */
	functions->Setup_Startup = FLI_Setup_Startup;
	functions->Setup_Dimensions = FLI_Setup_Dimensions;
	functions->Setup_Abort = FLI_Setup_Abort;
	functions->Setup_Get_NCols = FLI_Setup_Get_NCols;
	functions->Setup_Get_NRows = FLI_Setup_Get_NRows;
	functions->Setup_Shutdown = FLI_Setup_Shutdown;
	/* exposure */
	functions->Exposure_Expose = FLI_Exposure_Expose;
	functions->Exposure_Bias = FLI_Exposure_Bias;
	functions->Exposure_Abort = FLI_Exposure_Abort;
	functions->Exposure_Get_Exposure_Start_Time = FLI_Exposure_Get_Exposure_Start_Time;
	functions->Exposure_Loop_Pause_Length_Set = FLI_Exposure_Loop_Pause_Length_Set;
	/* temperature */
	functions->Temperature_Get = FLI_Temperature_Get;
	functions->Temperature_Set = FLI_Temperature_Set;
	functions->Temperature_Cooler_On = FLI_Temperature_Cooler_On;
	functions->Temperature_Cooler_Off = FLI_Temperature_Cooler_Off;
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_driver.c","FLI_Driver_Register",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
*/
