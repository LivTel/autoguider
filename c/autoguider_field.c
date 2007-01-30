/* autoguider_field.c
** Autoguider field routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_field.c,v 1.11 2007-01-30 17:35:24 cjm Exp $
*/
/**
 * Field routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.11 $
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

#include "ngatcil_ags_sdb.h"

#include "autoguider_buffer.h"
#include "autoguider_cil.h"
#include "autoguider_dark.h"
#include "autoguider_field.h"
#include "autoguider_flat.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"
#include "autoguider_object.h"

/* data types */
/**
 * Data type holding a bounding box point. This consists of the following:
 * <dl>
 * <dt>X</dt> <dd>Integer of X CCD pixel.</dd>
 * <dt>Y</dt> <dd>Integer of X CCD pixel.</dd>
 * </dl>
 */
struct Field_Point_Struct
{
	int X;
	int Y;
};

/**
 * Data type holding a bounding box. This consists of the following:
 * <dl>
 * <dt>Min</dt> <dd>Point (of struct Field_Point_Struct) specifying the min point of the bounds rectangle.</dd>
 * <dt>Max</dt> <dd>Point (of struct Field_Point_Struct) specifying the max point of the bounds rectangle.</dd>
 * </dl>
 * @see #Field_Point_Struct
 */
struct Field_Bounds_Struct
{
	struct Field_Point_Struct Min;
	struct Field_Point_Struct Max;
};

/**
 * Data type holding local data to autoguider_field. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in field images.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in field images.</dd>
 * <dt>Bin_X</dt> <dd>X binning in field images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in field images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in field images.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in field images.</dd>
 * <dt>Exposure_Length</dt> <dd>The exposure length in milliseconds.</dd>
 * <dt>Exposure_Length_Lock</dt> <dd>Boolean determining whether the exposure length can be dynamically changed
 *                                or is 'locked' to a specified length.</dd>
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
 * <dt>Bounds</dt> <dd>The area of the CCD considered auitable for obtaining guide stars.
 *           Ensures guide stars are not too near the edge of the CCD. Of type struct Field_Bounds_Struct.</dd>
 * </dl>
 * @see #Field_Bounds_Struct
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
	int Exposure_Length_Lock;
	int In_Use_Buffer_Index;
	int Last_Buffer_Index;
	int Is_Fielding;
	int Do_Dark_Subtract;
	int Do_Flat_Field;
	int Do_Object_Detect;
	int Field_Id;
	int Frame_Number;
	struct Field_Bounds_Struct Bounds;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_field.c,v 1.11 2007-01-30 17:35:24 cjm Exp $";
/**
 * Instance of field data.
 * @see #Field_Struct
 */
static struct Field_Struct Field_Data = 
{
	0,0,1,1,0,0,
	-1,FALSE,
	-1,1,FALSE,
	TRUE,TRUE,TRUE,
	0,0
};

/* internal functions */
static int Field_Set_Dimensions(void);
static int Field_Reduce(int buffer_index);
static int Field_Check_Done(int *done,int *dark_exposure_length_index);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Field initialisation routine. Loads default values from properties file.
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
	/* get bounds of guide star on CCD */
	retval = CCD_Config_Get_Integer("field.object_bounds.min.x",&(Field_Data.Bounds.Min.X));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 532;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting field object bounds (X Min) failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("field.object_bounds.min.y",&(Field_Data.Bounds.Min.Y));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 533;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting field object bounds (Y Min) failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("field.object_bounds.max.x",&(Field_Data.Bounds.Max.X));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 534;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting field object bounds (X Max) failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("field.object_bounds.max.y",&(Field_Data.Bounds.Max.Y));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 535;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Initialise:"
			"Getting field object bounds (Y Max) failed.");
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
 * @param lock A boolean, if TRUE the exposure length is fixed, otherwise the exposure length can change
 *        during guiding.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Field_Data
 */
int Autoguider_Field_Exposure_Length_Set(int exposure_length,int lock)
{
	if(exposure_length < 0)
	{
		Autoguider_General_Error_Number = 511;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Exposure_Length_Set:"
			"Exposure length out of range(%d).",exposure_length);
		return FALSE;
	}
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(lock))
	{
		Autoguider_General_Error_Number = 529;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Exposure_Length_Set:"
			"Exposure lock is not a boolean(%d).",lock);
		return FALSE;
	}
	Field_Data.Exposure_Length = exposure_length;
	Field_Data.Exposure_Length_Lock = lock;
	return TRUE;
}

/**
 * Perform a field operation.
 * <ul>
 * <li>If the autoguider is already fielding, or already guiding fail.
 * <li>Call Field_Set_Dimensions to load from config the chip dimensions to use.
 * <li>Call CCD_Setup_Dimensions to set the dimensions on the CCD.
 * <li>Setup the initial field exposure length, either from the config item 'ccd.exposure.field.default'
 *     if none has been specified, or use Autoguider_Dark_Get_Exposure_Length_Nearest to get the nearest
 *     suitable exposure length to the set one.
 * <li>Call Autoguider_Flat_Set to setup the correct flat field.
 * <li>Setup the Field_Id and reset the frame number.
 * <li>Loop until fielding is complete:
 *     <ul>
 *     <li>Call Autoguider_Dark_Set to setup the correct dark filename for current exposure length.
 *     <li>Set the In_Use_Buffer_Index to be <b>not</b> the Last_Buffer_Index.
 *     <li>Lock the raw field buffer using Autoguider_Buffer_Raw_Field_Lock.
 *     <li>Call CCD_Exposure_Expose to do the exposure.
 *     <li>Unlock the raw field buffer using Autoguider_Buffer_Raw_Field_Unlock.
 *     <li>Call Field_Reduce on the In_Use_Buffer_Index to reduce the raw data.
 *     <li>Set Last_Buffer_Index to be In_Use_Buffer_Index and switch off In_Use_Buffer_Index.
 *     <li>Call Field_Check_Done to see if we have objects to guide on, this mofies the exposure length and will
 *         quit the field loop if appropriate.
 *     </ul>
 * <li>Switch off the Is_Fielding flag.
 * </ul>
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Field_Data
 * @see #Field_Reduce
 * @see #Field_Set_Dimensions
 * @see #Field_Check_Done
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Field_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Field_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_To_Reduced_Field
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_Exposure_Length_Set
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_Exposure_Start_Time_Set
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_CCD_Temperature_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_dark.html#Autoguider_Dark_Set
 * @see autoguider_dark.html#Autoguider_Dark_Get_Exposure_Length_Nearest
 * @see autoguider_flat.html#Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_guide.html#Autoguider_Guide_Is_Guiding
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Expose
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Get_Exposure_Start_Time
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
int Autoguider_Field(void)
{
	struct CCD_Setup_Window_Struct window;
	enum CCD_TEMPERATURE_STATUS temperature_status;
	double current_temperature;
	struct timespec start_time;
	time_t time_secs;
	struct tm *time_tm = NULL;
	unsigned short *buffer_ptr = NULL;
	int retval,dark_exposure_length_index,done;

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
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_WORKING))
		Autoguider_General_Error(); /* no need to fail */
	if(!Autoguider_CIL_SDB_Packet_Send())
		Autoguider_General_Error(); /* no need to fail */
	/* get dimensions */
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* We could also have a Field_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	if(!Field_Set_Dimensions())
	{
		/* update SDB */
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
	/* setup CCD */
	/* diddly Consider some sort of mutex around CCD calls? */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:Calling CCD_Setup_Dimensions.");
#endif
	retval = CCD_Setup_Dimensions(Field_Data.Unbinned_NCols,Field_Data.Unbinned_NRows,
				      Field_Data.Bin_X,Field_Data.Bin_Y,FALSE,window);
	if(retval == FALSE)
	{
		/* update SDB */
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
	/* RJS suggests starting each time from a configured value as follows: */
	if((Field_Data.Exposure_Length_Lock == FALSE)||(Field_Data.Exposure_Length < 0))
	{
		retval = CCD_Config_Get_Integer("ccd.exposure.field.default",&(Field_Data.Exposure_Length));
		if(retval == FALSE)
		{
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
	/* now ensure suitable dark is available for exposure time, and find exposure length index */
	/* round field exposure length to nearest available dark */
	if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&Field_Data.Exposure_Length,&dark_exposure_length_index))
	{
		/* update SDB */
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
				      "Autoguider_Field:nearest guide exposure length = %d ms (index %d).",
				      Field_Data.Exposure_Length,dark_exposure_length_index);
#endif
	/* ensure the correct flat is loaded */
	retval = Autoguider_Flat_Set(Field_Data.Bin_X,Field_Data.Bin_Y);
	if(retval == FALSE)
	{
		/* update SDB */
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		return FALSE;
	}
	/* initialise Field ID/Frame Number */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Field_Data.Field_Id = /*diddly (time_tm->tm_year*1000000000)+*/(time_tm->tm_yday*1000000)+
		(time_tm->tm_hour*10000)+(time_tm->tm_min*100)+time_tm->tm_sec;
	Field_Data.Frame_Number = 0;
	/* start field loop */
	done = FALSE;
	while(done == FALSE)
	{
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_Exp_Time_Set(Field_Data.Exposure_Length))
			Autoguider_General_Error(); /* no need to fail */
		/* ensure the correct dark is loaded */
		retval = Autoguider_Dark_Set(Field_Data.Bin_X,Field_Data.Bin_Y,Field_Data.Exposure_Length);
		if(retval == FALSE)
		{
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
			/* reset fielding flag */
			Field_Data.Is_Fielding = FALSE;
			return FALSE;
		}
		/* lock out a readout buffer */
		/* Use the buffer index _not_ used by the last completed field readout */
		Field_Data.In_Use_Buffer_Index = (!Field_Data.Last_Buffer_Index);
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
					      "Autoguider_Field:Locking raw field buffer %d.",
					      Field_Data.In_Use_Buffer_Index);
#endif
		retval = Autoguider_Buffer_Raw_Field_Lock(Field_Data.In_Use_Buffer_Index,&buffer_ptr);
		if(retval == FALSE)
		{
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
		/* save the exposure length, start time, CCD temperature for this buffer 
		** for future reference (FITS headers) */
		if(!Autoguider_Buffer_Field_Exposure_Length_Set(Field_Data.In_Use_Buffer_Index,
								Field_Data.Exposure_Length))
			Autoguider_General_Error();
		retval = CCD_Exposure_Get_Exposure_Start_Time(&start_time);
		if(retval == TRUE)
		{
			if(!Autoguider_Buffer_Field_Exposure_Start_Time_Set(Field_Data.In_Use_Buffer_Index,start_time))
				Autoguider_General_Error();
		}
		retval = CCD_Temperature_Get(&current_temperature,&temperature_status);
		if(retval)
		{
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Field:"
					      "current temperature is %.2f C.",current_temperature);
#endif
			if(!Autoguider_Buffer_Field_CCD_Temperature_Set(Field_Data.In_Use_Buffer_Index,
									current_temperature))
				Autoguider_General_Error();
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
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Autoguider_Field:"
					      "Field buffer unlocked, last buffer now %d.",Field_Data.Last_Buffer_Index);
#endif
		/* Check whether we have found suitable objects to guide on */
		if(!Field_Check_Done(&done,&dark_exposure_length_index))
		{
			/* update SDB */
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
			/* reset fielding flag */
			Field_Data.Is_Fielding = FALSE;
			return FALSE;
		}
	}/* end while */
	/* reset fielding flag */
	Field_Data.Is_Fielding = FALSE;
	/* do NOT update SDB back to idle here,
	** so guiding goes straight from working to guiding
	** must manually reset to IDLE all places Autoguider_Field is called on it's own
	** if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
	** 	Autoguider_General_Error(); 
	** if(!Autoguider_CIL_SDB_Packet_Send())
	** 	Autoguider_General_Error(); 
	*/
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
 * @see autoguider_buffer.html#Autoguider_Buffer_Field_CCD_Temperature_Set
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
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Get_Exposure_Start_Time
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 */
int Autoguider_Field_Expose(void)
{
	struct CCD_Setup_Window_Struct window;
	enum CCD_TEMPERATURE_STATUS temperature_status;
	double current_temperature;
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
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_WORKING))
		Autoguider_General_Error(); /* no need to fail */
	if(!Autoguider_CIL_SDB_Packet_Send())
		Autoguider_General_Error(); /* no need to fail */
	/* get dimensions */
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* We could also have a Field_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	if(!Field_Set_Dimensions())
	{
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
		/* reset fielding flag */
		Field_Data.Is_Fielding = FALSE;
		Autoguider_General_Error_Number = 517;
		sprintf(Autoguider_General_Error_String,"Autoguider_Field_Expose:CCD_Setup_Dimensions failed.");
		return FALSE;
	}
	/* exposure length */
	if(Field_Data.Exposure_Length < 0)
	{
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
			Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
			Field_Data.Is_Fielding = FALSE;
			return FALSE;
		}
	}
	/* initialise Field ID/Frame Number */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Field_Data.Field_Id = /*diddly (time_tm->tm_year*1000000000)+*/(time_tm->tm_yday*1000000)+
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
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
	/* save the exposure length and start time for this buffer for future reference (FITS headers) */
	if(!Autoguider_Buffer_Field_Exposure_Length_Set(Field_Data.In_Use_Buffer_Index,
							Field_Data.Exposure_Length))
		Autoguider_General_Error();
	retval = CCD_Exposure_Get_Exposure_Start_Time(&start_time);
	if(retval == TRUE)
	{
		if(!Autoguider_Buffer_Field_Exposure_Start_Time_Set(Field_Data.In_Use_Buffer_Index,start_time))
			Autoguider_General_Error();
	}
	retval = CCD_Temperature_Get(&current_temperature,&temperature_status);
	if(retval)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Field_Expose:"
					      "current temperature is %.2f C.",current_temperature);
#endif
		if(!Autoguider_Buffer_Field_CCD_Temperature_Set(Field_Data.In_Use_Buffer_Index,
								current_temperature))
			Autoguider_General_Error();
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
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
		Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE);
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
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		Autoguider_General_Error(); /* no need to fail */
	if(!Autoguider_CIL_SDB_Packet_Send())
		Autoguider_General_Error(); /* no need to fail */
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

/**
 * Determine whether the specified position on the CCD is inside the field object bounds; i.e.
 * the centroid is far enough away from the edge to be determined a good centroid.
 * See "field.object_bounds.min/max.x/y" in the config file, this test is needed as the TCS
 * rejects centroids outside it's bounds (TCSINITGUI.DAT CONF->GUI, X/YMIN/MAX).
 * @param ccd_x The X position on the CCD.
 * @param ccd_y The Y position on the CCD.
 * @return The routine returnd TRUE if the specified point is inside the bounds, and FALSE if it is outside the bounds.
 * @see #Field_Data
 * @see #Field_Bounds_Struct
 */
int Autoguider_Field_In_Object_Bounds(float ccd_x,float ccd_y)
{
	return ((((int)ccd_x) >= Field_Data.Bounds.Min.X)&&(((int)ccd_x) <= Field_Data.Bounds.Max.X)&&
		(((int)ccd_y) >= Field_Data.Bounds.Min.Y)&&(((int)ccd_y) <= Field_Data.Bounds.Max.Y));
}

/**
 * Get the number of unbinned field columns (Field_Data.Unbinned_NCols).
 * @return The number of columns.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Unbinned_NCols(void)
{
	return Field_Data.Unbinned_NCols;
}

/**
 * Get the number of unbinned field rows (Field_Data.Unbinned_NRows).
 * @return The number of rows.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Unbinned_NRows(void)
{
	return Field_Data.Unbinned_NRows;
}

/**
 * Get the field column binning (Field_Data.Bin_X).
 * @return The X (column) binning.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Bin_X(void)
{
	return Field_Data.Bin_X;
}

/**
 * Get the field row binning (Field_Data.Bin_Y).
 * @return The Y (row) binning.
 * @see #Field_Data
 */
int Autoguider_Field_Get_Bin_Y(void)
{
	return Field_Data.Bin_Y;
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
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Reduce:Did NOT object detect.");
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

/**
 * Routine to check whether to stop field looping, and if not to adjust the Field_Data.Exposure_Length accordingly.
 * <ul>
 * <li>If Field_Data.Do_Object_Detect was FALSE we stop fielding (done = TRUE), no objects can be detected.
 * <li>If Field_Data.Exposure_Length_Lock is TRUE we stop fielding (done = TRUE), we can't change the exposure length
 *     to improve things.
 * <li>We use Autoguider_Object_List_Get_Count to get the number of detected objects.
 * <li>If the object_count is zero, 
 *     <ul>
 *     <li>We double the exposure length.
 *     <li>We use Autoguider_Dark_Get_Exposure_Length_Nearest to get the nearest index to that exposure length.
 *     <li>If the exposure list index hasn't changed neither has the exposure length, we stop fielding (done = TRUE)
 *         assuming we've reached the end of the list. (We should check that really!).
 *     <li>We set (done = FALSE) to retry fielding with a longer exposure length.
 *     </ul>
 * <li>We go through the list of objects:
 *     <ul>
 *     <li>We use Autoguider_Object_List_Get_Object to get the data for this object.
 *     <li>If the object is stellar:
 *         <ul>
 *         <li>If the object has peak counts greater than 100:
 *             <ul>
 *             <li>If the object has peak counts less than 40000:
 *                 <ul>
 *                 <li>If the object is more than 1 FWHM inside the CCD (it is not near the edge), and
 *                     is inside the object bounds (Autoguider_Field_In_Object_Bounds): 
 *                     This is a good object to use.
 *                 </ul>
 *             <li>else the object is too bright. If it is the only detected object:
 *                 <ul>
 *                 <li>We half the exposure length.
 *                 <li>We use Autoguider_Dark_Get_Exposure_Length_Nearest to get the nearest index 
 *                     to that exposure length.
 *                 <li>If the exposure list index hasn't changed neither has the exposure length, 
 *                     we stop fielding (done = TRUE) assuming we've reached the end of the list. 
 *                     (We should check that really!).
 *                 <li>We set (done = FALSE) to retry fielding with a shorter exposure length.
 *                 </ul>
 *             </ul>
 *         <li>else the object is too faint.
 *         </ul>
 *     <li>else the object is not stellar.
 *     <li>
 *     </ul>
 * <li>If the number of good objects are grerater than 1, we return (done is TRUE) as we have at 
 *     least one good guide star.
 * <li>Otherwise we double the exposure length.
 * <li>We use Autoguider_Dark_Get_Exposure_Length_Nearest to get the nearest index to that exposure length.
 * <li>If the exposure list index hasn't changed neither has the exposure length, we stop fielding (done = TRUE)
 *     assuming we've reached the end of the list. (We should check that really!).
 * <li>We set (done = FALSE) to retry fielding with a longer exposure length.
 * </ul>
 * @param done Address of an integer. On exit from this routine, should be set to a boolean value, TRUE meaning
 *        stop the field loop.
 * @param dark_exposure_length_index Address of an integer. On entry contains the index in the dark exposure list
 *       of the last exposure we did. On exit this should be set to the index in the dark exposure list of the 
 *       next field operation to do.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Autoguider_Field_In_Object_Bounds
 * @see #Field_Data
 * @see autoguider_dark.html#Autoguider_Dark_Get_Exposure_Length_Nearest
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_FIELD
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object
 */
static int Field_Check_Done(int *done,int *dark_exposure_length_index)
{
	struct Autoguider_Object_Struct object;
	int retval,object_count,new_dark_exposure_length_index,i,good_object_count;
	float fwhm;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Check_Done:started.");
#endif
	if(done == FALSE)
	{
		Autoguider_General_Error_Number = 530;
		sprintf(Autoguider_General_Error_String,"Field_Check_Done:Done was NULL.");
		return FALSE;
	}
	if(dark_exposure_length_index == FALSE)
	{
		Autoguider_General_Error_Number = 531;
		sprintf(Autoguider_General_Error_String,"Field_Check_Done:dark_exposure_length_index was NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
				      "Field_Check_Done:Current exposure length %d ms (index %d).",
				      Field_Data.Exposure_Length,(*dark_exposure_length_index));
#endif
	(*done) = FALSE;
	/* if we are not object detecting - nothing to determine how good fielding was - stop fielding. */
	if(Field_Data.Do_Object_Detect == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
				       "Field_Check_Done:Object detection off:Nothing to check:finish field.");
#endif
		(*done) = TRUE;
		return TRUE;
	}
	/* if exp length is locked - nothing we can do to improve fielding - stop fielding. */
	if(Field_Data.Exposure_Length_Lock == TRUE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
		      	       "Field_Check_Done:Exposure Length locked:Cannot improve results:finish field.");
#endif
		(*done) = TRUE;
		return TRUE;		
	}
	/* how many objects did we detect? */
	if(!Autoguider_Object_List_Get_Count(&object_count))
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
				       "Field_Check_Done:Failed to get object list count:finish field.");
#endif
		(*done) = TRUE;
		return FALSE;
	}
	/* no objects found - we need to increase exposure length */
	if(object_count < 1)
	{
		Field_Data.Exposure_Length = Field_Data.Exposure_Length*2.0f;
		/* round field exposure length to nearest available dark */
		/* diddly currently assumes this will change the exposure length - 
		** this is only true if list is spaced correctly - need to do something more complicated here */
		if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&Field_Data.Exposure_Length,
								&new_dark_exposure_length_index))
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	       "Field_Check_Done:Failed to get nearest dark exposure length:finish field.");
#endif
			(*done) = TRUE;
			return FALSE;
		}
		/* if we've tried to increase the exposure length but failed, return done. */
		if(new_dark_exposure_length_index == (*dark_exposure_length_index))
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	  "Field_Check_Done:Failed to get new nearest dark exposure length (%d):finish field.",
						      new_dark_exposure_length_index);
#endif
			(*done) = TRUE;
			return TRUE;
		}
		/* otherwise we've successfully increased the exposure length - return not done */
		(*dark_exposure_length_index) = new_dark_exposure_length_index;
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
		      "Field_Check_Done:No objects found:Increased exposure length to %d ms (index %d):retry field.",
					      Field_Data.Exposure_Length,(*dark_exposure_length_index));
#endif
		(*done) = FALSE;
		return TRUE;
	} /* end if no objects found */
	/* have we got a suitable object? */
	/* Is it stellar? Is it near the edge of the CCD? Does it have sufficient counts? */
	good_object_count = 0;
	for(i=0;i<object_count;i++)
	{
		/* get object */
		if(!Autoguider_Object_List_Get_Object(i,&object))
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	       "Field_Check_Done:Failed to get object %d of %d from list:finish field.",
						      i,object_count);
#endif
			(*done) = TRUE;
			return FALSE;
		}
		/* diddly object selection now more relaxed than this in Autoguider_Object_Guide_Object_Get? */
		if(object.Is_Stellar)
		{
			if(object.Peak_Counts > 100)
			{
				if(object.Peak_Counts < 40000)
				{
					fwhm = (object.FWHM_X+object.FWHM_Y)/2.0f;
					if(Autoguider_Field_In_Object_Bounds(object.CCD_X_Position,
									     object.CCD_Y_Position)&&
					   (object.CCD_X_Position > fwhm)&&
					   (object.CCD_X_Position < (Field_Data.Binned_NCols-fwhm))&&
					   (object.CCD_Y_Position > fwhm)&&
					   (object.CCD_Y_Position < (Field_Data.Binned_NRows-fwhm)))
					{
						good_object_count++;
#if AUTOGUIDER_DEBUG > 5
						Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
						    "Field_Check_Done:Object %d of %d:peak counts seem good %.2f.",
									      i,object_count,object.Peak_Counts);
#endif
					}
					else
					{
#if AUTOGUIDER_DEBUG > 5
						Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
						  "Field_Check_Done:Object %d of %d:Object too near edge (%.2f,%.2f).",
							i,object_count,object.CCD_X_Position,object.CCD_Y_Position);
#endif
					}
				}
				else
				{
#if AUTOGUIDER_DEBUG > 5
					Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
						      "Field_Check_Done:Object %d of %d:peak counts too large %.2f.",
							      i,object_count,object.Peak_Counts);
#endif
					if(object_count == 1)/* this is the only object in the list - reduce exp len */
					{
#if AUTOGUIDER_DEBUG > 1
						Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	       "Field_Check_Done:Only object has too many counts:reduce exp len:retry field.");
#endif
						Field_Data.Exposure_Length = Field_Data.Exposure_Length/2.0f;
						/* round field exposure length to nearest available dark */
						/* diddly currently assumes this will change the exposure length - 
						** this is only true if list is spaced correctly - 
						** need to do something more complicated here */
						if(!Autoguider_Dark_Get_Exposure_Length_Nearest(
										&Field_Data.Exposure_Length,
										&new_dark_exposure_length_index))
						{
#if AUTOGUIDER_DEBUG > 1
							Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
				          "Field_Check_Done:Failed to get nearest dark exposure length:finish field.");
#endif
							(*done) = TRUE;
							return FALSE;
						}
						/* if we've tried to increase the exposure length but failed, 
						** return done. */
						if(new_dark_exposure_length_index == (*dark_exposure_length_index))
						{
#if AUTOGUIDER_DEBUG > 1
							Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
							   "Field_Check_Done:"
							   "Failed to get new nearest dark exposure length (%d):"
							   "finish field.",new_dark_exposure_length_index);
#endif
							(*done) = TRUE;
							return TRUE;
						}
						/* otherwise we've successfully increased the exposure length - 
						** return not done */
						(*dark_exposure_length_index) = new_dark_exposure_length_index;
#if AUTOGUIDER_DEBUG > 1
						Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
							   "Field_Check_Done:Object too bright:"
							   "Decreased exposure length to %d ms (index %d):"
							   "retry field.",
							   Field_Data.Exposure_Length,(*dark_exposure_length_index));
#endif
		
						(*done) = FALSE;
						return TRUE;
					}/* end if object_count == 1*/
				}/* end else (object peak counts over 40k */
			}/* end if object peak counts > 100 */
			else
			{
#if AUTOGUIDER_DEBUG > 5
				Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
						      "Field_Check_Done:Object %d of %d:peak counts too small %.2f.",
							      i,object_count,object.Peak_Counts);
#endif
			}
		}
		else
		{
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
		      			      "Field_Check_Done:Object %d of %d is not stellar.",i,object_count);
#endif
		}
	}/* end for */
	/* object is good - return field done */
	if(good_object_count > 0 )
	{
		(*done) = TRUE;
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
					      "Field_Check_Done: %d good objects (of %d) found:finish field.",
					      good_object_count,object_count);
#endif
	}
	else
	{
		Field_Data.Exposure_Length = Field_Data.Exposure_Length*2.0f;
		/* round field exposure length to nearest available dark */
		/* diddly currently assumes this will change the exposure length - 
		** this is only true if list is spaced correctly - need to do something more complicated here */
		if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&Field_Data.Exposure_Length,
								&new_dark_exposure_length_index))
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	       "Field_Check_Done:Failed to get nearest dark exposure length:finish field.");
#endif
			(*done) = TRUE;
			return FALSE;
		}
		/* if we've tried to increase the exposure length but failed, return done. */
		if(new_dark_exposure_length_index == (*dark_exposure_length_index))
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
			      	  "Field_Check_Done:Failed to get new nearest dark exposure length (%d):finish field.",
						      new_dark_exposure_length_index);
#endif
			(*done) = TRUE;
			return TRUE;
		}
		/* otherwise we've successfully increased the exposure length - return not done */
		(*dark_exposure_length_index) = new_dark_exposure_length_index;
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,
		      "Field_Check_Done:No good objects found:"
					      "Increased exposure length to %d ms (index %d):retry field.",
					      Field_Data.Exposure_Length,(*dark_exposure_length_index));
#endif
		(*done) = FALSE;
		return TRUE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_FIELD,"Field_Check_Done:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.10  2007/01/26 19:00:23  cjm
** Changed field exposure length, such that it is always reset to "ccd.exposure.field.default",
** if the exposure length lock is off or the exposure length is < 0 (uninitialised).
**
** Revision 1.9  2007/01/26 15:29:42  cjm
** Set buffers start time/exposure length data.
** Added getters for field dimension data.
**
** Revision 1.8  2006/11/14 18:10:29  cjm
** Added the concept of field objects bounds.
** Selected objcets on the CCD for guiding on should be within a configured set of bounds.
** Added Autoguider_Field_In_Object_Bounds to query these bounds externally to autoguider_field.
**
** Revision 1.7  2006/08/29 13:55:42  cjm
** Replaced AGS states with AGG states.
** Commented out setting AG_STATE to idle at end of field - calling
** routines must do this or guide calls must set new state.
**
** Revision 1.6  2006/07/20 16:08:33  cjm
** Fixed missing ';'.
**
** Revision 1.5  2006/07/20 16:07:40  cjm
** Changed SDB call on failure.
** Added exposure SDB calls.
**
** Revision 1.4  2006/07/20 15:12:46  cjm
** Added SDB submission software.
**
** Revision 1.3  2006/06/20 13:05:21  cjm
** Added exposure length locking.
** Autoguider_Field now loops until some suitable objects have been found.
** Initial implementation to check that suitable objects have been found - this needs rewriting.
**
** Revision 1.2  2006/06/12 19:22:13  cjm
** Fixed Field_ID oddities.
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
