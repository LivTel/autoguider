/* andor_temperature.c
** Autoguder Andor CCD Library temperature routines
** $Header: /home/cjm/cvs/autoguider/ccd/andor/c/andor_temperature.c,v 1.5 2009-01-30 15:41:14 cjm Exp $
*/
/**
 * Temperature routines for the Andor autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.5 $
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
/* andor CCD library */
#include "atmcdLXd.h"
#include "log_udp.h"
#include "ccd_general.h"
#include "ccd_temperature.h"
#include "andor_general.h"
#include "andor_temperature.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: andor_temperature.c,v 1.5 2009-01-30 15:41:14 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Get the current temperature of the CCD.
 * Uses the Andor library GetTemperatureF.
 * @param temperature The address of a double to return ther temperature in, in degrees centigrade.
 * @param temperature_status The address of a enum to store the temperature status.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ../cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
int Andor_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status)
{
	unsigned int andor_retval;
	float temperature_f;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	if(temperature == NULL)
	{
		CCD_General_Error_Number = 1200;
		sprintf(CCD_General_Error_String,"Andor_Temperature_Get: temperature was NULL.");
		return FALSE;
	}
	if(temperature_status == NULL)
	{
		CCD_General_Error_Number = 1201;
		sprintf(CCD_General_Error_String,"Andor_Temperature_Get: temperature_status was NULL.");
		return FALSE;
	}
	andor_retval = GetTemperatureF(&temperature_f);
	(*temperature) = (double)temperature_f;
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_temperature.c","Andor_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			       "GetTemperatureF returned (%.2f,%d).",(*temperature),andor_retval);
#endif
	switch(andor_retval)
	{
		case DRV_NOT_INITIALIZED:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_UNKNOWN;
			break;
		case DRV_ACQUIRING:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_UNKNOWN;
			CCD_General_Error_Number = 1207;
			sprintf(CCD_General_Error_String,"Andor_Temperature_Get: "
				"GetTemperatureF returned DRV_ACQUIRING %s(%u,%.2f).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval,(*temperature));
			return FALSE;
		case DRV_ERROR_ACK:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_UNKNOWN;
			CCD_General_Error_Number = 1202;
			sprintf(CCD_General_Error_String,"Andor_Temperature_Get: GetTemperatureF failed %s(%u).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
			return FALSE;
		case DRV_TEMP_OFF:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_OFF;
			break;
		case DRV_TEMP_STABILIZED:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_OK;
			break;
		case DRV_TEMP_NOT_REACHED:
			(*temperature_status) = CCD_TEMPERATURE_STATUS_RAMPING;
			break;
		default:
			CCD_General_Error_Number = 1203;
			sprintf(CCD_General_Error_String,"Andor_Temperature_Get: "
				"GetTemperatureF returned odd error code %s(%u).",
				Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
			return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_temperature.c","Andor_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			       "returned (%.2f,%d).",(*temperature),(*temperature_status));
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Set the current temperature of the CCD. The cooler also needs to be switched on for this to take effect.
 * Uses the Andor library SetTemperature.
 * @param temperature The temperature to ramp the CCD to, in degrees centigrade.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int Andor_Temperature_Set(double target_temperature)
{
	unsigned int andor_retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	andor_retval = SetTemperature(target_temperature);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1204;
		sprintf(CCD_General_Error_String,"Andor_Temperature_Set: SetTemperature(%lf) failed %s(%d).",
			target_temperature,Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler on. Uses the Andor library CoolerON.
 * This returns immediately, but slowly ramps the temperaure to the temperature set value.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int Andor_Temperature_Cooler_On(void)
{
	unsigned int andor_retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Cooler_On",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	andor_retval = CoolerON();
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1205;
		sprintf(CCD_General_Error_String,"Andor_Temperature_Cooler_On: CoolerON() failed %s(%d).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Cooler_On",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler off. Uses the Andor library CoolerOFF.
 * This returns immediately, but slowly ramps the temperaure to 0C.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int Andor_Temperature_Cooler_Off(void)
{
	unsigned int andor_retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Cooler_Off",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	andor_retval = CoolerOFF();
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1206;
		sprintf(CCD_General_Error_String,"Andor_Temperature_Cooler_Off: CoolerOFF() failed %s(%d).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_temperature.c","Andor_Temperature_Cooler_Off",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2007/01/30 16:28:24  cjm
** Added error return in Andor_Temperature_Get when GetTemperatureF returns DRV_ACQUIRING, because the
** temperature returned in that state is -999.
**
** Revision 1.3  2007/01/29 14:43:07  cjm
** Added more Andor_Temperature_Get logging.
**
** Revision 1.2  2006/06/01 15:20:20  cjm
** Changed temperature status enum to CCD_TEMPERATURE_STATUS.
**
** Revision 1.1  2006/03/27 14:02:36  cjm
** Initial revision
**
*/
