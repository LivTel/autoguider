/* autoguider_dark.c
** Autoguider dark routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_dark.c,v 1.5 2014-01-31 17:17:17 cjm Exp $
*/
/**
 * Dark routines for the autoguider program.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fitsio.h"

#include "log_udp.h"

#include "ccd_config.h"
#include "ccd_general.h"
#include "ccd_setup.h"

#include "autoguider_dark.h"
#include "autoguider_field.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"

/* data types */
/**
 * Data type holding local data to autoguider_dark. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in dark images.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in dark images.</dd>
 * <dt>Bin_X</dt> <dd>X binning in dark images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in dark images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in dark images.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in dark images.</dd>
 * <dt>Exposure_Length</dt> <dd>The exposure length in milliseconds.</dd>
 * <dt>Reduced_Data</dt> <dd>Pointer to float data containing the dark.</dd>
 * <dt>Reduced_Mutex</dt> <dd>A mutex to lock access to the reduced data field.</dd>
 * <dt>Exposure_Length_List</dt> <dd>An allocated list of exposure lengths read from the config file,
 *                               should match the available dark list.</dd>
 * <dt>Exposure_Length_Count</dt> <dd>The number of exposure lengths in the list.</dd>
 * </dl>
 */
struct Dark_Struct
{
	int Unbinned_NCols;
	int Unbinned_NRows;
	int Bin_X;
	int Bin_Y;
	int Binned_NCols;
	int Binned_NRows;
	int Exposure_Length;
	float *Reduced_Data;
	pthread_mutex_t Reduced_Mutex;
	int *Exposure_Length_List;
	int Exposure_Length_Count;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_dark.c,v 1.5 2014-01-31 17:17:17 cjm Exp $";
/**
 * Instance of dark data.
 * @see #Dark_Struct
 */
static struct Dark_Struct Dark_Data = 
{
	0,0,-1,-1,0,0,
	0,NULL,PTHREAD_MUTEX_INITIALIZER,
	NULL,0
};

/* internal functions */
static int Dark_Load_Reduced(char *filename,int bin_x,int bin_y,int exposure_length);
static int Dark_Exposure_Length_List_Initialise(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise the dark buffers. Reads the field dimensions from the config file, and calls 
 * Autoguider_Dark_Set_Dimension to setup the dark buffers.
 * @see #Dark_Exposure_Length_List_Initialise
 * @see #Autoguider_Dark_Set_Dimension
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Dark_Initialise(void)
{
	int retval,ncols,nrows,x_bin,y_bin;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Initialise",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","started.");
#endif
	/* get default config */
	/* field */
	retval = CCD_Config_Get_Integer("ccd.field.ncols",&ncols);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 800;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Initialise:Getting field NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.nrows",&nrows);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 801;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Initialise:Getting field NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.x_bin",&x_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 802;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Initialise:"
			"Getting field X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.y_bin",&y_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 803;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Initialise:"
			"Getting field Y Binning failed.");
		return FALSE;
	}
	/* setup field buffer */
	retval = Autoguider_Dark_Set_Dimension(ncols,nrows,x_bin,y_bin);
	if(retval == FALSE)
		return FALSE;
	/* setup exposure length list */
	retval = Dark_Exposure_Length_List_Initialise();
	if(retval == FALSE)
		return FALSE;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Initialise",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","finished.");
#endif
	return TRUE;
}

/**
 * Set the dimensions, and (re) allocate the dark buffer accordingly.
 * Locks/unlocks the associated mutex.
 * @param ncols Number of unbinned columns.
 * @param nrows Number of unbinned rows.
 * @param x_bin X (column) binning.
 * @param y_bin Y (row) binning.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Set_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Set_Dimension",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","started.");
#endif
	Dark_Data.Unbinned_NCols = ncols;
	Dark_Data.Unbinned_NRows = nrows;
	Dark_Data.Bin_X = x_bin;
	Dark_Data.Bin_Y = x_bin;
	Dark_Data.Binned_NCols = ncols/x_bin;
	Dark_Data.Binned_NRows = nrows/y_bin;
	/* reduced buffer */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Dark_Data.Reduced_Data == NULL)
	{
		Dark_Data.Reduced_Data = (float *)malloc(Dark_Data.Binned_NCols*
						Dark_Data.Binned_NRows * sizeof(float));
	}
	else
	{
		Dark_Data.Reduced_Data = (float *)realloc(
                           Dark_Data.Reduced_Data,
			   (Dark_Data.Binned_NCols * Dark_Data.Binned_NRows * sizeof(float)));
	}
	if(Dark_Data.Reduced_Data == NULL)
	{
		/* unlock mutex */
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 804;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Set_Dimension:"
			"Reduced buffer %d failed to allocate/reallocate (%d,%d).",i,
			Dark_Data.Binned_NCols,Dark_Data.Binned_NRows);
		return FALSE;
	}
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Set_Dimension",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","finished.");
#endif
	return TRUE;
}

/**
 * Set which dark to load/use for dark subtraction.
 * Calls Dark_Load_Reduced to load a new dark, <b>only</b> if the exposure length/binning has changed.
 * @param bin_x The X binning factor.
 * @param bin_y The Y binning factor.
 * @param exposure_length The exposure length in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see #Dark_Load_Reduced
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Set(int bin_x,int bin_y,int exposure_length)
{
	char keyword_string[64];
	char *filename_string = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Set",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","started.");
#endif
	/* check parameters */
	if(bin_x < 1)
	{
		Autoguider_General_Error_Number = 805;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Set:"
			"X Binning out of range(%d).",bin_x);
		return FALSE;
	}
	if(bin_y < 1)
	{
		Autoguider_General_Error_Number = 806;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Set:"
			"Y Binning out of range(%d).",bin_y);
		return FALSE;
	}
	if(exposure_length < 0)
	{
		Autoguider_General_Error_Number = 807;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Set:"
			"Exposure length out of range(%d).",exposure_length);
		return FALSE;
	}
	if((bin_x == Dark_Data.Bin_X)&&(bin_y == Dark_Data.Bin_Y)&&(exposure_length == Dark_Data.Exposure_Length))
	{
		/* we already have the right dark loaded */
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Set",LOG_VERBOSITY_INTERMEDIATE,
				       "DARK","Correct dark already loaded:exiting.");
#endif
		return TRUE;
	}
	/* get the new dark filename */
	sprintf(keyword_string,"dark.filename.%d.%d.%d",bin_x,bin_y,exposure_length);
	retval = CCD_Config_Get_String(keyword_string,&filename_string);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 808;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Set:"
			"No dark configured for %d,%d,%d (%s).",bin_x,bin_y,exposure_length,keyword_string);
		return FALSE;
	}
	/* spawn a thread to load the new dark? */
	/* load the correct dark, if it already exists */
	retval = Dark_Load_Reduced(filename_string,bin_x,bin_y,exposure_length);
	if(retval == FALSE)
	{
		if(filename_string != NULL)
			free(filename_string);
		return FALSE;
	}
	if(filename_string != NULL)
		free(filename_string);
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Set",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","finished.");
#endif
	return TRUE;
}

/**
 * Routine to subtract the currently loaded reduced dark data from the passed in buffer.
 * @param buffer_ptr Some data requiring the currently loaded dark to be subtacted off it.
 *        If the buffer has an associated mutex, this should have been locked <b>before</b> calling this routine,
 *        this routine does <b>not</b> lock/unclock mutexs.
 * @param pixel_count The number of pixels in the buffer.
 * @param ncols The number of columns in the <b>full frame</b>.
 * @param nrows The number of columns in the <b>full frame</b>.
 * @param use_window Whether the buffer is a full frame or a window.
 * @param window If the buffer is a window, the dimensions of that window.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Subtract(float *buffer_ptr,int pixel_count,int ncols,int nrows,int use_window,
			     struct CCD_Setup_Window_Struct window)
{
	float *current_buffer_ptr = NULL;
	float *current_dark_ptr = NULL;
	int retval;
	int dark_start_x,dark_start_y,dark_end_x,dark_end_y,buffer_ncols,buffer_nrows,buffer_x,buffer_y;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Subtract",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","started.");
#endif
	/* check parameters */
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 818;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:buffer_ptr was NULL.");
		return FALSE;
	}
	if(use_window)
	{
		/* remember windows are inclusive - add 1 */
		if((((window.X_End - window.X_Start)+1)*((window.Y_End - window.Y_Start)+1)) != pixel_count)
		{
			Autoguider_General_Error_Number = 819;
			sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
			"Windowed buffer dimension mismatch: ncols (%d - %d) * nrows (%d - %d) != pixel_count %d.",
				window.X_End,window.X_Start,window.Y_End,window.Y_Start,pixel_count);
			return FALSE;
		}
		if(window.X_End >= Dark_Data.Binned_NCols)
		{
			Autoguider_General_Error_Number = 823;
			sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
			"Windowed buffer dimension mismatch: Window X end (%d) >= Dark binned ncols (%d).",
				window.X_End,Dark_Data.Binned_NCols);
			return FALSE;
		}
		if(window.Y_End >= Dark_Data.Binned_NRows)
		{
			Autoguider_General_Error_Number = 824;
			sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
			"Windowed buffer dimension mismatch: Window Y end (%d) >= Dark binned nrows (%d).",
				window.Y_End,Dark_Data.Binned_NRows);
			return FALSE;
		}
	}
	else
	{
		if((nrows*ncols) != pixel_count)
		{
			Autoguider_General_Error_Number = 820;
			sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
				"Unwindowed buffer dimension mismatch: ncols %d * nrows %d != pixel_count %d.",
				ncols,nrows,pixel_count);
			return FALSE;
		}
	}
	/* dark binned dimensions and full frame buffer dimensions should match */
	if(ncols != Dark_Data.Binned_NCols)
	{
		Autoguider_General_Error_Number = 821;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
			"buffer dimension mismatch: ncols %d != dark ncols %d.",
			ncols,Dark_Data.Binned_NCols);
		return FALSE;
	}
	if(nrows != Dark_Data.Binned_NRows)
	{
		Autoguider_General_Error_Number = 822;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Subtract:"
			"buffer dimension mismatch: nrows %d != dark nrows %d.",
			nrows,Dark_Data.Binned_NRows);
		return FALSE;
	}
	/* work out which part of the dark frame to use. */
	if(use_window)
	{
		/* windows are inclusive of the end row/column */
		/* the window dimensions start at (1,1) (Andor and PCO) but the dark image data indexes and the buffer indexes start at (0,0) (C) */
		dark_start_x = window.X_Start;
		dark_start_y = window.Y_Start;
		dark_end_x = window.X_End+1;
		dark_end_y = window.Y_End+1;
		buffer_ncols = (window.X_End-window.X_Start)+1;
		buffer_nrows = (window.Y_End-window.Y_Start)+1;
	}
	else
	{
		dark_start_x = 0;
		dark_start_y = 0;
		dark_end_x = ncols;
		dark_end_y = nrows;
		buffer_ncols = ncols;
		buffer_nrows = nrows;
	}
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Subtract",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","Subtracting from buffer y 0..%d.",buffer_nrows);
#endif
	/* subtract relevant bit of dark from buffer */
	for(buffer_y=0;buffer_y<buffer_nrows;buffer_y++)
	{
		current_buffer_ptr = buffer_ptr+(buffer_y*buffer_ncols);
		current_dark_ptr = Dark_Data.Reduced_Data+(((dark_start_y+buffer_y)*Dark_Data.Binned_NCols)+
							   dark_start_x);
#if AUTOGUIDER_DEBUG > 9
		if(buffer_y==0)
		{
			Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Subtract",
						      LOG_VERBOSITY_VERY_VERBOSE,"DARK",
					       "current_buffer_ptr %p = buffer_ptr %p+buffer_y %d * buffer_ncols %d.",
					       current_buffer_ptr,buffer_ptr,buffer_y,buffer_ncols);
			Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Subtract",
						      LOG_VERBOSITY_VERY_VERBOSE,"DARK",
					       "current_dark_ptr %p = Dark_Data.Reduced_Data %p+ (((dark_start_y %d + "
					       "buffer_y %d)*Dark_Data.Binned_NCols %d)+dark_start_x %d.",
					       current_dark_ptr,Dark_Data.Reduced_Data,dark_start_y,buffer_y,
					       Dark_Data.Binned_NCols,dark_start_x);
		}
#endif
		for(buffer_x=0;buffer_x<buffer_ncols;buffer_x++)
		{
#if AUTOGUIDER_DEBUG > 9
			if((buffer_y==0)&&(buffer_x < 10))
			{
				Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Subtract",
							      LOG_VERBOSITY_VERY_VERBOSE,"DARK",
				       "buffer_y %d, buffer_x %d : current_buffer_ptr %f -= current_dark_ptr %f.",
						       buffer_y,buffer_x,(*current_buffer_ptr),(*current_dark_ptr));
			}
#endif
			(*current_buffer_ptr) -= (*current_dark_ptr);
			/* no range checking is performed here. See Fault 1716 for details. */
			current_buffer_ptr++;
			current_dark_ptr++;
		}
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Subtract",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","finished.");
#endif
	return TRUE;
}

/**
 * Free the allocated buffers.
 * Locks/unlocks the associated mutex.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Shutdown(void)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Shutdown",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","started.");
#endif
	/* raw */

	/* reduced */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Dark_Data.Reduced_Data != NULL)
		free(Dark_Data.Reduced_Data);
	Dark_Data.Reduced_Data = NULL;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* exposure length list */
	if(Dark_Data.Exposure_Length_List != NULL)
		free(Dark_Data.Exposure_Length_List);
	Dark_Data.Exposure_Length_List = NULL;
	Dark_Data.Exposure_Length_Count = 0;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Shutdown",LOG_VERBOSITY_INTERMEDIATE,
			       "DARK","finished.");
#endif
	return TRUE;
}

/**
 * Change the passed in exposure length to the length nearest the parameter passed in.
 * @param exposure_length The address of an integer. The current exposure length is passed in,
 *       on successful return this is modified to be an exposure length equivalent to the nearest dark.
 * @param exposure_length_index The address of an integer. The index in the dark exposure_length_list
 *        of the returned nearest exposure length is returned. If NULL is passed in for this argument,
 *        the index is not returned.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Get_Exposure_Length_Nearest(int *exposure_length,int *exposure_length_index)
{
	int i,nearest_exposure_length,difference,nearest_difference,nearest_index;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Nearest",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","started.");
#endif
	if(exposure_length == NULL)
	{
		Autoguider_General_Error_Number = 825;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Get_Exposure_Length_Nearest:"
			"exposure length was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Nearest",
				      LOG_VERBOSITY_INTERMEDIATE,"DARK",
				      "Finding nearest exposure length to %d.",(*exposure_length));
#endif
	nearest_difference = 999999999;
	nearest_index = -1;
	for(i=0;i<Dark_Data.Exposure_Length_Count;i++)
	{
		difference = abs(Dark_Data.Exposure_Length_List[i]-(*exposure_length));
		if(difference < nearest_difference)
		{
			nearest_exposure_length = Dark_Data.Exposure_Length_List[i];
			nearest_difference = difference;
			nearest_index = i;
		}
	}/* end for */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Nearest",
				      LOG_VERBOSITY_INTERMEDIATE,"DARK",
				      "Nearest exposure length to %d was %d (index %d).",
				      (*exposure_length),nearest_exposure_length,nearest_index);
#endif
	(*exposure_length) = nearest_exposure_length;
	if(exposure_length_index != NULL)
		(*exposure_length_index) = nearest_index;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Nearest",
				      LOG_VERBOSITY_INTERMEDIATE,"DARK","finished.");
#endif
	return TRUE;
}

/**
 * Get the exposure length for the specified index.
 * @param index The index in the Exposure_Length_List of the exposure length to return.
 * @param exposure_length The address of an integer to store the exposure length (in milliseconds) 
 *        at the specified index. 
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Dark_Get_Exposure_Length_Index(int index,int *exposure_length)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Index",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","started.");
#endif
	if(exposure_length == NULL)
	{
		Autoguider_General_Error_Number = 826;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Get_Exposure_Length_Index:"
			"exposure_length was NULL.");
		return FALSE;
	}
	if((index < 0)||(index >= Dark_Data.Exposure_Length_Count))
	{
		Autoguider_General_Error_Number = 827;
		sprintf(Autoguider_General_Error_String,"Autoguider_Dark_Get_Exposure_Length_Index:"
			"index %d out of range 0..%d.",index,Dark_Data.Exposure_Length_Count);
		return FALSE;
	}
	(*exposure_length) = Dark_Data.Exposure_Length_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("dark","autoguider_dark.c","Autoguider_Dark_Get_Exposure_Length_Index",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","finished.");
#endif
	return TRUE;
}

/**
 * Get the number of exposure length's in the internal exposure length list.
 * @return The number of exposure length's in the list is returned.
 * @see #Dark_Data
 */
int Autoguider_Dark_Get_Exposure_Length_Count(void)
{
	return Dark_Data.Exposure_Length_Count;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Load a dark from the filename. It is expected to be for the specified binning.
 * It is loaded into the Reduced_Data field, assuming the Reduced_Mutex is locked correctly.
 * The NAXIS1 / NAXIS2 keywords in the FITS image must agree with Dark_Data.Binned_NCols and Dark_Data.Binned_NRows
 * @param filename The filename of a FITS image containing the dark to load.
 * @param bin_x The expected X binning of the FITS image.
 * @param bin_y The expected Y binning of the FITS image.
 * @param exposure_length The expected exposure length of the image, in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Dark_Load_Reduced(char *filename,int bin_x,int bin_y,int exposure_length)
{
	fitsfile *fits_fp = NULL;
	char cfitsio_error_buff[32]; /* fits_get_errstatus returns 30 chars max */
	int retval,naxis,naxis1,naxis2,cfitsio_status=0;

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("dark","autoguider_dark.c","Dark_Load_Reduced",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","started.");
#endif
	/* We should check binning matches expected dark binning? */
	/* Or we could call Autoguider_Dark_Set_Dimension? */
	/* lock the reduced data mutex */
	retval = Autoguider_General_Mutex_Lock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* initialise cfitsio status variable */
	cfitsio_status=0;
	/* open dark FITS file */
	retval = fits_open_file(&fits_fp,filename,READONLY,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 809;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
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
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 810;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"fits_read_key(NAXIS) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	if(naxis != 2)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 811;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
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
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 812;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"fits_read_key(NAXIS1) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	retval = fits_read_key(fits_fp,TINT,"NAXIS2",&naxis2,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 813;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"fits_read_key(NAXIS2) failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	/* check naxis1 == Dark_Data.Binned_NCols */
	if(naxis1 != Dark_Data.Binned_NCols)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 814;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"naxis1 %d does not match expected binned ncols %d.",naxis1,Dark_Data.Binned_NCols);
		return FALSE;
	}
	/* check naxis2 == Dark_Data.Binned_NRows */
	if(naxis2 != Dark_Data.Binned_NRows)
	{
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 815;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"naxis2 %d does not match expected binned nrows %d.",naxis2,Dark_Data.Binned_NRows);
		return FALSE;
	}
	/* read FITS image as FLOATS into the Dark_Data.Reduced_Data */
	retval = fits_read_img(fits_fp,TFLOAT,1,naxis1*naxis2,NULL,Dark_Data.Reduced_Data,NULL,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		fits_close_file(fits_fp,&cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 816;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"fits_read_img failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	retval = fits_close_file(fits_fp,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
		Autoguider_General_Error_Number = 817;
		sprintf(Autoguider_General_Error_String,"Dark_Load_Reduced:"
			"fits_close_file failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		return FALSE;
	}
	/* update current reduced dark meta-data */
	Dark_Data.Bin_X = bin_x;
	Dark_Data.Bin_Y = bin_y;
	Dark_Data.Exposure_Length = exposure_length;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Dark_Data.Reduced_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("dark","autoguider_dark.c","Dark_Load_Reduced",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","finished.");
#endif
	return TRUE;
}

/**
 * Load the list of exposure lengths into Dark_Data.
 * The list is sorted into ascending order.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Dark_Data
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 * @see autoguider.general.html#Autoguider_General_Int_List_Add
 * @see autoguider.general.html#Autoguider_General_Int_List_Sort
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
static int Dark_Exposure_Length_List_Initialise(void)
{
	char keyword_string[32];
	int index,done,exposure_length,retval;

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("dark","autoguider_dark.c","Dark_Exposure_Length_List_Initialise",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","started.");
#endif
	/* free currently allocated list? */
	if(Dark_Data.Exposure_Length_List != NULL)
		free(Dark_Data.Exposure_Length_List);
	Dark_Data.Exposure_Length_List = NULL;
	Dark_Data.Exposure_Length_Count = 0;
	/* read config */
	index = 0;
	done = FALSE;
	while(done == FALSE)
	{
		sprintf(keyword_string,"dark.exposure_length.%d",index);
		retval = CCD_Config_Get_Integer(keyword_string,&exposure_length);
		if(retval == TRUE)
		{
			/* add to list */
			if(!Autoguider_General_Int_List_Add(exposure_length,&(Dark_Data.Exposure_Length_List),
							    &(Dark_Data.Exposure_Length_Count)))
				return FALSE;
			/* try next index in the list */
			index++;
		}
		else
			done = TRUE;
	}/* end while */
	/* sort into ascending order */
	qsort(Dark_Data.Exposure_Length_List,Dark_Data.Exposure_Length_Count,sizeof(int),
	      Autoguider_General_Int_List_Sort);
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("dark","autoguider_dark.c","Dark_Exposure_Length_List_Initialise",
			       LOG_VERBOSITY_INTERMEDIATE,"DARK","finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2010/08/13 08:49:33  cjm
** Removed range checking on dark subtraction with ruined image stats causing spurious objects.
**
** Revision 1.3  2009/01/30 18:01:33  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.2  2006/08/29 14:35:38  cjm
** Changed limits detection in Autoguider_Dark_Subtract, so you can have
** X-End/Y-End at last pixel inclusive, and it doesn't trigger an error.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
