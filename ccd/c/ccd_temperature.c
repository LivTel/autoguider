/* ccd_temperature.c
** Autoguider CCD Library temperature routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_temperature.c,v 1.3 2010-07-28 09:45:29 cjm Exp $
*/
/**
 * Temperature routines for the autoguider CCD library.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_driver.h"
#include "ccd_general.h"
#include "ccd_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_temperature.c,v 1.3 2010-07-28 09:45:29 cjm Exp $";

/* data types */
/**
 * Internal temperature Data structure. Contains cached temperature data.
 * <dl>
 * <dt>Target_Temperature</dt> <dd>Cached target temperature, in degrees C.</dd>
 * <dt>Cached_Temperature</dt> <dd>Cached temperature, in degrees C.</dd>
 * <dt>Cached_Temperature_Status</dt> <dd>Cached Status of type CCD_TEMPERATURE_STATUS.</dd>
 * <dt>Cache_Date_Stamp</dt> <dd>Date cached data was acquired, of type struct timespec.</dd>
 * </dl>
 * @see #CCD_TEMPERATURE_STATUS
 */
struct Temperature_Struct
{
	double Target_Temperature;
	double Cached_Temperature;
	enum CCD_TEMPERATURE_STATUS Cached_Temperature_Status;
	struct timespec Cache_Date_Stamp;
};

/* internal variables */
/**
 * Internal temperature Data.
 * @see #Temperature_Struct
 */
static struct Temperature_Struct Temperature_Data = {0.0,0.0,CCD_TEMPERATURE_STATUS_UNKNOWN,{0,0L}};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Get the current temperature of the CCD. If retrieving the temperature is successful,
 * the cached temperature data is updated.
 * @param temperature The address of a double to return ther temperature in, in degrees centigrade.
 * @param temperature_status The address of a enum to store the temperature status.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see #Temperature_Data
 * @see ccd_temperature.html#CCD_TEMPERATURE_STATUS
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Get",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
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
	/* update temperature cache data */
	Temperature_Data.Cached_Temperature = (*temperature);
	Temperature_Data.Cached_Temperature_Status = (*temperature_status);
	clock_gettime(CLOCK_REALTIME,&(Temperature_Data.Cache_Date_Stamp));
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Get",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Set the current target temperature of the CCD. The cooler also needs to be switched on for this to take effect.
 * @param temperature The temperature to ramp the CCD to, in degrees centigrade.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see #Temperature_Data
 * @see ccd_temperature.html#CCD_TEMPERATURE_STATUS
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Set(double target_temperature)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Set",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
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
	/* save target temperature in cache. */
	Temperature_Data.Target_Temperature = target_temperature;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Set",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
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
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Cooler_On(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cooler_On",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
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
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cooler_On",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
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
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Temperature_Cooler_Off(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cooler_Off",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"started.");
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
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cooler_Off",LOG_VERBOSITY_INTERMEDIATE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Get the cached temperature data from the last CCD_Temperature_Get call.
 * @param temperature A pointer to a double. If non-null, on return filled with the cached temperature in C.
 * @param temperature_status A pointer to a enum CCD_TEMPERATURE_STATUS. If non-null, on return filled with the cached 
 *        temperature status.
 * @param cache_date_stamp A pointer to a struct timespec. If non-null, on return filled with the cache date stamp.
 * @return The routine returns TRUE if successful, and FALSE if it fails.
 * @see #Temperature_Data
 * @see #CCD_Temperature_Status_To_String
 * @see #CCD_TEMPERATURE_STATUS
 */
int CCD_Temperature_Cached_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status,
					   struct timespec *cache_date_stamp)
{
	char time_buff[32];

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cached_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
		       "CCD_Temperature_Cached_Temperature_Get() started.");
#endif
	if(temperature != NULL)
	{
		(*temperature) = Temperature_Data.Cached_Temperature;
#ifdef CCD_DEBUG
		CCD_General_Log_Format("ccd","ccd_temperature.c","CCD_Temperature_Cached_Temperature_Get",
				      LOG_VERBOSITY_VERBOSE,NULL,
				      "CCD_Temperature_Cached_Temperature_Get() found cached temperature %.2f.",
				      Temperature_Data.Cached_Temperature);
#endif
	}
	if(temperature_status != NULL)
	{
		(*temperature_status) = Temperature_Data.Cached_Temperature_Status;
#ifdef CCD_DEBUG
		CCD_General_Log_Format("ccd","ccd_temperature.c","CCD_Temperature_Cached_Temperature_Get",
				      LOG_VERBOSITY_VERBOSE,NULL,
				   "CCD_Temperature_Cached_Temperature_Get() found cached temperature status %d(%s).",
				      Temperature_Data.Cached_Temperature_Status,
				      CCD_Temperature_Status_To_String(Temperature_Data.Cached_Temperature_Status));
#endif
	}
	if(cache_date_stamp != NULL)
	{
		(*cache_date_stamp) = Temperature_Data.Cache_Date_Stamp;

#ifdef CCD_DEBUG
		CCD_General_Get_Time_String(Temperature_Data.Cache_Date_Stamp,time_buff,31);
		CCD_General_Log_Format("ccd","ccd_temperature.c","CCD_Temperature_Cached_Temperature_Get",
				      LOG_VERBOSITY_VERBOSE,NULL,
				      "CCD_Temperature_Cached_Temperature_Get() found cache date stamp %d(%s).",
				      Temperature_Data.Cache_Date_Stamp.tv_sec,time_buff);
#endif
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Cached_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
		       "CCD_Temperature_Cached_Temperature_Get() returned TRUE.");
#endif
	return TRUE;
}

/**
 * Routine to get the last temperature target sent to the temperature controller.
 * @param target_temperature The address of a double to store the last target temperature.
 * @return The routine returns TRUE if successful, and FALSE if it fails.
 * @see #Temperature_Data
 */
int CCD_Temperature_Target_Temperature_Get(double *target_temperature)
{
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Target_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			"CCD_Temperature_Target_Temperature_Get() started.");
#endif
	if(target_temperature != NULL)
		(*target_temperature) = Temperature_Data.Target_Temperature;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_temperature.c","CCD_Temperature_Target_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			"CCD_Temperature_Target_Temperature_Get() returned TRUE.");
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
** Revision 1.2  2009/01/30 18:00:24  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.1  2006/06/01 15:27:37  cjm
** Initial revision
**
*/
