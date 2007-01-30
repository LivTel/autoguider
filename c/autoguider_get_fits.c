/* autoguider_get_fits.c
** Autoguider get fits routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_get_fits.c,v 1.3 2007-01-30 17:35:24 cjm Exp $
*/
/**
 * Routines to return an in-memory FITS image for the field or guide buffer.
 * @author Chris Mottram
 * @version $Revision: 1.3 $
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
#ifdef SLALIB
#include "slalib.h"
#endif /* SLALIB */
#ifdef NGATASTRO
#include "ngat_astro.h"
#include "ngat_astro_mjd.h"
#endif /* NGATASTRO */

#include "autoguider_buffer.h"
#include "autoguider_field.h"
#include "autoguider_fits_header.h"
#include "autoguider_general.h"
#include "autoguider_get_fits.h"
#include "autoguider_guide.h"

#include "ccd_temperature.h"
#include "ccd_setup.h"

/* internal hash defines */
/**
 * Offset between degrees centigrade and degress Kelvin.
 */
#define CENTIGRADE_TO_KELVIN (273.15)

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_get_fits.c,v 1.3 2007-01-30 17:35:24 cjm Exp $";

/* internal functions */
static void Get_Fits_TimeSpec_To_Date_String(struct timespec time,char *time_string);
static void Get_Fits_TimeSpec_To_Date_Obs_String(struct timespec time,char *time_string);
static void Get_Fits_TimeSpec_To_UtStart_String(struct timespec time,char *time_string);
static int Get_Fits_TimeSpec_To_Mjd(struct timespec time,int leap_second_correction,double *mjd);

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
 * @see #Autoguider_Get_Fits_Get_Header
 * @see #Autoguider_Get_Fits_From_Buffer
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Copy
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Binned_NCols
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Binned_NRows
 * @see autoguider_field.html#Autoguider_Field_Get_Last_Buffer_Index
 * @see autoguider_fits_header.html#Autoguider_Fits_Header_Initialise
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
	struct Fits_Header_Struct fits_header;
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
	/* initialise fits header */
	if(!Autoguider_Fits_Header_Initialise(&fits_header))
		Autoguider_General_Error();
	if(!Autoguider_Get_Fits_Get_Header(buffer_type,buffer_state,&fits_header))
		Autoguider_General_Error();
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
							 buffer_data_ptr,buffer_data_length,ncols,nrows,&fits_header);
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
							 buffer_data_ptr,buffer_data_length,ncols,nrows,&fits_header);
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
 * @param fits_header The address of a structure of type Fits_Header_Struct containing 
 *        FITS header information for this frame.
 *        The header data is freed after been written.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED
 * @see autoguider_fits_header.html#Fits_Header_Struct
 * @see autoguider_fits_header.html#Autoguider_Fits_Header_Write_To_Fits
 * @see autoguider_fits_header.html#Autoguider_Fits_Header_Free
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Get_Fits_From_Buffer(void **buffer_ptr,size_t *buffer_length,int buffer_state,
				    void *data_buffer_ptr,size_t data_buffer_length,int ncols,int nrows,
				    struct Fits_Header_Struct *fits_header)
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
	/* write the extra fits headers */
	retval = Autoguider_Fits_Header_Write_To_Fits((*fits_header),fits_fp);
	if(retval == FALSE) /* on failure, report error but continue. */
		Autoguider_General_Error();
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
	/* free fits headers */
	retval = Autoguider_Fits_Header_Free(fits_header);
	if(retval == FALSE) /* on failure, report error but continue. */
		Autoguider_General_Error();
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_From_Buffer:finished.");
#endif
	return TRUE;
}

/**
 * Routine to fill in a FITS header for the specified buffer type and buffer state.
 * @param buffer_type Which buffer to get the latest image from (field or guide).
 * @param buffer_state Whether to get the raw or reduced data.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD
 * @see #AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW
 * @see #AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED
 * @see #CENTIGRADE_TO_KELVIN
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_Exposure_Start_Time_Get
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_Exposure_Length_Get
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_CCD_Temperature_Get
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_Exposure_Start_Time_Get
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_Exposure_Length_Get
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_CCD_Temperature_Get
 * @see autoguider_field.html#Autoguider_Field_Get_Bin_X
 * @see autoguider_field.html#Autoguider_Field_Get_Bin_Y
 * @see autoguider_field.html#Autoguider_Field_Get_Unbinned_NCols
 * @see autoguider_field.html#Autoguider_Field_Get_Unbinned_NRow
 * @see autoguider_field.html#Autoguider_Field_Get_Last_Buffer_Index
 * @see autoguider_fits_header.html#Autoguider_Fits_Header_Add_Int
 * @see autoguider_fits_header.html#Autoguider_Fits_Header_Add_String
 * @see autoguider_guide.html#Autoguider_Guide_Get_Last_Buffer_Index
 * @see autoguider_guide.html#Autoguider_Guide_Bin_X_Get
 * @see autoguider_guide.html#Autoguider_Guide_Bin_Y_Get
 * @see autoguider_guide.html#Autoguider_Guide_Binned_NCols_Get
 * @see autoguider_guide.html#Autoguider_Guide_Binned_NRows_Get
 * @see autoguider_guide.html#Autoguider_Guide_Window_Get
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Double
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 */
int Autoguider_Get_Fits_Get_Header(int buffer_type,int buffer_state,struct Fits_Header_Struct *fits_header)
{
	struct CCD_Setup_Window_Struct window;
	struct timespec start_time;
	char date_string[32];
	double current_temperature,target_temperature,exptime,mjd;
	int exposure_length_ms,buffer_index,retval;
	int ccdximsi,ccdyimsi,ccdxbin,ccdybin;
	int ccdwmode,ccdwxoff,ccdwyoff,ccdwxsiz,ccdwysiz,ccdstemp,ccdatemp;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_Get_Header:started.");
#endif
	/* check parameters */
	if((buffer_type != AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD) &&
	   (buffer_type != AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE))
	{
		Autoguider_General_Error_Number = 618;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_Get_Header:Illegal buffer type %d.",
			buffer_type);
		return FALSE;
	}
	if((buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW) &&
	   (buffer_state != AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED))
	{
		Autoguider_General_Error_Number = 619;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_Get_Header:Illegal buffer state %d.",
			buffer_state);
		return FALSE;
	}
	if(fits_header == NULL)
	{
		Autoguider_General_Error_Number = 620;
		sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_Get_Header:fits_header was NULL.");
		return FALSE;
	}
	/* Field or Guide? */
	if(buffer_type == AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD)
	{
		Autoguider_Fits_Header_Add_String(fits_header,"OBSTYPE","FIELD",
						  "Was this a field or guide observation");
		ccdxbin = Autoguider_Field_Get_Bin_X();
		ccdybin = Autoguider_Field_Get_Bin_Y();
		if(ccdxbin != 0)
			ccdximsi = Autoguider_Field_Get_Unbinned_NCols()/ccdxbin;
		else
			ccdximsi = 0;
		if(ccdybin != 0)
			ccdyimsi = Autoguider_Field_Get_Unbinned_NRows()/ccdybin;
		else
			ccdyimsi = 0;
		ccdwmode = FALSE;
		ccdwxoff = 0;
		ccdwyoff = 0;
		ccdwxsiz = 0;
		ccdwysiz = 0;
		/* get the last field buffer index */
		buffer_index = Autoguider_Field_Get_Last_Buffer_Index();
		if(buffer_index < 0)
		{
			Autoguider_General_Error_Number = 621;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_Get_Header:"
				"buffer_index less than 0.");
			return FALSE;
		}
		if(!Autoguider_Buffer_Field_Exposure_Start_Time_Get(buffer_index,&start_time))
			Autoguider_General_Error();
		if(!Autoguider_Buffer_Field_Exposure_Length_Get(buffer_index,&exposure_length_ms))
			Autoguider_General_Error();
		/* temperature */
		if(!Autoguider_Buffer_Field_CCD_Temperature_Get(buffer_index,&current_temperature))
			Autoguider_General_Error();
	}
	else if(buffer_type == AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE)
	{
		Autoguider_Fits_Header_Add_String(fits_header,"OBSTYPE","GUIDE",
						  "Was this a field or guide observation");
		ccdxbin = Autoguider_Guide_Bin_X_Get();
		ccdybin = Autoguider_Guide_Bin_Y_Get();
		ccdximsi = Autoguider_Guide_Binned_NCols_Get();
		ccdyimsi = Autoguider_Guide_Binned_NRows_Get();
		ccdwmode = TRUE;
		window = Autoguider_Guide_Window_Get();
		ccdwxoff = window.X_Start; /* should be from top, but is this from bottom? */
		ccdwyoff = window.Y_Start; /* should be from top, but is this from bottom? */
		ccdwxsiz = window.X_End-window.X_Start;
		ccdwysiz = window.Y_End-window.Y_Start;
		/* get the last guide buffer index */
		buffer_index = Autoguider_Guide_Get_Last_Buffer_Index();
		if(buffer_index < 0)
		{
			Autoguider_General_Error_Number = 622;
			sprintf(Autoguider_General_Error_String,"Autoguider_Get_Fits_Get_Header:"
				"buffer_index less than 0.");
			return FALSE;
		}
		if(!Autoguider_Buffer_Guide_Exposure_Start_Time_Get(buffer_index,&start_time))
			Autoguider_General_Error();
		if(!Autoguider_Buffer_Guide_Exposure_Length_Get(buffer_index,&exposure_length_ms))
			Autoguider_General_Error();
		/* temperature */
		if(!Autoguider_Buffer_Guide_CCD_Temperature_Get(buffer_index,&current_temperature))
			Autoguider_General_Error();
	}/* end if buffer_type */
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDXIMSI",ccdximsi,
				       "Size of binned imaging area (pixels)");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDYIMSI",ccdyimsi,
				       "Size of binned imaging area (pixels)");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDXBIN",ccdxbin,"X/Horizontal/Column binning");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDYBIN",ccdybin,"Y/Vertical/Row binning");
	Autoguider_Fits_Header_Add_Logical(fits_header,"CCDWMODE",ccdwmode,"Is the readout windowed");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDWXOFF",ccdwxoff,"Offset of top left corner of window");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDWYOFF",ccdwyoff,"Offset of top left corner of window");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDWXSIZ",ccdwxsiz,"Width of window in pixels");
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDWYSIZ",ccdwysiz,"Height of window in pixels");
	/* EXPTIME */
	exptime = ((double)exposure_length_ms)/1000.0;
	Autoguider_Fits_Header_Add_Float(fits_header,"EXPTIME",exptime,"Exposure length in seconds.");
	/* DATE */
	Get_Fits_TimeSpec_To_Date_String(start_time,date_string);
        Autoguider_Fits_Header_Add_String(fits_header,"DATE",date_string,"The date of the observation");
	/* DATE-OBS */
        Get_Fits_TimeSpec_To_Date_Obs_String(start_time,date_string);
        Autoguider_Fits_Header_Add_String(fits_header,"DATE-OBS",date_string,"The date of the observation");
	/* UTSTART */
        Get_Fits_TimeSpec_To_UtStart_String(start_time,date_string);
        Autoguider_Fits_Header_Add_String(fits_header,"UTSTART",date_string,"The date of the observation");
	/* MJD */
	Get_Fits_TimeSpec_To_Mjd(start_time,FALSE,&mjd);
	Autoguider_Fits_Header_Add_Float(fits_header,"MJD",mjd,"Modified Julian Date");
	/*
diddly fixed loaded stuff like PRESCAN POSTSCAN READNOIS DETECTOR EPERDN
diddly modifiable programmable stuff TAGID/USERID/PROPID
	*/
	/* field - raw or reduced? */
	if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW)
	{
		Autoguider_Fits_Header_Add_String(fits_header,"REDTYPE","RAW",
						  "Is this raw data or has it been reduced");
	}
	else if(buffer_state == AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED)
	{
		Autoguider_Fits_Header_Add_String(fits_header,"REDTYPE","REDUCED",
						  "Is this raw data or has it been reduced");
	}
	/* CCDATEMP */
	ccdatemp = (int)(current_temperature+CENTIGRADE_TO_KELVIN);
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_Get_Header:"
				      "ccdatemp is %.d K.",ccdatemp);
#endif
	Autoguider_Fits_Header_Add_Int(fits_header,"CCDATEMP",ccdatemp,
					       "CCD Temperature at time of writing FITS header (Kelvin)");
	/* target temperature */
	retval = CCD_Config_Get_Double("ccd.temperature.target",&target_temperature);
	if(retval)
	{
		ccdstemp = (int)(target_temperature+CENTIGRADE_TO_KELVIN);
		Autoguider_Fits_Header_Add_Int(fits_header,"CCDSTEMP",ccdstemp,"Target Temperature (Kelvin)");
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS,"Autoguider_Get_Fits_Get_Header:finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Routine to convert a timespec structure to a DATE sytle string to put into a FITS header.
 * This uses gmtime and strftime to format the string. The resultant string is of the form:
 * <b>CCYY-MM-DD</b>, which is equivalent to %Y-%m-%d passed to strftime.
 * @param time The time to convert.
 * @param time_string The string to put the time representation in. The string must be at least
 * 	12 characters long.
 */
static void Get_Fits_TimeSpec_To_Date_String(struct timespec time,char *time_string)
{
	struct tm *tm_time = NULL;

	tm_time = gmtime(&(time.tv_sec));
	strftime(time_string,12,"%Y-%m-%d",tm_time);
}

/**
 * Routine to convert a timespec structure to a DATE-OBS sytle string to put into a FITS header.
 * This uses gmtime and strftime to format most of the string, and tags the milliseconds on the end.
 * The resultant form of the string is <b>CCYY-MM-DDTHH:MM:SS.sss</b>.
 * @param time The time to convert.
 * @param time_string The string to put the time representation in. The string must be at least
 * 	24 characters long.
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS
 */
static void Get_Fits_TimeSpec_To_Date_Obs_String(struct timespec time,char *time_string)
{
	struct tm *tm_time = NULL;
	char buff[32];
	int milliseconds;

	tm_time = gmtime(&(time.tv_sec));
	strftime(buff,32,"%Y-%m-%dT%H:%M:%S.",tm_time);
	milliseconds = (((double)time.tv_nsec)/((double)AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS));
	sprintf(time_string,"%s%03d",buff,milliseconds);
}

/**
 * Routine to convert a timespec structure to a UTSTART sytle string to put into a FITS header.
 * This uses gmtime and strftime to format most of the string, and tags the milliseconds on the end.
 * @param time The time to convert.
 * @param time_string The string to put the time representation in. The string must be at least
 * 	14 characters long.
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS
 */
static void Get_Fits_TimeSpec_To_UtStart_String(struct timespec time,char *time_string)
{
	struct tm *tm_time = NULL;
	char buff[16];
	int milliseconds;

	tm_time = gmtime(&(time.tv_sec));
	strftime(buff,16,"%H:%M:%S.",tm_time);
	milliseconds = (((double)time.tv_nsec)/((double)AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS));
	sprintf(time_string,"%s%03d",buff,milliseconds);
}

/**
 * Routine to convert a timespec structure to a Modified Julian Date (decimal days) to put into a FITS header.
 * <p>If SLALIB is defined, this uses slaCldj to get the MJD for zero hours, 
 * and then adds hours/minutes/seconds/milliseconds on the end as a decimal.
 * <p>If NGATASTRO is defined, this uses NGAT_Astro_Timespec_To_MJD to get the MJD.
 * <p>If neither SLALIB or NGATASTRO are defined at compile time, this routine should throw an error
 * when compiling.
 * <p>This routine is still wrong for last second of the leap day, as gmtime will return 1st second of the next day.
 * Also note the passed in leap_second_correction should change at midnight, when the leap second occurs.
 * None of this should really matter, 1 second will not affect the MJD for several decimal places.
 * @param time The time to convert.
 * @param leap_second_correction A number representing whether a leap second will occur. This is normally zero,
 * 	which means no leap second will occur. It can be 1, which means the last minute of the day has 61 seconds,
 *	i.e. there are 86401 seconds in the day. It can be -1,which means the last minute of the day has 59 seconds,
 *	i.e. there are 86399 seconds in the day.
 * @param mjd The address of a double to store the calculated MJD.
 * @return The routine returns TRUE if it succeeded, FALSE if it fails. 
 *         slaCldj and NGAT_Astro_Timespec_To_MJD can fail.
 */
static int Get_Fits_TimeSpec_To_Mjd(struct timespec time,int leap_second_correction,double *mjd)
{
#ifdef SLALIB
	struct tm *tm_time = NULL;
	int year,month,day;
	double seconds_in_day = 86400.0;
	double elapsed_seconds;
	double day_fraction;
#endif
	int retval;

#ifdef SLALIB
/* check leap_second_correction in range */
/* convert time to ymdhms*/
	tm_time = gmtime(&(time.tv_sec));
/* convert tm_time data to format suitable for slaCldj */
	year = tm_time->tm_year+1900; /* tm_year is years since 1900 : slaCldj wants full year.*/
	month = tm_time->tm_mon+1;/* tm_mon is 0..11 : slaCldj wants 1..12 */
	day = tm_time->tm_mday;
/* call slaCldj to get MJD for 0hr */
	slaCldj(year,month,day,mjd,&retval);
	if(retval != 0)
	{
		Autoguider_General_Error_Number = 623;
		sprintf(Autoguider_General_Error_String,"Get_Fits_TimeSpec_To_Mjd:slaCldj(%d,%d,%d) failed(%d).",year,
			month,day,retval);
		return FALSE;
	}
/* how many seconds were in the day */
	seconds_in_day = 86400.0;
	seconds_in_day += (double)leap_second_correction;
/* calculate the number of elapsed seconds in the day */
	elapsed_seconds = (double)tm_time->tm_sec + (((double)time.tv_nsec) / 1.0E+09);
	elapsed_seconds += ((double)tm_time->tm_min) * 60.0;
	elapsed_seconds += ((double)tm_time->tm_hour) * 3600.0;
/* calculate day fraction */
	day_fraction = elapsed_seconds / seconds_in_day;
/* add day_fraction to mjd */
	(*mjd) += day_fraction;
#else
#ifdef NGATASTRO
	retval = NGAT_Astro_Timespec_To_MJD(time,leap_second_correction,mjd);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 624;
		sprintf(Autoguider_General_Error_String,"Exposure_TimeSpec_To_Mjd:NGAT_Astro_Timespec_To_MJD failed:");
		/* concatenate NGAT Astro library error onto Exposure_Error_String */
		NGAT_Astro_Error_String(Autoguider_General_Error_String+strlen(Autoguider_General_Error_String));
		return FALSE;
	}
#else
#error Neither NGATASTRO or SLALIB are defined: No library defined for MJD calculation.
#endif
#endif
	return TRUE;
}


/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2007/01/26 15:29:42  cjm
** Added Autoguider_Get_Fits_Get_Header and associated routines to populate
** FITS headers.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
