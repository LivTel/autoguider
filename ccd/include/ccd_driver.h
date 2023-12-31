/* ccd_driver.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_driver.h,v 1.2 2006-09-07 15:36:52 cjm Exp $
*/
#ifndef CCD_DRIVER_H
#define CCD_DRIVER_H

/* needed for struct timespec in function struct */
#include <time.h>
/* needed for CCD_Setup_Window_Struct in function struct */
#include "ccd_setup.h"
/* needed for enum CCD_TEMPERATURE_STATUS in function struct */
#include "ccd_temperature.h"

/* data types */
/**
 * Structure containing function pointers for driver routines.
 * <ul>
 * <li>Setup_Startup
 * <li>Setup_Dimensions
 * <li>Setup_Abort
 * <li>Setup_Get_NCols
 * <li>Setup_Get_NRows
 * <li>Setup_Shutdown
 * <li>Exposure_Expose
 * <li>Exposure_Bias
 * <li>Exposure_Abort
 * <li>Exposure_Get_Exposure_Start_Time
 * <li>Exposure_Loop_Pause_Length_Set
 * <li>Temperature_Get
 * <li>Temperature_Set
 * <li>Temperature_Cooler_On
 * <li>Temperature_Cooler_Off
 * </ul>
 */
struct CCD_Driver_Function_Struct
{
	int (*Setup_Startup)(void);
	int (*Setup_Dimensions)(int ncols,int nrows,int nsbin,int npbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
	void (*Setup_Abort)(void);
	int (*Setup_Get_NCols)(void);
	int (*Setup_Get_NRows)(void);
	int (*Setup_Shutdown)(void);
	int (*Exposure_Expose)(int open_shutter,struct timespec start_time,int exposure_time,
			       void *buffer,size_t buffer_length);
	int (*Exposure_Bias)(void *buffer,size_t buffer_length);
	int (*Exposure_Abort)(void);
	struct timespec (*Exposure_Get_Exposure_Start_Time)(void);
	int (*Exposure_Loop_Pause_Length_Set)(int ms);
	int (*Temperature_Get)(double *temperature,enum CCD_TEMPERATURE_STATUS *temperature_status);
	int (*Temperature_Set)(double target_temperature);
	int (*Temperature_Cooler_On)(void);
	int (*Temperature_Cooler_Off)(void);
};


extern int CCD_Driver_Register(char *shared_library_name,char *registration_function);
extern int CCD_Driver_Get_Functions(struct CCD_Driver_Function_Struct *functions);
extern int CCD_Driver_Close(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/04/28 14:28:14  cjm
** Initial revision
**
*/

#endif
