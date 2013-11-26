/* fli_temperature.c
** Autoguder FLI CCD Library temperature routines
** $Header: /home/cjm/cvs/autoguider/ccd/fli/c/fli_temperature.c,v 1.1 2013-11-26 16:28:36 cjm Exp $
*/
/**
 * Temperature routines for the FLI autoguider CCD library.
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
#include <math.h>
#include <stdio.h>
#include <string.h>
/* fli CCD library */
#include "libfli.h"
#include "log_udp.h"
#include "ccd_general.h"
#include "ccd_temperature.h"
#include "fli_general.h"
#include "fli_temperature.h"

/* structs */
/**
 * Data type holding local data to fli_temperature. This consists of the following:
 * <dl>
 * <dt>Target_Temperature</dt> <dd>The temperature we last asked the FLI library to attain at the CCD cold finger.
 *     Set by FLI_Temperature_Set, and used by FLI_Temperature_Get as a simple way to create a meaningful
 *     CCD_TEMPERATURE_STATUS (given the FLI library has no way to do this directly).</dd>
 * </dl>
 * @see ../cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
struct Temperature_Struct
{
	double Target_Temperature;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: fli_temperature.c,v 1.1 2013-11-26 16:28:36 cjm Exp $";

/**
 * Instance of the temperature data.
 * @see #Temperature_Struct
 */
static struct Temperature_Struct Temperature_Data = 
{
	0.0
};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Get the current temperature of the CCD.
 * Uses the FLI library FLIGetTemperature.
 * @param temperature The address of a double to return ther temperature in, in degrees centigrade.
 * @param temperature_status The address of a enum to store the temperature status.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see #Temperature_Data
 * @see ../cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
int FLI_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status)
{
	long fli_retval;
	double power;

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	if(temperature == NULL)
	{
		CCD_General_Error_Number = 1300;
		sprintf(CCD_General_Error_String,"FLI_Temperature_Get: temperature was NULL.");
		return FALSE;
	}
	if(temperature_status == NULL)
	{
		CCD_General_Error_Number = 1301;
		sprintf(CCD_General_Error_String,"FLI_Temperature_Get: temperature_status was NULL.");
		return FALSE;
	}
	fli_retval = FLIGetTemperature(FLI_Setup_Get_Dev(),temperature);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1302;
		sprintf(CCD_General_Error_String,"FLI_Temperature_Get: FLIGetTemperature failed %s(%ld).",
			strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_temperature.c","FLI_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			       "FLIGetTemperature returned %.2lf C.",(*temperature));
#endif
	/* simple test to set the temperature status to something
	** assumes we are always cooling at some level. */
	if(fabs((*temperature)-Temperature_Data.Target_Temperature) > 1.0)
		(*temperature_status) = CCD_TEMPERATURE_STATUS_RAMPING;
	else
		(*temperature_status) = CCD_TEMPERATURE_STATUS_OK;
	/* retrieve cooler power */
	fli_retval = FLIGetCoolerPower(FLI_Setup_Get_Dev(),&power);
	if(fli_retval == 0)
	{
#ifdef FLI_DEBUG
		CCD_General_Log_Format("ccd","fli_temperature.c","FLI_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
				       "FLIGetCoolerPower returned %.2lf %%.",power);
#endif
	}
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Set the current temperature of the CCD. 
 * @param temperature The temperature to ramp the CCD to, in degrees centigrade.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see #Temperature_Data
 */
int FLI_Temperature_Set(double target_temperature)
{
	long fli_retval;

#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_temperature.c","FLI_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,
			       "started with target_temperature = %.2lf.",target_temperature);
#endif
	Temperature_Data.Target_Temperature = target_temperature;
	fli_retval = FLISetTemperature(FLI_Setup_Get_Dev(),target_temperature);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1303;
		sprintf(CCD_General_Error_String,"FLI_Temperature_Set: FLISetTemperature(%lf) failed %s(%d).",
			target_temperature,strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler on. This does nothing for FLI cameras, as setting the temperature automatically does this.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int FLI_Temperature_Cooler_On(void)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Cooler_On",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Cooler_On",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler off. This does nothing for FLI cameras, as setting the temperature automatically does this.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int FLI_Temperature_Cooler_Off(void)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Cooler_Off",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_temperature.c","FLI_Temperature_Cooler_Off",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
