/* autoguider_buffer.c
** Autoguider buffer routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_buffer.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * Buffer routines for the autoguider program.
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
#include <pthread.h> /* mutex */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
 *     the actual raw field image buffers.
 * <dt>Raw_Mutex_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT mutexs to protect the raw buffers 
 *     from multiple access.
 * <dt>Reduced_Buffer_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT pointer to allocated arrays of unsigned shorts,
 *     the actual reduced field image buffers.
 * <dt>Reduced_Mutex_List</dt> <dd>Array of AUTOGUIDER_BUFFER_COUNT mutexs to protect the reduced buffers 
 *     from multiple access.
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
	/* diddly timestamps? */
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
static char rcsid[] = "$Id: autoguider_buffer.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";
/**
 * Instance of buffer data.
 * @see #Buffer_Struct
 */
static struct Buffer_Struct Buffer_Data = 
{
	{
		0,0,1,1,0,0,
		{NULL,NULL},
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER},
		{NULL,NULL},
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}
	},
	{
		0,0,1,1,0,0,
		{NULL,NULL},
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER},
		{NULL,NULL},
		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER}
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Buffer_Initialise(void)
{
	int retval,ncols,nrows,x_bin,y_bin;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Initialise:started.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Initialise:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Set_Field_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Set_Field_Dimension:started.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Set_Field_Dimension:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
int Autoguider_Buffer_Set_Guide_Dimension(int ncols,int nrows,int x_bin,int y_bin)
{
	int i,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Set_Guide_Dimension:started.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Set_Guide_Dimension:finished.");
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
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 */
int Autoguider_Buffer_Raw_To_Reduced_Field(int index)
{
	unsigned short *raw_buffer_ptr = NULL;
	unsigned short *current_raw_buffer_ptr = NULL;
	float *reduced_buffer_ptr = NULL;
	float *current_reduced_buffer_ptr = NULL;
	int pixel_count,i;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Raw_To_Reduced_Field:started.");
#endif
	if(!Buffer_One_Raw_Lock(&(Buffer_Data.Field),index,&raw_buffer_ptr))
		return FALSE;
	if(!Buffer_One_Reduced_Lock(&(Buffer_Data.Field),index,&reduced_buffer_ptr))
	{
		Buffer_One_Raw_Unlock(&(Buffer_Data.Field),index);
		return FALSE;
	}
	pixel_count = Autoguider_Buffer_Get_Field_Pixel_Count();
	/* diddly rjs recommends pointer interation as faster */
	/*
	for(i=0;i<pixel_count;i++)
	{
		reduced_buffer_ptr[i] = (float)(raw_buffer_ptr[i]);
	}
	*/
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Raw_To_Reduced_Field:finished.");
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
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 */
int Autoguider_Buffer_Raw_To_Reduced_Guide(int index)
{
	unsigned short *raw_buffer_ptr = NULL;
	float *reduced_buffer_ptr = NULL;
	int pixel_count,i;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Raw_To_Reduced_Guide:started.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Raw_To_Reduced_Guide:finished.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Shutdown:started.");
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
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Autoguider_Buffer_Shutdown:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Raw_Lock(struct Buffer_One_Struct *data,int index,unsigned short **buffer_ptr)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Lock(%d):started.",
				      index);
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Lock(%d):"
				      "Locking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Lock(%d):"
				      "Mutex locked.",index);
#endif
	(*buffer_ptr) = data->Raw_Buffer_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Lock:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Raw_Unlock(struct Buffer_One_Struct *data,int index)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Unlock(%d):started.",
				      index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 410;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Raw_Unlock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Unlock(%d):"
				      "Unlocking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Unlock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Unlock(%d):"
				      "Mutex unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Unlock:finished.");
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Copy(%d):started.",index);
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Copy(%d):"
				      "Locking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Copy(%d):"
				      "Mutex locked.",index);
#endif
	/* do copy */
	memcpy(buffer_ptr,data->Raw_Buffer_List[index],
	       ((data->Binned_NCols * data->Binned_NRows)*sizeof(unsigned short)));
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(data->Raw_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Copy(%d):"
				      "Mutex unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Raw_Copy:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Reduced_Lock(struct Buffer_One_Struct *data,int index,float **buffer_ptr)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Lock(%d):started.",
				      index);
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Lock(%d):"
				      "Locking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Lock(%d):"
				      "Mutex locked.",index);
#endif
	(*buffer_ptr) = data->Reduced_Buffer_List[index];
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Lock:finished.");
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
 * @see autoguider.general.html#AUTOGUIDER_GENERAL_LOG_BIT_BUFFER
 * @see autoguider.general.html#Autoguider_General_Error_Number
 * @see autoguider.general.html#Autoguider_General_Error_String
 */
static int Buffer_One_Reduced_Unlock(struct Buffer_One_Struct *data,int index)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Unlock(%d):started.",
				      index);
#endif
	if((index < 0)||(index >= AUTOGUIDER_BUFFER_COUNT))
	{
		Autoguider_General_Error_Number = 418;
		sprintf(Autoguider_General_Error_String,"Buffer_One_Reduced_Unlock:"
			"Index %d out of range (%d,%d).",index,0,AUTOGUIDER_BUFFER_COUNT);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Unlock(%d):"
				      "Unlocking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Unlock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Unlock(%d):"
				      "Mutex unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Unlock:finished.");
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Copy(%d):started.",index);
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
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Copy(%d):"
				      "Locking mutex.",index);
#endif
	retval = Autoguider_General_Mutex_Lock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Copy(%d):"
				      "Mutex locked.",index);
#endif
	/* do copy */
	memcpy(buffer_ptr,data->Reduced_Buffer_List[index],
	       ((data->Binned_NCols * data->Binned_NRows)*sizeof(float)));
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(data->Reduced_Mutex_List[index]));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Copy(%d):"
				      "Mutex unlocked.",index);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_BUFFER,"Buffer_One_Reduced_Copy:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
