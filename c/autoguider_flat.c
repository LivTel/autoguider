/* autoguider_flat.c
** Autoguider flat routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_flat.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * Flat routines for the autoguider program.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fitsio.h"

#include "ccd_config.h"
#include "ccd_general.h"
#include "ccd_setup.h"

#include "autoguider_field.h"
#include "autoguider_flat.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"

/* data types */
/**
 * Data type holding local data to autoguider_flat. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in flat images.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in flat images.</dd>
 * <dt>Bin_X</dt> <dd>X binning in flat images in memory allocated for Reduced_Inverted_Data, 
 *            but not necessarrily loaded.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in flat images in memory allocated for Reduced_Inverted_Data, 
 *            but not necessarrily loaded.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in flat images.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in flat images.</dd>
 * <dt>Reduced_Inverted_Bin_X</dt> <dd>X binning in flat data actually loaded into Reduced_Inverted_Data.</dd>
 * <dt>Reduced_Inverted_Bin_Y</dt> <dd>Y binning in flat data actually loaded into Reduced_Inverted_Data.</dd>
 * <dt>Reduced_Inverted_Data</dt> <dd>Pointer to float data containing the reduced inverted flat.</dd>
 * <dt>Reduced_Mutex</dt> <dd>A mutex to lock access to the reduced data field.</dd>
 * </dl>
 */
struct Flat_Struct
{
	int Unbinned_NCols;
	int Unbinned_NRows;
	int Bin_X;
	int Bin_Y;
	int Binned_NCols;
	int Binned_NRows;
	int Reduced_Inverted_Bin_X;
	int Reduced_Inverted_Bin_Y;
	float *Reduced_Inverted_Data;
	pthread_mutex_t Reduced_Mutex;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_flat.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";
/**
 * Instance of flat data.
 * @see #Flat_Struct
 */
static struct Flat_Struct Flat_Data = 
{
	0,0,-1,-1,0,0,
	-1,-1,NULL,PTHREAD_MUTEX_INITIALIZER
};

/* internal functions */
static int Flat_Load_Reduced(char *filename,int bin_x,int bin_y);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise the flat buffers. Reads the field dimensions from the config file, and calls 
 * Autoguider_Flat_Set_Dimension to setup the flat buffers. 
 * Autoguider_Flat_Set is then called to load the default flat.
 * @see #Autoguider_Flat_Set_Dimension
 * @see #Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Flat_Initialise(void)
{
	int retval,ncols,nrows,x_bin,y_bin;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Initialise:started.");
#endif
	/* get default config */
	/* field */
	retval = CCD_Config_Get_Integer("ccd.field.ncols",&ncols);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 900;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Initialise:Getting field NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.nrows",&nrows);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 901;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Initialise:Getting field NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.x_bin",&x_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 902;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Initialise:"
			"Getting field X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.y_bin",&y_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 903;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Initialise:"
			"Getting field Y Binning failed.");
		return FALSE;
	}
	/* setup field buffer */
	retval = Autoguider_Flat_Set_Dimension(ncols,nrows,x_bin,y_bin);
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Set the dimensions, and (re) allocate the flat buffer accordingly.
 * Locks/unlocks the associated mutex.
 * @param ncols Number of unbinned columns.
 * @param nrows Number of unbinned rows.
 * @param x_bin X (column) binning.
 * @param y_bin Y (row) binning.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Flat_Data
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Flat_Set_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Set_Dimension:started.");
#endif
	Flat_Data.Unbinned_NCols = ncols;
	Flat_Data.Unbinned_NRows = nrows;
	Flat_Data.Bin_X = x_bin;
	Flat_Data.Bin_Y = x_bin;
	Flat_Data.Binned_NCols = ncols/x_bin;
	Flat_Data.Binned_NRows = nrows/y_bin;
	/* reduced buffer */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Flat_Data.Reduced_Inverted_Data == NULL)
	{
		Flat_Data.Reduced_Inverted_Data = (float *)malloc(Flat_Data.Binned_NCols*
						Flat_Data.Binned_NRows * sizeof(float));
	}
	else
	{
		Flat_Data.Reduced_Inverted_Data = (float *)realloc(
                           Flat_Data.Reduced_Inverted_Data,
			   (Flat_Data.Binned_NCols * Flat_Data.Binned_NRows * sizeof(float)));
	}
	if(Flat_Data.Reduced_Inverted_Data == NULL)
	{
		/* unlock mutex */
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 904;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Set_Dimension:"
			"Reduced buffer %d failed to allocate/reallocate (%d,%d).",i,
			Flat_Data.Binned_NCols,Flat_Data.Binned_NRows);
		return FALSE;
	}
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Set_Dimension:finished.");
#endif
	return TRUE;
}

/**
 * Set which flat to load/use for flat fielding.
 * Calls Flat_Load_Reduced to load a new flat, <b>only</b> if the binning has changed.
 * @param bin_x The X binning factor.
 * @param bin_y The Y binning factor.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Flat_Data
 * @see #Flat_Load_Reduced
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Flat_Set(int bin_x,int bin_y)
{
	char keyword_string[64];
	char *filename_string = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Set:started.");
#endif
	/* check parameters */
	if(bin_x < 1)
	{
		Autoguider_General_Error_Number = 905;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Set:"
			"X Binning out of range(%d).",bin_x);
		return FALSE;
	}
	if(bin_y < 1)
	{
		Autoguider_General_Error_Number = 906;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Set:"
			"Y Binning out of range(%d).",bin_y);
		return FALSE;
	}
	/* check binning matches allocated binning dimensions */
	if((bin_x != Flat_Data.Bin_X)||(bin_y != Flat_Data.Bin_Y))
	{
		Autoguider_General_Error_Number = 924;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Set:"
			"Requested binning does not match allocated: requested (%d,%d) vs allocated (%d,%d).",
			bin_x,bin_y,Flat_Data.Bin_X,Flat_Data.Bin_Y);
		return FALSE;
	}

	if((bin_x == Flat_Data.Reduced_Inverted_Bin_X)&&(bin_y == Flat_Data.Reduced_Inverted_Bin_Y))
	{
		/* we already have the right flat loaded */
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Set:"
				       "Correct flat already loaded:exiting.");
#endif
		return TRUE;
	}
	/* get the new flat filename */
	sprintf(keyword_string,"flat.filename.%d.%d",bin_x,bin_y);
	retval = CCD_Config_Get_String(keyword_string,&filename_string);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 907;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Set:"
			"No flat configured for %d,%d (%s).",bin_x,bin_y,keyword_string);
		return FALSE;
	}
	/* spawn a thread to load the new flat? */
	/* load the correct flat, if it already exists */
	retval = Flat_Load_Reduced(filename_string,bin_x,bin_y);
	if(retval == FALSE)
	{
		if(filename_string != NULL)
			free(filename_string);
		return FALSE;
	}
	if(filename_string != NULL)
		free(filename_string);
	/* updated loaded binning data numbers */
	Flat_Data.Reduced_Inverted_Bin_X = bin_x;
	Flat_Data.Reduced_Inverted_Bin_Y = bin_y;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Set:finished.");
#endif
	return TRUE;
}

/**
 * Routine to subtract the currently loaded reduced flat data from the passed in buffer.
 * @param buffer_ptr Some data requiring the currently loaded flat to be subtacted off it.
 *        If the buffer has an associated mutex, this should have been locked <b>before</b> calling this routine,
 *        this routine does <b>not</b> lock/unclock mutexs.
 * @param pixel_count The number of pixels in the buffer.
 * @param ncols The number of columns in the <b>full frame</b>.
 * @param nrows The number of columns in the <b>full frame</b>.
 * @param use_window Whether the buffer is a full frame or a window.
 * @param window If the buffer is a window, the dimensions of that window.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Flat_Field(float *buffer_ptr,int pixel_count,int ncols,int nrows,int use_window,
			     struct CCD_Setup_Window_Struct window)
{
	float *current_buffer_ptr = NULL;
	float *current_flat_ptr = NULL;
	int retval,flat_zero_count;
	int flat_start_x,flat_start_y,flat_end_x,flat_end_y,buffer_ncols,buffer_nrows,buffer_x,buffer_y;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Field:started.");
#endif
	/* check parameters */
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 908;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:buffer_ptr was NULL.");
		return FALSE;
	}
	if(use_window)
	{
		/* remember windows are inclusive - add 1 */
		if((((window.X_End - window.X_Start)+1)*((window.Y_End - window.Y_Start)+1)) != pixel_count)
		{
			Autoguider_General_Error_Number = 909;
			sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
			"Windowed buffer dimension mismatch: ncols (%d - %d) * nrows (%d - %d) != pixel_count %d.",
				window.X_End,window.X_Start,window.Y_End,window.Y_Start,pixel_count);
			return FALSE;
		}
		if((window.X_End+1) >= Flat_Data.Binned_NCols)
		{
			Autoguider_General_Error_Number = 910;
			sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
			"Windowed buffer dimension mismatch: Window X end (%d+1) >= Flat binned ncols (%d).",
				window.X_End,Flat_Data.Binned_NCols);
			return FALSE;
		}
		if((window.Y_End+1) >= Flat_Data.Binned_NRows)
		{
			Autoguider_General_Error_Number = 911;
			sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
			"Windowed buffer dimension mismatch: Window Y end (%d+1) >= Flat binned nrows (%d).",
				window.Y_End,Flat_Data.Binned_NRows);
			return FALSE;
		}
	}
	else
	{
		if((nrows*ncols) != pixel_count)
		{
			Autoguider_General_Error_Number = 912;
			sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
				"Unwindowed buffer dimension mismatch: ncols %d * nrows %d != pixel_count %d.",
				ncols,nrows,pixel_count);
			return FALSE;
		}
	}
	/* flat binned dimensions and full frame buffer dimensions should match */
	if(ncols != Flat_Data.Binned_NCols)
	{
		Autoguider_General_Error_Number = 913;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
			"buffer dimension mismatch: ncols %d != flat ncols %d.",
			ncols,Flat_Data.Binned_NCols);
		return FALSE;
	}
	if(nrows != Flat_Data.Binned_NRows)
	{
		Autoguider_General_Error_Number = 914;
		sprintf(Autoguider_General_Error_String,"Autoguider_Flat_Field:"
			"buffer dimension mismatch: nrows %d != flat nrows %d.",
			nrows,Flat_Data.Binned_NRows);
		return FALSE;
	}
	/* work out which part of the flat to use. */
	if(use_window)
	{
		/* windows are inclusive of the end row/column */
		flat_start_x = window.X_Start;
		flat_start_y = window.Y_Start;
		flat_end_x = window.X_End+1;
		flat_end_y = window.Y_End+1;
		buffer_ncols = (window.X_End-window.X_Start)+1;
		buffer_nrows = (window.Y_End-window.Y_Start)+1;
	}
	else
	{
		flat_start_x = 0;
		flat_start_y = 0;
		flat_end_x = ncols;
		flat_end_y = nrows;
		buffer_ncols = ncols;
		buffer_nrows = nrows;
	}
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Field:"
			       "Subtracting from buffer y 0..%d.",buffer_nrows);
#endif
	flat_zero_count = 0;
	/* subtract relevant bit of flat from buffer */
	for(buffer_y=0;buffer_y<buffer_nrows;buffer_y++)
	{
		current_buffer_ptr = buffer_ptr+(buffer_y*buffer_ncols);
		current_flat_ptr = Flat_Data.Reduced_Inverted_Data+(((flat_start_y+buffer_y)*Flat_Data.Binned_NCols)+
							   flat_start_x);
#if AUTOGUIDER_DEBUG > 9
		if(buffer_y==0)
		{
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Field:"
					       "current_buffer_ptr %p = buffer_ptr %p+buffer_y %d * buffer_ncols %d.",
					       current_buffer_ptr,buffer_ptr,buffer_y,buffer_ncols);
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Field:"
					       "current_flat_ptr %p = Flat_Data.Reduced_Inverted_Data %p+ (((flat_start_y %d + "
					       "buffer_y %d)*Flat_Data.Binned_NCols %d)+flat_start_x %d.",
					       current_flat_ptr,Flat_Data.Reduced_Inverted_Data,flat_start_y,buffer_y,
					       Flat_Data.Binned_NCols,flat_start_x);
		}
#endif
		for(buffer_x=0;buffer_x<buffer_ncols;buffer_x++)
		{
#if AUTOGUIDER_DEBUG > 9
			if((buffer_y==0)&&(buffer_x < 10))
			{
				Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,
				       "Autoguider_Flat_Field:"
				       "buffer_y %d, buffer_x %d : current_buffer_ptr %f *= current_flat_ptr %f.",
						       buffer_y,buffer_x,(*current_buffer_ptr),(*current_flat_ptr));
			}
#endif
			if((*current_flat_ptr) != 0.0f)
			{
				/* flat should have been inverted at load time - so multiple through by it */
				(*current_buffer_ptr) *= (*current_flat_ptr);
				/* range check ? */
				if((*current_buffer_ptr) < 0.0f)
					(*current_buffer_ptr) = 0.0f;
			}
			else
			{
				flat_zero_count++;
#if AUTOGUIDER_DEBUG > 1
				if(flat_zero_count < 10)
				{
					Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,
						    "Autoguider_Flat_Field:flat has zero value at (%d,%d).",
								      buffer_x,buffer_y);
				}
#endif
			}
			/* diddly also check for range overrun floatmax? */
			current_buffer_ptr++;
			current_flat_ptr++;
		}
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,
				      "Autoguider_Flat_Field:flat has %d zero values.",flat_zero_count);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Field:finished.");
#endif
	return TRUE;
}

/**
 * Free the allocated buffers.
 * Locks/unlocks the associated mutex.
 * @see #Flat_Data
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Flat_Shutdown(void)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Shutdown:started.");
#endif
	/* raw */

	/* reduced */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Flat_Data.Reduced_Inverted_Data != NULL)
		free(Flat_Data.Reduced_Inverted_Data);
	Flat_Data.Reduced_Inverted_Data = NULL;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Autoguider_Flat_Shutdown:finished.");
#endif
	return TRUE;
}


/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Load a flat from the filename. It is expected to be for the specified binning.
 * It is loaded into the Reduced_Inverted_Data field, assuming the Reduced_Mutex is locked correctly.
 * The NAXIS1 / NAXIS2 keywords in the FITS image must agree with Flat_Data.Binned_NCols and Flat_Data.Binned_NRows.
 * Once the flat is loaded, it is inverted: each pixel = 1/pixel value. This allows the actual flat routine to
 * multiply through by the flat rather than divide through, this is quicker.
 * @param filename The filename of a FITS image containing the flat to load.
 * @param bin_x The expected X binning of the FITS image.
 * @param bin_y The expected Y binning of the FITS image.
 * @see #Flat_Data
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_FLAT
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Flat_Load_Reduced(char *filename,int bin_x,int bin_y)
{
	fitsfile *fits_fp = NULL;
	char cfitsio_error_buff[32]; /* fits_get_errstatus returns 30 chars max */
	int retval,naxis,naxis1,naxis2,pixel_count,cfitsio_status=0,i;

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Flat_Load_Reduced:started.");
#endif
	/* diddly check binning matches expected flat binning? */
	/* diddly or call Autoguider_Flat_Set_Dimension? */

	/* lock the reduced data mutex */
	retval = Autoguider_General_Mutex_Lock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* initialise cfitsio status variable */
	cfitsio_status=0;
	/* open flat FITS file */
	retval = fits_open_file(&fits_fp,filename,READONLY,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 915;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_open_file failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	/* read and check NAXIS */
	retval = fits_read_key(fits_fp,TINT,"NAXIS",&naxis,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 916;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_read_key(NAXIS) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	if(naxis != 2)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 917;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"Illegal value of NAXIS(%d).",naxis);
		return FALSE;
	}
	/* read NAXIS1/NAXIS2 */
	retval = fits_read_key(fits_fp,TINT,"NAXIS1",&naxis1,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 918;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_read_key(NAXIS1) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	retval = fits_read_key(fits_fp,TINT,"NAXIS2",&naxis2,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 919;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_read_key(NAXIS2) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	/* check naxis1 == Flat_Data.Binned_NCols */
	if(naxis1 != Flat_Data.Binned_NCols)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 920;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"naxis1 %d does not match expected binned ncols %d.",naxis1,Flat_Data.Binned_NCols);
		return FALSE;
	}
	/* check naxis2 == Flat_Data.Binned_NRows */
	if(naxis2 != Flat_Data.Binned_NRows)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 921;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"naxis2 %d does not match expected binned nrows %d.",naxis2,Flat_Data.Binned_NRows);
		return FALSE;
	}
	pixel_count = naxis1*naxis2;
	/* read FITS image as FLOATS into the Flat_Data.Reduced_Inverted_Data 
        ** The data in the FITS image is NOT inverted, see below. */
	retval = fits_read_img(fits_fp,TFLOAT,1,pixel_count,NULL,Flat_Data.Reduced_Inverted_Data,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 922;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_read_img failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	retval = fits_close_file(fits_fp,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 923;
		sprintf(Autoguider_General_Error_String,"Flat_Load_Reduced:"
			"fits_close_file failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	/* invert the flat, so we have to multiply through it rather than divide by it */
	for(i=0;i<pixel_count;i++)
	{
		/* could check whether value is close to 0.0f */ 
		if(Flat_Data.Reduced_Inverted_Data[i] != 0.0f)
		{
#if AUTOGUIDER_DEBUG > 9
			if(i < 10)
			{
				Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Flat_Load_Reduced:"
							      "Inverting pixel %d: %f = 1/%f.",i,
							      Flat_Data.Reduced_Inverted_Data[i],
							      1.0f/Flat_Data.Reduced_Inverted_Data[i]);
			}
#endif
			Flat_Data.Reduced_Inverted_Data[i] = 1.0f/Flat_Data.Reduced_Inverted_Data[i];
		}
		else
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Flat_Load_Reduced:"
						      "Pixel %d has value %f:Setting inverted pixel value to 1.0f.",i,
						      Flat_Data.Reduced_Inverted_Data[i]);
#endif
			Flat_Data.Reduced_Inverted_Data[i] = 1.0f;
		}
	}
	/* update current reduced flat meta-data */
	Flat_Data.Bin_X = bin_x;
	Flat_Data.Bin_Y = bin_y;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Flat_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FLAT,"Flat_Load_Reduced:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
