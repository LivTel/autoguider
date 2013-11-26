/* fli_temperature.h
** $Header: /home/cjm/cvs/autoguider/ccd/fli/include/fli_temperature.h,v 1.1 2013-11-26 16:28:41 cjm Exp $
*/
#ifndef FLI_TEMPERATURE_H
#define FLI_TEMPERATURE_H

extern int FLI_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status);
extern int FLI_Temperature_Set(double target_temperature);
extern int FLI_Temperature_Cooler_On(void);
extern int FLI_Temperature_Cooler_Off(void);
/*
** $Log: not supported by cvs2svn $
*/
#endif
