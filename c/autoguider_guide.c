/* autoguider_guide.c
** Autoguider guide routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_guide.c,v 1.40 2014-01-31 16:27:51 cjm Exp $
*/
/**
 * Guide routines for the autoguider program.
 * @author Chris Mottram
 * @version $Revision: 1.40 $
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
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log_udp.h"

#include "ccd_config.h"
#include "ccd_exposure.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

#include "ngatcil_ags_sdb.h"
#include "ngatcil_tcs_guide_packet.h"

#include "autoguider_buffer.h"
#include "autoguider_cil.h"
#include "autoguider_dark.h"
#include "autoguider_field.h"
#include "autoguider_flat.h"
#include "autoguider_general.h"
#include "autoguider_guide.h"
#include "autoguider_object.h"

/* enums */
/**
 * Object count scale type enumeration.
 * <ul>
 * <li>GUIDE_SCALE_TYPE_PEAK
 * <li>GUIDE_SCALE_TYPE_INTEGRATED
 * </ul>
 */
enum GUIDE_SCALE_TYPE
{
	GUIDE_SCALE_TYPE_PEAK=0,GUIDE_SCALE_TYPE_INTEGRATED=1
};

/* data types */
/**
 * Structure holding data pertaining to guide exposure length scaling.
 * <dl>
 * <dt>Type</dt> <dd>Whether to scale guide exposure lengths by peak or integrated counts.</dd>
 * <dt>Autoscale</dt> <dd>A boolean, used to determine from a config whether exposure length scaling should occur.</dd>
 * <dt>Target_Counts</dt> <dd>Target counts.</dd>
 * <dt>Min_Peak_Counts</dt> <dd>Minimum peak counts in the guide object.</dd>
 * <dt>Max_Peak_Counts</dt> <dd>Maximum peak counts in the guide object.</dd>
 * <dt>Min_Integrated_Counts</dt> <dd>Minimum integrated counts in the guide object.</dd>
 * <dt>Max_Integrated_Counts</dt> <dd>Maximum integrated counts in the guide object.</dd>
 * <dt>Scale_Index</dt> <dd>How many guide loops has the centroid been sub-optimal/non-existant/
 *     ready for resacaling.</dd>
 * <dt>Scale_Count</dt> <dd>How many guide loops to wait before rescaling.</dd>
 * <dt>Scale_Up</dt> <dd>Boolean, if TRUE scale direction is up (increased exp length), otherwise down.</dd>
 * </dl>
 * @see #GUIDE_SCALE_TYPE
 */
struct Guide_Exposure_Length_Scaling_Struct
{
	enum GUIDE_SCALE_TYPE Type;
	int Autoscale;
	int Target_Counts;
	int Min_Peak_Counts;
	int Max_Peak_Counts;
	int Min_Integrated_Counts;
	int Max_Integrated_Counts;
	int Scale_Index;
	int Scale_Count;
	int Scale_Up;
};

/**
 * Structure holding data pertaining to guide window tracking.
 * <dl>
 * <dt>Guide_Window_Tracking</dt> <dd>A boolean, if TRUE move the guide window if the centroid approaches an edge.</dd>
 * <dt>Guide_Window_Edge_Pixel_Count</dt> <dd>In pixels, if guide centroid is closer than this number of pixels to 
 *     the edge of the guide window, set the guide packet "near window edge" flag.</dd>
 * <dt>Guide_Window_Track_Pixel_Count</dt> <dd>In pixels, if guide centroid is closer than this number of pixels to 
 *     the edge of the guide window, recentre the guide window on the guide centroid.</dd>
 * <dt>Guide_Window_Resize</dt> <dd>A boolean, if TRUE the guide window can be resized, otherwise it must be maintained at the default size.</dd>
 * </dl>
 */
struct Guide_Window_Tracking_Struct
{
	int Guide_Window_Tracking;
	int Guide_Window_Edge_Pixel_Count;
	int Guide_Window_Track_Pixel_Count;
	int Guide_Window_Resize;
};

/**
 * Data type holding local data to autoguider_guide for one buffer. This consists of the following:
 * <dl>
 * <dt>Unbinned_NCols</dt> <dd>Number of unbinned columns in the <b>full frame</b>.</dd>
 * <dt>Unbinned_NRows</dt> <dd>Number of unbinned rows in <b>full frame</b>.</dd>
 * <dt>Bin_X</dt> <dd>X binning in guide images.</dd>
 * <dt>Bin_Y</dt> <dd>Y binning in guide images.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in <b>full frame</b>.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in <b>full frame</b>.</dd>
 * <dt>Window</dt> <dd>The guide window, an instance of CCD_Setup_Window_Struct. The dimensions are inclusive, 
 *             i.e. a window of ((X_Start=100,Y_Start=200),(X_End=200,Y_End=300)) has a size of (101,101) pixels.</dd>
 * <dt>Exposure_Length</dt> <dd>The exposure length in milliseconds.</dd>
 * <dt>Exposure_Length_Lock</dt> <dd>Boolean determining whether the exposure length can be dynamically changed
 *                                or is 'locked' to a specified length.</dd>
 * <dt>Exposure_Length_Autoscale</dt> <dd>Boolean determining whether the exposure length can be dynamically changed -
 *                                read from config file rather than changed per invocation.</dd>
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
 * <dt>Loop_Cadence</dt> <dd>The time taken to complete one whole guide loop (the last one!), 
 *                        in decimal seconds.</dd>
 * <dt>Exposure_Length_Scaling</dt> <dd>A structure of type Guide_Exposure_Length_Scaling_Struct
 *                                  holding data/config about scaling the exposure length of guide exposures.</dd>
 * <dt>Guide_Window_Tracking</dt> <dd>Structure of type Guide_Window_Tracking_Struct 
 *            holding guide window tracking data.</dd>
 * <dt>Timecode_Scaling_Factor</dt> <dd>A float, used to scale the Loop_Cadence by when constructing the
 *     timecode so send in the TCS UDP guide packet.</dd>
 * <dt>Use_Cadence_For_SDB_Exp_Time</dt> <dd>A boolean, if TRUE the guide loop sends the loop cadence (total loop
 *     execution time) to the SDB each time round the loop as if it were the exposure length. This is meant to
 *     help the TCS apply the right guide corrections, which uses the SDB exposure length.</dd>
 * <dt>Initial_Object_CCD_X_Position</dt> <dd>A float, set to the selected guide objects initial CCD X position 
 *     (before the guide loop is started). This is used within the guide loop to choose which object to guide upon 
 *     if multiple objects are detected within the guide window.</dd>
 * <dt>Initial_Object_CCD_Y_Position</dt> <dd>A float, set to the selected guide objects initial CCD Y position 
 *     (before the guide loop is started). This is used within the guide loop to choose which object to guide upon 
 *     if multiple objects are detected within the guide window.</dd>
 * <dt>Last_Object</dt> <dd>A copy of the last guide object detected and used to send a guide centroid, for status purposes.</dd>
 * </dl>
 * @see #Guide_Exposure_Length_Scaling_Struct
 * @see #Guide_Window_Tracking_Struct
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see autoguider_object.html#Autoguider_Object_Struct
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
	int Exposure_Length_Autoscale;
	int In_Use_Buffer_Index;
	int Last_Buffer_Index;
	int Quit_Guiding;
	int Is_Guiding;
	int Do_Dark_Subtract;
	int Do_Flat_Field;
	int Do_Object_Detect;
	int Guide_Id;
	int Frame_Number;
	double Loop_Cadence;
	struct Guide_Exposure_Length_Scaling_Struct Exposure_Length_Scaling;
	struct Guide_Window_Tracking_Struct Guide_Window_Tracking;
	float Timecode_Scaling_Factor;
	int Use_Cadence_For_SDB_Exp_Time;
	float Initial_Object_CCD_X_Position;
	float Initial_Object_CCD_Y_Position;
	struct Autoguider_Object_Struct Last_Object;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_guide.c,v 1.40 2014-01-31 16:27:51 cjm Exp $";
/**
 * Instance of guide data.
 * @see #Guide_Struct
 * @see #Guide_Exposure_Length_Scaling_Struct
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct 
 */
static struct Guide_Struct Guide_Data = 
{
	0,0,1,1,0,0,
	{0,0,0,0},
	-1,FALSE,FALSE,
	-1,1,FALSE,FALSE,
	TRUE,TRUE,TRUE,
	0,0,
	0.0,
	{GUIDE_SCALE_TYPE_PEAK,FALSE,0,0,0,0,0,0,0,TRUE},
	{FALSE,10,10,FALSE},
	2.0f, FALSE, 0.0f, 0.0f,
	{0,0.0f,0.0f,0.0f,0.0f,0.0f,0,0.0f,0,0.0f,0.0f}
};

/* internal routines */
static void *Guide_Thread(void *user_arg);
static int Guide_Reduce(void);
static int Guide_Exposure_Length_Scale(void);
static int Guide_Window_Track(void);
static int Guide_Packet_Send(int terminating,float timecode_secs);
static int Guide_Scaling_Config_Load(void);
static int Guide_Dimension_Config_Load(void);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Guide initialisation routine. Loads default values from properties file.
 * Loaded config includes the following:
 * <ul>
 * <li>"guide.dark_subtract"
 * <li>"guide.flat_field"
 * <li>"guide.object_detect"
 * <li>"guide.window.tracking"
 * <li>"guide.window.edge.pixels"
 * <li>"guide.window.track.pixels"
 * <li>"guide.window.resize"
 * <li>"guide.timecode.scale"
 * <li>"guide.sdb.exposure_length.use_cadence"
 * </ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see #Guide_Dimension_Config_Load
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Boolean
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Float
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Guide_Initialise(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_Initialise",LOG_VERBOSITY_INTERMEDIATE,
			       "GUIDE","started.");
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
	/* get dimensions config
	** reloaded by Autoguider_Guide_On, but needed initially for guide window checking */
	if(!Guide_Dimension_Config_Load())
		return FALSE;
	/* guide window tracking */
	retval = CCD_Config_Get_Boolean("guide.window.tracking",
					&(Guide_Data.Guide_Window_Tracking.Guide_Window_Tracking));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 748;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting guide window tracking boolean failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.window.edge.pixels",
					&(Guide_Data.Guide_Window_Tracking.Guide_Window_Edge_Pixel_Count));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 751;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting guide window edge pixels failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.window.track.pixels",
					&(Guide_Data.Guide_Window_Tracking.Guide_Window_Track_Pixel_Count));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 752;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting guide window track pixels failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Boolean("guide.window.resize",
					&(Guide_Data.Guide_Window_Tracking.Guide_Window_Resize));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 757;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting guide window tracking resizing boolean failed.");
		return FALSE;
	}
	/* timecode scaling */
	retval = CCD_Config_Get_Float("guide.timecode.scale",&(Guide_Data.Timecode_Scaling_Factor));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 753;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting guide timecode scaling factor failed.");
		return FALSE;
	}
	/* set SDB exposure length from guide cadence? */
	retval = CCD_Config_Get_Boolean("guide.sdb.exposure_length.use_cadence",
					&(Guide_Data.Use_Cadence_For_SDB_Exp_Time));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 755;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Initialise:"
			"Getting whether to use guide cadence for SDB exposure length boolean failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_Initialise",LOG_VERBOSITY_INTERMEDIATE,
			       "GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Setup the autoguider window.
 * <ul>
 * <li>We range check the input window values, against the Guide_Data maximum dimensions.
 * <li>We call CCD_Setup_Dimensions_Check to allow the camera driver to modify the window dimensions
 *     (not all cameras can support arbitary windows).
 * <li>We call Autoguider_CIL_SDB_Packet_Window_Set to set the internal SDB values, ready to send
 *     to the SDB later.
 * </ul>
 * @param sx The start X position.
 * @param sy The start Y position.
 * @param ex The end X position.
 * @param ey The end Y position.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Window_Set
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions_Check
 */
int Autoguider_Guide_Window_Set(int sx,int sy,int ex,int ey)
{
	if((sx < 1)||(sy < 1)||(ex < 1)||(ey < 1))
	{
		Autoguider_General_Error_Number = 700;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d).",sx,sy,ex,ey);
		return FALSE;
	}
	/* check sx < ex, sy < ey */
	if(sx >= ex)
	{
		Autoguider_General_Error_Number = 734;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d) : Start X %d after End X %d.",sx,sy,ex,ey,sx,ex);
		return FALSE;
	}
	if(sy >= ey)
	{
		Autoguider_General_Error_Number = 735;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d) : Start Y %d after End Y %d.",sx,sy,ex,ey,sy,ey);
		return FALSE;
	}
	/* check vs overall dimensions - 
	** Binned_NCols/NRows should be loaded/computed as part of Guide initialisation. */
	if(ex > (Guide_Data.Binned_NCols-1))
	{
		Autoguider_General_Error_Number = 736;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d) : End X %d off end of CCD %d.",sx,sy,ex,ey,ex,
			Guide_Data.Binned_NCols);
		return FALSE;
	}
	if(ey > (Guide_Data.Binned_NRows-1))
	{
		Autoguider_General_Error_Number = 737;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"Window out of range (%d,%d,%d,%d) : End y %d off end of CCD %d.",sx,sy,ex,ey,ey,
			Guide_Data.Binned_NRows);
		return FALSE;
	}
	Guide_Data.Window.X_Start = sx;
	Guide_Data.Window.Y_Start = sy;
	Guide_Data.Window.X_End = ex;
	Guide_Data.Window.Y_End = ey;
	/* Check the window is legal. Some detectors (PCO) don't support arbitary windows, this call
	** allows the driver to modifiy the window to something that the camera will allow */
	if(!CCD_Setup_Dimensions_Check(&(Guide_Data.Unbinned_NCols),&(Guide_Data.Unbinned_NRows),
				       &(Guide_Data.Bin_X),&(Guide_Data.Bin_Y),TRUE,&(Guide_Data.Window)))
	{
		Autoguider_General_Error_Number = 758;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set:"
			"CCD_Setup_Dimensions_Check on input Window (%d,%d,%d,%d) failed.",sx,sy,ex,ey);
		return FALSE;
	}
	/* update SDB values
	** We must now use the Guide_Data.Window values as they may have been modified by 
	** CCD_Setup_Dimensions_Check */
	if(!Autoguider_CIL_SDB_Packet_Window_Set(Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,
						 Guide_Data.Window.X_End,Guide_Data.Window.Y_End))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Autoguider_Guide_Window_Set",
					 LOG_VERBOSITY_INTERMEDIATE,"GUIDE"); /* no need to fail */
	}
	return TRUE;
}

/**
 * Setup the guide exposure length, and whether it is dynamically changable during a guide session.
 * @param exposure_length The exposure length in milliseconds.
 * @param lock A boolean, if TRUE the exposure length is fixed, otherwise the exposure length can change
 *        during guiding.
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
 * @see #Guide_Dimension_Config_Load
 * @see #Guide_Scaling_Config_Load
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_dark.html#Autoguider_Dark_Set
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_flat.html#Autoguider_Flat_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Guide_On(void)
{
	time_t time_secs;
	struct tm *time_tm = NULL;
	pthread_t guide_thread;
	pthread_attr_t attr;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_On",LOG_VERBOSITY_TERSE,
			       "GUIDE","started.");
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
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_On",LOG_VERBOSITY_VERBOSE,
			       "GUIDE","Getting Dimensions.");
#endif
	/* get dimensions */
	if(!Guide_Dimension_Config_Load())
		return FALSE;
	/* get/reload scaling config */
	if(!Guide_Scaling_Config_Load())
		return FALSE;
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
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_On",LOG_VERBOSITY_VERBOSE,
					      "GUIDE","Exposure Length set to default:%d ms.",
					      Guide_Data.Exposure_Length);
#endif
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_On",LOG_VERBOSITY_VERBOSE,
					      "GUIDE","Using current Exposure Length:%d ms.",
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
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_Exp_Time_Set(Guide_Data.Exposure_Length))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Autoguider_Guide_On",
					 LOG_VERBOSITY_INTERMEDIATE,"GUIDE"); /* no need to fail */
	}
	/* initialise thread quit variable */
	Guide_Data.Quit_Guiding = FALSE;
	/* initialise Guide ID */
	time_secs = time(NULL);
	time_tm = gmtime(&time_secs);
	Guide_Data.Guide_Id = (time_tm->tm_yday*1000000)+(time_tm->tm_hour*10000)+(time_tm->tm_min*100)+
		time_tm->tm_sec;
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
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_On",LOG_VERBOSITY_TERSE,
			       "GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Routine to stop the guide loop (thread).
 * Sets Guide_Data.Quit_Guiding. Now also sets AGS SDB state to E_AGG_STATE_IDLE - this is a bit dodgy,
 * as it is still guiding until the Guide_Thread loop has quit - but it allows TCS "autoguide off" commands
 * to keep trying to reset the SDB AG_STATE, which should stop autoguider lock-ups where "autoguide on" keeps
 * returning "Already autoguiding".
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Autoguider_Guide_Is_Guiding
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see ../ngatcil/cdocs/ngatcil_ags_sdb.html#eAggState_e
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
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Autoguider_Guide_Off",LOG_VERBOSITY_TERSE,
					 "GUIDE"); /* no need to fail */
	}
	if(!Autoguider_CIL_SDB_Packet_Send())
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Autoguider_Guide_Off",LOG_VERBOSITY_TERSE,
					 "GUIDE"); /* no need to fail */
	}
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
 * Routine to set whether to move the guide window when the centroid approaches the window edge.
 * @param doit If TRUE, move the guide window, otherwise don't.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_general.html#AUTOGUIDER_GENERAL_IS_BOOLEAN
 */
int Autoguider_Guide_Set_Guide_Window_Tracking(int doit)
{
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(doit))
	{
		Autoguider_General_Error_Number = 749;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Set_Guide_Window_Tracking:"
			"Illegal argument %d.",doit);
		return FALSE;
	}
	Guide_Data.Guide_Window_Tracking.Guide_Window_Tracking = doit;
	return TRUE;
}

/**
 * Routine to get whether to move the guide window when the centroid approaches the window edge.
 * @return The routine returns TRUE if we can track the guide window when guiding, and FALSE if we don't.
 * @see #Guide_Data
 */
int Autoguider_Guide_Get_Guide_Window_Tracking(void)
{
	return Guide_Data.Guide_Window_Tracking.Guide_Window_Tracking;
}

/**
 * Routine to set which field object to guide on.
 * <ul>
 * <li>If we are already fielding or guiding, returns error.
 * <li>Uses Autoguider_Object_List_Get_Object to get the specified object index.
 * <li>Uses Autoguider_Guide_Window_Set_From_XY to set the guide window from the objects postion on the CCD.
 * <li>If the exposure length is NOT locked:
 *     <ul>
 *     <li>Use Autoguider_Field_Get_Exposure_Length to get the last field exposure length.
 *     <li>Get "guide.counts.scale_type" config.
 *     <li>Get "guide.counts.target.%s" for the specified scale type, 
 *         and "ccd.exposure.minimum" and "ccd.exposure.maximum" config.
 *     <li>The guide exposure length is generated from the field exposure length, object counts scaled appropriately
 *         and bounded by the min/max.
 *     <li>We round the guide exposure length to a suitable dark value using 
 *         Autoguider_Dark_Get_Exposure_Length_Nearest.
 *     <li>We use Autoguider_Guide_Exposure_Length_Set to set the guide exposure length.
 *     </ul>
 * </ul>
 * @param index The index in the list of objects of the one we want to guide upon.
 * @return The routine returns TRUE on success, and FALSE if a failure occurs.
 * @see #Guide_Data
 * @see #Autoguider_Guide_Is_Guiding
 * @see #Autoguider_Guide_Window_Set_From_XY
 * @see #Autoguider_Guide_Exposure_Length_Set
 * @see autoguider_dark.html#Autoguider_Dark_Get_Exposure_Length_Nearest
 * @see autoguider_field.html#Autoguider_Field_Is_Fielding
 * @see autoguider_field.html#Autoguider_Field_Get_Exposure_Length
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object
 * @see autoguider_object.html#Autoguider_Object_Struct
 */
int Autoguider_Guide_Set_Guide_Object(int index)
{
	struct Autoguider_Object_Struct object;
	char keyword_string[32];
	char *scale_type_string = NULL;
	int target_counts;/*min_counts,max_counts,*/
	int field_exposure_length,guide_exposure_index,guide_exposure_length;
	int min_exposure_length,max_exposure_length,retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",LOG_VERBOSITY_TERSE,
			       "GUIDE","started.");
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
	/* set guide window data */
	if(!Autoguider_Guide_Window_Set_From_XY(object.CCD_X_Position,object.CCD_Y_Position))
		return FALSE;
	/* set initial object position for deconvolving multiple sources in the guide window */
	Guide_Data.Initial_Object_CCD_X_Position = object.CCD_X_Position;
	Guide_Data.Initial_Object_CCD_Y_Position = object.CCD_Y_Position;
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
		/* Probably not needed given exp length scaled to nearest available dark ? */
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
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",
					      LOG_VERBOSITY_TERSE,"GUIDE",
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
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",
					      LOG_VERBOSITY_TERSE,"GUIDE",
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
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",
					      LOG_VERBOSITY_TERSE,"GUIDE",
					      "best guide exposure length = %d.",guide_exposure_length);
#endif
		/* round guide exposure length to nearest available dark */
		if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&guide_exposure_length,&guide_exposure_index))
			return FALSE;
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",
					      LOG_VERBOSITY_TERSE,"GUIDE",
					      "nearest guide exposure length = %d (index %d).",
					      guide_exposure_length,guide_exposure_index);
#endif
		/* set */
		if(!Autoguider_Guide_Exposure_Length_Set(guide_exposure_length,FALSE))
			return FALSE;
	}/* end if exposure length not locked */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_Set_Guide_Object",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * This routine sets the guide window from the specified x and y position on the CCD.
 * <ul>
 * <li>Gets "guide.ncols.default" and "guide.nrows.default" config.
 * <li>Computes start and end coordinates of guide window from config and specified position.
 * <li>Calls Autoguider_Guide_Window_Set to set the computed window.
 * </ul>
 * @param ccd_x_position The X position of the centre of the window, in pixels from the edge of the CCD.
 * @param ccd_y_position The Y position of the centre of the window, in pixels from the edge of the CCD.
 * @return The routine returns TRUE on success, and FALSE if a failure occurs.
 * @see #Guide_Data
 * @see #Autoguider_Guide_Window_Set
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
int Autoguider_Guide_Window_Set_From_XY(int ccd_x_position,int ccd_y_position)
{
	int retval,default_window_width,default_window_height,sx,sy,ex,ey;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log_Format("guide","autoguider_guide.c","Autoguider_Guide_Window_Set_From_XY",
				      LOG_VERBOSITY_INTERMEDIATE,"GUIDE",
				      "Guide Window Set From XY(%d,%d):started.",
				      ccd_x_position,ccd_y_position);
#endif
	/* get default guide window size */
	retval = CCD_Config_Get_Integer("guide.ncols.default",&default_window_width);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 724;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set_From_XY:"
			"Getting guide NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.nrows.default",&default_window_height);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 725;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Window_Set_From_XY:"
			"Getting guide NRows failed.");
		return FALSE;
	}
	/* compute window position */
	sx = ccd_x_position-(default_window_width/2);
	if(sx < 1)
		sx = 1;
	sy = ccd_y_position-(default_window_height/2);
	if(sy < 1)
		sy = 1;
	ex = sx + (default_window_width-1); /* inclusive pixels in window */
	/* guide windows are inclusive i.e. pixel 0..1023 - 1023 is the last pixel where npixels is 1024 */
	if(ex >= Guide_Data.Binned_NCols)
	{
		ex = Guide_Data.Binned_NCols - 1;
		if(Guide_Data.Guide_Window_Tracking.Guide_Window_Resize == FALSE)
		{
			sx = ex - (default_window_width-1); /* inclusive pixels in window */
		}
	}
	ey = sy + (default_window_height-1);  /* inclusive pixels in window */
	if(ey >= Guide_Data.Binned_NRows)
	{
		ey = Guide_Data.Binned_NRows - 1;
		if(Guide_Data.Guide_Window_Tracking.Guide_Window_Resize == FALSE)
		{
			sy = ey - (default_window_height-1); /* inclusive pixels in window */
		}		
	}
	/* set guide window data */
	if(!Autoguider_Guide_Window_Set(sx,sy,ex,ey))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Autoguider_Guide_Window_Set_From_XY",
			       LOG_VERBOSITY_INTERMEDIATE,"GUIDE","finished.");
#endif
	return TRUE;
}
		
/**
 * Routine to get the loop cadence. This is the time taken to complete the 
 * whole guide loop, in seconds.
 * @return The time taken to complete a guide loop, in seconds.
 * @see #Guide_Data
 */
double Autoguider_Guide_Loop_Cadence_Get(void)
{
	return Guide_Data.Loop_Cadence;
}

/**
 * Routine to get the guide exposure length.
 * @return The routine returns the exposure length in milliseconds.
 * @see #Guide_Data
 */
int Autoguider_Guide_Exposure_Length_Get(void)
{
	return Guide_Data.Exposure_Length;
}

/**
 * Routine to get the number of unbinned columns in the <b>full frame</b>.
 * @return The number of unbinned columns in the <b>full frame</b>.
 * @see #Guide_Data
 */
int Autoguider_Guide_Unbinned_NCols_Get(void)
{
	return Guide_Data.Unbinned_NCols;
}

/**
 * Routine to get the number of unbinned rows in the <b>full frame</b>.
 * @return The number of unbinned rows in the <b>full frame</b>.
 * @see #Guide_Data
 */
int Autoguider_Guide_Unbinned_NRows_Get(void)
{
	return Guide_Data.Unbinned_NRows;
}

/**
 * Routine to get the column/X binning.
 * @return The binning.
 * @see #Guide_Data
 */
int Autoguider_Guide_Bin_X_Get(void)
{
	return Guide_Data.Bin_X;
}

/**
 * Routine to get the row/Y binning.
 * @return The binning.
 * @see #Guide_Data
 */
int Autoguider_Guide_Bin_Y_Get(void)
{
	return Guide_Data.Bin_Y;
}

/**
 * Routine to get the number of binned columns in the <b>full frame</b>.
 * @return The number of binned columns in the <b>full frame</b>.
 * @see #Guide_Data
 */
int Autoguider_Guide_Binned_NCols_Get(void)
{
	return Guide_Data.Binned_NCols;
}

/**
 * Routine to get the number of binned rows in the <b>full frame</b>.
 * @return The number of binned rows in the <b>full frame</b>.
 * @see #Guide_Data
 */
int Autoguider_Guide_Binned_NRows_Get(void)
{
	return Guide_Data.Binned_NRows;
}

/**
 * Routine to get the current guide window position.
 * @return A struct of type CCD_Setup_Window_Struct, containing the guide window.
 * @see #Guide_Data
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 */
struct CCD_Setup_Window_Struct Autoguider_Guide_Window_Get(void)
{
	return Guide_Data.Window;
}

/**
 * Set the autoguider timecode scaling factor. This is used to multiply by the loop cadence when
 * calculating the timecode value to send to the TCS in the UDP guide packet.
 * @param value The timecode scaling factor. This <b>must</b> be positive, and <i>should</i> be greater than
 *        0.5 to stop the TCS thinking the autoguider has timed out.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 */
int Autoguider_Guide_Timecode_Scaling_Set(float value)
{
	if(value < 0.0)
	{
		Autoguider_General_Error_Number = 754;
		sprintf(Autoguider_General_Error_String,"Autoguider_Guide_Timecode_Scaling_Set:"
			"Timecode scaling %f out of range.",value);
		return FALSE;
	}
	Guide_Data.Timecode_Scaling_Factor = value;
	return TRUE;
}

/**
 * Routine to get the timecode scaling factor. This is used to multiply by the loop cadence when
 * calculating the timecode value to send to the TCS in the UDP guide packet.
 * @return A float. The timecode scaling factor.
 * @see #Guide_Data
 */
float Autoguider_Guide_Timecode_Scaling_Get(void)
{
	return Guide_Data.Timecode_Scaling_Factor;
}

/**
 * Routine to get the guide object used to generate the last successful guide packet/centroid sent to the TCS/SDB.
 * @return A struct of type Autoguider_Object_Struct, containing the detected object.
 * @see #Guide_Data
 * @see autoguider_guide.html#Autoguider_Object_Struct
 */
struct Autoguider_Object_Struct Autoguider_Guide_Last_Object_Get(void)
{
	return Guide_Data.Last_Object;
}

/**
 * Return the selected guide objects initial CCD X position (before the guide loop is started).
 * @return The guide objects initial CCD X position, in pixels.
 * @see #Guide_Data
 */
float Autoguider_Guide_Initial_Object_CCD_X_Position_Get(void)
{
	return Guide_Data.Initial_Object_CCD_X_Position;
}
/**
 * Return the selected guide objects initial CCD Y position (before the guide loop is started).
 * @return The guide objects initial CCD Y position, in pixels.
 * @see #Guide_Data
 */
float Autoguider_Guide_Initial_Object_CCD_Y_Position_Get(void)
{
	return Guide_Data.Initial_Object_CCD_Y_Position;
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
 * <li>Call Autoguider_CIL_Guide_Packet_Open to setup the TCS Guide Packet UDP socket.
 * <li>Whilst Guide_Data.Quit_Guiding is FALSE:
 *     <ul>
 *     <li>Set Guide_Data.In_Use_Buffer_Index to _not_ the last buffer index.
 *     <li>Call Autoguider_Buffer_Raw_Guide_Lock to lock the in use buffer index.
 *     <li>Call CCD_Exposure_Expose and readout into the locked buffer.
 *     <li>Call Autoguider_Buffer_Raw_Guide_Unlock to unlock the in use buffer index.
 *     <li>Call Autoguider_Buffer_Raw_To_Reduced_Guide on the Guide_Data.In_Use_Buffer_Index to copy the new raw data
 *         into the equivalent reduced guide buffer. This (internally) (re)locks/unclocks the Raw guide mutex and
 *         the reduced guide mutex.
 *     <li>Call Guide_Reduce to dark subtract and flat-field the reduced data, and detect objects, if required.
 *     <li>Call Guide_Exposure_Length_Scale to change the exposure length, if necessary.
 *     <li>Get the time taken to complete the guide loop (Guide_Data.Loop_Cadence), for the guide packet/stats etc.
 *     <li>Call Guide_Packet_Send to send a guide packet back to the TCS, if required.
 *     <li>Call Guide_Window_Track to check and change the guide window, if necessary. This must be done after
 *         the guide packet has been sent for the Window guide packet flag to be set correctly.
 *     <li>Set Guide_Data.Last_Buffer_Index to the in use buffer index.
 *     <li>Set the in use buffer index to -1.
 *     </ul>
 * <li>Set Guide_Data.Is_Guiding FALSE.
 * <li>Call Guide_Packet_Send to send a <b>terminating</b> guide packet back to the TCS.
 * <li>Call Autoguider_CIL_Guide_Packet_Close to close the TCS Guide Packet UDP socket.
 * </ul>
 * @see #Guide_Data
 * @see #Guide_Reduce
 * @see #Guide_Exposure_Length_Scale
 * @see #Guide_Packet_Send
 * @see #Guide_Window_Track
 * @see autoguider_buffer.html#Autoguider_Buffer_Get_Guide_Pixel_Count
 * @see autoguider_buffer.html#Autoguider_Buffer_Set_Guide_Dimension
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Lock
 * @see autoguider_buffer.html#Autoguider_Buffer_Raw_Guide_Unlock
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_CCD_Temperature_Set
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_Exposure_Length_Set
 * @see autoguider_buffer.html#Autoguider_Buffer_Guide_Exposure_Start_Time_Set
 * @see autoguider_cil.html#Autoguider_CIL_Guide_Packet_Open
 * @see autoguider_cil.html#Autoguider_CIL_Guide_Packet_Close
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_State_Set
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_general.html#fdifftime
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Expose
 * @see ../ccd/cdocs/ccd_exposure.html#CCD_Exposure_Get_Exposure_Start_Time
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_Temperature_Get
 * @see ../ccd/cdocs/ccd_temperature.html#CCD_TEMPERATURE_STATUS
 * @see ../ngatcil/cdocs/ngatcil_ags_sdb.html#eAggState_e
 */
static void *Guide_Thread(void *user_arg)
{
	enum CCD_TEMPERATURE_STATUS temperature_status;
	struct timespec start_time,loop_start_time,current_time;
	unsigned short *buffer_ptr = NULL;
	double current_temperature;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
			       LOG_VERBOSITY_VERY_TERSE,"GUIDE","started.");
#endif
	/* set is guiding flag */
	Guide_Data.Is_Guiding = TRUE;
	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_GUIDEONBRIGHT))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
	}
	if(!Autoguider_CIL_SDB_Packet_Send())
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
	}
	/* reset frame number */
	Guide_Data.Frame_Number = 0;
	/* get loop start time for stats/guide packet */
	clock_gettime(CLOCK_REALTIME,&loop_start_time);
	/* setup dimensions at start of loop - can be changed if guide window tracking */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
				      LOG_VERBOSITY_VERY_TERSE,"GUIDE",
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
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE","Failed on CCD_Setup_Dimensions.");
#endif
		/* reset guiding flag */
		Guide_Data.Is_Guiding = FALSE;
		Autoguider_General_Error_Number = 711;
		sprintf(Autoguider_General_Error_String,"Guide_Thread:CCD_Setup_Dimensions failed.");
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",LOG_VERBOSITY_VERY_TERSE,"GUIDE");
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		return NULL;
	}
	/* ensure the buffer is the right size */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
				      LOG_VERBOSITY_VERY_TERSE,"GUIDE",
	   "Calling Autoguider_Buffer_Set_Guide_Dimension(ncols=%d(%d-%d),nrows=%d(%d-%d),binx=%d,biny=%d).",
				      (Guide_Data.Window.X_End-Guide_Data.Window.X_Start)+1,
				      Guide_Data.Window.X_End,Guide_Data.Window.X_Start,
				      (Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start)+1,
				      Guide_Data.Window.Y_End,Guide_Data.Window.Y_Start,
				      Guide_Data.Bin_X,Guide_Data.Bin_Y);
#endif
	retval = Autoguider_Buffer_Set_Guide_Dimension((Guide_Data.Window.X_End-Guide_Data.Window.X_Start)+1,
						       (Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start)+1,
						       Guide_Data.Bin_X,Guide_Data.Bin_Y);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE",
				       "Failed on Autoguider_Buffer_Set_Guide_Dimension.");
#endif
		/* reset guiding flag */
		Guide_Data.Is_Guiding = FALSE;
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		return NULL;
	}
	/* open guide packet socket */
	retval = Autoguider_CIL_Guide_Packet_Open();
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE",
				       "Failed on Autoguider_CIL_Guide_Packet_Open.");
#endif
		/* reset guiding flag */
		Guide_Data.Is_Guiding = FALSE;
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		return NULL;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
			       LOG_VERBOSITY_VERY_TERSE,"GUIDE","starting guide loop.");
#endif
	/* start guide loop */
	while(Guide_Data.Quit_Guiding == FALSE)
	{
		/* lock out a readout buffer */
		/* Use the buffer index _not_ used by the last completed field readout */
		Guide_Data.In_Use_Buffer_Index = (!Guide_Data.Last_Buffer_Index);
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
					      LOG_VERBOSITY_INTERMEDIATE,"GUIDE",
					      "Locking raw guide buffer %d.",Guide_Data.In_Use_Buffer_Index);
#endif
		retval = Autoguider_Buffer_Raw_Guide_Lock(Guide_Data.In_Use_Buffer_Index,&buffer_ptr);
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
					       LOG_VERBOSITY_VERY_TERSE,"GUIDE",
					       "Failed on Autoguider_Buffer_Raw_Guide_Lock.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error_Number = 712;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:"
				"Autoguider_Buffer_Raw_Guide_Lock failed.");
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_INTERMEDIATE,"GUIDE","raw guide buffer locked.");
#endif
		/* do a guide exposure */
		start_time.tv_sec = 0;
		start_time.tv_nsec = 0;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
					      LOG_VERBOSITY_VERY_TERSE,"GUIDE",
					      "Calling CCD_Exposure_Expose with exposure length %d ms.",
					      Guide_Data.Exposure_Length);
#endif
		retval = CCD_Exposure_Expose(TRUE,start_time,Guide_Data.Exposure_Length,buffer_ptr,
					     Autoguider_Buffer_Get_Guide_Pixel_Count());
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
					       LOG_VERBOSITY_VERY_TERSE,"GUIDE","Failed on CCD_Exposure_Expose.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* attempt buffer unlock, and reset in use index */
			Autoguider_Buffer_Raw_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error_Number = 713;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:CCD_Exposure_Expose failed.");
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
		/* save the exposure length,start time and CCD temperature for this buffer 
		** for future reference (FITS headers) */
		if(!Autoguider_Buffer_Guide_Exposure_Length_Set(Guide_Data.In_Use_Buffer_Index,
								Guide_Data.Exposure_Length))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
		}
		retval = CCD_Exposure_Get_Exposure_Start_Time(&start_time);
		if(retval == TRUE)
		{
			if(!Autoguider_Buffer_Guide_Exposure_Start_Time_Set(Guide_Data.In_Use_Buffer_Index,start_time))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			}
		}
		retval = CCD_Temperature_Get(&current_temperature,&temperature_status);
		if(retval)
		{
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
						      LOG_VERBOSITY_VERY_VERBOSE,"GUIDE",
						      "current temperature is %.2f C.",current_temperature);
#endif
			if(!Autoguider_Buffer_Guide_CCD_Temperature_Set(Guide_Data.In_Use_Buffer_Index,
									current_temperature))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			}
		}
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_TERSE,"GUIDE","exposure completed.");
#endif
		/* unlock readout buffer */
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
					      LOG_VERBOSITY_VERY_VERBOSE,"GUIDE",
					      "Unlocking guide buffer %d.",Guide_Data.In_Use_Buffer_Index);
#endif
		retval = Autoguider_Buffer_Raw_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
					       LOG_VERBOSITY_VERY_TERSE,"GUIDE",
					       "Failed on Autoguider_Buffer_Raw_Guide_Unlock.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			Autoguider_General_Error_Number = 714;
			sprintf(Autoguider_General_Error_String,"Guide_Thread:"
				"Autoguider_Buffer_Raw_Guide_Unlock failed.");
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
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
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
					       LOG_VERBOSITY_VERY_TERSE,"GUIDE","Failed on Guide_Reduce.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
		/* Do any necessary exposure length scaling */
		retval = Guide_Exposure_Length_Scale();
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
						      LOG_VERBOSITY_VERY_TERSE,"GUIDE",
						      "Failed on Guide_Exposure_Length_Scale.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
		/* get loop time for stats/guide packet */
		clock_gettime(CLOCK_REALTIME,&current_time);
		Guide_Data.Loop_Cadence = fdifftime(current_time,loop_start_time);
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
					      LOG_VERBOSITY_INTERMEDIATE,"GUIDE",
					      "Last loop took %.2f seconds.",Guide_Data.Loop_Cadence);
#endif
		clock_gettime(CLOCK_REALTIME,&loop_start_time);
		/* send position update to TCS */
		retval = Guide_Packet_Send(FALSE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
						      LOG_VERBOSITY_INTERMEDIATE,"GUIDE",
						      "Failed on Guide_Packet_Send.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
		/* Update SDB Guide Exposure time
		** We used to only update this when the exposure length changes.
		** However, the TCS seems to use this when scaling the guide corrections,
		** but does not take into account overheads (readout/objectdetection/flat fielding/dark subtraction).
		** We now therefore send the Cadence (in s) as the Exposure time (in ms) 
		* to try and compensate for this. */
		if(Guide_Data.Use_Cadence_For_SDB_Exp_Time)
		{
			if(!Autoguider_CIL_SDB_Packet_Exp_Time_Set((int)(Guide_Data.Loop_Cadence*1000.0)))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
		}
		/* Do any necessary guide window tracking */
		retval = Guide_Window_Track();
		if(retval == FALSE)
		{
#if AUTOGUIDER_DEBUG > 1
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
						      LOG_VERBOSITY_VERY_TERSE,"GUIDE",
						      "Failed on Guide_Window_Track.");
#endif
			/* reset guiding flag */
			Guide_Data.Is_Guiding = FALSE;
			/* reset in use buffer index */
			Guide_Data.In_Use_Buffer_Index = -1;
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			/* send tcs guide packet termination packet. */
			Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
			/* close tcs guide packet socket */
			Autoguider_CIL_Guide_Packet_Close();
			/* update SDB */
			if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			if(!Autoguider_CIL_SDB_Packet_Send())
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
			}
			return NULL;
		}
		/* reset buffer indexs */
		Guide_Data.Last_Buffer_Index = Guide_Data.In_Use_Buffer_Index;
		Guide_Data.In_Use_Buffer_Index = -1;
		Guide_Data.Frame_Number++;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Thread",
					      LOG_VERBOSITY_INTERMEDIATE,"GUIDE",
					      "Guide buffer unlocked, last buffer now %d.",
					      Guide_Data.Last_Buffer_Index);
#endif
	}/* end while guiding */
	Guide_Data.Is_Guiding = FALSE;
	/* send termination packet to TCS */
	retval = Guide_Packet_Send(TRUE,Guide_Data.Loop_Cadence*Guide_Data.Timecode_Scaling_Factor);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE","Failed on Guide_Packet_Send.");
#endif
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
		/* close tcs guide packet socket */
		Autoguider_CIL_Guide_Packet_Close();
       		/* update SDB */
	       	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
		       	Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		return NULL;
	}
	/* close guide packet socket */
	retval = Autoguider_CIL_Guide_Packet_Close();
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE",
				       "Failed on Autoguider_CIL_Guide_Packet_Close.");
#endif
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
       		/* update SDB */
	       	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
		{
		       	Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
		}
		return NULL;
	}
       	/* update SDB */
	if(!Autoguider_CIL_SDB_Packet_State_Set(E_AGG_STATE_IDLE))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
	}
	if(!Autoguider_CIL_SDB_Packet_Send())
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
				       LOG_VERBOSITY_VERY_TERSE,"GUIDE"); /* no need to fail */
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Thread",
			       LOG_VERBOSITY_VERY_TERSE,"GUIDE","finished.");
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
 * @see autoguider_object.html#Autoguider_Object_Detect
 */
static int Guide_Reduce(void)
{
	float *reduced_buffer_ptr = NULL;
	int retval,guide_width,guide_height;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
			       LOG_VERBOSITY_TERSE,"GUIDE","started.");
#endif
	/* copy raw data to reduced data */
	retval = Autoguider_Buffer_Raw_To_Reduced_Guide(Guide_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
				       LOG_VERBOSITY_TERSE,"GUIDE","Autoguider_Buffer_Raw_To_Reduced_Guide failed.");
#endif
		return FALSE;
	}
	/* lock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Guide_Lock(Guide_Data.In_Use_Buffer_Index,&reduced_buffer_ptr);
	if(retval == FALSE)
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Reduce",
					      LOG_VERBOSITY_TERSE,"GUIDE",
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
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
					       LOG_VERBOSITY_TERSE,"GUIDE","Autoguider_Dark_Subtract failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
				       LOG_VERBOSITY_TERSE,"GUIDE","Did NOT subtract dark.");
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
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
					       LOG_VERBOSITY_TERSE,"GUIDE","Autoguider_Flat_Field failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
					      LOG_VERBOSITY_TERSE,"GUIDE","Did NOT flat field.");
#endif
	}
	/* object detect */
	if(Guide_Data.Do_Object_Detect)
	{
		guide_width = (Guide_Data.Window.X_End - Guide_Data.Window.X_Start)+1;
		guide_height = (Guide_Data.Window.Y_End - Guide_Data.Window.Y_Start)+1;
		retval = Autoguider_Object_Detect(reduced_buffer_ptr,guide_width,guide_height,
						  Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,TRUE,
						  Guide_Data.Guide_Id,Guide_Data.Frame_Number);
		if(retval == FALSE)
		{
			Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
					       LOG_VERBOSITY_TERSE,"GUIDE","Autoguider_Object_Detect failed.");
#endif
			return FALSE;
		}
	}
	else
	{
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",
				       LOG_VERBOSITY_TERSE,"GUIDE","Did NOT object detect.");
#endif
	}
	/* unlock reduction buffer */
	retval = Autoguider_Buffer_Reduced_Guide_Unlock(Guide_Data.In_Use_Buffer_Index);
	if(retval == FALSE)
	{
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Reduce",LOG_VERBOSITY_VERY_TERSE,"GUIDE",
			       "finished.");
#endif
	return TRUE;
}

/**
 * Check and change the exposure length for the next guide exposure, if necessary.
 * <ul>
 * <li>The "Guide_Data.Exposure_Length_Lock" is checked - if TRUE, we return TRUE without change.
 * <li>The "Guide_Data.Exposure_Length_Scaling.Autoscale" is checked - if FALSE, we return TRUE without change.
 * <li>The "Guide_Data.Do_Object_Detect" is checked - if FALSE, we return TRUE without change.
 * <li>Autoguider_Object_List_Get_Count is called to get the number of detected objects.
 * <li>If there are more than one object, we return TRUE without change.
 * <li>If there is less than one object, we increment "Guide_Data.Exposure_Length_Scaling.Scale_Index".
 * <li>If there is one object:
 *     <ul>
 *     <li>If "Guide_Data.Exposure_Length_Scaling.Type" is scale type peak:
 *         <ul>
 *         <li>If the object's Peak Counts are less than "Guide_Data.Exposure_Length_Scaling.Min_Peak_Counts",
 *             increment "Guide_Data.Exposure_Length_Scaling.Scale_Index".
 *         <li>If the object's Peak Counts are greater than "Guide_Data.Exposure_Length_Scaling.Max_Peak_Counts",
 *             increment "Guide_Data.Exposure_Length_Scaling.Scale_Index".
 *         <li>Otherwise, reset "Guide_Data.Exposure_Length_Scaling.Scale_Index" to zero.
 *         </ul>
 *     <li>else If "Guide_Data.Exposure_Length_Scaling.Type" is scale type integrated:
 *         <ul>
 *         <li>If the object's Total Counts (integrated counts) are less than 
 *             "Guide_Data.Exposure_Length_Scaling.Min_Integrated_Counts" increment 
 *             "Guide_Data.Exposure_Length_Scaling.Scale_Index".
 *         <li>If the object's Total Counts (integrated counts) are greater than 
 *             "Guide_Data.Exposure_Length_Scaling.Max_Integrated_Counts" increment 
 *             "Guide_Data.Exposure_Length_Scaling.Scale_Index".
 *         <li>Otherwise, reset "Guide_Data.Exposure_Length_Scaling.Scale_Index" to zero.
 *         </ul>
 *     </ul>
 * </ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see #Guide_Scaling_Config_Load
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object
 */
static int Guide_Exposure_Length_Scale(void)
{
	struct Autoguider_Object_Struct object;
	int retval,object_count,guide_exposure_length,guide_exposure_index;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,"GUIDE",
			       "started.");
#endif
	/* The exposure length is locked - we cannot change it! */
	if(Guide_Data.Exposure_Length_Lock == TRUE)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
				       "GUIDE","Exposure Length is locked.");
#endif
		return TRUE;
	}
	/* if exposure length autoscale is false we cannot change it */
	if(Guide_Data.Exposure_Length_Scaling.Autoscale == FALSE)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
				       "GUIDE","Exposure Length autoscale is false.");
#endif
		return TRUE;
	}
	/* if we are not object detecting, we have no data to scale exposure length on */
	if(Guide_Data.Do_Object_Detect == FALSE)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
				       "GUIDE","Guide object detection is OFF.");
#endif
		return TRUE;
	}
	/* how many objects found in guide frame */
	if(!Autoguider_Object_List_Get_Count(&object_count))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
				       "GUIDE","Failed to get object count.");
#endif
		return TRUE;/* don't stop guiding */
	}
	/* check number of detected objects */
	if(object_count > 1)
	{
		/* too many objects - what do we do here! */
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
					      LOG_VERBOSITY_TERSE,"GUIDE",
					      "More than one guide object (%d).",object_count);
#endif
		return TRUE;/* don't stop guiding */
	}
	else if(object_count < 1)
	{
		/* start counting how long we have lost object */
		Guide_Data.Exposure_Length_Scaling.Scale_Index++;
	}
	else /* object_count == 1 */
	{
		/* get first object */
		if(!Autoguider_Object_List_Get_Object(0,&object))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Thread",
						 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
					       LOG_VERBOSITY_TERSE,"GUIDE","Failed to get object 0.");
#endif
			return TRUE; /* don't stop guiding */
		}
		/* check stats of object */
		if(Guide_Data.Exposure_Length_Scaling.Type == GUIDE_SCALE_TYPE_PEAK)
		{
			if(object.Peak_Counts < Guide_Data.Exposure_Length_Scaling.Min_Peak_Counts)
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Peak Counts %.2f < Min Peak Counts %d: "
							      "Increasing scale index.",object.Peak_Counts,
							      Guide_Data.Exposure_Length_Scaling.Min_Peak_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index++;
			}
			else if(object.Peak_Counts > Guide_Data.Exposure_Length_Scaling.Max_Peak_Counts)
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Peak Counts %.2f > Max Peak Counts %d: "
							      "Increasing scale index.",object.Peak_Counts,
							      Guide_Data.Exposure_Length_Scaling.Max_Peak_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index++;
			}
			else
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Peak Counts %.2f within range: "
							      "Reseting scale index.",object.Peak_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index = 0;
			}
		}
		else if(Guide_Data.Exposure_Length_Scaling.Type == GUIDE_SCALE_TYPE_INTEGRATED)
		{
			if(object.Total_Counts < Guide_Data.Exposure_Length_Scaling.Min_Integrated_Counts)
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Total Counts %.2f < Min Total Counts %d: "
							      "Increasing scale index.",object.Total_Counts,
							      Guide_Data.Exposure_Length_Scaling.Min_Integrated_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index++;
			}
			else if(object.Total_Counts > Guide_Data.Exposure_Length_Scaling.Max_Integrated_Counts)
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Total Counts %.2f > Max Total Counts %d: "
							      "Increasing scale index.",object.Total_Counts,
							      Guide_Data.Exposure_Length_Scaling.Max_Integrated_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index++;
			}
			else
			{
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Total Counts %.2f in range: "
							      "Reseting scale index.",object.Total_Counts);
#endif
				Guide_Data.Exposure_Length_Scaling.Scale_Index = 0;
			}
		}
	}/* end if on object_count */
	/* is it time to rescale? */
	if(Guide_Data.Exposure_Length_Scaling.Scale_Index >= Guide_Data.Exposure_Length_Scaling.Scale_Count)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c",
					      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
					      "GUIDE","Scale Index %d > Scale Count %d.",
					      Guide_Data.Exposure_Length_Scaling.Scale_Index,
					      Guide_Data.Exposure_Length_Scaling.Scale_Count);
#endif
		/* reset rescaling index */
		Guide_Data.Exposure_Length_Scaling.Scale_Index = 0;
		/* recompute exposure length */
		guide_exposure_length = Guide_Data.Exposure_Length;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c",
					      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
					      "GUIDE","Current guide exposure length %d.",guide_exposure_length);
#endif
		if(object_count == 1)
		{
			/* compute based on current counts */
			/* Guide_Data.Exposure_Length_Scaling.Target_Counts set to target peak/integrated
			** counts by Guide_Scaling_Config_Load as appropriate. */
			if(Guide_Data.Exposure_Length_Scaling.Type == GUIDE_SCALE_TYPE_PEAK)
			{
				guide_exposure_length = (int)((float)guide_exposure_length * 
				       (((float)Guide_Data.Exposure_Length_Scaling.Target_Counts)/object.Peak_Counts));
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Recomputed guide exposure length %d:"
							      "scaled by %d/%.2f.",guide_exposure_length,
					      Guide_Data.Exposure_Length_Scaling.Target_Counts,object.Peak_Counts);
#endif
			}
			else if(Guide_Data.Exposure_Length_Scaling.Type == GUIDE_SCALE_TYPE_INTEGRATED)
			{
				guide_exposure_length = (int)((float)guide_exposure_length * 
				      (((float)Guide_Data.Exposure_Length_Scaling.Target_Counts)/object.Total_Counts));
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",LOG_VERBOSITY_TERSE,
							      "GUIDE","Recomputed guide exposure length %d:"
							      "scaled by %d/%.2f.",guide_exposure_length,
					      Guide_Data.Exposure_Length_Scaling.Target_Counts,object.Total_Counts);
#endif
			}
		}
		else /* no objects found */
		{
			/* get current exposure dark index */
			if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&guide_exposure_length,&guide_exposure_index))
			{
				return FALSE;
			}
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
						      LOG_VERBOSITY_TERSE,"GUIDE",
						      "Current guide exposure length %d has index %d.",
						      guide_exposure_length,guide_exposure_index);
#endif
			guide_exposure_index++;
			/* check we can expose for longer.
			** If not return FALSE - stop the guide loop - we've lost the guide star. */
			if(guide_exposure_index >= Autoguider_Dark_Get_Exposure_Length_Count())
			{
#if AUTOGUIDER_DEBUG > 4
				Autoguider_General_Log_Format("guide","autoguider_guide.c",
							      "Guide_Exposure_Length_Scale",
							      LOG_VERBOSITY_TERSE,"GUIDE",
							      "guide exposure index %d greater than dark count %d: "
				     "Autoguiding failed as no objects detected at longest exposure length.",
					guide_exposure_index,Autoguider_Dark_Get_Exposure_Length_Count());
#endif
				Autoguider_General_Error_Number = 738;
				sprintf(Autoguider_General_Error_String,"Guide_Exposure_Length_Scale:"
					"guide exposure index %d greater than dark count %d: "
					"Autoguiding failed as no objects detected at longest exposure length.",
					guide_exposure_index,Autoguider_Dark_Get_Exposure_Length_Count());
				return FALSE;
			}
			/* get the exposure length for this new index */
			if(!Autoguider_Dark_Get_Exposure_Length_Index(guide_exposure_index,&guide_exposure_length))
				return FALSE;
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
						      LOG_VERBOSITY_TERSE,"GUIDE",
						      "New guide exposure length %d has index %d.",
						      guide_exposure_length,guide_exposure_index);
#endif
		}
		/* round guide exposure length to nearest available dark */
		if(!Autoguider_Dark_Get_Exposure_Length_Nearest(&guide_exposure_length,&guide_exposure_index))
			return FALSE;
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
					      LOG_VERBOSITY_TERSE,"GUIDE","New guide exposure length %d has index %d.",
					      guide_exposure_length,guide_exposure_index);
#endif
		/* set */
		if(!Autoguider_Guide_Exposure_Length_Set(guide_exposure_length,FALSE))
			return FALSE;
		/* ensure the correct dark is loaded */
		retval = Autoguider_Dark_Set(Guide_Data.Bin_X,Guide_Data.Bin_Y,Guide_Data.Exposure_Length);
		if(retval == FALSE)
			return FALSE;
		/* update SDB */
		if(!Autoguider_CIL_SDB_Packet_Exp_Time_Set(Guide_Data.Exposure_Length))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
						      LOG_VERBOSITY_TERSE,"GUIDE"); /* no need to fail */
		}
	}/* end if scale index >= scale count */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Exposure_Length_Scale",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Determine whether to do guide window tracking, and if needed, move the guide window to
 * surround the centroid position.
 * <ul>
 * <li>If Do_Object_Detect is FALSE we return TRUE - no window tracking can be done if no object detection is running.
 * <li>If Guide_Window_Tracking is FALSE we return TRUE - window tracking is not enabled.
 * <li>We retrieve the number of objects with Autoguider_Object_List_Get_Count.
 * <li>If the number of objects is less than or greater than 1 we return TRUE - we need 1 object only.
 * <li>We use Autoguider_Object_List_Get_Object to retrieve the object data.
 * <li>If the object's position is close to the edge of the Window:
 *     <ul>
 *     <li>We call Autoguider_Guide_Window_Set_From_XY to reset the window centre to the objects CCD position.
 *     <li>We call CCD_Setup_Dimensions to tell the camera electronics the new window position.
 *     <li>We call Autoguider_Buffer_Set_Guide_Dimension to set the guide buffer dimensions - this may have 
 *         been changed by the new window centre being near to the physical edge of the CCD.
 *     </ul>
 * </ul>
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see #Autoguider_Guide_Window_Set_From_XY
 * @see autoguider_buffer.html#Autoguider_Buffer_Set_Guide_Dimension
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Object
 * @see ../ccd/cdocs/ccd_setup.html#CCD_Setup_Dimensions
 */
static int Guide_Window_Track(void)
{
	struct Autoguider_Object_Struct object;
	int retval,object_count;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
			       LOG_VERBOSITY_TERSE,"GUIDE","started.");
#endif
	/* if we are not object detecting, we have no objects to decide whether the object is near the edge */
	if(Guide_Data.Do_Object_Detect == FALSE)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
				       LOG_VERBOSITY_TERSE,"GUIDE","Guide object detection is OFF.");
#endif
		return TRUE;/* don't stop guiding */
	}
	/* if we are not guide window tracking, stop */
	if(Guide_Data.Guide_Window_Tracking.Guide_Window_Tracking == FALSE)
	{
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
				       LOG_VERBOSITY_TERSE,"GUIDE","Guide window tracking is OFF.");
#endif
		return TRUE;/* don't stop guiding */
	}
	/* how many objects found in guide frame */
	if(!Autoguider_Object_List_Get_Count(&object_count))
	{
		Autoguider_General_Error("guide","autoguider_guide.c","Guide_Window_Track",
					 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
				       LOG_VERBOSITY_TERSE,"GUIDE","Failed to get object count.");
#endif
		return TRUE;/* don't stop guiding */
	}
	/* check number of detected objects */
	if(object_count > 1)
	{
		/* too many objects - what do we do here! */
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Window_Track",
					      LOG_VERBOSITY_TERSE,"GUIDE","More than one guide object (%d).",
					      object_count);
#endif
		return TRUE;/* don't stop guiding */
	}
	else if(object_count < 1)
	{
		/* no objects - what do we do here! */
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Window_Track",
					      LOG_VERBOSITY_TERSE,"GUIDE","No guide object (%d).",object_count);
#endif
		return TRUE;/* don't stop guiding */
	}
	else /* object_count == 1 */
	{
		/* get first object */
		if(!Autoguider_Object_List_Get_Object(0,&object))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Window_Track",
						 LOG_VERBOSITY_TERSE,"GUIDE");
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
					       LOG_VERBOSITY_TERSE,"GUIDE","Failed to get object 0.");
#endif
			return TRUE; /* don't stop guiding */
		}
		/* check nearness to window edge */
		/*
		**fwhm = ((object.FWHM_X + object.FWHM_Y)/2.0f);
		**if(((object.CCD_X_Position-Guide_Data.Window.X_Start) < (fwhm*2.0f))||
		**   ((Guide_Data.Window.X_End-object.CCD_X_Position) < (fwhm*2.0f))||
		**   ((object.CCD_Y_Position-Guide_Data.Window.Y_Start) < (fwhm*2.0f))||
		**   ((Guide_Data.Window.Y_End-object.CCD_Y_Position) < (fwhm*2.0f)))
		*/
		if(((object.CCD_X_Position-Guide_Data.Window.X_Start) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Track_Pixel_Count)||
		   ((Guide_Data.Window.X_End-object.CCD_X_Position) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Track_Pixel_Count)||
		   ((object.CCD_Y_Position-Guide_Data.Window.Y_Start) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Track_Pixel_Count)||
		   ((Guide_Data.Window.Y_End-object.CCD_Y_Position) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Track_Pixel_Count))
		{
			/* object too near guide window edge - re-position it */
			/* set guide window data */
			if(!Autoguider_Guide_Window_Set_From_XY(object.CCD_X_Position,object.CCD_Y_Position))
				return FALSE;
			/* setup new CCD window dimensions */
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Window_Track",
						      LOG_VERBOSITY_TERSE,"GUIDE",
						      "Calling CCD_Setup_Dimensions(ncols=%d,nrows=%d,binx=%d,biny=%d,"
						      "window={xs=%d,ys=%d,xe=%d,ye=%d}).",
						      Guide_Data.Unbinned_NCols,Guide_Data.Unbinned_NRows,
						      Guide_Data.Bin_X,Guide_Data.Bin_Y,
						      Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,
						      Guide_Data.Window.X_End,Guide_Data.Window.Y_End);
#endif
			retval = CCD_Setup_Dimensions(Guide_Data.Unbinned_NCols,Guide_Data.Unbinned_NRows,
						      Guide_Data.Bin_X,Guide_Data.Bin_Y,TRUE,Guide_Data.Window);
			if(retval == FALSE)
			{
				Autoguider_General_Error_Number = 750;
				sprintf(Autoguider_General_Error_String,
					"Guide_Window_Track:CCD_Setup_Dimensions failed.");
				return FALSE;
			}
			/* ensure the buffer is the right size */
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Window_Track",
						      LOG_VERBOSITY_TERSE,"GUIDE",
				  "Calling Autoguider_Buffer_Set_Guide_Dimension(ncols=%d(%d-%d),nrows=%d(%d-%d),"
						      "binx=%d,biny=%d).",
						      Guide_Data.Window.X_End-Guide_Data.Window.X_Start,
						      Guide_Data.Window.X_End,Guide_Data.Window.X_Start,
						      Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start,
						      Guide_Data.Window.Y_End,Guide_Data.Window.Y_Start,
						      Guide_Data.Bin_X,Guide_Data.Bin_Y);
#endif
			retval = Autoguider_Buffer_Set_Guide_Dimension((Guide_Data.Window.X_End-
									Guide_Data.Window.X_Start)+1,
			     (Guide_Data.Window.Y_End-Guide_Data.Window.Y_Start)+1,Guide_Data.Bin_X,Guide_Data.Bin_Y);
			if(retval == FALSE)
			{
				return FALSE;
			}
		}/* end if guide star too near window edge.*/
	}/* end if one guide object detected */
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Window_Track",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Routine to send a guide packet to the TCS. Autoguider_CIL_Guide_Packet_Send is called to send the guide packet
 * (assumes Autoguider_CIL_Guide_Packet_Open has been called). The data in autoguider_object is used to get
 * the centroid.
 * <ul>
 * <li>If object detection is switched off, no guide packet is sent.
 * <li>The number of detected objects is retrieved using Autoguider_Object_List_Get_Count.
 * <li>If no objects were detected, a NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED status guide packet is returned.
 * <li>If more than one object was detected, the one nearest the initial guide object's position is selected using
 *     Autoguider_Object_List_Get_Nearest_Object.
 * <li>Otherwise the first object is retrieved.
 * <li>We load config 'guide.counts.min.peak','guide.counts.max.peak' and 'guide.ellipticity' for reliability tests.
 * <li>A set of reliability tests are performed to get an integer between 0 and 7.
 * <li>The reliability number is transformed into a status char.
 * <li>We check whether the centroid is within 1 FWHM of the edge of the window, and if so set the status char to
 *     NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW.
 * <li>We send the guide packet to the TCS.
 * <li>We use Autoguider_CIL_SDB_Packet_Centroid_Set and Autoguider_CIL_SDB_Packet_Send to
 *     update the SDB centroid.
 * <li>We update the Guide_Data Last_Object to contain the object used to generate the guide packet/centroid.
 * </ul>
 * If an error occurs during processing, a guide packet with status NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * and "unreliable packet" timecode is sent, with as much information (centroid etc) filled in as possible.
 * @param terminating Boolean. If TRUE the autoguider is stopping guiding.
 * @param timecode_secs The number of seconds the TCS should wait for until the next guide packet will be sent. 
 *        This is a float in the range 0..9999 seconds.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see autoguider_cil.html#Autoguider_CIL_Guide_Packet_Send
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Send
 * @see autoguider_cil.html#Autoguider_CIL_SDB_Packet_Centroid_Set
 * @see autoguider_general.html#Autoguider_General_Error
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see autoguider_object.html#Autoguider_Object_List_Get_Count
 * @see autoguider_object.html#Autoguider_Object_List_Get_Nearest_Object
 * @see autoguider_object.htmlAutoguider_Object_List_Get_Object
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW
 * @see ../ngatcil/cdocs/ngatcil_tcs_guide_packet.html#NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Float
 */
static int Guide_Packet_Send(int terminating,float timecode_secs)
{
	int object_count,reliability,guide_counts_min_peak,guide_counts_max_peak,retval;
	struct Autoguider_Object_Struct object;
	char status_char;
	float fwhm,guide_ellipticity,mag,guide_mag_const,exposure_length_s,counts_per_s,log_counts_per_s;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Packet_Send",LOG_VERBOSITY_TERSE,"GUIDE",
			       "started.");
#endif
	if(Guide_Data.Do_Object_Detect)
	{
		/* how many objects found in guide frame */
		if(!Autoguider_Object_List_Get_Count(&object_count))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",LOG_VERBOSITY_TERSE,
						 "GUIDE");
			/* if count get failed send unreliable and return */
			if(!Autoguider_CIL_Guide_Packet_Send(0.0f,0.0f,terminating,TRUE,timecode_secs,
							     NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
							 LOG_VERBOSITY_VERY_TERSE,"GUIDE");
			}
			return TRUE;
		}
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",LOG_VERBOSITY_VERBOSE,
					      "GUIDE","There are %d objects.",object_count);
#endif
		/* if none send unreliable and return */
		if(object_count < 1)
		{
			if(!Autoguider_CIL_Guide_Packet_Send(0.0f,0.0f,terminating,TRUE,timecode_secs,
							     NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
							 LOG_VERBOSITY_TERSE,"GUIDE");
			}
			return TRUE;
		}
		/* if object count > 1 select nearest object to initial guide object position */
		if(object_count > 1)
		{
			if(!Autoguider_Object_List_Get_Nearest_Object(Guide_Data.Initial_Object_CCD_X_Position,
								      Guide_Data.Initial_Object_CCD_Y_Position,
								      &object))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
							 LOG_VERBOSITY_TERSE,"GUIDE");
				/* if object get nearest failed send unreliable and return */
				if(!Autoguider_CIL_Guide_Packet_Send(0.0f,0.0f,terminating,TRUE,timecode_secs,
								     NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED))
				{
					Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
								 LOG_VERBOSITY_TERSE,"GUIDE");
				}
				return TRUE;
			}
		}
		else /* object_count is one - only 1 detected object in guide frame */
		{
			/* get first object */
			if(!Autoguider_Object_List_Get_Object(0,&object))
			{
				Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
							 LOG_VERBOSITY_TERSE,"GUIDE");
				/* if object get failed send unreliable and return */
				if(!Autoguider_CIL_Guide_Packet_Send(0.0f,0.0f,terminating,TRUE,timecode_secs,
								     NGATCIL_TCS_GUIDE_PACKET_STATUS_FAILED))
				{
					Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
								 LOG_VERBOSITY_TERSE,"GUIDE");
				}
				return TRUE;
			}
		}
		/* object is the best detected object on the guide frame */
		/* reliability tests */
		/* load config */
		retval = CCD_Config_Get_Integer("guide.counts.min.peak",&guide_counts_min_peak);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 731;
			sprintf(Autoguider_General_Error_String,"Guide_Packet_Send:"
				"Failed to load config:'guide.counts.min.peak'.");
			return FALSE;
		}
		retval = CCD_Config_Get_Integer("guide.counts.max.peak",&guide_counts_max_peak);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 732;
			sprintf(Autoguider_General_Error_String,"Guide_Packet_Send:"
				"Failed to load config:'guide.counts.max.peak'.");
			return FALSE;
		}
		retval = CCD_Config_Get_Float("guide.ellipticity",&guide_ellipticity);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 733;
			sprintf(Autoguider_General_Error_String,"Guide_Packet_Send:"
				"Failed to load config:'guide.ellipticity'.");
			return FALSE;
		}
		retval = CCD_Config_Get_Float("guide.mag.const",&guide_mag_const);
		if(retval == FALSE)
		{
			Autoguider_General_Error_Number = 756;
			sprintf(Autoguider_General_Error_String,"Guide_Packet_Send:"
				"Failed to load config:'guide.mag.const'.");
			return FALSE;
		}
		/*
		** 0 means confident
		** Bit 0 set means FWHM approaching limit
		** Bit 1 set means brightness approaching limit
		** Bit 2 set means critical error
		*/
		reliability = 0;
		if(object.FWHM_Y != 0.0f)
		{
			if(fabs((object.FWHM_X/object.FWHM_Y)-1.0f) > guide_ellipticity)
			{
#if AUTOGUIDER_DEBUG > 5
				Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
							      LOG_VERBOSITY_VERBOSE,"GUIDE",
							      "Guide_Packet_Send:Detected FWHM limit:"
							      "Object has fwhmx=%.2f,fwhmy=%.2f,ellipticity=%.2f.",
							      object.FWHM_X,object.FWHM_Y,guide_ellipticity);
#endif
				reliability += (1<<0);
			}
		}
		if((object.Peak_Counts < guide_counts_min_peak)||(object.Peak_Counts > guide_counts_max_peak))
		{
			reliability += (1<<1);
		}
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
					      LOG_VERBOSITY_VERBOSE,"GUIDE","Object has %#x reliability.",reliability);
#endif
		status_char = '0'+reliability;
#if AUTOGUIDER_DEBUG > 5
		Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
					      LOG_VERBOSITY_TERSE,"GUIDE","Object has status char %c.",status_char);
#endif
		/* calculate fwhm */
		fwhm = ((object.FWHM_X + object.FWHM_Y)/2.0f);
		/* check nearness to window edge */
		/* if(((object.CCD_X_Position-Guide_Data.Window.X_Start) < (fwhm*2.0f))||
		**   ((Guide_Data.Window.X_End-object.CCD_X_Position) < (fwhm*2.0f))||
		**   ((object.CCD_Y_Position-Guide_Data.Window.Y_Start) < (fwhm*2.0f))||
		**   ((Guide_Data.Window.Y_End-object.CCD_Y_Position) < (fwhm*2.0f)))
		*/
		if(((object.CCD_X_Position-Guide_Data.Window.X_Start) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Edge_Pixel_Count)||
		   ((Guide_Data.Window.X_End-object.CCD_X_Position) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Edge_Pixel_Count)||
		   ((object.CCD_Y_Position-Guide_Data.Window.Y_Start) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Edge_Pixel_Count)||
		   ((Guide_Data.Window.Y_End-object.CCD_Y_Position) < 
		    Guide_Data.Guide_Window_Tracking.Guide_Window_Edge_Pixel_Count))
		{
			status_char = NGATCIL_TCS_GUIDE_PACKET_STATUS_WINDOW;
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
						      LOG_VERBOSITY_VERBOSE,"GUIDE",
						      "Object (%.2f,%.2f) too near edge of window "
						      "((%.2f,%.2f),(%.2f,%.2f)): status char now %c.",
						      object.CCD_X_Position,object.CCD_Y_Position,
						      Guide_Data.Window.X_Start,Guide_Data.Window.Y_Start,
						      Guide_Data.Window.X_End,Guide_Data.Window.Y_End,status_char);
#endif
		}
		/* reset status char to '0' here.
		** This was assumed to be a warning. However, the TCS sets the AGSTATE UNLOCKED
		** when the status char is non-zero. This causes the RCS to issue another  "autoguider on"
		** even when the current guide loop is still running. */
		status_char = '0';
		/* send reliable guide packet */
		if(!Autoguider_CIL_Guide_Packet_Send(object.CCD_X_Position,object.CCD_Y_Position,
						     terminating,FALSE,timecode_secs,status_char))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
						 LOG_VERBOSITY_TERSE,"GUIDE");
		}
		/* update SDB mag */
		/* ensure exposure length will not cause div by zero, log10 arg is +ve */
		if((Guide_Data.Exposure_Length != 0)&&(object.Total_Counts > 0.0f))
		{
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
						      LOG_VERBOSITY_VERBOSE,"GUIDE",
						      "Computing Magnitude using Const %f,"
						      "Exposure Length %d ms, Total Counts %f.",
						      guide_mag_const,Guide_Data.Exposure_Length,object.Total_Counts);
#endif
			exposure_length_s = ((float)Guide_Data.Exposure_Length/1000.0f);
			counts_per_s = object.Total_Counts/exposure_length_s;
			log_counts_per_s = (float)log10((double)(counts_per_s));
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
						      LOG_VERBOSITY_VERBOSE,"GUIDE",
						      "Computing Magnitude using exposure_length_s=%f,"
						      "counts_per_s=%f, log_counts_per_s=%f.",
						      exposure_length_s,counts_per_s,log_counts_per_s);
#endif			
			mag = guide_mag_const - (2.5f * log_counts_per_s);
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
						      LOG_VERBOSITY_VERBOSE,"GUIDE","Magnitude %f.",mag);
#endif
		}
		else
		{
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("guide","autoguider_guide.c","Guide_Packet_Send",
						      LOG_VERBOSITY_VERBOSE,"GUIDE",
						      "NOT Computing Magnitude:Argument out of range:"
						      "Const %f,Exposure Length %d ms, Total Counts %f.",
						      guide_mag_const,Guide_Data.Exposure_Length,object.Total_Counts);
#endif
			mag = 20.0f;
		}
		/* set the SDB guide centroid */		
		if(!Autoguider_CIL_SDB_Packet_Centroid_Set(object.CCD_X_Position,object.CCD_Y_Position,fwhm,mag))
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
						 LOG_VERBOSITY_TERSE,"GUIDE"); /* no need to fail */
		}
		if(!Autoguider_CIL_SDB_Packet_Send())
		{
			Autoguider_General_Error("guide","autoguider_guide.c","Guide_Packet_Send",
						 LOG_VERBOSITY_TERSE,"GUIDE"); /* no need to fail */
		}
		/* keep track of the guide object we used to generate the guide centroid, for status purposes */
		Guide_Data.Last_Object = object;
	}/* end if object detection enabled */
	else
	{
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log("guide","autoguider_guide.c","Guide_Packet_Send",
				       LOG_VERBOSITY_VERBOSE,"GUIDE","Object Detection Off:Not sending packet.");
#endif
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Packet_Send",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Load guide scaling configuration. Gets the following configuration:
 * <ul>
 * <li>"guide.counts.scale_type" string - "integrated" or "peak".
 * <li>"guide.counts.target.peak/integrated" - integer.
 * <li>"guide.exposure_length.autoscale" - boolean.
 * <li>"guide.counts.min.peak" - integer.
 * <li>"guide.counts.max.peak" - integer.
 * <li>"guide.counts.min.integrated" - integer.
 * <li>"guide.counts.max.integrated" - integer.
 * <li>"guide.exposure_length.scale_count" - integer.
 * </ul>
 * The data is used to populate Guide_Data.Exposure_Length_Scaling
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Guide_Data
 * @see #GUIDE_SCALE_TYPE
 * @see #Guide_Exposure_Length_Scaling_Struct
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Boolean
 */
static int Guide_Scaling_Config_Load(void)
{
	char keyword_string[32];
	char *scale_type_string = NULL;
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Scaling_Config_Load",
			       LOG_VERBOSITY_TERSE,"GUIDE","starteed.");
#endif
	retval = CCD_Config_Get_String("guide.counts.scale_type",&scale_type_string);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 739;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Getting guide counts scale type failed.");
		return FALSE;
	}
	if(strcmp(scale_type_string,"integrated") ==0)
		Guide_Data.Exposure_Length_Scaling.Type = GUIDE_SCALE_TYPE_INTEGRATED;
	else if(strcmp(scale_type_string,"peak") ==0)
		Guide_Data.Exposure_Length_Scaling.Type = GUIDE_SCALE_TYPE_PEAK;
	else
	{
		Autoguider_General_Error_Number = 740;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"guide.counts.scale_type has illegal scale type '%s'.",scale_type_string);
		if(scale_type_string != NULL)
			free(scale_type_string);
		return FALSE;
	}
	/* get target guide counts */
	sprintf(keyword_string,"guide.counts.target.%s",scale_type_string);
	retval = CCD_Config_Get_Integer(keyword_string,&(Guide_Data.Exposure_Length_Scaling.Target_Counts));
	if(retval == FALSE)
	{
		if(scale_type_string != NULL)
			free(scale_type_string);
		Autoguider_General_Error_Number = 741;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
				"Getting target guide counts failed(%s).",keyword_string);
		return FALSE;
	}
	/* free scale type string */
	if(scale_type_string != NULL)
		free(scale_type_string);
	/* get autoscale boolean */
	retval = CCD_Config_Get_Boolean("guide.exposure_length.autoscale",
					&(Guide_Data.Exposure_Length_Scaling.Autoscale));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 742;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
				"Getting guide exposure length autoscaling failed.");
		return FALSE;
	}
	/* get counts limits */
	retval = CCD_Config_Get_Integer("guide.counts.min.peak",&(Guide_Data.Exposure_Length_Scaling.Min_Peak_Counts));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 743;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Failed to load config:'guide.counts.min.peak'.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.counts.max.peak",&(Guide_Data.Exposure_Length_Scaling.Max_Peak_Counts));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 744;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Failed to load config:'guide.counts.max.peak'.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.counts.min.integrated",
					&(Guide_Data.Exposure_Length_Scaling.Min_Integrated_Counts));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 745;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Failed to load config:'guide.counts.min.integrated'.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.counts.max.integrated",
					&(Guide_Data.Exposure_Length_Scaling.Max_Integrated_Counts));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 746;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Failed to load config:'guide.counts.max.integrated'.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("guide.exposure_length.scale_count",
					&(Guide_Data.Exposure_Length_Scaling.Scale_Count));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 747;
		sprintf(Autoguider_General_Error_String,"Guide_Scaling_Config_Load:"
			"Failed to load config:'guide.exposure_length.scale_count'.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Scaling_Config_Load",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}

/**
 * Load guide dimension configuration.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see autoguider_general.html#Autoguider_General_Error
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Log_Format
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 */
static int Guide_Dimension_Config_Load(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Dimension_Config_Load",
			       LOG_VERBOSITY_TERSE,"GUIDE","started.");
#endif
	/* nb this code is replicated in autoguider_buffer.c : Autoguider_Buffer_Initialise.
	** We could perhaps only load from config once, and have getters in autoguider_buffer.c. */
	/* Also see Autoguider_Field (autoguider_field.c) */ 
	/* Guide_Initialise calls this routine to initialise these values sensibly for guide window setting,
	** Autoguider_Guide_On calls this to reload from config every time,
	** as we are going to allow dynamic reloading of the config file. */
	retval = CCD_Config_Get_Integer("ccd.guide.ncols",&(Guide_Data.Unbinned_NCols));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 704;
		sprintf(Autoguider_General_Error_String,"Guide_Dimension_Config_Load:Getting guide NCols failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.nrows",&(Guide_Data.Unbinned_NRows));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 705;
		sprintf(Autoguider_General_Error_String,"Guide_Dimension_Config_Load:Getting guide NRows failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.x_bin",&(Guide_Data.Bin_X));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 706;
		sprintf(Autoguider_General_Error_String,"Guide_Dimension_Config_Load:Getting guide X Binning failed.");
		return FALSE;
	}
	retval = CCD_Config_Get_Integer("ccd.guide.y_bin",&(Guide_Data.Bin_Y));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 707;
		sprintf(Autoguider_General_Error_String,"Guide_Dimension_Config_Load:Getting guide Y Binning failed.");
		return FALSE;
	}
	Guide_Data.Binned_NCols = Guide_Data.Unbinned_NCols / Guide_Data.Bin_X;
	Guide_Data.Binned_NRows = Guide_Data.Unbinned_NRows / Guide_Data.Bin_Y;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("guide","autoguider_guide.c","Guide_Dimension_Config_Load",
			       LOG_VERBOSITY_TERSE,"GUIDE","finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.39  2010/02/15 11:49:04  cjm
** Reset status char to '0' in Guide_Packet_Send.
** This was assumed to be a warning. However, the TCS sets the AGSTATE UNLOCKED
** when the status char is non-zero. This causes the RCS to issue another  "autoguider on"
** even when the current guide loop is still running.
**
** Revision 1.38  2009/01/30 18:01:33  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.37  2008/03/14 12:02:27  cjm
** Added Initial_Object_CCD_X/Y_Position data to hold initial position of
** object we are guiding upon.
** Autoguider_Guide_Set_Guide_Object now sets these.
** Guide_Packet_Send now uses Autoguider_Object_List_Get_Nearest_Object when multiple sources
** are detected in the guide window to select the source nearest Initial_Object_CCD_X/Y_Position.
** The guide packet is then sent back OK rather than failed.
**
** Revision 1.36  2007/08/20 14:48:37  cjm
** Added math.h, so prototype of log10 is defined correctly.
**
** Revision 1.35  2007/08/17 10:16:31  cjm
** Changed guide magnitude computation - broken into steps. Last version always returned "20.0" (const),
** I think because Exposure_Length was not cast to float. More logging to see which bit breaks now...
**
** Revision 1.34  2007/08/16 16:53:50  cjm
** Added logging for guide magnitude computation.
**
** Revision 1.33  2007/08/15 16:45:26  cjm
** Added extra checks for +ve Total_Counts in magnitude calculation.
**
** Revision 1.32  2007/08/15 16:18:48  cjm
** Upgraded mag calculation in  Guide_Packet_Send.
** Now needs 'guide.mag.const' configuration.
**
** Revision 1.31  2007/02/13 14:37:08  cjm
** Added Autoguider_CIL_SDB_Packet_Send call to Use_Cadence_For_SDB_Exp_Time code,
** so SDB Exp Time updated even if no centroid is found.
** Only really useful for testing...
**
** Revision 1.30  2007/02/13 13:52:04  cjm
** Filled in error code.
**
** Revision 1.29  2007/02/13 13:51:18  cjm
** Added Use_Cadence_For_SDB_Exp_Time implementation.
** The SDB Exposure Length can now be updated every loop with the loop cadence (total guide loop time),
** should may help the TCS to scale corrections better.
**
** Revision 1.28  2007/02/09 14:37:23  cjm
** Added Timecode_Scaling_Factor data.
** Initialised from "guide.timecode.scale",
** can be set and get, and used to scale the TCS UDP guide packet timecode.
**
** Revision 1.27  2007/01/30 17:35:24  cjm
** Added CCD_Temperature_Get/Autoguider_Buffer_Field_CCD_Temperature_Set calls
** to save CCD temperature per buffer, for later FITS header inclusion.
**
** Revision 1.26  2007/01/26 15:29:42  cjm
** Set buffers start time/exposure length data.
** Added getters for guide dimension data.
**
** Revision 1.25  2007/01/19 14:23:44  cjm
** Added autoguider guide window tracking:
** Guide_Window_Tracking_Struct
** Guide_Window_Tracking routine
** Autoguider_Guide_Set_Guide_Window_Tracking
** Autoguider_Guide_Get_Guide_Window_Tracking
** Autoguider_Guide_Window_Set_From_XY
**
** Revision 1.24  2006/11/28 19:07:53  cjm
** Changed log message.
**
** Revision 1.23  2006/11/06 14:49:02  cjm
** Trying to fix Guide_Exposure_Length_Scale comments.
**
** Revision 1.22  2006/11/06 12:19:00  cjm
** Initial guide exposure length implementation.
**
** Revision 1.21  2006/09/12 13:24:02  cjm
** Changed "Detected FWHM limit" log.
**
** Revision 1.20  2006/09/12 11:12:59  cjm
** Changed guide.ellipticity config retrieval to use CCD_Config_Get_Float rather than
** CCD_Config_Get_Integer. Hopefully will stop guide ellipticity test always thinking
** guide star was too elliptical (because guide ellipticity was zero?).
**
** Revision 1.19  2006/08/29 14:39:13  cjm
** Changed setting of guide window, so end x/y are always less than Binned_NCols/NRows (Autoguider_Guide_Set_Guide_Object).
**
** Revision 1.18  2006/08/29 13:55:31  cjm
** Added Autoguider_CIL_SDB_Packet_Send so that reseting SDB AG_STATE to idle
** is sent to the SDB on errors in the Guide_Thread.
** Added  Autoguider_CIL_SDB_Packet_State_Set and Autoguider_CIL_SDB_Packet_Send calls
** to Autoguider_Guide_Off so it tries to set the SDB state every time the autoguider
** is told to stop.
**
** Revision 1.17  2006/08/29 13:20:48  cjm
** SDB now uses AGG_STATE rather than AGS_STATE.
** Checks on position near edge of window relaxed.
**
** Revision 1.16  2006/07/20 16:07:51  cjm
** Changed SDB calls on failure.
**
** Revision 1.15  2006/07/20 15:15:03  cjm
** Added SDB updating calls.
**
** Revision 1.14  2006/07/17 13:45:39  cjm
** Made compilable.
**
** Revision 1.13  2006/07/17 13:43:11  cjm
** Added Guide_Dimension_Config_Load.
** Added calls to Guide_Dimension_Config_Load in Autoguider_Guide_Initialise and
** Autoguider_Guide_On.
** Autoguider_Guide_Set_Guide_Object now creates a window that doesn't exceed the number of rows/columns on chip.
**
** Revision 1.12  2006/07/16 20:13:54  cjm
** Added divide by zero protection on FWHMs.
**
** Revision 1.11  2006/06/29 20:39:38  cjm
** Made Guide_Packet_Send configurable.
**
** Revision 1.10  2006/06/29 17:04:34  cjm
** Changed window test so window start position always at least 1, 0 does not work.
**
** Revision 1.9  2006/06/27 20:45:02  cjm
** Relaxed FWHM constraint due to dodgy optics.
**
** Revision 1.8  2006/06/22 15:51:56  cjm
** Added Loop_Cadence to data for status reporting purposes.
** Added routine to return exposure length for status reporting purposes.
**
** Revision 1.7  2006/06/21 14:07:40  cjm
** Rewritten status char calculation following more info from TTL.
**
** Revision 1.6  2006/06/21 10:28:46  cjm
** Guide packets now send X and Y positions.
**
** Revision 1.5  2006/06/20 18:42:38  cjm
** Changed calculation of guide loop cadence so it is positive.
**
** Revision 1.4  2006/06/20 13:05:21  cjm
** More documentation.
**
** Revision 1.3  2006/06/12 19:22:08  cjm
** Added sending of TCS guide packets.
**
** Revision 1.2  2006/06/02 17:23:50  cjm
** Changed guide id calculation.
**
** Revision 1.1  2006/06/01 15:18:30  cjm
** Initial revision
**
*/
