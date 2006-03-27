/* andor_temperature.h
** $Header: /home/cjm/cvs/autoguider/ccd/andor/include/andor_temperature.h,v 1.1 2006-03-27 14:03:01 cjm Exp $
*/
#ifndef ANDOR_TEMPERATURE_H
#define ANDOR_TEMPERATURE_H
/**
 * Type describing what the Andor temperature is currently up to
 * <ul>
 * <li><b>ANDOR_TEMPERATURE_STATUS_OFF</b>
 * <li><b>ANDOR_TEMPERATURE_STATUS_AMBIENT</b>
 * <li><b>ANDOR_TEMPERATURE_STATUS_OK</b>
 * <li><b>ANDOR_TEMPERATURE_STATUS_RAMPING</b>
 * <li><b>ANDOR_TEMPERATURE_STATUS_UNKNOWN</b> The camera is in the middle of an acquisition and can't measure the
 *                                        current temperature.
 * </ul>
 * @see #Andor_Temperature_Get
 */
enum ANDOR_TEMPERATURE_STATUS
{
	ANDOR_TEMPERATURE_STATUS_OFF, ANDOR_TEMPERATURE_STATUS_AMBIENT, ANDOR_TEMPERATURE_STATUS_OK,
	ANDOR_TEMPERATURE_STATUS_RAMPING, ANDOR_TEMPERATURE_STATUS_UNKNOWN
};

/**
 * Typedef of the temperature status enumeration.
 * @see #ANDOR_TEMPERATURE_STATUS
 */
typedef enum ANDOR_TEMPERATURE_STATUS ANDOR_TEMPERATURE_STATUS_T;

extern int Andor_Temperature_Get(double *temperature,enum ANDOR_TEMPERATURE_STATUS *temperature_status);
extern int Andor_Temperature_Set(double target_temperature);
extern int Andor_Temperature_Cooler_On(void);
extern int Andor_Temperature_Cooler_Off(void);
extern char *Andor_Temperature_Status_To_String(enum ANDOR_TEMPERATURE_STATUS temperature_status);
/*
** $Log: not supported by cvs2svn $
*/
#endif
