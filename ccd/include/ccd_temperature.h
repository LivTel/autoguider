/* ccd_temperature.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_temperature.h,v 1.1 2006-06-01 15:27:58 cjm Exp $
*/
#ifndef CCD_TEMPERATURE_H
#define CCD_TEMPERATURE_H
/**
 * Type describing what the current status of the temperature subsystem is.
 * <ul>
 * <li><b>CCD_TEMPERATURE_STATUS_OFF</b>
 * <li><b>CCD_TEMPERATURE_STATUS_AMBIENT</b>
 * <li><b>CCD_TEMPERATURE_STATUS_OK</b>
 * <li><b>CCD_TEMPERATURE_STATUS_RAMPING</b>
 * <li><b>CCD_TEMPERATURE_STATUS_UNKNOWN</b> The camera is in the middle of an acquisition and can't measure the
 *                                        current temperature.
 * </ul>
 * @see #Andor_Temperature_Get
 */
enum CCD_TEMPERATURE_STATUS
{
	CCD_TEMPERATURE_STATUS_OFF, CCD_TEMPERATURE_STATUS_AMBIENT, CCD_TEMPERATURE_STATUS_OK,
	CCD_TEMPERATURE_STATUS_RAMPING, CCD_TEMPERATURE_STATUS_UNKNOWN
};


extern int CCD_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status);
extern int CCD_Temperature_Set(double target_temperature);
extern int CCD_Temperature_Cooler_On(void);
extern int CCD_Temperature_Cooler_Off(void);
extern char *CCD_Temperature_Status_To_String(enum CCD_TEMPERATURE_STATUS temperature_status);

/*
** $Log: not supported by cvs2svn $
*/
#endif
