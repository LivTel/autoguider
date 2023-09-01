/* pco_driver.c
** Autoguider PCO CMOS camera library driver interface routines
** $Id$
*/
/**
 * Driver interface routines for the PCO autoguider library.
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
#include "pco_driver.h"
#include "pco_exposure.h"
#include "pco_setup.h"
#include "pco_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Fill in the driver function structure.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see pco_setup.html#PCO_Setup_Startup
 * @see pco_setup.html#PCO_Setup_Dimensions
 * @see pco_setup.html#PCO_Setup_Abort
 * @see pco_setup.html#PCO_Setup_Get_NCols
 * @see pco_setup.html#PCO_Setup_Get_NRows
 * @see pco_setup.html#PCO_Setup_Shutdown
 * @see pco_exposure.html#PCO_Exposure_Expose
 * @see pco_exposure.html#PCO_Exposure_Bias
 * @see pco_exposure.html#PCO_Exposure_Abort
 * @see pco_exposure.html#PCO_Exposure_Get_Exposure_Start_Time
 * @see pco_exposure.html#PCO_Exposure_Loop_Pause_Length_Set
 * @see pco_temperature.html#PCO_Temperature_Get
 * @see pco_temperature.html#PCO_Temperature_Set
 * @see pco_temperature.html#PCO_Temperature_Cooler_On
 * @see pco_temperature.html#PCO_Temperature_Cooler_Off
 */
int PCO_Driver_Register(struct CCD_Driver_Function_Struct *functions)
{
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_driver.c","PCO_Driver_Register",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
#endif
	if(functions == NULL)
	{
		CCD_General_Error_Number = 1000;
		sprintf(CCD_General_Error_String,"PCO_Driver_Register: functions were NULL.");
		return FALSE;
	}
	/* setup */
	functions->Setup_Startup = PCO_Setup_Startup;
	functions->Setup_Dimensions = PCO_Setup_Dimensions;
	functions->Setup_Abort = PCO_Setup_Abort;
	functions->Setup_Get_NCols =PCO_Setup_Get_NCols;
	functions->Setup_Get_NRows = PCO_Setup_Get_NRows;
	functions->Setup_Shutdown = PCO_Setup_Shutdown;
	/* exposure */
	functions->Exposure_Expose = PCO_Exposure_Expose;
	functions->Exposure_Bias = PCO_Exposure_Bias;
	functions->Exposure_Abort = PCO_Exposure_Abort;
	functions->Exposure_Get_Exposure_Start_Time = PCO_Exposure_Get_Exposure_Start_Time;
	functions->Exposure_Loop_Pause_Length_Set = PCO_Exposure_Loop_Pause_Length_Set;
	/* temperature */
	functions->Temperature_Get = PCO_Temperature_Get;
	functions->Temperature_Set = PCO_Temperature_Set;
	functions->Temperature_Cooler_On = PCO_Temperature_Cooler_On;
	functions->Temperature_Cooler_Off = PCO_Temperature_Cooler_Off;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_driver.c","PCO_Driver_Register",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
#endif
	return TRUE;
}
