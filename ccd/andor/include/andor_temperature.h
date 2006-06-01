/* andor_temperature.h
** $Header: /home/cjm/cvs/autoguider/ccd/andor/include/andor_temperature.h,v 1.2 2006-06-01 15:25:36 cjm Exp $
*/
#ifndef ANDOR_TEMPERATURE_H
#define ANDOR_TEMPERATURE_H

extern int Andor_Temperature_Get(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status);
extern int Andor_Temperature_Set(double target_temperature);
extern int Andor_Temperature_Cooler_On(void);
extern int Andor_Temperature_Cooler_Off(void);
/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/03/27 14:03:01  cjm
** Initial revision
**
*/
#endif
