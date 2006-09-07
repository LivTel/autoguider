/* andor_driver.c
** Autoguder Andor CCD Library driver interface routines
** $Header: /home/cjm/cvs/autoguider/ccd/andor/c/andor_driver.c,v 1.3 2006-09-07 14:57:48 cjm Exp $
*/
/**
 * Driver interface routines for the Andor autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.3 $
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

#include "ccd_general.h"
#include "andor_driver.h"
#include "andor_exposure.h"
#include "andor_general.h"
#include "andor_setup.h"
#include "andor_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: andor_driver.c,v 1.3 2006-09-07 14:57:48 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Fill in the driver function structure.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see andor_setup.html#Andor_Setup_Startup
 * @see andor_setup.html#Andor_Setup_Dimensions
 * @see andor_setup.html#Andor_Setup_Abort
 * @see andor_setup.html#Andor_Setup_Get_NCols
 * @see andor_setup.html#Andor_Setup_Get_NRows
 * @see andor_setup.html#Andor_Setup_Shutdown
 * @see andor_exposure.html#Andor_Exposure_Expose
 * @see andor_exposure.html#Andor_Exposure_Bias
 * @see andor_exposure.html#Andor_Exposure_Abort
 * @see andor_exposure.html#Andor_Exposure_Get_Exposure_Start_Time
 * @see andor_temperature.html#Andor_Temperature_Get
 * @see andor_temperature.html#Andor_Temperature_Set
 * @see andor_temperature.html#Andor_Temperature_Cooler_On
 * @see andor_temperature.html#Andor_Temperature_Cooler_Off
 */
int Andor_Driver_Register(struct CCD_Driver_Function_Struct *functions)
{
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_DRIVER,"Andor_Driver_Register started.");
#endif
	if(functions == NULL)
	{
		CCD_General_Error_Number = 1300;
		sprintf(CCD_General_Error_String,"Andor_Driver_Register: functions were NULL.");
		return FALSE;
	}
	/* setup */
	functions->Setup_Startup = Andor_Setup_Startup;
	functions->Setup_Dimensions = Andor_Setup_Dimensions;
	functions->Setup_Abort = Andor_Setup_Abort;
	functions->Setup_Get_NCols = Andor_Setup_Get_NCols;
	functions->Setup_Get_NRows = Andor_Setup_Get_NRows;
	functions->Setup_Shutdown = Andor_Setup_Shutdown;
	/* exposure */
	functions->Exposure_Expose = Andor_Exposure_Expose;
	functions->Exposure_Bias = Andor_Exposure_Bias;
	functions->Exposure_Abort = Andor_Exposure_Abort;
	functions->Exposure_Get_Exposure_Start_Time = Andor_Exposure_Get_Exposure_Start_Time;
	functions->Exposure_Loop_Pause_Length_Set = Andor_Exposure_Loop_Pause_Length_Set;
	/* temperature */
	functions->Temperature_Get = Andor_Temperature_Get;
	functions->Temperature_Set = Andor_Temperature_Set;
	functions->Temperature_Cooler_On = Andor_Temperature_Cooler_On;
	functions->Temperature_Cooler_Off = Andor_Temperature_Cooler_Off;
#ifdef ANDOR_DEBUG
	CCD_General_Log(ANDOR_GENERAL_LOG_BIT_DRIVER,"Andor_Driver_Register finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/04/28 14:28:02  cjm
** Added Log comment.
**
*/
