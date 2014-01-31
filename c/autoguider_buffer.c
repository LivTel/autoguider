/* autoguider_buffer.c
** Autoguider buffer routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_buffer.c,v 1.6 2014-01-31 17:17:17 cjm Exp $
*/
/**
 * Buffer routines for the autoguider program.
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

#include <errno.h>
#include <pthread.h> /* mutex */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log_udp.h"

#include "ccd_config.h"
#include "ccd_general.h"

#include "autoguider_general.h"
#include "autoguider_buffer.h"

/* hash defines */
/**
 * Number of buffers of each type.
 */
#define AUTOGUIDER_BUFFER_COUNT      (2)

/* data types */
/**
 * Data type holding local data to autoguider_buffer for one buffer. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in field images.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in field images.</dd>
 * <dt>Bin_X</dt> <dd>X binning in field images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in field images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in field images.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in field images.</dd>
 * <dt>Raw_Buffer_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT pointer to allocated arrays of unsigned shorts,
 *     the actual raw field image buffers.</dd>
 * <dt>Raw_Mutex_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT mutexs to protect the raw buffers 
 *     from multiple access.</dd>
 * <dt>Reduced_Buffer_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT pointer to allocated arrays of unsigned shorts,
 *     the actual reduced field image buffers.</dd>
 * <dt>Reduced_Mutex_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT mutexs to protect the reduced buffers 
 *     from multiple access.</dd>
 * <dt>Exposure_Start_Time_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT timespecs to hold the time the exposure
 *     started that generated the data in the buffer.</dd>
 * <dt>Exposure_Start_Time_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT timespecs to hold the exposure length
 *     (in milliseconds) that generated the data in the buffer.</dd>
 * <dt>CCD_Temperature_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT doubles to hold the CCD temperature
 *     (in degrees centigrade) at the time the data was read out.</dd>
 * </dl>
 * @see #AUTOGUIDER_BUFFER_COUNT
 */
struct Buffer_One_Struct
{
	int Unbinned_NCols;
	int Unbinned_NRows;
	int Bin_X;
	int Bin_Y;
	int Binned_NCols;
	int Binned_NRows;
	unsigned short *Raw_Buffer_List[AUTOGUIDER_BUFFER_COUNT];
	pthread_mutex_t Raw_Mutex_List[AUTOGUIDER_BUFFER_COUNT];
	float *Reduced_Buffer_List[AUTOGUIDER_BUFFER_COUNT];
	pthread_mutex_t Reduced_Mutex_List[AUTOGUIDER_BUFFER_COUNT];
	struct timespec Exposure_Start_Time_List[AUTOGUIDER_BUFFER_COUNT];
	int Exposure_Length_List[AUTOGUIDER_BUFFER_COUNT];
	double CCD_Temperature_List[AUTOGUIDER_BUFFER_COUNT];
};

/**
 * Data type holding local data to autoguider_buffer. This consists of the following:
 * <dl>
 * <dt>Field</dt> <dd>A struct Buffer_One_Struct for Field images.</dd>
 * <dt>Guide</dt> <dd>A struct Buffer_One_Struct for Guide images.</dd>
 * </dl>
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see #Buffer_One_Struct
 */
struct Buffer_Struct
{
	struct Buffer_One_Struct Field;
	struct Buffer_One_Struct Guide;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_buffer.c,v 1.6 2014-01-31 17:17:17 cjm Exp $";
/**
 * Instance of buffer data.
 * @see #Buffer_Struct
 */
static struct Buffer_Struct Buffer_Data = 
{
	{
		0,0,1,1,0,0, /* dimensions */
		{NULL,NULL}, /* Raw_Buffer_List */
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}, /* Raw_Mutex_List */
		{NULL,NULL}, /* Reduced_Buffer_List */
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}, /* Reduced_Mutex_List */
		{{0,0},{0,0}},{0,0}, /* Exposure_Start_Time_List/Exposure_Length_List */
		{0,0} /* CCD_TemperatureList */
	},
	{
		0,0,1,1,0,0, /* dimensions */
		{NULL,NULL}, /* Raw_Buffer_List */
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}, /* Raw_Mutex_List */
		{NULL,NULL}, /* Reduced_Buffer_List */
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}, /* Reduced_Mutex_List */
		{{0,0},{0,0}},{0,0}, /* Exposure_Start_Time_List/Exposure_Length_List */
		{0,0} /* CCD_TemperatureList */
	}
};

/* internal functions */
static int Buffer_One_Raw_Lock(struct Buffer_One_Struct *data,int index,unsigned short **buffer_ptr);
static int Buffer_One_Raw_Unlock(struct Buffer_One_Struct *data,int index);
static int Buffer_One_Raw_Copy(struct Buffer_One_Struct *data,int index,unsigned short *buffer_ptr,
			       size_t buffer_length);
static int Buffer_One_Reduced_Lock(struct Buffer_One_Struct *data,int index,float **buffer_ptr);
static int Buffer_One_Reduced_Unlock(struct Buffer_One_Struct *data,int index);
static int Buffer_One_Reduced_Copy(struct Buffer_One_Struct *data,int index,float *buffer_ptr,
			       size_t buffer_length);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise the readout buffers. Assumes CCD_Config_Load has already loaded the configuration.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Buffer_Initialise(void)
{
	int retval,ncols,nrows,x_bin,y_bin;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Initialise",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	/* get default config */
	/* field */
	retval = CCD_Config_Get_Integer("ccd.field.ncols",&ncols);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 400;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:Getting field NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.nrows",&nrows);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 401;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:Getting field NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.x_bin",&x_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 402;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:"
			"Getting field X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.y_bin",&y_bin);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 403;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:"
			"Getting field Y Binning failed.");
		return FALSE;
	}
	/* setup field buffer */
	retval = Autoguider_Buffer_Set_Field_Dimension(ncols,nrows,x_bin,y_bin);
	if(retval == FALSE)
		return FALSE;
	/* get default config */
	/* guide */
	retval = CCD_Config_Get_Integer("guide.ncols.default",&ncols);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 404;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:Getting guide NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.nrows.default",&nrows);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 405;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Initialise:Getting guide NRows failed.");
		return FALSE;
	}
	/* setup guide buffer */
	retval = Autoguider_Buffer_Set_Guide_Dimension(ncols,nrows,1,1);
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Initialise",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Set the field dimensions, and (re) allocate the buffers accordingly.
 * Locks/unlocks the associated mutex.
 * @param ncols Number of unbinned columns.
 * @param nrows Number of unbinned rows.
 * @param x_bin X (column) binning.
 * @param y_bin Y (row) binning.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Set_Field_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Set_Field_Dimension",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	Buffer_Data.Field.Unbinned_NCols = ncols;
	Buffer_Data.Field.Unbinned_NRows = nrows;
	Buffer_Data.Field.Bin_X = x_bin;
	Buffer_Data.Field.Bin_Y = x_bin;
	Buffer_Data.Field.Binned_NCols = ncols/x_bin;
	Buffer_Data.Field.Binned_NRows = nrows/y_bin;
	for(i=0;i < AUTOGUIDER_BUFFER_COUNT; i++)
	{
		/* raw */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Field.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Field.Raw_Buffer_List[i] == NULL)
		{
			Buffer_Data.Field.Raw_Buffer_List[i] = (unsigned short *)malloc(Buffer_Data.Field.Binned_NCols*
						Buffer_Data.Field.Binned_NRows * sizeof(unsigned short));
		}
		else
		{
			Buffer_Data.Field.Raw_Buffer_List[i] = (unsigned short *)realloc(
                           Buffer_Data.Field.Raw_Buffer_List[i],
			   (Buffer_Data.Field.Binned_NCols * Buffer_Data.Field.Binned_NRows * sizeof(unsigned short)));
		}
		if(Buffer_Data.Field.Raw_Buffer_List[i] == NULL)
		{
			/* unlock mutex */
			Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Raw_Mutex_List[i]));
			Autoguider_General_Error_Number = 406;
			sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Set_Field_Dimension:"
				"Raw Field buffer %d failed to allocate/reallocate (%d,%d).",i,
				Buffer_Data.Field.Binned_NCols,Buffer_Data.Field.Binned_NRows);
			return FALSE;
		}
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		/* reduced */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Field.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Field.Reduced_Buffer_List[i] == NULL)
		{
			Buffer_Data.Field.Reduced_Buffer_List[i] = (float *)malloc(Buffer_Data.Field.Binned_NCols*
						Buffer_Data.Field.Binned_NRows * sizeof(float));
		}
		else
		{
			Buffer_Data.Field.Reduced_Buffer_List[i] = (float *)realloc(
                           Buffer_Data.Field.Reduced_Buffer_List[i],
			   (Buffer_Data.Field.Binned_NCols * Buffer_Data.Field.Binned_NRows * sizeof(float)));
		}
		if(Buffer_Data.Field.Reduced_Buffer_List[i] == NULL)
		{
			/* unlock mutex */
			Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Reduced_Mutex_List[i]));
			Autoguider_General_Error_Number = 414;
			sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Set_Field_Dimension:"
				"Reduced Field buffer %d failed to allocate/reallocate (%d,%d).",i,
				Buffer_Data.Field.Binned_NCols,Buffer_Data.Field.Binned_NRows);
			return FALSE;
		}
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Set_Field_Dimension",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Set the guide dimensions, and (re) allocate the buffers accordingly.
 * Locks/unlocks the associated mutex.
 * @param ncols Number of unbinned columns.
 * @param nrows Number of unbinned rows.
 * @param x_bin X (column) binning.
 * @param y_bin Y (row) binning.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Set_Guide_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Set_Guide_Dimension",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	Buffer_Data.Guide.Unbinned_NCols = ncols;
	Buffer_Data.Guide.Unbinned_NRows = nrows;
	Buffer_Data.Guide.Bin_X = x_bin;
	Buffer_Data.Guide.Bin_Y = x_bin;
	Buffer_Data.Guide.Binned_NCols = ncols/x_bin;
	Buffer_Data.Guide.Binned_NRows = nrows/y_bin;
	for(i=0;i < AUTOGUIDER_BUFFER_COUNT; i++)
	{
		/* raw */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Guide.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Guide.Raw_Buffer_List[i] == NULL)
		{
			Buffer_Data.Guide.Raw_Buffer_List[i] = (unsigned short *)malloc(Buffer_Data.Guide.Binned_NCols*
						Buffer_Data.Guide.Binned_NRows * sizeof(unsigned short));
		}
		else
		{
			Buffer_Data.Guide.Raw_Buffer_List[i] = (unsigned short *)realloc(
                           Buffer_Data.Guide.Raw_Buffer_List[i],
			   (Buffer_Data.Guide.Binned_NCols * Buffer_Data.Guide.Binned_NRows * sizeof(unsigned short)));
		}
		if(Buffer_Data.Guide.Raw_Buffer_List[i] == NULL)
		{
			/* unlock mutex */
			Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Raw_Mutex_List[i]));
			Autoguider_General_Error_Number = 407;
			sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Set_Guide_Dimension:"
				"Raw Guide buffer %d failed to allocate/reallocate (%d,%d).",i,
				Buffer_Data.Guide.Binned_NCols,Buffer_Data.Guide.Binned_NRows);
			return FALSE;
		}
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		/* reduced */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Guide.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Guide.Reduced_Buffer_List[i] == NULL)
		{
			Buffer_Data.Guide.Reduced_Buffer_List[i] = (float *)malloc(Buffer_Data.Guide.Binned_NCols*
						Buffer_Data.Guide.Binned_NRows * sizeof(float));
		}
		else
		{
			Buffer_Data.Guide.Reduced_Buffer_List[i] = (float *)realloc(
                           Buffer_Data.Guide.Reduced_Buffer_List[i],
			   (Buffer_Data.Guide.Binned_NCols * Buffer_Data.Guide.Binned_NRows * sizeof(float)));
		}
		if(Buffer_Data.Guide.Reduced_Buffer_List[i] == NULL)
		{
			/* unlock mutex */
			Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Reduced_Mutex_List[i]));
			Autoguider_General_Error_Number = 415;
			sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Set_Guide_Dimension:"
				"Reduced Guide buffer %d failed to allocate/reallocate (%d,%d).",i,
				Buffer_Data.Guide.Binned_NCols,Buffer_Data.Guide.Binned_NRows);
			return FALSE;
		}
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Set_Guide_Dimension",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Get the number of <b>pixels</b> in the field buffer (multiply by sizeof(unsigned short) to get the number of bytes).
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Field_Pixel_Count(void)
{
	return Buffer_Data.Field.Binned_NCols * Buffer_Data.Field.Binned_NRows;
}

/**
 * Get the number of binned column <b>pixels</b> in the field buffer.
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Field_Binned_NCols(void)
{
	return Buffer_Data.Field.Binned_NCols;
}

/**
 * Get the number of binned row <b>pixels</b> in the field buffer.
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Field_Binned_NRows(void)
{
	return Buffer_Data.Field.Binned_NRows;
}

/**
 * Get the number of <b>pixels</b> in the guide buffer (multiply by sizeof(unsigned short) to get the number of bytes).
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Guide_Pixel_Count(void)
{
	return Buffer_Data.Guide.Binned_NCols * Buffer_Data.Guide.Binned_NRows;
}

/**
 * Get the number of binned column <b>pixels</b> in the guide buffer.
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Guide_Binned_NCols(void)
{
	return Buffer_Data.Guide.Binned_NCols;
}

/**
 * Get the number of binned row <b>pixels</b> in the guide buffer.
 * @return The number of pixels.
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Get_Guide_Binned_NRows(void)
{
	return Buffer_Data.Guide.Binned_NRows;
}

/**
 * Mutex lock and return the specified raw field buffer. This should subsequently be unlocked by 
 * Autoguider_Buffer_Raw_Field_Unlock.
 * @param index The index in the raw field buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to unsigned short, on return, filled with the locked
 *        buffer's start address.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Lock
 * @see #Autoguider_Buffer_Raw_Field_Unlock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Raw_Field_Lock(int index,unsigned short **buffer_ptr)
{
	return Buffer_One_Raw_Lock(&(Buffer_Data.Field),index,buffer_ptr);
}

/**
 * Mutex unlock the specified raw field buffer. This undoes the Autoguider_Buffer_Raw_Field_Lock operation.
 * @param index The index in the raw field buffer list to unlock and return.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Unlock
 * @see #Autoguider_Buffer_Raw_Field_Lock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Raw_Field_Unlock(int index)
{
	return Buffer_One_Raw_Unlock(&(Buffer_Data.Field),index);
}

/**
 * Mutex lock and return the specified raw guide buffer. This should subsequently be unlocked by 
 * Autoguider_Buffer_Raw_Guide_Unlock.
 * @param index The index in the raw guide buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to unsigned short, on return, filled with the locked
 *        buffer's start address.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Lock
 * @see #Autoguider_Buffer_Raw_Guide_Unlock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Raw_Guide_Lock(int index,unsigned short **buffer_ptr)
{
	return Buffer_One_Raw_Lock(&(Buffer_Data.Guide),index,buffer_ptr);
}

/**
 * Mutex unlock the specified raw guide buffer. This undoes the Autoguider_Buffer_Raw_Guide_Lock operation.
 * @param index The index in the raw guide buffer list to unlock and return.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Unlock
 * @see #Autoguider_Buffer_Raw_Guide_Lock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Raw_Guide_Unlock(int index)
{
	return Buffer_One_Raw_Unlock(&(Buffer_Data.Guide),index);
}

/**
 * Copy the specified raw field buffer into the supplied buffer. The raw field buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param index The index in the field buffer list to copy.
 * @param buffer_ptr A pointer to unsigned short, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 *        Use Autoguider_Buffer_Get_Field_Pixel_Count to get the correct pixel count for this buffer.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Copy
 * @see #Buffer_Data
 * @see #Autoguider_Buffer_Get_Field_Pixel_Count
 */
int Autoguider_Buffer_Raw_Field_Copy(int index,unsigned short *buffer_ptr,size_t buffer_length)
{
	return Buffer_One_Raw_Copy(&(Buffer_Data.Field),index,buffer_ptr,buffer_length);
}

/**
 * Copy the specified raw guide buffer into the supplied buffer. The raw guide buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param index The index in the guide buffer list to copy.
 * @param buffer_ptr A pointer to unsigned short, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 *        Use Autoguider_Buffer_Get_Guide_Pixel_Count to get the correct pixel count for this buffer.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Raw_Copy
 * @see #Buffer_Data
 * @see #Autoguider_Buffer_Get_Guide_Pixel_Count
 */
int Autoguider_Buffer_Raw_Guide_Copy(int index,unsigned short *buffer_ptr,size_t buffer_length)
{
	return Buffer_One_Raw_Copy(&(Buffer_Data.Guide),index,buffer_ptr,buffer_length);
}

/**
 * Mutex lock and return the specified reduced field buffer. This should subsequently be unlocked by 
 * Autoguider_Buffer_Reduced_Field_Unlock.
 * @param index The index in the field buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to float, on return, filled with the locked
 *        buffer's start address.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Lock
 * @see #Autoguider_Buffer_Reduced_Field_Unlock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Reduced_Field_Lock(int index,float **buffer_ptr)
{
	return Buffer_One_Reduced_Lock(&(Buffer_Data.Field),index,buffer_ptr);
}

/**
 * Mutex unlock the specified reduced field buffer. This undoes the Autoguider_Buffer_Reduced_Field_Lock operation.
 * @param index The index in the reduced field buffer list to unlock and return.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Unlock
 * @see #Autoguider_Buffer_Reduced_Field_Lock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Reduced_Field_Unlock(int index)
{
	return Buffer_One_Reduced_Unlock(&(Buffer_Data.Field),index);
}

/**
 * Mutex lock and return the specified reduced guide buffer. This should subsequently be unlocked by 
 * Autoguider_Buffer_Reduced_Guide_Unlock.
 * @param index The index in the reduced guide buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to float, on return, filled with the locked
 *        buffer's start address.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Lock
 * @see #Autoguider_Buffer_Reduced_Guide_Unlock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Reduced_Guide_Lock(int index,float **buffer_ptr)
{
	return Buffer_One_Reduced_Lock(&(Buffer_Data.Guide),index,buffer_ptr);
}

/**
 * Mutex unlock the specified reduced guide buffer. This undoes the Autoguider_Buffer_Reduced_Guide_Lock operation.
 * @param index The index in the reduced guide buffer list to unlock and return.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Unlock
 * @see #Autoguider_Buffer_Reduced_Guide_Lock
 * @see #Buffer_Data
 */
int Autoguider_Buffer_Reduced_Guide_Unlock(int index)
{
	return Buffer_One_Reduced_Unlock(&(Buffer_Data.Guide),index);
}

/**
 * Copy the specified reduced field buffer into the supplied buffer. The reduced field buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param index The index in the field buffer list to copy.
 * @param buffer_ptr A pointer to float, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 *        Use Autoguider_Buffer_Get_Field_Pixel_Count to get the correct pixel count for this buffer.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Copy
 * @see #Buffer_Data
 * @see #Autoguider_Buffer_Get_Field_Pixel_Count
 */
int Autoguider_Buffer_Reduced_Field_Copy(int index,float *buffer_ptr,size_t buffer_length)
{
	return Buffer_One_Reduced_Copy(&(Buffer_Data.Field),index,buffer_ptr,buffer_length);
}

/**
 * Copy the specified reduced guide buffer into the supplied buffer. The reduced guide buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param index The index in the guide buffer list to copy.
 * @param buffer_ptr A pointer to float, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 *        Use Autoguider_Buffer_Get_Guide_Pixel_Count to get the correct pixel count for this buffer.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_One_Reduced_Copy
 * @see #Buffer_Data
 * @see #Autoguider_Buffer_Get_Guide_Pixel_Count
 */
int Autoguider_Buffer_Reduced_Guide_Copy(int index,float *buffer_ptr,size_t buffer_length)
{
	return Buffer_One_Reduced_Copy(&(Buffer_Data.Guide),index,buffer_ptr,buffer_length);
}

/**
 * Copy the specified raw field buffer to the equivalent reduced buffer, doing unsigned short to float
 * data type conversion. Both the raw and reduced mutex for the specified index are locked during this operation.
 * @param index The index in the field buffer list to copy.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 * @see #Buffer_One_Raw_Lock
 * @see #Buffer_One_Raw_Unlock
 * @see #Buffer_One_Reduced_Lock
 * @see #Buffer_One_Reduced_Unlock
 * @see autoguider_general.html#Autoguider_General_Log
 */
int Autoguider_Buffer_Raw_To_Reduced_Field(int index)
{
	unsigned short *raw_buffer_ptr = NULL;
	unsigned short *current_raw_buffer_ptr = NULL;
	float *reduced_buffer_ptr = NULL;
	float *current_reduced_buffer_ptr = NULL;
	int pixel_count,i;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Raw_To_Reduced_Field",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if(!Buffer_One_Raw_Lock(&(Buffer_Data.Field),index,&raw_buffer_ptr))
		return FALSE;
	if(!Buffer_One_Reduced_Lock(&(Buffer_Data.Field),index,&reduced_buffer_ptr))
	{
		Buffer_One_Raw_Unlock(&(Buffer_Data.Field),index);
		return FALSE;
	}
	pixel_count = Autoguider_Buffer_Get_Field_Pixel_Count();
	current_reduced_buffer_ptr = reduced_buffer_ptr;
	current_raw_buffer_ptr = raw_buffer_ptr;
	for(i=0;i<pixel_count;i++)
	{
		(*current_reduced_buffer_ptr) = (float)(*current_raw_buffer_ptr);
		current_raw_buffer_ptr++;
		current_reduced_buffer_ptr++;
	}
	if(!Buffer_One_Reduced_Unlock(&(Buffer_Data.Field),index))
	{
		Buffer_One_Raw_Unlock(&(Buffer_Data.Field),index);
		return FALSE;
	}
        if(!Buffer_One_Raw_Unlock(&(Buffer_Data.Field),index))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Raw_To_Reduced_Field",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Copy the specified raw guide buffer to the equivalent reduced buffer, doing unsigned short to float
 * data type conversion. Both the raw and reduced mutex for the specified index are locked during this operation.
 * @param index The index in the guide buffer list to copy.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 * @see #Buffer_One_Raw_Lock
 * @see #Buffer_One_Raw_Unlock
 * @see #Buffer_One_Reduced_Lock
 * @see #Buffer_One_Reduced_Unlock
 * @see autoguider_general.html#Autoguider_General_Log
 */
int Autoguider_Buffer_Raw_To_Reduced_Guide(int index)
{
	unsigned short *raw_buffer_ptr = NULL;
	float *reduced_buffer_ptr = NULL;
	int pixel_count,i;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Raw_To_Reduced_Guide",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if(!Buffer_One_Raw_Lock(&(Buffer_Data.Guide),index,&raw_buffer_ptr))
		return FALSE;
	if(!Buffer_One_Reduced_Lock(&(Buffer_Data.Guide),index,&reduced_buffer_ptr))
	{
		Buffer_One_Raw_Unlock(&(Buffer_Data.Guide),index);
		return FALSE;
	}
	pixel_count = Autoguider_Buffer_Get_Guide_Pixel_Count();
	for(i=0;i<pixel_count;i++)
	{
		reduced_buffer_ptr[i] = (float)(raw_buffer_ptr[i]);
	}
	if(!Buffer_One_Reduced_Unlock(&(Buffer_Data.Guide),index))
	{
		Buffer_One_Raw_Unlock(&(Buffer_Data.Guide),index);
		return FALSE;
	}
        if(!Buffer_One_Raw_Unlock(&(Buffer_Data.Guide),index))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Raw_To_Reduced_Guide",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to set the exposure start time for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param start_time The start time, of type struct timespec.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_Exposure_Start_Time_Set(int index,struct timespec start_time)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Start_Time_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 422;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Start_Time_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Field.Exposure_Start_Time_List[index] = start_time;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Start_Time_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the exposure start time for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param start_time The address of a struct timespec to store the start time.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_Exposure_Start_Time_Get(int index,struct timespec *start_time)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Start_Time_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 423;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Start_Time_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(start_time == NULL)
	{
		Autoguider_General_Error_Number = 424;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Start_Time_Get:"
			"start_time was NULL.");
		return FALSE;
	}
	(*start_time) = Buffer_Data.Field.Exposure_Start_Time_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Start_Time_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to set the exposure length for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param exposure_length_ms The exposure length, in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_Exposure_Length_Set(int index,int exposure_length_ms)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Length_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 425;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Length_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Field.Exposure_Length_List[index] = exposure_length_ms;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Length_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the exposure length for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param exposure_length_ms The address of an integer to store the exposure length, in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_Exposure_Length_Get(int index,int *exposure_length_ms)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Length_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 426;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Length_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(exposure_length_ms == NULL)
	{
		Autoguider_General_Error_Number = 427;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_Exposure_Length_Get:"
			"exposure_length_ms was NULL.");
		return FALSE;
	}
	(*exposure_length_ms) = Buffer_Data.Field.Exposure_Length_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_Exposure_Length_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to set the ccd temperature for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param current_ccd_temperature The CCD temperature, in degrees centigrade.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_CCD_Temperature_Set(int index,double current_ccd_temperature)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_CCD_Temperature_Set",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER",
				      "Autoguider_Buffer_Field_CCD_Temperature_Set(%d,%.2f):started.",
				      index,current_ccd_temperature);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 434;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_CCD_Temperature_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Field.CCD_Temperature_List[index] = current_ccd_temperature;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_CCD_Temperature_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the CCD temperature for the specified field buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param current_ccd_temperature The address of a double to store the CCD temperature, in degrees centigrade.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Field_CCD_Temperature_Get(int index,double *current_ccd_temperature)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_CCD_Temperature_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 435;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_CCD_Temperature_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(current_ccd_temperature == NULL)
	{
		Autoguider_General_Error_Number = 436;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Field_CCD_Temperature_Get:"
			"current_ccd_temperature was NULL.");
		return FALSE;
	}
	(*current_ccd_temperature) = Buffer_Data.Field.CCD_Temperature_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Autoguider_Buffer_Field_CCD_Temperature_Get",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER",
				      "Autoguider_Buffer_Field_CCD_Temperature_Get(%d,%.2f):finished.",
				      index,(*current_ccd_temperature));
#endif
	return TRUE;
}

/**
 * Routine to set the exposure start time for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param start_time The start time, of type struct timespec.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_Exposure_Start_Time_Set(int index,struct timespec start_time)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Start_Time_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 428;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Start_Time_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Guide.Exposure_Start_Time_List[index] = start_time;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Start_Time_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the exposure start time for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param start_time The address of a struct timespec to store the start time.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_Exposure_Start_Time_Get(int index,struct timespec *start_time)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Start_Time_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 429;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Start_Time_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(start_time == NULL)
	{
		Autoguider_General_Error_Number = 430;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Start_Time_Get:"
			"start_time was NULL.");
		return FALSE;
	}
	(*start_time) = Buffer_Data.Guide.Exposure_Start_Time_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Start_Time_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to set the exposure length for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param exposure_length_ms The exposure length, in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_Exposure_Length_Set(int index,int exposure_length_ms)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Length_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 431;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Length_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Guide.Exposure_Length_List[index] = exposure_length_ms;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Length_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the exposure length for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param exposure_length_ms The address on an integer to store the exposure length, in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_Exposure_Length_Get(int index,int *exposure_length_ms)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Length_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 432;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Length_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(exposure_length_ms == NULL)
	{
		Autoguider_General_Error_Number = 433;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_Exposure_Length_Get:"
			"exposure_length_ms was NULL.");
		return FALSE;
	}
	(*exposure_length_ms) = Buffer_Data.Guide.Exposure_Length_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_Exposure_Length_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to set the ccd temperature for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param current_ccd_temperature The CCD temperature, in degrees centigrade.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_CCD_Temperature_Set(int index,double current_ccd_temperature)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_CCD_Temperature_Set",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER",
				      "Autoguider_Buffer_Guide_CCD_Temperature_Set(%d,%.2f):started.",
				      index,current_ccd_temperature);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 437;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_CCD_Temperature_Set:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	Buffer_Data.Guide.CCD_Temperature_List[index] = current_ccd_temperature;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_CCD_Temperature_Set",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Routine to get the CCD temperature for the specified guide buffer.
 * @param index Which buffer (0..AUTOGUIDER_BUFFER_COUNT).
 * @param current_ccd_temperature The address of a double to store the CCD temperature, in degrees centigrade.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Guide_CCD_Temperature_Get(int index,double *current_ccd_temperature)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_CCD_Temperature_Get",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 438;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_CCD_Temperature_Get:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(current_ccd_temperature == NULL)
	{
		Autoguider_General_Error_Number = 439;
		sprintf(Autoguider_General_Error_String,"Autoguider_Buffer_Guide_CCD_Temperature_Get:"
			"current_ccd_temperature was NULL.");
		return FALSE;
	}
	(*current_ccd_temperature) = Buffer_Data.Guide.CCD_Temperature_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Autoguider_Buffer_Guide_CCD_Temperature_Get",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER",
				      "Autoguider_Buffer_Guide_CCD_Temperature_Get(%d,%.2f):finished.",
				      index,(*current_ccd_temperature));
#endif
	return TRUE;
}

/**
 * Free the allocated buffers.
 * Locks/unlocks the associated mutex.
 * @see #Buffer_Data
 * @see #AUTOGUIDER_BUFFER_COUNT
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 */
int Autoguider_Buffer_Shutdown(void)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Shutdown",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started.");
#endif
	for(i=0;i < AUTOGUIDER_BUFFER_COUNT; i++)
	{
		/* field buffer */

		/* field raw */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Field.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Field.Raw_Buffer_List[i] != NULL)
			free(Buffer_Data.Field.Raw_Buffer_List[i]);
		Buffer_Data.Field.Raw_Buffer_List[i] = NULL;
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;

		/* field reduced */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Field.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Field.Reduced_Buffer_List[i] != NULL)
			free(Buffer_Data.Field.Reduced_Buffer_List[i]);
		Buffer_Data.Field.Reduced_Buffer_List[i] = NULL;
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Field.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;

		/* guide buffer */

		/* guide raw buffer */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Guide.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Guide.Raw_Buffer_List[i] != NULL)
			free(Buffer_Data.Guide.Raw_Buffer_List[i]);
		Buffer_Data.Guide.Raw_Buffer_List[i] = NULL;
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Raw_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		/* guide reduced buffer */
		/* lock mutex */
		retval = Autoguider_General_Mutex_Lock(&(Buffer_Data.Guide.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
		if(Buffer_Data.Guide.Reduced_Buffer_List[i] != NULL)
			free(Buffer_Data.Guide.Reduced_Buffer_List[i]);
		Buffer_Data.Guide.Reduced_Buffer_List[i] = NULL;
		/* unlock mutex */
		retval = Autoguider_General_Mutex_Unlock(&(Buffer_Data.Guide.Reduced_Mutex_List[i]));
		if(retval == FALSE)
			return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Autoguider_Buffer_Shutdown",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Mutex lock and return the specified buffer. This should subsequently be unlocked by Buffer_One_Raw_Unlock.
 * @param data Pointer to one of the field or guide buffer data structure.
 * @param index The index in the buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to unsigned short, on return, filled with the locked
 *        buffer's start address.
 * @see #Buffer_One_Raw_Unlock
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Raw_Lock(struct Buffer_One_Struct *data,int index,unsigned short **buffer_ptr)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 408;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Lock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 409;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Lock:buffer_ptr was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Locking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d locked.",index);
#endif
	(*buffer_ptr) = data->Raw_Buffer_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Raw_Lock",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Undo a Buffer_One_Raw_Lock operation, to allow another thread to lock the specified buffer.
 * @param data Pointer to one of the field or guide buffer data structure.
 * @param index The index in the buffer list to unlock.
 * @see #Buffer_One_Raw_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Raw_Unlock(struct Buffer_One_Struct *data,int index)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 410;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Unlock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Unlocking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Unlock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Raw_Unlock",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Copy the specified buffer into the supplied buffer. The buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param data A pointer to the Buffer_One_Struct containing the buffer to copy from.
 * @param index The index in the buffer list to copy.
 * @param buffer_ptr A pointer to unsigned short, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 */
static int Buffer_One_Raw_Copy(struct Buffer_One_Struct *data,int index,unsigned short *buffer_ptr,size_t buffer_length)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 411;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Copy:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 412;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Copy:buffer_ptr was NULL.");
		return FALSE;
	}
	/* check buffer length  - remember buffer_length in pixels not bytes! */
	if(buffer_length != (data->Binned_NCols * data->Binned_NRows))
	{
		Autoguider_General_Error_Number = 413;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Copy:buffer_length %d pixels != %d pixels.",
			buffer_length,(data->Binned_NCols * data->Binned_NRows));
		return FALSE;
	}
	/* lock mutex */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Locking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d locked.",index);
#endif
	/* do copy */
	memcpy(buffer_ptr,data->Raw_Buffer_List[index],
	       ((data->Binned_NCols * data->Binned_NRows)*sizeof(unsigned short)));
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Raw_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Raw_Copy",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Mutex lock and return the specified buffer. This should subsequently be unlocked by Buffer_One_Reduced_Unlock.
 * @param data Pointer to one of the field or guide buffer data structure.
 * @param index The index in the buffer list to lock and return.
 * @param buffer_ptr The address of a pointer to unsigned short, on return, filled with the locked
 *        buffer's start address.
 * @see #Buffer_One_Reduced_Unlock
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Reduced_Lock(struct Buffer_One_Struct *data,int index,float **buffer_ptr)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 416;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Lock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 417;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Lock:buffer_ptr was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Locking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Lock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d locked.",index);
#endif
	(*buffer_ptr) = data->Reduced_Buffer_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Reduced_Lock",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Undo a Buffer_One_Reduced_Lock operation, to allow another thread to lock the specified buffer.
 * @param data Pointer to one of the field or guide buffer data structure.
 * @param index The index in the buffer list to unlock.
 * @see #Buffer_One_Reduced_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Lock
 * @see autoguider.general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider.general.html#Autoguider_General_Log
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Reduced_Unlock(struct Buffer_One_Struct *data,int index)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 418;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Unlock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Unlocking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Unlock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Unlock",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Reduced_Unlock",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/**
 * Copy the specified buffer into the supplied buffer. The buffer is first mutex locked
 * by this routine, to stop another thread modifying it.
 * @param data A pointer to the Buffer_One_Struct containing the buffer to copy from.
 * @param index The index in the buffer list to copy.
 * @param buffer_ptr A pointer to unsigned short, on return, filled with the copied
 *        buffer's image data. This should be allocated <b>before</b> being passed to this routine.
 * @param buffer_length The number of <b>pixels</b> in the buffer pointed to by buffer_ptr.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Buffer_Data
 */
static int Buffer_One_Reduced_Copy(struct Buffer_One_Struct *data,int index,float *buffer_ptr,size_t buffer_length)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","started for index %d.",index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 419;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Copy:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
	if(buffer_ptr == NULL)
	{
		Autoguider_General_Error_Number = 420;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Copy:buffer_ptr was NULL.");
		return FALSE;
	}
	/* check buffer length  - remember buffer_length in pixels not bytes! */
	if(buffer_length != (data->Binned_NCols * data->Binned_NRows))
	{
		Autoguider_General_Error_Number = 421;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Copy:buffer_length %d pixels != %d pixels.",
			buffer_length,(data->Binned_NCols * data->Binned_NRows));
		return FALSE;
	}
	/* lock mutex */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Locking mutex %d.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d locked.",index);
#endif
	/* do copy */
	memcpy(buffer_ptr,data->Reduced_Buffer_List[index],
	       ((data->Binned_NCols * data->Binned_NRows)*sizeof(float)));
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("buffer","autoguider_buffer.c","Buffer_One_Reduced_Copy",
				      LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","Mutex %d unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("buffer","autoguider_buffer.c","Buffer_One_Reduced_Copy",
			       LOG_VERBOSITY_VERY_VERBOSE,"BUFFER","finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.5  2011/09/08 09:23:39  cjm
** Added #include <stdlib.h> for malloc under newer kernels.
**
** Revision 1.4  2009/01/30 18:01:33  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.3  2007/01/30 17:35:24  cjm
** Added CCD temperature getters/setters for FITS headers support.
**
** Revision 1.2  2007/01/26 15:29:42  cjm
** Added routines to store and retrieve exposure start times and exposure lengths for the buffers.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
