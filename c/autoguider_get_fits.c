/* autoguider_get_fits.c
** Autoguider get fits routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_get_fits.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * Routines to return an in-memory FITS image for the field or guide buffer.
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "fitsio.h"

#include "autoguider_buffer.h"
#include "autoguider_field.h"
#include "autoguider_general.h"
#include "autoguider_get_fits.h"
#include "autoguider_guide.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_get_fits.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Create an in memory FITS image from the specified buffer.
 * @param buffer_type Which buffer to get the latest image from (field or guide).
 * @param buffer_state Whether to get the raw or reduced data.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD
 * @see #AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED
 * @see #Autoguider_Get_Fits_From_Buffer
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Copy
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Binned_NCols
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Binned_NRows
 * @see autoguider_field.html#Autoguider_Field_Get_Last_Buffer_Index
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Copy
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Binned_NCols
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Binned_NRows
 * @see autoguider_guide.html#Autoguider_Guide_Get_Last_Buffer_Index
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Get_Fits(int buffer_type,int buffer_state,void **buffer_ptr,size_t *buffer_length)
{
	fitsfile *fits_fp = NULL;
	void *buffer_data_ptr = NULL;
	size_t buffer_data_length = 0;
	size_t pixel_length;
	int buffer_index,retval,ncols,nrows;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits:started.");
#endif
	/* check parameters */
	if((buffer_type != AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD) &&
	   (buffer_type != AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE))
	{
		Autoguider_General_Error_Number = 600;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:Illegal buffer type %d.",buffer_type);
		return FALSE;
	}
	if((buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW) &&
	   (buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED))
	{
		Autoguider_General_Error_Number = 616;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:Illegal buffer state %d.",buffer_state);
		return FALSE;
	}
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 601;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:buffer_ptr was NULL.");
		return FALSE;
	}
	if(buffer_length == NULL)
	{
		Autoguider_General_Error_Number = 602;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:buffer_length was NULL.");
		return FALSE;
	}
	/* pixel_length based on whether raw or reduced: raw is unsigned short, reduced is float */
	if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW)
		pixel_length = sizeof(unsigned short);
	else if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED)
		pixel_length = sizeof(float);
	else /* this can never happen - see test above */
		pixel_length = 0;
	if(buffer_type == AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD)
	{
		/* get the last field buffer index */
		buffer_index = Autoguider_Field_Get_Last_Buffer_Index();
		if(buffer_index < 0)
		{
			Autoguider_General_Error_Number = 603;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:buffer_index less than 0.");
			return FALSE;
		}
		/* copy data into data buffer */
		buffer_data_length = Autoguider_Buffer_Get_Field_Pixel_Count();
		buffer_data_ptr = (void *)malloc(buffer_data_length*pixel_length);
		if(buffer_data_ptr == NULL)
		{
			Autoguider_General_Error_Number = 604;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:"
				"Allocating buffer_data_ptr failed (%ld,%ld).",buffer_data_length,pixel_length);
			return FALSE;
		}
		if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW)
		{
			retval = Autoguider_Buffer_Raw_Field_Copy(buffer_index,buffer_data_ptr,buffer_data_length);
			if(retval == FALSE)
			{
				/* free allocated data */
				if(buffer_data_ptr != NULL)
					free(buffer_data_ptr);
				return FALSE;
			}
		}
		else if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED)
		{
			retval = Autoguider_Buffer_Reduced_Field_Copy(buffer_index,buffer_data_ptr,buffer_data_length);
			if(retval == FALSE)
			{
				/* free allocated data */
				if(buffer_data_ptr != NULL)
					free(buffer_data_ptr);
				return FALSE;
			}
		}
		/* get dimensions */
		ncols = Autoguider_Buffer_Get_Field_Binned_NCols();
		nrows = Autoguider_Buffer_Get_Field_Binned_NRows();
		/* create fits in memory */
		retval = Autoguider_Get_Fits_From_Buffer(buffer_ptr,buffer_length,buffer_state,
							 buffer_data_ptr,buffer_data_length,ncols,nrows);
		if(retval == FALSE)
		{
			/* free allocated data */
			if(buffer_data_ptr != NULL)
				free(buffer_data_ptr);
			return FALSE;
		}
		/* free allocated data */
		if(buffer_data_ptr != NULL)
			free(buffer_data_ptr);
	}
	else if(buffer_type == AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE)
	{
		/* get the last guide buffer index */
		buffer_index = Autoguider_Guide_Get_Last_Buffer_Index();
		if(buffer_index < 0)
		{
			Autoguider_General_Error_Number = 613;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:buffer_index less than 0.");
			return FALSE;
		}
		/* copy data into data buffer */
		buffer_data_length = Autoguider_Buffer_Get_Guide_Pixel_Count();
		buffer_data_ptr = (void *)malloc(buffer_data_length*pixel_length);
		if(buffer_data_ptr == NULL)
		{
			Autoguider_General_Error_Number = 614;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:"
				"Allocating buffer_data_ptr failed (%ld,%ld).",buffer_data_length,pixel_length);
			return FALSE;
		}
		if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW)
		{
			retval = Autoguider_Buffer_Raw_Guide_Copy(buffer_index,buffer_data_ptr,buffer_data_length);
			if(retval == FALSE)
			{
				/* free allocated data */
				if(buffer_data_ptr != NULL)
					free(buffer_data_ptr);
				return FALSE;
			}
		}
		else if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED)
		{
			retval = Autoguider_Buffer_Reduced_Guide_Copy(buffer_index,buffer_data_ptr,buffer_data_length);
			if(retval == FALSE)
			{
				/* free allocated data */
				if(buffer_data_ptr != NULL)
					free(buffer_data_ptr);
				return FALSE;
			}
		}
		/* get dimensions */
		ncols = Autoguider_Buffer_Get_Guide_Binned_NCols();
		nrows = Autoguider_Buffer_Get_Guide_Binned_NRows();
		/* create fits in memory */
		retval = Autoguider_Get_Fits_From_Buffer(buffer_ptr,buffer_length,buffer_state,
							 buffer_data_ptr,buffer_data_length,ncols,nrows);
		if(retval == FALSE)
		{
			/* free allocated data */
			if(buffer_data_ptr != NULL)
				free(buffer_data_ptr);
			return FALSE;
		}
		/* free allocated data */
		if(buffer_data_ptr != NULL)
			free(buffer_data_ptr);
	}
	else
	{
		Autoguider_General_Error_Number = 615;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits:Illegal buffer type %d.",buffer_type);
		return FALSE;

	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits:finished.");
#endif
	return TRUE;
}

/**
 * Create a buffer containing a FITS image in memory from the specified data and dimensional information.
 * @param buffer_ptr The address of a pointer to store the created FITS in memory into.
 * @param buffer_length The address of a length word to store the finished length of the created FITS in memory into.
 * @param buffer_state Whether the data is raw or reduced data. This determines whether the data is unsigned short
 *   or float, the FITS is created as appropriate.
 * @param data_buffer_ptr A pointer to a a buffer containing the read-out data.
 * @param data_buffer_length The length of the data buffer.
 * @param ncols The number of <b>binned</b> columns in the image.
 * @param nrows The number of <b>binned</b> rows in the image.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Get_Fits_From_Buffer(void **buffer_ptr,size_t *buffer_length,int buffer_state,
				    void *data_buffer_ptr,size_t data_buffer_length,int ncols,int nrows)
{
	fitsfile *fits_fp = NULL;
	char cfitsio_error_buff[32]; /* fits_get_errstatus returns 30 chars max */
	long axes[2];
	int retval,bitpix,datatype,cfitsio_status=0;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_From_Buffer:started.");
#endif
	/* check parameters */
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 605;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:buffer_ptr was NULL.");
		return FALSE;
	}
	if(buffer_length == NULL)
	{
		Autoguider_General_Error_Number = 606;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:buffer_length was NULL.");
		return FALSE;
	}
	if((buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW) &&
	   (buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED))
	{
		Autoguider_General_Error_Number = 617;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:Illegal buffer state %d.",
			buffer_state);
		return FALSE;
	}
	if(data_buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 607;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:data_buffer_ptr was NULL.");
		return FALSE;
	}
	/* create a unsigned short FITS image for raw, and a float FITS image for reduced */
	if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW)
	{
		bitpix = USHORT_IMG;
		datatype = TUSHORT;
	}
	else if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED)
	{
		bitpix = FLOAT_IMG;
		datatype = TFLOAT;
	}
	(*buffer_length) = 288000;
	(*buffer_ptr) = (void*)malloc((*buffer_length));
	if((*buffer_ptr) == NULL)
	{
		Autoguider_General_Error_Number = 608;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:"
			"Allocating buffer_ptr failed(%d).",(*buffer_length));
		return FALSE;
	}
	cfitsio_status=0;
	retval = fits_create_memfile(&fits_fp,buffer_ptr,buffer_length,288000,realloc,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Error_Number = 609;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:"
			"fits_create_memfile failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		/* free allocated FITS file */
		if((*buffer_ptr) != NULL)
			free((*buffer_ptr));
		(*buffer_ptr) = NULL;
		(*buffer_length) = 0;
		return FALSE;
	}
	/* create image block */
	axes[0] = ncols;
	axes[1] = nrows;
	retval = fits_create_img(fits_fp,bitpix,2,axes,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Error_Number = 610;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:"
			"fits_create_img failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		/* free allocated FITS file */
		fits_close_file(fits_fp,&cfitsio_status);
		if((*buffer_ptr) != NULL)
			free((*buffer_ptr));
		(*buffer_ptr) = NULL;
		(*buffer_length) = 0;
		return FALSE;
	}
	/* write data into FITS file */
	retval = fits_write_img(fits_fp,datatype,1,data_buffer_length,data_buffer_ptr,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Error_Number = 611;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:"
			"fits_write_img failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		/* free allocated FITS file */
		fits_close_file(fits_fp,&cfitsio_status);
		if((*buffer_ptr) != NULL)
			free((*buffer_ptr));
		(*buffer_ptr) = NULL;
		(*buffer_length) = 0;
		return FALSE;
	}
	/* ensure data we have written is in the actual data buffer, not CFITSIO's internal buffers */
	/* closing the file ensures this. */ 
	retval = fits_close_file(fits_fp,&cfitsio_status);
	if(retval)
	{
		fits_get_errstatus(cfitsio_status,cfitsio_error_buff);
		fits_report_error(stderr,cfitsio_status);
		Autoguider_General_Error_Number = 612;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_From_Buffer:"
			"fits_close_file failed(%d) : %s.",cfitsio_status,cfitsio_error_buff);
		/* free allocated FITS file */
		fits_close_file(fits_fp,&cfitsio_status);
		if((*buffer_ptr) != NULL)
			free((*buffer_ptr));
		(*buffer_ptr) = NULL;
		(*buffer_length) = 0;
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_From_Buffer:finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
*/
