/* autoguider_field.c
** Autoguider field routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_field.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * Field routines for the autoguider program.
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

#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "autoguider_dark.h"
#include "autoguider_field.h"
#include "autoguider_flat.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"
#include "autoguider_object.h"

/* data types */
/**
 * Data type holding local data to autoguider_field for one buffer. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in field images.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in field images.</dd>
 * <dt>Bin_X</dt> <dd>X binning in field images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in field images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in field images.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in field images.</dd>
 * <dt>Exposure_Length</dt> <dd>The exposure length in milliseconds.</dd>
 * <dt>In_Use_Buffer_Index</dt> <dd>The buffer index currently being read out to by an ongoing field operation, or -1.</dd>
 * <dt>Last_Buffer_Index</dt> <dd>The buffer index of the last completed field readout.</dd>
 * <dt>Is_Fielding</dt> <dd>Boolean determining whether Autoguider_Field is running.</dd>
 * <dt>Do_Dark_Subtract</dt> <dd>Boolean determining whether to do dark subtraction when reducing the image.</dd>
 * <dt>Do_Flat_Field</dt> <dd>Boolean determining whether to do flat fielding when reducing the image.</dd>
 * <dt>Do_Object_Detect</dt> <dd>Boolean determining whether to do object detection when reducing the image.</dd>
 * <dt>Field_Id</dt> <dd>A unique integer used as an identifier of a field acquisition session. 
 *       Changed at the start of each field.</dd>
 * <dt>Frame_Number</dt> <dd>The number of each frame took within a field acquisition session. 
 *       Reset at the start of each field.</dd>
 * </dl>
 */
struct Field_Struct
{
	int Unbinned_NCols;
	int Unbinned_NRows;
	int Bin_X;
	int Bin_Y;
	int Binned_NCols;
	int Binned_NRows;
	int Exposure_Length;
	int In_Use_Buffer_Index;
	int Last_Buffer_Index;
	int Is_Fielding;
	int Do_Dark_Subtract;
	int Do_Flat_Field;
	int Do_Object_Detect;
	int Field_Id;
	int Frame_Number;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_field.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";
/**
 * Instance of field data.
 * @see #Field_Struct
 */
static struct Field_Struct Field_Data = 
{
	0,0,1,1,0,0,
	-1,
	-1,1,FALSE,
	TRUE,TRUE,TRUE,
	0,0
};

/* internal functions */
static int Field_Set_Dimensions(void);
static int Field_Reduce(int buffer_index);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * FIeld initialisation routine. Loads default values from properties file.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Field_Data
 */
int Autoguider_Field_Initialise(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Initialise:started.");
#endif
	/* get reduction booleans */
	retval = CCD_Config_Get_Boolean("field.dark_subtract",&(Field_Data.Do_Dark_Subtract));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 513;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting dark subtraction boolean failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Boolean("field.flat_field",&(Field_Data.Do_Flat_Field));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 514;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting flat field boolean failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Boolean("field.object_detect",&(Field_Data.Do_Object_Detect));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 515;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting object detect boolean failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Setup the field exposure length.
 * @param exposure_length The exposure length in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Field_Data
 */
int Autoguider_Field_Exposure_Length_Set(int exposure_length)
{
	if(exposure_length < 0)
	{
		Autoguider_General_Error_Number = 511;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Exposure_Length_Set:"
			"Exposure length out of range(%d).",exposure_length);
		return FALSE;
	}
	Field_Data.Exposure_Length = exposure_length;
	return TRUE;
}

/**
 * Perform a field operation.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see #Field_Reduce
 * @see #Field_Set_Dimensions
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_To_Reduced_Field
 * @see autoguider_dark.html#Autoguider_Dark_Set
 * @see autoguider_flat.html#Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_Is_Guiding
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Expose
 */
int Autoguider_Field(void)
{
	struct CCD_Setup_Window_Struct window;
	struct timespec start_time;
	time_t time_secs;
	struct tm *time_tm = NULL;
	unsigned short *buffer_ptr = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:started.");
#endif
	/* ensure we are not already fielding */
	if(Field_Data.Is_Fielding)
	{
		Autoguider_General_Error_Number = 509;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:Already fielding.");
		return FALSE;
	}
	if(Autoguider_Guide_Is_Guiding())
	{
		Autoguider_General_Error_Number = 510;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:Already guiding.");
		return FALSE;
	}
	Field_Data.Is_Fielding = TRUE;
	/* get dimensions */
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* We could also have a Field_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	if(!Field_Set_Dimensions())
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
	/* setup CCD */
	/* diddly Consider some sort of mutex around CCD calls? */
	/* also state checking, are we already fielding/guiding ? */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:Calling CCD_Setup_Dimensions.");
#endif
	retval = CCD_Setup_Dimensions(Field_Data.Unbinned_NCols,Field_Data.Unbinned_NRows,
				      Field_Data.Bin_X,Field_Data.Bin_Y,FALSE,window);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 504;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:CCD_Setup_Dimensions failed.");
		return FALSE;
	}
	/* default exposure length */
	/* we have to do something more complicated here */
	/* depending on whether we have moved on sky, we should start with the default and increase (loop!)
	** until we get objects on the CCD, or we should re-use the last value? */
	if(Field_Data.Exposure_Length < 0)
	{
		retval = CCD_Config_Get_Integer("ccd.exposure.field.default",&(Field_Data.Exposure_Length));
		if(retval == FALSE)
		{
			/* reset fielding flag */
			Field_Data.Is_Fielding = FALSE;
			Autoguider_General_Error_Number = 505;
			sprintf(Autoguider_General_Error_String,"Autoguider_Field:"
				"Getting default field exposure length failed.");
			return FALSE;
		}
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
					      "Autoguider_Field:Exposure Length set to default:%d ms.",
					      Field_Data.Exposure_Length);
#endif
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
					      "Autoguider_Field:Using current Exposure Length:%d ms.",
					      Field_Data.Exposure_Length);
#endif
	}
	/* ensure the correct dark and flat is loaded */
	retval = Autoguider_Dark_Set(Field_Data.Bin_X,Field_Data.Bin_Y,Field_Data.Exposure_Length);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
	retval = Autoguider_Flat_Set(Field_Data.Bin_X,Field_Data.Bin_Y);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
	/* initialise Field ID/Frame Number */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Field_Data.Field_Id = (time_tm->tm_year*1000000000)+(time_tm->tm_yday*1000000)+
		(time_tm->tm_hour*10000)+(time_tm->tm_min*100)+time_tm->tm_sec;
	Field_Data.Frame_Number = 0;
	/* lock out a readout buffer */
	/* Use the buffer index _not_ used by the last completed field readout */
	Field_Data.In_Use_Buffer_Index = (!Field_Data.Last_Buffer_Index);
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:Locking raw field buffer %d.",
				      Field_Data.In_Use_Buffer_Index);
#endif
	retval = Autoguider_Buffer_Raw_Field_Lock(Field_Data.In_Use_Buffer_Index,&buffer_ptr);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		Autoguider_General_Error_Number = 506;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:Autoguider_Buffer_Raw_Field_Lock failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:field buffer locked.");
#endif
	/* do a field */
	start_time.tv_sec = 0;
	start_time.tv_nsec = 0;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:"
				      "Calling CCD_Exposure_Expose with exposure length %d ms.",
				      Field_Data.Exposure_Length);
#endif
	retval = CCD_Exposure_Expose(TRUE,start_time,Field_Data.Exposure_Length,buffer_ptr,
				     Autoguider_Buffer_Get_Field_Pixel_Count());
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* attempt buffer unlock, and reset in use index */
		Autoguider_Buffer_Raw_Field_Unlock(Field_Data.In_Use_Buffer_Index);
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		Autoguider_General_Error_Number = 507;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:CCD_Exposure_Expose failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 7
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:exposure completed.");
#endif
	/* unlock readout buffer */
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:"
				      "Unlocking raw field buffer %d.",
				      Field_Data.In_Use_Buffer_Index);
#endif
	retval = Autoguider_Buffer_Raw_Field_Unlock(Field_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 508;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field:Autoguider_Buffer_Raw_Field_Unlock failed.");
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		return FALSE;
	}
	/* reduce data */
	/* Field_Reduce calls
	** Autoguider_Buffer_Raw_To_Reduced_Field which re-locks the raw field mutex, so has to be called
	** after Autoguider_Buffer_Raw_Field_Unlock (as we are using fast mutexs that fails on multiple locks
	** by the same thread) */
	retval = Field_Reduce(Field_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		return FALSE;
	}
	/* do object detection here */

	/* reset buffer indexs */
	Field_Data.Last_Buffer_Index = Field_Data.In_Use_Buffer_Index;
	Field_Data.In_Use_Buffer_Index = -1;
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:"
				      "Field buffer unlocked, last buffer now %d.",Field_Data.Last_Buffer_Index);
#endif
	/* reset fielding flag */
	Field_Data.Is_Fielding = FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:finished.");
#endif
	return TRUE;
}

/**
 * Perform a single field exposure operation. Uses the field buffer and configuration.
 * Used to provide the autoguider with a simple object imaging capability.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see #Field_Reduce
 * @see #Field_Get_Dimensions
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_To_Reduced_Field
 * @see autoguider_dark.html#Autoguider_Dark_Set
 * @see autoguider_flat.html#Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_Is_Guiding
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Expose
 */
int Autoguider_Field_Expose(void)
{
	struct CCD_Setup_Window_Struct window;
	struct timespec start_time;
	time_t time_secs;
	struct tm *time_tm = NULL;
	unsigned short *buffer_ptr = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:started.");
#endif
	/* ensure we are not already fielding */
	if(Field_Data.Is_Fielding)
	{
		Autoguider_General_Error_Number = 512;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:Already fielding.");
		return FALSE;
	}
	if(Autoguider_Guide_Is_Guiding())
	{
		Autoguider_General_Error_Number = 516;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:Already guiding.");
		return FALSE;
	}
	Field_Data.Is_Fielding = TRUE;
	/* get dimensions */
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* We could also have a Field_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	if(!Field_Set_Dimensions())
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;		
		return FALSE;
	}
	/* setup CCD */
	/* diddly Consider some sort of mutex around CCD calls? */
	/* also state checking, are we already fielding/guiding ? */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:"
			       "Calling CCD_Setup_Dimensions.");
#endif
	retval = CCD_Setup_Dimensions(Field_Data.Unbinned_NCols,Field_Data.Unbinned_NRows,
				      Field_Data.Bin_X,Field_Data.Bin_Y,FALSE,window);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 517;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:CCD_Setup_Dimensions failed.");
		return FALSE;
	}
	/* exposure length */
	if(Field_Data.Exposure_Length < 0)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 518;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:No exposure length set.");
			return FALSE;
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
					      "Autoguider_Field_Expose:Using current Exposure Length:%d ms.",
					      Field_Data.Exposure_Length);
#endif
	}
	/* ensure the correct dark and flat is loaded */
	if(Field_Data.Do_Dark_Subtract)
	{
		/* if the dark does not exists, don't worry, just turn off dark subtraction */
		retval = Autoguider_Dark_Set(Field_Data.Bin_X,Field_Data.Bin_Y,Field_Data.Exposure_Length);
		if(retval == FALSE)
		{
			/* reset fielding flag */
			Field_Data.Is_Fielding = FALSE;
			return FALSE;
		}
	}
	if(Field_Data.Do_Flat_Field)
	{
		/* if the flat does not exists, don't worry, just turn off flat fielding */
		retval = Autoguider_Flat_Set(Field_Data.Bin_X,Field_Data.Bin_Y);
		if(retval == FALSE)
		{
			/* reset fielding flag */
			Field_Data.Is_Fielding = FALSE;
			return FALSE;
		}
	}
	/* initialise Field ID/Frame Number */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Field_Data.Field_Id = (time_tm->tm_year*1000000000)+(time_tm->tm_yday*1000000)+
		(time_tm->tm_hour*10000)+(time_tm->tm_min*100)+time_tm->tm_sec;
	Field_Data.Frame_Number = 0;
	/* lock out a readout buffer */
	/* Use the buffer index _not_ used by the last completed field readout */
	Field_Data.In_Use_Buffer_Index = (!Field_Data.Last_Buffer_Index);
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:"
				      "Locking raw field buffer %d.",Field_Data.In_Use_Buffer_Index);
#endif
	retval = Autoguider_Buffer_Raw_Field_Lock(Field_Data.In_Use_Buffer_Index,&buffer_ptr);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		Autoguider_General_Error_Number = 519;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:"
			"Autoguider_Buffer_Raw_Field_Lock failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:field buffer locked.");
#endif
	/* do a field */
	start_time.tv_sec = 0;
	start_time.tv_nsec = 0;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:"
				      "Calling CCD_Exposure_Expose with exposure length %d ms.",
				      Field_Data.Exposure_Length);
#endif
	retval = CCD_Exposure_Expose(TRUE,start_time,Field_Data.Exposure_Length,buffer_ptr,
				     Autoguider_Buffer_Get_Field_Pixel_Count());
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* attempt buffer unlock, and reset in use index */
		Autoguider_Buffer_Raw_Field_Unlock(Field_Data.In_Use_Buffer_Index);
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		Autoguider_General_Error_Number = 520;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:CCD_Exposure_Expose failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 7
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:exposure completed.");
#endif
	/* unlock readout buffer */
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:"
				      "Unlocking raw field buffer %d.",
				      Field_Data.In_Use_Buffer_Index);
#endif
	retval = Autoguider_Buffer_Raw_Field_Unlock(Field_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 521;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:"
			"Autoguider_Buffer_Raw_Field_Unlock failed.");
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		return FALSE;
	}
	/* reduce data */
	/* Field_Reduce calls
	** Autoguider_Buffer_Raw_To_Reduced_Field which re-locks the raw field mutex, so has to be called
	** after Autoguider_Buffer_Raw_Field_Unlock (as we are using fast mutexs that fails on multiple locks
	** by the same thread) */
	retval = Field_Reduce(Field_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		/* reset in use buffer index */
		Field_Data.In_Use_Buffer_Index = -1;
		return FALSE;
	}
	/* reset buffer indexs */
	Field_Data.Last_Buffer_Index = Field_Data.In_Use_Buffer_Index;
	Field_Data.In_Use_Buffer_Index = -1;
#if AUTOGUIDER_DEBUG > 9
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:"
				      "Field buffer unlocked, last buffer now %d.",Field_Data.Last_Buffer_Index);
#endif
	/* reset fielding flag */
	Field_Data.Is_Fielding = FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field_Expose:finished.");
#endif
	return TRUE;
}

/**
 * Return whether a field operation is currently underway.
 * @return The routine returns TRUE if Autoguider_Field is currently fielding, FALSE otherwise.
 * @see #Autoguider_Field
 * @see #Field_Data
 */
int Autoguider_Field_Is_Fielding(void)
{
	return Field_Data.Is_Fielding;
}

/**
 * Return the last buffer index used by a field operation.
 * @return The routine returns a buffer index, this can be -1 if no fielding has yet been done.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Last_Buffer_Index(void)
{
	return Field_Data.Last_Buffer_Index;
}

/**
 * Routine to set whether to dark subtract when fielding.
 * @param doit If TRUE, do dark subtraction, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Field_Set_Do_Dark_Subtract(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 526;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Set_Do_Dark_Subtract:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Field_Data.Do_Dark_Subtract = doit;
	return TRUE;
}

/**
 * Routine to set whether we are dark subtracting when fielding.
 * @return The routine returns TRUE if we are dark subtracting when fielding, and FALSE if we are not.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Do_Dark_Subtract(void)
{
	return Field_Data.Do_Dark_Subtract;
}

/**
 * Routine to set whether to flat field when fielding.
 * @param doit If TRUE, do flat fielding, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Field_Set_Do_Flat_Field(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 527;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Set_Do_Flat_Field:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Field_Data.Do_Flat_Field = doit;
	return TRUE;
}

/**
 * Routine to set whether we are flat fielding when fielding.
 * @param doit If TRUE, do flat fielding, otherwise don't.
 * @return The routine returns TRUE if we are flat fielding when guiding, and FALSE if we are not.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Do_Flat_Field(void)
{
	return Field_Data.Do_Dark_Subtract;
}

/**
 * Routine to set whether to object detect when fielding.
 * @param doit If TRUE, do object detection, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Field_Set_Do_Object_Detect(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 528;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Set_Do_Object_Detect:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Field_Data.Do_Object_Detect = doit;
	return TRUE;
}

/**
 * Routine to set whether we are object detecting when fielding.
 * @param doit If TRUE, do dark subtraction, otherwise don't.
 * @return The routine returns TRUE if we are object detecting when guiding, and FALSE if we are not.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Do_Object_Detect(void)
{
	return Field_Data.Do_Object_Detect;
}

/**
 * Get the current exposure length.
 * @return The current field exposure length in milliseconds.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Exposure_Length(void)
{
	return Field_Data.Exposure_Length;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Routine to load field dimensions from the config file into the Field_Data.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Field_Data
 * @see #Field_Struct
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
static int Field_Set_Dimensions(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Set_Dimensions:started.");
#endif
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* We could also have a Field_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	retval = CCD_Config_Get_Integer("ccd.field.ncols",&(Field_Data.Unbinned_NCols));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 522;
		sprintf(Autoguider_General_Error_String,"Field_Set_Dimensions:Getting field NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.nrows",&(Field_Data.Unbinned_NRows));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 523;
		sprintf(Autoguider_General_Error_String,"Field_Set_Dimensions:Getting field NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.x_bin",&(Field_Data.Bin_X));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 524;
		sprintf(Autoguider_General_Error_String,"Field_Set_Dimensions:"
			"Getting field X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.field.y_bin",&(Field_Data.Bin_Y));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 525;
		sprintf(Autoguider_General_Error_String,"Field_Set_Dimensions:"
			"Getting field Y Binning failed.");
		return FALSE;
	}
	Field_Data.Binned_NCols = Field_Data.Unbinned_NCols / Field_Data.Bin_X;
	Field_Data.Binned_NRows = Field_Data.Unbinned_NRows / Field_Data.Bin_Y;
#if AUTOGUIDER_DEBUG > 3
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Set_Dimensions:finished.");
#endif
	return TRUE;
}
/**
 * Internal routine to reduced the field data in the Field_Data.In_Use_Buffer_Index buffer.
 * The raw/reduced mutexs should <b>not</b> be locked when this is called.
 * @param buffer_index The buffer index to reduce, should usually be called with Field_Data.In_Use_Buffer_Index.
 *       Passed as a parameter to potentially allow this routine to run in a different thread.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_To_Reduced_Field
 * @see autoguider_buffer.html#Autoguider_Buffer_Reduced_Field_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Reduced_Field_Unlock
 * @see autoguider_dark.html#Autoguider_Dark_Subtract
 * @see autoguider_flat.html#Autoguider_Flat_Field
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_object.html#Autoguider_Object_Detect
 */
static int Field_Reduce(int buffer_index)
{
	struct CCD_Setup_Window_Struct blank_window = {-1,-1,-1,-1};
	float *reduced_buffer_ptr = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce(%d):started.",buffer_index);
#endif
	/* copy raw data to reduced data */
	retval = Autoguider_Buffer_Raw_To_Reduced_Field(buffer_index);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
				       "Autoguider_Buffer_Raw_To_Reduced_Field failed.");
#endif
		return FALSE;
	}
	/* lock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Field_Lock(buffer_index,&reduced_buffer_ptr);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
				       "Autoguider_Buffer_Reduced_Field_Lock(%d) failed.",buffer_index);
#endif
		return FALSE;
	}
	/* dark subtraction */
	if(Field_Data.Do_Dark_Subtract)
	{
		retval = Autoguider_Dark_Subtract(reduced_buffer_ptr,Autoguider_Buffer_Get_Field_Pixel_Count(),
						  Field_Data.Binned_NCols,Field_Data.Binned_NRows,
						  FALSE,blank_window);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Field_Unlock(buffer_index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
					       "Autoguider_Dark_Subtract failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
				       "Did NOT subtract dark.");
#endif
	}
	/* flat field */
	if(Field_Data.Do_Flat_Field)
	{
		retval = Autoguider_Flat_Field(reduced_buffer_ptr,Autoguider_Buffer_Get_Field_Pixel_Count(),
						  Field_Data.Binned_NCols,Field_Data.Binned_NRows,
						  FALSE,blank_window);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Field_Unlock(buffer_index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
					       "Autoguider_Flat_Field failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
				       "Did NOT flat field.");
#endif
	}
	/* object detect */
	if(Field_Data.Do_Object_Detect)
	{
		retval = Autoguider_Object_Detect(reduced_buffer_ptr,Field_Data.Binned_NCols,Field_Data.Binned_NRows,
						  0,0,TRUE,Field_Data.Field_Id,Field_Data.Frame_Number);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Field_Unlock(buffer_index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
					       "Autoguider_Object_Detect failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:"
				       "Did NOT object detect.");
#endif
	}
	/* unlock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Field_Unlock(buffer_index);
	if(retval == FALSE)
	{
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
