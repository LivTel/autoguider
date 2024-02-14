/* pco_temeprature.cpp
** Autoguider PCO CMOS library temperature control/monitor routines
*/
/**
 * Temperature control and monitor routines for the PCO camera driver.
 * @author Chris Mottram
 * @version $Id$
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_general.h"
#include "ccd_temperature.h"
#include "pco_command.h"
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
 * Get the current temperature of the detector. We call PCO_Command_Get_Temperature to get the sensor temperature,
 * and compare it to the set-point temperature returned by PCO_Command_Get_Cooling_Setpoint_Temperature, to
 * set the temperature_status appropriately.
 * @param temperature The address of a double to return the temperature in, in degrees centigrade.
 * @param temperature_status The address of a enum to store the temperature status.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see pco_command.html#PCO_Command_Get_Temperature
 * @see pco_command.html#PCO_Command_Get_Cooling_Setpoint_Temperature
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 * @see ../../cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
int PCO_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status)
{
	int retval,valid_sensor_temp,camera_temp,valid_psu_temp,psu_temp,setpoint_temperature;
	double sensor_temp,setpoint_temperature_d;
	
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_temperature.c","PCO_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,"Started.");
#endif
	if(temperature == NULL)
	{
		CCD_General_Error_Number = 1500;
		sprintf(CCD_General_Error_String,"PCO_Temperature_Get: temperature was NULL.");
		return FALSE;
	}
	if(temperature_status == NULL)
	{
		CCD_General_Error_Number = 1501;
		sprintf(CCD_General_Error_String,"PCO_Temperature_Get: temperature_status was NULL.");
		return FALSE;
	}
	/* get the camera temperatures and return the sensor temperature */
	if(!PCO_Command_Get_Temperature(&valid_sensor_temp,&sensor_temp,&camera_temp,&valid_psu_temp,&psu_temp))
		return FALSE;
	if(valid_sensor_temp == FALSE)
	{
		(*temperature) = 0.0;
		(*temperature_status) = CCD_TEMPERATURE_STATUS_UNKNOWN;
		CCD_General_Error_Number = 1502;
		sprintf(CCD_General_Error_String,"PCO_Temperature_Get: Returned sensor temperature was not valid.");
		return FALSE;
	}
	(*temperature) = sensor_temp;
	/* get the cooling set-point temperature */
	if(!PCO_Command_Get_Cooling_Setpoint_Temperature(&setpoint_temperature))
		return FALSE;
	setpoint_temperature_d = (double)setpoint_temperature;
	/* if the sensor temperature is close (1 C) to the set-point, we are OK, otherwise RAMPING */
	if(fabs(sensor_temp-setpoint_temperature_d) <= 1.0)
		(*temperature_status) = CCD_TEMPERATURE_STATUS_OK;
	else
		(*temperature_status) = CCD_TEMPERATURE_STATUS_RAMPING;
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_temperature.c","PCO_Temperature_Get",LOG_VERBOSITY_VERBOSE,NULL,
			       "Finished with returned temperature %.2f C.",(*temperature));
#endif
	return TRUE;
}

/**
 * Set the target temperature for the detector to attain.
 * @param temperature The temperature to ramp the CCD to, in degrees centigrade.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see pco_command.html#PCO_Command_Set_Cooling_Setpoint_Temperature
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_General_Log_Format
 */
int PCO_Temperature_Set(double target_temperature)
{
#ifdef PCO_DEBUG
	CCD_General_Log_Format("ccd","pco_temperature.c","PCO_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,
			       "Started with target temperature %.2f C.",target_temperature);
#endif
	if(!PCO_Command_Set_Cooling_Setpoint_Temperature(target_temperature))
		return FALSE;
#ifdef PCO_DEBUG
	CCD_General_Log("ccd","pco_temperature.c","PCO_Temperature_Set",LOG_VERBOSITY_VERBOSE,NULL,
			"Finished.");
#endif
	return TRUE;
}

/**
 * Turn the cooler on. This is unimplemented for PCO cameras.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int PCO_Temperature_Cooler_On(void)
{
	/* do nothing? */
	return TRUE;
}

/**
 * Turn the cooler on. This is unimplemented for PCO cameras.
 * @return Returns TRUE on success, and FALSE if an error occurs.
 */
int PCO_Temperature_Cooler_Off(void)
{
	/* do nothing? */
	return TRUE;
}
