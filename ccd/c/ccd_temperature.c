/* ccd_temperature.c
** Autoguider CCD Library temperature routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_temperature.c,v 1.1 2006-06-01 15:27:37 cjm Exp $
*/
/**
 * Temperature routines for the autoguider CCD library.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ccd_driver.h"
#include "ccd_general.h"
#include "ccd_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_temperature.c,v 1.1 2006-06-01 15:27:37 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Get the current temperature of the CCD.
 * @param temperature The address of a double to return ther temperature in, in degrees centigrade.
 * @param temperature_status The address of a enum to store the temperature status.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_temperature.html#CCD_TEMPERATURE_STATUS
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_GENERAL_LOG_BIT_TEMPERATURE
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Get() started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Temperature_Get == NULL)
	{
		CCD_General_Error_Number = 500;
		sprintf(CCD_General_Error_String,"CCD_Temperature_Get:Temperature_Get function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Temperature_Get))(temperature,temperature_status);
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Get() finished.");
#endif
	return TRUE;
}

/**
 * Set the current target temperature of the CCD. The cooler also needs to be switched on for this to take effect.
 * @param temperature The temperature to ramp the CCD to, in degrees centigrade.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_temperature.html#CCD_TEMPERATURE_STATUS
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_GENERAL_LOG_BIT_TEMPERATURE
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Set(double target_temperature)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Set() started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Temperature_Set == NULL)
	{
		CCD_General_Error_Number = 501;
		sprintf(CCD_General_Error_String,"CCD_Temperature_Set:Temperature_Set function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Temperature_Set))(target_temperature);
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Set() finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler on. The CCD should then cool to the target temperature set by CCD_Temperature_Set.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_GENERAL_LOG_BIT_TEMPERATURE
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Cooler_On(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Cooler_On() started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Temperature_Cooler_On == NULL)
	{
		CCD_General_Error_Number = 502;
		sprintf(CCD_General_Error_String,"CCD_Temperature_Cooler_On:Temperature_Cooler_On function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Temperature_Cooler_On))();
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Cooler_On() finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler off.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_GENERAL_LOG_BIT_TEMPERATURE
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Cooler_Off(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Cooler_Off() started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Temperature_Cooler_Off == NULL)
	{
		CCD_General_Error_Number = 503;
		sprintf(CCD_General_Error_String,"CCD_Temperature_Cooler_Off:"
			"Temperature_Cooler_Off function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Temperature_Cooler_Off))();
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_TEMPERATURE,"CCD_Temperature_Cooler_Off() finished.");
#endif
	return TRUE;
}

/**
 * Routine to translate the specified temperature status to a suitable string.
 * @param temperature_status The status to translate.
 * @return A string describing the specified state, or "UNKNOWN".
 * @see #CCD_TEMPERATURE_STATUS
 * @see #CCD_Temperature_Get
 */
char *CCD_Temperature_Status_To_String(enum CCD_TEMPERATURE_STATUS temperature_status)
{
	switch(temperature_status)
	{
		case CCD_TEMPERATURE_STATUS_OFF:
			return "OFF";
		case CCD_TEMPERATURE_STATUS_AMBIENT:
			return "AMBIENT";
		case CCD_TEMPERATURE_STATUS_OK:
			return "OK";
		case CCD_TEMPERATURE_STATUS_RAMPING:
			return "RAMPING";
		case CCD_TEMPERATURE_STATUS_UNKNOWN:
			return "UNKNOWN";
		default:
			return "UNKNOWN";
	}
	return "UNKNOWN";
}

/*
** $Log: not supported by cvs2svn $
*/
