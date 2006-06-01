/* autoguider_guide.c
** Autoguider guide routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_guide.c,v 1.1 2006-06-01 15:18:30 cjm Exp $
*/
/**
 * Guide routines for the autoguider program.
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
#include <pthread.h>
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
 * Data type holding local data to autoguider_guide for one buffer. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in the <b>full frame</b>.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in <b>full frame</b>.</dd>
 * <dt>Bin_X</dt> <dd>X binning in guide images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in guide images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in <b>full frame</b>.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in <b>full frame</b>.</dd>
 * <dt>Window</dt> <dd>The guide window, an instance of CCD_Setup_Window_Struct .</dd>
 * <dt>Exposure_Length</dt> <dd>The exposure length in milliseconds.</dd>
 * <dt>Exposure_Length_Lock</dt> <dd>Boolean determining whether the exposure length can be dynamically changed
 *                                or is 'locked' to a specified length.</dd>
 * <dt>In_Use_Buffer_Index</dt> <dd>The buffer index currently being read out to by an ongoing guide operation, or -1.</dd>
 * <dt>Last_Buffer_Index</dt> <dd>The buffer index of the last completed guide readout.</dd>
 * <dt>Quit_Guiding</dt> <dd>Boolean to be set to tell the guide thread to stop.</dd>
 * <dt>Is_Guiding</dt> <dd>Boolean determining whether the guide thread is running.</dd>
 * <dt>Do_Dark_Subtract</dt> <dd>Boolean determining whether to do dark subtraction when reducing the image.</dd>
 * <dt>Do_Flat_Field</dt> <dd>Boolean determining whether to do flat fielding when reducing the image.</dd>
 * <dt>Do_Object_Detect</dt> <dd>Boolean determining whether to do object detection when reducing the image.</dd>
 * <dt>Guide_Id</dt> <dd>A unique integer used as an identifier of a guide session. 
 *       Changed at the start of each guide on.</dd>
 * <dt>Frame_Number</dt> <dd>The number of each frame took within a guide session. 
 *       Reset at the start of each guide on.</dd>
 * </dl>
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct 
 */
struct Guide_Struct
{
	int Unbinned_NCols;
	int Unbinned_NRows;
	int Bin_X;
	int Bin_Y;
	int Binned_NCols;
	int Binned_NRows;
	struct CCD_Setup_Window_Struct Window;
	int Exposure_Length;
	int Exposure_Length_Lock;
	int In_Use_Buffer_Index;
	int Last_Buffer_Index;
	int Quit_Guiding;
	int Is_Guiding;
	int Do_Dark_Subtract;
	int Do_Flat_Field;
	int Do_Object_Detect;
	int Guide_Id;
	int Frame_Number;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_guide.c,v 1.1 2006-06-01 15:18:30 cjm Exp $";
/**
 * Instance of guide data.
 * @see #Guide_Struct
 */
static struct Guide_Struct Guide_Data = 
{
	0,0,1,1,0,0,
	{0,0,0,0},
	-1,FALSE,
	-1,1,FALSE,FALSE,
	TRUE,TRUE,TRUE,
	0,0
};

/* internal routines */
static void *Guide_Thread(void *user_arg);
static int Guide_Reduce(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Guide initialisation routine. Loads default values from properties file.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 */
int Autoguider_Guide_Initialise(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Initialise:started.");
#endif
	/* get reduction booleans */
	retval = CCD_Config_Get_Boolean("guide.dark_subtract",&(Guide_Data.Do_Dark_Subtract));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 716;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting dark subtraction boolean failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Boolean("guide.flat_field",&(Guide_Data.Do_Flat_Field));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 717;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting flat field boolean failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Boolean("guide.object_detect",&(Guide_Data.Do_Object_Detect));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 718;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting object detect boolean failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Initialise:finished.");
#endif
	return TRUE;
}

/**
 * Setup the autoguider window.
 * @param sx The start X position.
 * @param sy The start Y position.
 * @param ex The end X position.
 * @param ey The end Y position.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 */
int Autoguider_Guide_Window_Set(int sx,int sy,int ex,int ey)
{
	if((sx < 0)||(sy < 0)||(ex < 0)||(ey < 0))
	{
		Autoguider_General_Error_Number = 700;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d).",sx,sy,ex,ey);
		return FALSE;
	}
	/* check sx < ex, sy < ey */
	/* check vs overall dimensions - what if they are not initialised yet! */
	Guide_Data.Window.X_Start = sx;
	Guide_Data.Window.Y_Start = sy;
	Guide_Data.Window.X_End = ex;
	Guide_Data.Window.Y_End = ey;
	return TRUE;
}

/**
 * Setup the guide exposure length, and whether it is dynamically changable during a guide session.
 * @param exposure_length The exposure length in milliseconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 */
int Autoguider_Guide_Exposure_Length_Set(int exposure_length,int lock)
{
	if(exposure_length < 0)
	{
		Autoguider_General_Error_Number = 701;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Exposure_Length_Set:"
			"Exposure length out of range(%d).",exposure_length);
		return FALSE;
	}
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(lock))
	{
		Autoguider_General_Error_Number = 721;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Exposure_Length_Set:"
			"Exposure lock is not a boolean(%d).",lock);
		return FALSE;
	}
	Guide_Data.Exposure_Length = exposure_length;
	Guide_Data.Exposure_Length_Lock = lock;
	return TRUE;
}

/**
 * Routine to start the guide loop (thread).
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Autoguider_Guide_Is_Guiding
 * @see #Guide_Thread
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_dark.html#Autoguider_Dark_Set
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_flat.html#Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GUIDE
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Guide_On(void)
{
	time_t time_secs;
	struct tm *time_tm = NULL;
	pthread_t guide_thread;
	pthread_attr_t attr;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_On:started.");
#endif
	if(Autoguider_Guide_Is_Guiding() == TRUE)
	{
		Autoguider_General_Error_Number = 702;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:"
			"Failed to start guiding:Guiding loop already running.");
		return FALSE;
	}
	if(Autoguider_Field_Is_Fielding() == TRUE)
	{
		Autoguider_General_Error_Number = 703;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:"
			"Failed to start guiding:Fielding operation already in progress.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_On:Getting Dimensions.");
#endif
	/* get dimensions */
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* Also see Autoguider_Field (autogudier_field.c) */ 
	/* We could also have a Guide_Initialise that did this, but reloading from config every time may be good,
	** if we are going to allow dynamic reloading of the config file. */
	retval = CCD_Config_Get_Integer("ccd.guide.ncols",&(Guide_Data.Unbinned_NCols));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 704;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:Getting guide NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.nrows",&(Guide_Data.Unbinned_NRows));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 705;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:Getting guide NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.x_bin",&(Guide_Data.Bin_X));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 706;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:Getting guide X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.y_bin",&(Guide_Data.Bin_Y));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 707;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:Getting guide Y Binning failed.");
		return FALSE;
	}
	Guide_Data.Binned_NCols = Guide_Data.Unbinned_NCols / Guide_Data.Bin_X;
	Guide_Data.Binned_NRows = Guide_Data.Unbinned_NRows / Guide_Data.Bin_Y;
	/* default exposure length */
	/* we have to do something more complicated here */
	/* depending on whether we have moved on sky, we should start with the default and increase (loop!)
	** until we get objects on the CCD, or we should re-use the last value? */
	if(Guide_Data.Exposure_Length < 0)
	{
		retval = CCD_Config_Get_Integer("ccd.exposure.guide.default",&(Guide_Data.Exposure_Length));
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 708;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:"
				"Getting default guide exposure length failed.");
			return FALSE;
		}
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,
					      "Autoguider_Guide_On:Exposure Length set to default:%d ms.",
					      Guide_Data.Exposure_Length);
#endif
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,
					      "Autoguider_Guide_On:Using current Exposure Length:%d ms.",
					      Guide_Data.Exposure_Length);
#endif
	}
	/* ensure the correct dark and flat is loaded */
	retval = Autoguider_Dark_Set(Guide_Data.Bin_X,Guide_Data.Bin_Y,Guide_Data.Exposure_Length);
	if(retval == FALSE)
		return FALSE;
	retval = Autoguider_Flat_Set(Guide_Data.Bin_X,Guide_Data.Bin_Y);
	if(retval == FALSE)
		return FALSE;
	/* initialise thread quit variable */
	Guide_Data.Quit_Guiding = FALSE;
	/* initialise Guide ID */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Guide_Data.Guide_Id = (time_tm->tm_year*1000000000)+(time_tm->tm_yday*1000000)+
		(time_tm->tm_hour*10000)+(time_tm->tm_min*100)+time_tm->tm_sec;
	/* spawn guide thread */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	retval = pthread_create(&guide_thread,&attr,&Guide_Thread,(void *)NULL);
	if(retval != 0)
	{
		Autoguider_General_Error_Number = 709;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_On:Failed to create guide thread.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_On:finished.");
#endif
	return TRUE;
}

/**
 * Routine to stop the guide loop (thread).
 * Sets Guide_Data.Quit_Guiding.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Autoguider_Guide_Is_Guiding
 */
int Autoguider_Guide_Off(void)
{
	if(Autoguider_Guide_Is_Guiding() == FALSE)
	{
		Autoguider_General_Error_Number = 710;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Off:"
			"Failed to stop guiding:Guiding loop not running.");
		return FALSE;
	}
	Guide_Data.Quit_Guiding = TRUE;
	return TRUE;
}

/**
 * Return whether a guide operation is currently underway.
 * @return The routine returns TRUE if currently fielding, FALSE otherwise.
 * @see #Autoguider_Guide
 * @see #Guide_Data
 */
int Autoguider_Guide_Is_Guiding(void)
{
	return Guide_Data.Is_Guiding;
}

/**
 * Return the last buffer index used by a field operation.
 * @return The routine returns a buffer index, this can be -1 if no fielding has yet been done.
 * @see #Guide_Data
 */
int Autoguider_Guide_Get_Last_Buffer_Index(void)
{
	return Guide_Data.Last_Buffer_Index;
}

/**
 * Routine to set whether to dark subtract when guiding.
 * @param doit If TRUE, do dark subtraction, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Guide_Set_Do_Dark_Subtract(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 715;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Do_Dark_Subtract:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Guide_Data.Do_Dark_Subtract = doit;
	return TRUE;
}

/**
 * Routine to set whether we are dark subtracting when guiding.
 * @return The routine returns TRUE if we are dark subtracting when guiding, and FALSE if we are not.
 * @see #Guide_Data
 */
int Autoguider_Guide_Get_Do_Dark_Subtract(void)
{
	return Guide_Data.Do_Dark_Subtract;
}

/**
 * Routine to set whether to flat field when guiding.
 * @param doit If TRUE, do flat fielding, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Guide_Set_Do_Flat_Field(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 719;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Do_Flat_Field:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Guide_Data.Do_Flat_Field = doit;
	return TRUE;
}

/**
 * Routine to set whether we are flat fielding when guiding.
 * @return The routine returns TRUE if we are flat fielding when guiding, and FALSE if we are not.
 * @see #Guide_Data
 */
int Autoguider_Guide_Get_Do_Flat_Field(void)
{
	return Guide_Data.Do_Dark_Subtract;
}

/**
 * Routine to set whether to object detect when guiding.
 * @param doit If TRUE, do object detection, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Guide_Set_Do_Object_Detect(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 720;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Do_Object_Detect:Illegal argument %d.",
			doit);
		return FALSE;
	}
	Guide_Data.Do_Object_Detect = doit;
	return TRUE;
}

/**
 * Routine to set whether we are object detecting when guiding.
 * @return The routine returns TRUE if we are object detecting when guiding, and FALSE if we are not.
 * @see #Guide_Data
 */
int Autoguider_Guide_Get_Do_Object_Detect(void)
{
	return Guide_Data.Do_Object_Detect;
}

/**
 * Routine to set which field object to guide on.
 * @param The index in the list of objects of the one we want to guide upon.
 * @return The routine returns TRUE on success, and FALSE if a failure occurs.
 * @see #Guide_Data
 * @see #Autoguider_Guide_Is_Guiding
 * @see #Autoguider_Guide_Window_Set
 * @see #Autoguider_Guide_Exposure_Length_Set
 * @see autoguider_dark.html#Autoguider_Dark_Get_Exposure_Length_Nearest
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_field.html#Autoguider_Field_Get_Exposure_Length
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GUIDE
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object
 * @see autoguider_object.html#Autoguider_Object_Struct
 */
int Autoguider_Guide_Set_Guide_Object(int index)
{
	struct Autoguider_Object_Struct object;
	char keyword_string[32];
	char *scale_type_string = NULL;
	int default_window_width,default_window_height,sx,sy,ex,ey,target_counts;/*min_counts,max_counts,*/
	int field_exposure_length,guide_exposure_index,guide_exposure_length;
	int min_exposure_length,max_exposure_length,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:started.");
#endif
	if(Autoguider_Guide_Is_Guiding() == TRUE)
	{
		Autoguider_General_Error_Number = 722;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
			"Failed to set guide object:Guiding loop already running.");
		return FALSE;
	}
	if(Autoguider_Field_Is_Fielding() == TRUE)
	{
		Autoguider_General_Error_Number = 723;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
			"Failed to set guide object:Fielding operation already in progress.");
		return FALSE;
	}
	/* get the object at index */
	if(!Autoguider_Object_List_Get_Object(index,&object))
		return FALSE;
	/* get default guide window size */
	retval = CCD_Config_Get_Integer("guide.ncols.default",&default_window_width);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 724;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
			"Getting guide NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.nrows.default",&default_window_height);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 725;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
			"Getting guide NRows failed.");
		return FALSE;
	}
	/* compute window position */
	sx = object.CCD_X_Position-(default_window_width/2);
	if(sx < 0)
		sx = 0;
	sy = object.CCD_Y_Position-(default_window_height/2);
	if(sy < 0)
		sy = 0;
	ex = sx + default_window_width;
	ey = sy + default_window_height;
	/* set guide window data */
	if(!Autoguider_Guide_Window_Set(sx,sy,ex,ey))
		return FALSE;
	/* only change exposure length if it is not locked */
	if(Guide_Data.Exposure_Length_Lock == FALSE)
	{
		/* scale guide exposure length based on object counts and field exposure length
		** NB currently assumes selected object is from the last "field"
		** there is no way at present to determine the source of objects in the objects list. */
		field_exposure_length = Autoguider_Field_Get_Exposure_Length();
		/* get exposure scaling configuration */
		retval = CCD_Config_Get_String("guide.counts.scale_type",&scale_type_string);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 726;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
				"Getting guide counts scale type failed.");
			return FALSE;
		}
		if((strcmp(scale_type_string,"integrated")!=0)&&(strcmp(scale_type_string,"peak")!=0))
		{
			Autoguider_General_Error_Number = 727;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
				"guide.counts.scale_type has illegal scale type '%s'.",scale_type_string);
			if(scale_type_string != NULL)
				free(scale_type_string);
			return FALSE;
		}
		/* get target guide counts */
		sprintf(keyword_string,"guide.counts.target.%s",scale_type_string);
		retval = CCD_Config_Get_Integer(keyword_string,&target_counts);
		if(retval == FALSE)
		{
			if(scale_type_string != NULL)
				free(scale_type_string);
			Autoguider_General_Error_Number = 728;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
				"Getting target guide counts failed(%s).",keyword_string);
			return FALSE;
		}
		/* get min/max exposure length */
		retval = CCD_Config_Get_Integer("ccd.exposure.minimum",&min_exposure_length);
		if(retval == FALSE)
		{
			if(scale_type_string != NULL)
				free(scale_type_string);
			Autoguider_General_Error_Number = 729;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
				"Getting min exposure length failed.");
			return FALSE;
		}
		retval = CCD_Config_Get_Integer("ccd.exposure.maximum",&max_exposure_length);
		if(retval == FALSE)
		{
			if(scale_type_string != NULL)
				free(scale_type_string);
			Autoguider_General_Error_Number = 730;
			sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Object:"
			"Getting max exposure length failed.");
			return FALSE;
		}
		/* and actually do guide exposure length scaling */
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:"
				       "field exposure length =%d, target_counts = %d, integrated counts = %.2f, "
				       "peak counts = %.2f, scale type = %s.",field_exposure_length,target_counts,
				       object.Total_Counts,object.Peak_Counts,scale_type_string);
#endif
		if(strcmp(scale_type_string,"integrated")==0)
		{
			guide_exposure_length = field_exposure_length * (target_counts/object.Total_Counts);
		}
		else if(strcmp(scale_type_string,"peak")==0)
		{
			guide_exposure_length = field_exposure_length * (target_counts/object.Peak_Counts);
		}
		/* free scale type */
		if(scale_type_string != NULL)
			free(scale_type_string);
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:"
				       "guide exposure length = %d.",guide_exposure_length);
#endif
		if(guide_exposure_length < min_exposure_length)
		{
			guide_exposure_length = min_exposure_length;
		}
		if(guide_exposure_length > max_exposure_length)
		{
			guide_exposure_length = max_exposure_length;
		}
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:"
			       "best guide exposure length = %d.",guide_exposure_length);
#endif
		/* round guide exposure length to nearest available dark */
		if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&guide_exposure_length,&guide_exposure_index))
			return FALSE;
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:"
					      "nearest guide exposure length = %d (index %d).",
					      guide_exposure_length,guide_exposure_index);
#endif
		/* set */
		if(!Autoguider_Guide_Exposure_Length_Set(guide_exposure_length,FALSE))
			return FALSE;
	}/* end if exposure length not locked */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Autoguider_Guide_Set_Guide_Object:finished.");
#endif
	return TRUE;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Thread routine to repeatedly read out the CCD for guiding.
 * <ul>
 * <li>Set Guide_Data.Is_Guiding TRUE.
 * <li>Call CCD_Setup_Dimensions.
 * <li>Call Autoguider_Buffer_Set_Guide_Dimension to set the guide buffer to the window size.
 * <li>Whilst Guide_Data.Quit_Guiding is FALSE:
 *     <ul>
 *     <li>Set Guide_Data.In_Use_Buffer_Index to _not_ the last buffer index.
 *     <li>Call Autoguider_Buffer_Raw_Guide_Lock to lock the in use buffer index.
 *     <li>Call CCD_Exposure_Expose and readout into the locked buffer.
 *     <li>Call Autoguider_Buffer_Raw_Guide_Unlock to unlock the in use buffer index.
 *     <li>Call Autoguider_Buffer_Raw_To_Reduced_Guide on the Guide_Data.In_Use_Buffer_Index to copy the new raw data
 *         into the equivalent reduced guide buffer. This (internally) (re)locks/unclocks the Raw guide mutex and
 *         the reduced guide mutex.
 *     <li>Call Guide_Reduce to dark subtract and flat-field the reduced data, if required.
 *     <li>Set Guide_Data.Last_Buffer_Index to the in use buffer index.
 *     <li>Set the in use buffer index to -1.
 *     </ul>
 * <li>Set Guide_Data.Is_Guiding FALSE.
 * </ul>
 * @see #Guide_Data
 * @see #Guide_Reduce
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Set_Guide_Dimension
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Unlock
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GUIDE
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Expose
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 */
static void *Guide_Thread(void *user_arg)
{
	struct timespec start_time;
	unsigned short *buffer_ptr = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:started.");
#endif
	/* set is guiding flag */
	Guide_Data.Is_Guiding = TRUE;
	/* reset frame number */
	Guide_Data.Frame_Number = 0;
	/* setup dimensions once at start of loop */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
	   "Calling CCD_Setup_Dimensions(ncols=%d,nrows=%d,binx=%d,biny=%d,window={xs=%d,ys=%d,xe=%d,ye=%d}).",
				      Guide_Data.Unbinned_NCols,Guide_Data.Unbinned_NRows,
				      Guide_Data.Bin_X,Guide_Data.Bin_Y,
				      Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,
				      Guide_Data.Window.X_End,Guide_Data.Window.Y_End);
#endif
	retval = CCD_Setup_Dimensions(Guide_Data.Unbinned_NCols,Guide_Data.Unbinned_NRows,
				      Guide_Data.Bin_X,Guide_Data.Bin_Y,TRUE,Guide_Data.Window);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
				       "Failed on CCD_Setup_Dimensions.");
#endif
		/* reset guiding flag */
		Guide_Data.Is_Guiding = FALSE;
		Autoguider_General_Error_Number = 711;
		sprintf(Autoguider_General_Error_String,"Guide_Thread:CCD_Setup_Dimensions failed.");
		Autoguider_General_Error();
		return NULL;
	}
	/* ensure the buffer is the right size */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
	   "Calling Autoguider_Buffer_Set_Guide_Dimension(ncols=%d(%d-%d),nrows=%d(%d-%d),binx=%d,biny=%d).",
				      Guide_Data.Window.X_End-Guide_Data.Window.X_Start,
				      Guide_Data.Window.X_End,Guide_Data.Window.X_Start,
				      Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start,
				      Guide_Data.Window.Y_End,Guide_Data.Window.Y_Start,
				      Guide_Data.Bin_X,Guide_Data.Bin_Y);
#endif
	retval = Autoguider_Buffer_Set_Guide_Dimension((Guide_Data.Window.X_End-Guide_Data.Window.X_Start)+1,
						       (Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start)+1,
						       Guide_Data.Bin_X,Guide_Data.Bin_Y);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
				       "Failed on Autoguider_Buffer_Set_Guide_Dimension.");
#endif
		/* reset guiding flag */
		Guide_Data.Is_Guiding = FALSE;
		Autoguider_General_Error();
		return NULL;
	}

#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:starting guide loop.");
#endif
	/* start guide loop */
	while(Guide_Data.Quit_Guiding == FALSE)
	{
		/* lock out a readout buffer */
		/* Use the buffer index _not_ used by the last completed field readout */
		Guide_Data.In_Use_Buffer_Index = (!Guide_Data.Last_Buffer_Index);
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,
					      "Guide_Thread:Locking raw guide buffer %d.",
					      Guide_Data.In_Use_Buffer_Index);
#endif
		retval = Autoguider_Buffer_Raw_Guide_Lock(Guide_Data.In_Use_Buffer_Index,&buffer_ptr);
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
					       "Failed on Autoguider_Buffer_Raw_Guide_Lock.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error_Number = 712;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:"
				"Autoguider_Buffer_Raw_Guide_Lock failed.");
			Autoguider_General_Error();
			return NULL;
		}
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:raw guide buffer locked.");
#endif
		/* do a guide exposure */
		start_time.tv_sec = 0;
		start_time.tv_nsec = 0;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
				      "Calling CCD_Exposure_Expose with exposure length %d ms.",
					      Guide_Data.Exposure_Length);
#endif
		retval = CCD_Exposure_Expose(TRUE,start_time,Guide_Data.Exposure_Length,buffer_ptr,
					     Autoguider_Buffer_Get_Guide_Pixel_Count());
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
					       "Failed on CCD_Exposure_Expose.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* attempt buffer unlock, and reset in use index */
			Autoguider_Buffer_Raw_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error_Number = 713;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:CCD_Exposure_Expose failed.");
			Autoguider_General_Error();
			return NULL;
		}
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:exposure completed.");
#endif
		/* unlock readout buffer */
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,
					      "Guide_Thread:Unlocking guide buffer %d.",
					      Guide_Data.In_Use_Buffer_Index);
#endif
		retval = Autoguider_Buffer_Raw_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
					       "Failed on Autoguider_Buffer_Raw_Guide_Unlock.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			Autoguider_General_Error_Number = 714;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:"
				"Autoguider_Buffer_Raw_Guide_Unlock failed.");
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error();
			return NULL;
		}
		/* reduce data */
		/* Guide_Reduce calls
		** Autoguider_Buffer_Raw_To_Reduced_Guide which re-locks the raw guide mutex, so has to be called
		** after Autoguider_Buffer_Raw_Guide_Unlock (as we are using fast mutexs that fails on multiple locks
		** by the same thread) */
		retval = Guide_Reduce();
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
					       "Failed on Guide_Reduce.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error();
			return NULL;
		}
		/* send position update to TCS */
		/* reset buffer indexs */
		Guide_Data.Last_Buffer_Index = Guide_Data.In_Use_Buffer_Index;
		Guide_Data.In_Use_Buffer_Index = -1;
		Guide_Data.Frame_Number++;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:"
					      "Guide buffer unlocked, last buffer now %d.",
					      Guide_Data.Last_Buffer_Index);
#endif
	}/* end while guiding */
	Guide_Data.Is_Guiding = FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Thread:finished.");
#endif
	return NULL;
}

/**
 * Internal routine to reduced the guide data in the Guide_Data.In_Use_Buffer_Index buffer.
 * The Guide_Data.In_Use_Buffer_Index raw/reduced mutexs should <b>not</b> be locked when this is called.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_To_Reduced_Guide
 * @see autoguider_buffer.html#Autoguider_Buffer_Reduced_Guide_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Reduced_Guide_Unlock
 * @see autoguider_dark.html#Autoguider_Dark_Subtract
 * @see autoguider_flat.html#Autoguider_Flat_Field
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_LOG_BIT_GUIDE
 * @see autoguider_object.html#Autoguider_Object_Detect
 */
static int Guide_Reduce(void)
{
	float *reduced_buffer_ptr = NULL;
	int retval,guide_width,guide_height;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:started.");
#endif
	/* copy raw data to reduced data */
	retval = Autoguider_Buffer_Raw_To_Reduced_Guide(Guide_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
				       "Autoguider_Buffer_Raw_To_Reduced_Guide failed.");
#endif
		return FALSE;
	}
	/* lock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Guide_Lock(Guide_Data.In_Use_Buffer_Index,&reduced_buffer_ptr);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
				       "Autoguider_Buffer_Reduced_Guide_Lock(%d) failed.",
				       Guide_Data.In_Use_Buffer_Index);
#endif
		return FALSE;
	}
	/* dark subtraction */
	if(Guide_Data.Do_Dark_Subtract)
	{
		retval = Autoguider_Dark_Subtract(reduced_buffer_ptr,Autoguider_Buffer_Get_Guide_Pixel_Count(),
						  Guide_Data.Binned_NCols,Guide_Data.Binned_NRows,
						  TRUE,Guide_Data.Window);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
					       "Autoguider_Dark_Subtract failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
				       "Did NOT subtract dark.");
#endif
	}
	/* flat field */
	if(Guide_Data.Do_Flat_Field)
	{
		retval = Autoguider_Flat_Field(reduced_buffer_ptr,Autoguider_Buffer_Get_Guide_Pixel_Count(),
						  Guide_Data.Binned_NCols,Guide_Data.Binned_NRows,
						  TRUE,Guide_Data.Window);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
					       "Autoguider_Flat_Field failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
				       "Did NOT flat field.");
#endif
	}
	/* object detect */
	if(Guide_Data.Do_Object_Detect)
	{
		guide_width = (Guide_Data.Window.X_End - Guide_Data.Window.X_Start)+1;
		guide_height = (Guide_Data.Window.Y_End - Guide_Data.Window.Y_Start)+1;
		retval = Autoguider_Object_Detect(reduced_buffer_ptr,guide_width,guide_height,
						  Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,FALSE,
						  Guide_Data.Guide_Id,Guide_Data.Frame_Number);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
					       "Autoguider_Object_Detect failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:"
				       "Did NOT object detect.");
#endif
	}
	/* unlock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log(AUTOGUIDER_GENERAL_LOG_BIT_GUIDE,"Guide_Reduce:finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
