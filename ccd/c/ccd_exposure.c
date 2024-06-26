/* ccd_exposure.c
** Autoguider CCD Library exposure routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_exposure.c,v 1.6 2014-01-31 17:23:56 cjm Exp $
*/
/**
 * Exposure routines for the autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.6 $
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

#include "fitsio.h"
#include "log_udp.h"
#include "ccd_config.h"
#include "ccd_driver.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_exposure.c,v 1.6 2014-01-31 17:23:56 cjm Exp $";

/* internal function declarations */
static int fexist(char *filename);
static void Exposure_Flip_X(int ncols,int nrows,unsigned short *exposure_data);
static void Exposure_Flip_Y(int ncols,int nrows,unsigned short *exposure_data);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Does nothing. Delete?
 */
void CCD_Exposure_Initialise(void)
{
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Initialise",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			"started.");
#endif
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Initialise",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			"finished.");
#endif
}

/**
 * Do an exposure.
 * The returned exposed data is flipped, if the setup code has been configured to do this.
 * @param open_shutter A boolean, TRUE to open the shutter, FALSE to leave it closed (dark).
 * @param start_time The time to start the exposure. If both the fields in the <i>struct timespec</i> are zero,
 * 	the exposure can be started at any convenient time.
 * @param exposure_length The length of time to open the shutter for in milliseconds. This must be greater than zero.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in <b>pixels</b>.
 * @return Returns TRUE if the exposure succeeds and the data read out into the buffer, returns FALSE if an error
 *	occurs or the exposure is aborted.
 * @see #Exposure_Flip_X
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 * @see ccd_setup.html#CCD_Setup_Get_Flip_X
 * @see ccd_setup.html#CCD_Setup_Get_Flip_Y
 * @see ccd_setup.html#CCD_Setup_Get_NCols
 * @see ccd_setup.html#CCD_Setup_Get_NRows
 */
int CCD_Exposure_Expose(int open_shutter,struct timespec start_time,int exposure_time,
			void *buffer,size_t buffer_length)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Expose",LOG_VERBOSITY_VERY_TERSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Exposure_Expose == NULL)
	{
		CCD_General_Error_Number = 400;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Expose:Exposure_Expose function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Exposure_Expose))(open_shutter,start_time,exposure_time,buffer,buffer_length);
	if(retval == FALSE)
		return FALSE;
	/* do we need to flip the output data */
	if(CCD_Setup_Get_Flip_X())
	{
		Exposure_Flip_X(CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows(),buffer);
	}
	if(CCD_Setup_Get_Flip_Y())
	{
		Exposure_Flip_Y(CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows(),buffer);
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Expose",LOG_VERBOSITY_VERY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Do a bias exposure.
 * The returned image data is flipped, if the setup code has been configured to do this.
 * @param buffer A pointer to a previously allocated area of memory, of length buffer_length. This should have the
 *        correct size to save the read out image into.
 * @param buffer_length The length of the buffer in bytes.
 * @return Returns TRUE if the exposure succeeds and the data read out into the buffer, returns FALSE if an error
 *	occurs or the exposure is aborted.
 * @see #Exposure_Flip_X
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 * @see ccd_setup.html#CCD_Setup_Get_Flip_X
 * @see ccd_setup.html#CCD_Setup_Get_Flip_Y
 * @see ccd_setup.html#CCD_Setup_Get_NCols
 * @see ccd_setup.html#CCD_Setup_Get_NRows
 */
int CCD_Exposure_Bias(void *buffer,size_t buffer_length)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Bias",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Exposure_Bias == NULL)
	{
		CCD_General_Error_Number = 401;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Bias:Exposure_Bias function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Exposure_Bias))(buffer,buffer_length);
	if(retval == FALSE)
		return FALSE;
	/* do we need to flip the output data */
	if(CCD_Setup_Get_Flip_X())
	{
		Exposure_Flip_X(CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows(),buffer);
	}
	if(CCD_Setup_Get_Flip_Y())
	{
		Exposure_Flip_Y(CCD_Setup_Get_NCols(),CCD_Setup_Get_NRows(),buffer);
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Bias",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Abort an exposure.
 * @return Returns TRUE if the routine succeeds and FALSE if an error occurs.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Exposure_Abort(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Abort",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Exposure_Abort == NULL)
	{
		CCD_General_Error_Number = 402;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Abort:Exposure_Abort function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Exposure_Abort))();
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Abort",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * This routine gets the time stamp for the start of the exposure.
 * @return The time stamp for the start of the exposure.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Exposure_Get_Exposure_Start_Time(struct timespec *timespec)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Get_Exposure_Start_Time",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"started.");
#endif
	/* check parameters */
	if(timespec == NULL)
	{
		CCD_General_Error_Number = 408;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Get_Exposure_Start_Time:timespec was NULL.");
		return FALSE;
	}
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Exposure_Get_Exposure_Start_Time == NULL)
	{
		CCD_General_Error_Number = 409;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Get_Exposure_Start_Time:"
			"Exposure_Get_Exposure_Start_Time function was NULL.");
		return FALSE;
	}
	/* call driver function */
	(*timespec) = (*(functions.Exposure_Get_Exposure_Start_Time))();
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Get_Exposure_Start_Time",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Set how long to pause in the loop waiting for an exposure to complete in Andor_Exposure_Expose.
 * @param ms The length of time to sleep for, in milliseconds (between 1 and 999).
 * @return Returns TRUE if the routine succeeds and FALSE if an error occurs.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Exposure_Loop_Pause_Length_Set(int ms)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Loop_Pause_Length_Set",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Exposure_Loop_Pause_Length_Set == NULL)
	{
		CCD_General_Error_Number = 410;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Loop_Pause_Length_Set:"
			"Exposure_Loop_Pause_Length_Set function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Exposure_Loop_Pause_Length_Set))(ms);
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Loop_Pause_Length_Set",LOG_VERBOSITY_VERY_VERBOSE,
			NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Save the exposure to disk.
 * @param filename The name of the file to save the image into. If it does not exist, it is created.
 * @param buffer Pointer to a previously allocated array of unsigned shorts containing the image pixel values.
 * @param buffer_length The length of the buffer in bytes.
 * @param ncols The number of binned image columns (the X size/width of the image).
 * @param nrows The number of binned image rows (the Y size/height of the image).
 * @return Returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 * @see ccd_general.html#CCD_General_Log
 * @see #fexist
 */
int CCD_Exposure_Save(char *filename,void *buffer,size_t buffer_length,int ncols,int nrows)
{
	static fitsfile *fits_fp = NULL;
	char buff[32]; /* fits_get_errstatus returns 30 chars max */
	long axes[2];
	int status = 0,retval,ivalue;
	double dvalue;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Save",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* check existence of FITS image and create or append as appropriate? */
	if(fexist(filename))
	{
		retval = fits_open_file(&fits_fp,filename,READWRITE,&status);
		if(retval)
		{
			fits_get_errstatus(status,buff);
			fits_report_error(stderr,status);
			CCD_General_Error_Number = 403;
			sprintf(CCD_General_Error_String,"CCD_Exposure_Save: File open failed(%s,%d,%s).",
				filename,status,buff);
			return FALSE;
		}
	}
	else
	{
		/* open file */
		if(fits_create_file(&fits_fp,filename,&status))
		{
			fits_get_errstatus(status,buff);
			fits_report_error(stderr,status);
			CCD_General_Error_Number = 404;
			sprintf(CCD_General_Error_String,"CCD_Exposure_Save: File create failed(%s,%d,%s).",
				filename,status,buff);
			return FALSE;
		}
		/* create image block */
		axes[0] = nrows;
		axes[1] = ncols;
		retval = fits_create_img(fits_fp,USHORT_IMG,2,axes,&status);
		if(retval)
		{
			fits_get_errstatus(status,buff);
			fits_report_error(stderr,status);
			fits_close_file(fits_fp,&status);
			CCD_General_Error_Number = 405;
			sprintf(CCD_General_Error_String,"CCD_Exposure_Save: Create image failed(%s,%d,%s).",
				filename,status,buff);
			return FALSE;
		}
	}
	/* write the data */
	retval = fits_write_img(fits_fp,TUSHORT,1,ncols*nrows,buffer,&status);
	if(retval)
	{
		fits_get_errstatus(status,buff);
		fits_report_error(stderr,status);
		fits_close_file(fits_fp,&status);
		CCD_General_Error_Number = 406;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Save: File write image failed(%s,%d,%s).",
			filename,status,buff);
		return FALSE;
	}
	/* ensure data we have written is in the actual data buffer, not CFITSIO's internal buffers */
	/* closing the file ensures this. */ 
	retval = fits_close_file(fits_fp,&status);
	if(retval)
	{
		fits_get_errstatus(status,buff);
		fits_report_error(stderr,status);
		fits_close_file(fits_fp,&status);
		CCD_General_Error_Number = 407;
		sprintf(CCD_General_Error_String,"CCD_Exposure_Save: File close file failed(%s,%d,%s).",
			filename,status,buff);
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","CCD_Exposure_Save",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */

/**
 * Return whether the specified filename exists or not.
 * @param filename A string representing the filename to test.
 * @return The routine returns TRUE if the filename exists, and FALSE if it does not exist. 
 */
static int fexist(char *filename)
{
	FILE *fptr = NULL;

	fptr = fopen(filename,"r");
	if(fptr == NULL )
		return FALSE;
	fclose(fptr);
	return TRUE;
}

/**
 * Flip the image data in the X direction.
 * @param ncols The number of columns in the image data.
 * @param nrows The number of rows in the image data.
 * @param exposure_data The image data received from the CCD, as an unsigned short. 
 *        The data in this array is flipped in the X direction.
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
static void Exposure_Flip_X(int ncols,int nrows,unsigned short *exposure_data)
{
	int x,y;
	unsigned short int tempval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_exposure.c","Exposure_Flip_X",LOG_VERBOSITY_INTERMEDIATE,"exposure",
				  "Started flipping image of size (%d,%d) in X.",ncols,nrows);
#endif
	/* for each row */
	for(y=0;y<nrows;y++)
	{
		/* for the first half of the columns.
		** Note the middle column will be missed, this is OK as it
		** does not need to be flipped if it is in the middle */
		for(x=0;x<(ncols/2);x++)
		{
			/* Copy exposure_data[x,y] to tempval */
			tempval = *(exposure_data+(y*ncols)+x);
			/* Copy exposure_data[ncols-(x+1),y] to exposure_data[x,y] */
			*(exposure_data+(y*ncols)+x) = *(exposure_data+(y*ncols)+(ncols-(x+1)));
			/* Copy tempval = exposure_data[ncols-(x+1),y] */
			*(exposure_data+(y*ncols)+(ncols-(x+1))) = tempval;
		}
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","Exposure_Flip_X",LOG_VERBOSITY_INTERMEDIATE,"exposure","Finished.");
#endif
}

/**
 * Flip the image data in the Y direction.
 * @param ncols The number of columns in the image data.
 * @param nrows The number of rows in the image data.
 * @param exposure_data The image data received from the CCD, as an unsigned short. 
 *        The data in this array is flipped in the Y direction.
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
static void Exposure_Flip_Y(int ncols,int nrows,unsigned short *exposure_data)
{
	int x,y;
	unsigned short int tempval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_exposure.c","Exposure_Flip_Y",LOG_VERBOSITY_INTERMEDIATE,"exposure",
				  "Started flipping image of size (%d,%d) in Y.",ncols,nrows);
#endif
	/* for the first half of the rows.
	** Note the middle row will be missed, this is OK as it
	** does not need to be flipped if it is in the middle */
	for(y=0;y<(nrows/2);y++)
	{
		/* for each column */
		for(x=0;x<ncols;x++)
		{
			/* Copy exposure_data[x,y] to tempval */
			tempval = *(exposure_data+(y*ncols)+x);
			/* Copy exposure_data[x,nrows-(y+1)] to exposure_data[x,y] */
			*(exposure_data+(y*ncols)+x) = *(exposure_data+(((nrows-(y+1))*ncols)+x));
			/* Copy tempval = exposure_data[x,nrows-(y+1)] */
			*(exposure_data+(((nrows-(y+1))*ncols)+x)) = tempval;
		}
	}
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_exposure.c","Exposure_Flip_Y",LOG_VERBOSITY_INTERMEDIATE,"exposure","Finished.");
#endif
}


/*
** $Log: not supported by cvs2svn $
** Revision 1.5  2009/01/30 18:00:24  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.4  2006/09/07 15:36:26  cjm
** Added CCD_Exposure_Loop_Pause_Length_Set.
**
** Revision 1.3  2006/04/28 14:26:40  cjm
** Added CCD_Exposure_Get_Exposure_Start_Time.
**
** Revision 1.2  2006/04/10 15:53:25  cjm
** Comment fix.
**
** Revision 1.1  2006/04/10 15:52:49  cjm
** Initial revision
**
*/
