/* autoguider_object.c
** Autoguider object detection routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_object.c,v 1.18 2014-01-31 17:15:45 cjm Exp $
*/
/**
 * Object detection routines for the autoguider program.
 * Uses libdprt_object.
 * Has it's own buffer, as Object_List_Get destroys the data within it's buffer argument.
 * @author Chris Mottram
 * @version $Revision: 1.18 $
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log_udp.h"

#include "dprt_libfits.h"
#include "object.h"
#include "ccd_config.h"

#include "autoguider_field.h"
#include "autoguider_general.h"
#include "autoguider_object.h"

/* hash defines */
/**
 * Maximum number of pixels to do stats on.
 */
#define MAXIMUM_STATS_COUNT (100000)

/* data types */
/**
 * Object threshold stats type enumeration. What algorithm we use to compute the object detection threshold, pixels
 * above this value are deemed to be part of an object.
 * <ul>
 * <li>OBJECT_THRESHOLD_STATS_TYPE_SIMPLE
 * <li>OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP
 * </ul>
 */
enum OBJECT_THRESHOLD_STATS_TYPE
{
	OBJECT_THRESHOLD_STATS_TYPE_SIMPLE,OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP
};

/**
 * Data type holding local data to autoguider_object. This consists of the following:
 * <dl>
 * <dt>Ellipticity_Limit</dt> <dd>Loaded from config, passed into the object detection routine, how round
 *                            an object has to be to be deemed a valid object</dd>
 * <dt>Threshold_Stats_Type</dt> <dd>A variable of type OBJECT_THRESHOLD_STATS_TYPE, loaded from config,
 *                                   used to determine the algorithm used to determine the background S.D.</dd>
 * <dt>Threshold_Sigma</dt> <dd>Loaded from config, used to compute the object detection threshold 
 *                              above the background.</dd>
 * <dt>Threshold_Sigma_Reject</dt> <dd>Loaded from config, used to compute the background S.D. 
 *                                 when Threshold_Stats_Type is OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP.</dd>
 * <dt>Min_Connected_Pixel_Count</dt> <dd>Number of connected pixels required for an object to be considered valid.</dd>
 * <dt>Binned_NCols</dt> <dd>Number of binned columns in the image_data.</dd>
 * <dt>Binned_NRows</dt> <dd>Number of binned rows in the image_data.</dd>
 * <dt>Image_Data</dt> <dd>Pointer to float data containing the image data.</dd>
 * <dt>Image_Data_Allocated_Pixel_Count</dt> <dd>Number of pixels allocated for Image_Data.</dd>
 * <dt>Image_Data_Mutex</dt> <dd>A mutex to lock access to the image data.</dd>
 * <dt>Object_List</dt> <dd>A list of Autoguider_Object_Struct containing the objects found in the image data.</dd>
 * <dt>Object_Count</dt> <dd>The number of objects currently in Object_List.</dd>
 * <dt>Allocated_Object_Count</dt> <dd>The number of objects allocated space for in Object_List.</dd>
 * <dt>Object_List_Mutex</dt> <dd>A mutex to lock access to the object list.</dd>
 * <dt>Stats_List</dt> <dd>A subset of pixel data messed around with to get mean/median/SD.</dd>
 * <dt>Stats_Count</dt> <dd>The number of pixels in Stats_List (up to a maximum of MAXIMUM_STATS_COUNT).</dd>
 * <dt>Median</dt> <dd>The median value in Stats_List.</dd>
 * <dt>Mean</dt> <dd>The mean value in Stats_List.</dd>
 * <dt>Background_Standard_Deviation</dt> <dd>The standard deviation of values in Stats_List.</dd>
 * <dt>Threshold</dt> <dd>The computed ADU threshold, pixels with values above the Threshold are deemed part of an object.</dd>
 * <dt>Id</dt> <dd>A unique Id for this particular guide/field run.</dd>
 * <dt>Frame_Number</dt> <dd>The guide/field frame number that generated these objects .</dd>
 * </dl>
 * @see #OBJECT_THRESHOLD_STATS_TYPE
 * @see #Autoguider_Object_Struct
 * @see #MAXIMUM_STATS_COUNT
 */
struct Object_Internal_Struct
{
	/* loaded config values */
	float Ellipticity_Limit;
	enum OBJECT_THRESHOLD_STATS_TYPE Threshold_Stats_Type;
	float Threshold_Sigma;
	float Threshold_Sigma_Reject;
	int Min_Connected_Pixel_Count;
	/* input image related data */
	int Binned_NCols;
	int Binned_NRows;
	float *Image_Data;
	int Image_Data_Allocated_Pixel_Count;
	pthread_mutex_t Image_Data_Mutex;
	/* object data */
	struct Autoguider_Object_Struct *Object_List;
	int Object_Count;
	int Allocated_Object_Count;
	pthread_mutex_t Object_List_Mutex;
	/* stats data */
	float Stats_List[MAXIMUM_STATS_COUNT];
	int Stats_Count;
	float Median;
	float Mean;
	float Background_Standard_Deviation;
	float Threshold;
	/* frame data */
	int Id;
	int Frame_Number;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_object.c,v 1.18 2014-01-31 17:15:45 cjm Exp $";
/**
 * Instance of object data.
 * @see #Object_Internal_Struct
 */
static struct Object_Internal_Struct Object_Data = 
{
	0.5,OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP,7.0,5.0,8,
	-1,-1,
	NULL,0,PTHREAD_MUTEX_INITIALIZER,
	NULL,0,0,PTHREAD_MUTEX_INITIALIZER,
	{0.0f,0.0f,0.0f,0.0f,0.0f},0,
	0.0f,0.0f,0.0f,0.0f,0,0
};

static int Object_Buffer_Set(float *buffer,int naxis1,int naxis2);
static int Object_Buffer_Copy(float *buffer,int naxis1,int naxis2);
static int Object_Create_Object_List(int use_standard_deviation,int start_x,int start_y);
static int Object_Set_Threshold(int use_standard_deviation);
static void Object_Fill_Stats_List(void);
static int Object_Get_Mean_Standard_Deviation_Simple(void);
static int Object_Get_Mean_Standard_Deviation_Sigma_Reject(void);
static int Object_Sort_Float_List(const void *p1, const void *p2);
static int Object_Sort_Object_List_By_Total_Counts(const void *p1, const void *p2);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise the object detection module.
 * <ul>
 * <li>Load the "object.ellipticity.limit" config from the configuration file and stored in Object_Data.Ellipticity_Limit.
 * <li>Set the object detection library stellar ellipticity by calling Object_Stellar_Ellipticity_Limit_Set with the 
 *     loaded config.
 * <li>We load the "object.threshold.stats.type" as a string, and set Object_Data.Threshold_Stats_Type based on whether 
 *     it is "simple" or "sigma_clip".
 * <li>We load "object.threshold.sigma" from config and set Object_Data.Threshold_Sigma, used to set the sigma above the
 *     background for object detection.
 * <li>We load "object.threshold.sigma.reject" from config and set Object_Data.Threshold_Sigma_Reject,which is 
 *     used to compute the background S.D. when Threshold_Stats_Type is OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP.
 * <li>We load "object.min_connected_pixel_count" from config and set Object_Data.Min_Connected_Pixel_Count,
 *     which is the number of connected pixels required for an object to be considered valid.
 * </ul>
 * @see #Object_Data
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Float
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../ccd/cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../../libdprt/object/cdocs/object.html#Object_Stellar_Ellipticity_Limit_Set
 */
int Autoguider_Object_Initialise(void)
{
	char *stats_type_string = NULL;
	int retval;
	
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Detect",LOG_VERBOSITY_TERSE,
			       "OBJECT","started.");
#endif
	/* load object module config */
	/* get/set ellipticity/is_stellar threshold */
	if(!CCD_Config_Get_Float("object.ellipticity.limit",&(Object_Data.Ellipticity_Limit)))
	{
		Autoguider_General_Error_Number = 1017;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to load config:'object.ellipticity.limit'.");
		return FALSE;
	}
	/* we can set this once here */
	if(!Object_Stellar_Ellipticity_Limit_Set(Object_Data.Ellipticity_Limit))
	{
		Autoguider_General_Error_Number = 1018;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to object stellar ellipticity to %f.",Object_Data.Ellipticity_Limit);
		return FALSE;
	}
	/* get object.threshold.stats.type to determine whether to use simple or sigma_clip stats. */
	retval = CCD_Config_Get_String("object.threshold.stats.type",&stats_type_string);
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1020;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to load config:'object.threshold.stats.type'.");
		return FALSE;
	}
	if(strcmp(stats_type_string,"simple")==0)
		Object_Data.Threshold_Stats_Type = OBJECT_THRESHOLD_STATS_TYPE_SIMPLE;
	else if(strcmp(stats_type_string,"sigma_clip")==0)
		Object_Data.Threshold_Stats_Type = OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP;
	else
	{
		Autoguider_General_Error_Number = 1022;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Config:'object.threshold.stats.type' had illegal value : %s.",stats_type_string);
		free(stats_type_string);
		return FALSE;
	}
	/* free allocated stats type string */
	free(stats_type_string);
	/* get object threshold sigma */
	retval = CCD_Config_Get_Float("object.threshold.sigma",&(Object_Data.Threshold_Sigma));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1023;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to load config:'object.threshold.sigma'.");
		return FALSE;
	}
	/* get threshold sigma reject */
	retval = CCD_Config_Get_Float("object.threshold.sigma.reject",&(Object_Data.Threshold_Sigma_Reject));
	if(retval == FALSE)
	{
		Autoguider_General_Error_Number = 1024;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to load config:'object.threshold.sigma.reject'.");
		return FALSE;
	}
	/* min connected pixel count */
	if(!CCD_Config_Get_Integer("object.min_connected_pixel_count",&(Object_Data.Min_Connected_Pixel_Count)))
	{
		Autoguider_General_Error_Number = 1025;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Initialise:"
			"Failed to load config:'object.min_connected_pixel_count'.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Initialise",LOG_VERBOSITY_TERSE,
			       "OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Detect objects on the passed in image data.
 * <ul>
 * <li>Calls Object_Buffer_Set to ensure the Object_Data's Image_Data buffer is allocated correctly.
 * <li>Calls Object_Buffer_Copy to copy the input buffer to the Object_Data's Image_Data (object detection
 *     is destructive to the input buffer).
 * <li>Sets the internal Id and Frame numbers.
 * <li>Calls Object_Create_Object_List to detect the objects from the object buffer and populate the Object_Data
 *     Object_List.
 * </ul>
 * @param buffer A float array containing the buffer with reduced data in it.
 * @param naxis1 The number of columns in the buffer.
 * @param naxis2 The number of rows in the buffer.
 * @param start_x The start of the buffer's X position on the physical CCD. 0 for full frame.
 * @param start_y The start of the buffer's Y position on the physical CCD. 0 for full frame.
 * @param use_standard_deviation Whether to use the frame's standard deviation when calculating object threshold 
 *        for detection. 
 * @param id An identifier for the buffer/exposure that is about to be object detected. 
 * @param frame_number The guide/field frame number that generated these objects.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Object_Buffer_Set
 * @see #Object_Buffer_Copy
 * @see #Object_Create_Object_List
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
int Autoguider_Object_Detect(float *buffer,int naxis1,int naxis2,int start_x,int start_y,int use_standard_deviation,
			     int id,int frame_number)
{
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Detect",LOG_VERBOSITY_TERSE,
			       "OBJECT","started.");
#endif
	if(buffer == NULL)
	{
		Autoguider_General_Error_Number = 1000;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Detect:buffer was NULL.");
		return FALSE;
	}
	if(!Object_Buffer_Set(buffer,naxis1,naxis2))
		return FALSE;
	if(!Object_Buffer_Copy(buffer,naxis1,naxis2))
		return FALSE;
	Object_Data.Id = id;
	Object_Data.Frame_Number = frame_number;
	if(!Object_Create_Object_List(use_standard_deviation,start_x,start_y))
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Detect",LOG_VERBOSITY_TERSE,
			       "OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Free up internal object data.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 */
int Autoguider_Object_Shutdown(void)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Shutdown",LOG_VERBOSITY_TERSE,
			       "OBJECT","started.");
#endif
	/* image data */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Object_Data.Image_Data != NULL)
		free(Object_Data.Image_Data);
	Object_Data.Image_Data = NULL;
	Object_Data.Image_Data_Allocated_Pixel_Count = 0;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* object list */
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Object_Data.Object_List != NULL)
		free(Object_Data.Object_List);
	Object_Data.Object_List = NULL;
	Object_Data.Object_Count = 0;
	Object_Data.Allocated_Object_Count = 0;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Shutdown",LOG_VERBOSITY_TERSE,
			       "OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Get the number of objects currently detected.
 * The relevant Object_List mutex is locked and un-locked.
 * @param count The address of an integer to store the count.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 */
int Autoguider_Object_List_Get_Count(int *count)
{
	int retval;

	if(count == NULL)
	{
		Autoguider_General_Error_Number = 1006;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Count:count was NULL.");
		return FALSE;
	}
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	(*count) = Object_Data.Object_Count;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	return TRUE;
}

/**
 * Get the specified detected object from the object list.
 * The relevant Object_List mutex is locked and un-locked.
 * @param index The index in the list of the object to retrieve.
 * @param object The address of an Autoguider_Object_Struct to store the selected object.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Autoguider_Object_Struct
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 */
int Autoguider_Object_List_Get_Object(int index,struct Autoguider_Object_Struct *object)
{
	int retval;

	if(object == NULL)
	{
		Autoguider_General_Error_Number = 1007;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Count:object was NULL.");
		return FALSE;
	}
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	if((index < 0)||(index >= Object_Data.Object_Count))
	{
		Autoguider_General_Error_Number = 1008;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Count:"
			"index '%d' out of range (0..%d).",index,Object_Data.Object_Count);
		/* unlock mutex after reporting error using 'locked' data Object_Count */
		Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
		return FALSE;
	}
	(*object) = Object_Data.Object_List[index];
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	return TRUE;
}

/**
 * Get the detected object nearest the specified CCD pixel position from the object list.
 * The relevant Object_List mutex is locked and un-locked.
 * @param ccd_x_position The X position on the CCD.
 * @param ccd_y_position The Y position on the CCD.
 * @param object The address of an Autoguider_Object_Struct to store the selected object.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Autoguider_Object_Struct
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 */
int Autoguider_Object_List_Get_Nearest_Object(float ccd_x_position,float ccd_y_position,
					      struct Autoguider_Object_Struct *object)
{
	int index,selected_object_index;
	float distance,closest_distance,xsq,ysq,xdiff,ydiff;
	int retval;

	if(object == NULL)
	{
		Autoguider_General_Error_Number = 1021;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Nearest_Object:object was NULL.");
		return FALSE;
	}
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 7
	Autoguider_General_Log_Format("object","autoguider_object.c",
				      "Autoguider_Object_List_Get_Nearest_Object",LOG_VERBOSITY_TERSE,"OBJECT",
				      "Selecting object nearest (%.2f,%.2f) from %d objects.",
				      ccd_x_position,ccd_y_position,Object_Data.Object_Count);
#endif
	closest_distance = 999999.9f;
	for(index = 0; index < Object_Data.Object_Count; index++)
	{
		/* NB no test against field object bounds
		** Probably not needed in this case - unless specified pixel is in a silly place */
		xdiff = Object_Data.Object_List[index].CCD_X_Position-ccd_x_position;
		ydiff = Object_Data.Object_List[index].CCD_Y_Position-ccd_y_position;
		xsq = xdiff*xdiff;
		ysq = ydiff*ydiff;
		distance = (float)sqrt((double)(xsq + ysq));
#if AUTOGUIDER_DEBUG > 9
		Autoguider_General_Log_Format("object","autoguider_object.c",
					      "Autoguider_Object_List_Get_Nearest_Object",
					      LOG_VERBOSITY_VERBOSE,"OBJECT",
					     "Object index %d (%.2f,%.2f) is %.2f pixels away from (%.2f,%.2f).",index,
					      Object_Data.Object_List[index].CCD_X_Position,
					      Object_Data.Object_List[index].CCD_Y_Position,
					      distance,ccd_x_position,ccd_y_position);
#endif
		if(distance <  closest_distance)
		{
			closest_distance = distance;
			selected_object_index = index;
		}
	}/* end for */
	(*object) = Object_Data.Object_List[selected_object_index];
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	return TRUE;
}

/**
 * Routine to select a suitable guide object from the list.
 * @param on_type How to select the AG guide object.
 * @param pixel_x If on_type is COMMAND_AG_ON_TYPE_PIXEL, the x pixel position.
 * @param pixel_y If on_type is COMMAND_AG_ON_TYPE_PIXEL, the y pixel position.
 * @param rank If on_type is COMMAND_AG_ON_TYPE_RANK, the rank (ordered index of brightness).
 *        According to the TCS user interface, should be in the range 1..10 (i.e. 1 based).
 * @param selected_object_index The address of an integer to store the selected object index.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see autoguider_command.html#COMMAND_AG_ON_TYPE
 * @see autoguider_field.html#Autoguider_Field_In_Object_Bounds
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Log
 */
int Autoguider_Object_Guide_Object_Get(enum COMMAND_AG_ON_TYPE on_type,float pixel_x,float pixel_y,
				       int rank,int *selected_object_index)
{
	int index,retval,in_bounds_count;
	float max_total_counts,distance,closest_distance,xsq,ysq,xdiff,ydiff;

	if(selected_object_index == NULL)
	{
		Autoguider_General_Error_Number = 1012;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Guide_Object_Get:"
			"Selected object index is NULL.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Guide_Object_Get",
			       LOG_VERBOSITY_TERSE,"OBJECT","started.");
#endif
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(Object_Data.Object_Count < 1)
	{
		/* unlock mutex */
		Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
		Autoguider_General_Error_Number = 1013;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Guide_Object_Get:No objects.");
		return FALSE;
	}
	(*selected_object_index) = -1;
	switch(on_type)
	{
		case COMMAND_AG_ON_TYPE_BRIGHTEST:
			index = 0;
			max_total_counts = 0.0f;
			while(index < Object_Data.Object_Count)
			{
				if(Autoguider_Field_In_Object_Bounds(Object_Data.Object_List[index].CCD_X_Position,
								     Object_Data.Object_List[index].CCD_Y_Position)&&
				   (Object_Data.Object_List[index].Total_Counts > max_total_counts))
				{
					max_total_counts = Object_Data.Object_List[index].Total_Counts;
					(*selected_object_index) = index;
#if AUTOGUIDER_DEBUG > 9
					Autoguider_General_Log_Format("object","autoguider_object.c",
								      "Autoguider_Object_Guide_Object_Get",
								      LOG_VERBOSITY_VERBOSE,"OBJECT",
								      "Object index %d (%.2f,%.2f) "
								      "is brightest so far (%.2f).",index,
							       Object_Data.Object_List[index].CCD_X_Position,
							       Object_Data.Object_List[index].CCD_Y_Position,
							       Object_Data.Object_List[index].Total_Counts);
#endif
				}
				index++;
			}/* end while */
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("object","autoguider_object.c",
						      "Autoguider_Object_Guide_Object_Get",LOG_VERBOSITY_VERBOSE,
						      "OBJECT","Brightest Object index %d (%.2f,%.2f) "
						  "with total counts (%.2f).",(*selected_object_index),
						 Object_Data.Object_List[(*selected_object_index)].CCD_X_Position,
						 Object_Data.Object_List[(*selected_object_index)].CCD_Y_Position,
						 Object_Data.Object_List[(*selected_object_index)].Total_Counts);
#endif
			break;
		case COMMAND_AG_ON_TYPE_PIXEL:
			index = 0;
			closest_distance = 999999.9f;
			while(index < Object_Data.Object_Count)
			{
				/* NB no test against field object bounds
				** Probably not needed in this case - unless specified pixel is in a silly place */
				xdiff = Object_Data.Object_List[index].CCD_X_Position-pixel_x;
				ydiff = Object_Data.Object_List[index].CCD_Y_Position-pixel_y;
				xsq = xdiff*xdiff;
				ysq = ydiff*ydiff;
				distance = (float)sqrt((double)(xsq + ysq));
#if AUTOGUIDER_DEBUG > 9
				Autoguider_General_Log_Format("object","autoguider_object.c",
							      "Autoguider_Object_Guide_Object_Get",
							      LOG_VERBOSITY_VERBOSE,"OBJECT",
							      "Object index %d (%.2f,%.2f) "
							      "is %.2f pixels away from (%.2f,%.2f).",index,
							      Object_Data.Object_List[index].CCD_X_Position,
							      Object_Data.Object_List[index].CCD_Y_Position,
							      distance,pixel_x,pixel_y);
#endif
				if(distance <  closest_distance)
				{
					closest_distance = distance;
					(*selected_object_index) = index;
				}
				index++;
			}/* end while */
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("object","autoguider_object.c",
						      "Autoguider_Object_Guide_Object_Get",LOG_VERBOSITY_VERBOSE,
						      "OBJECT","Closest Object index %d (%.2f,%.2f) "
					       "is %.2f pixels away from (%.2f,%.2f).",(*selected_object_index),
					       Object_Data.Object_List[(*selected_object_index)].CCD_X_Position,
					       Object_Data.Object_List[(*selected_object_index)].CCD_Y_Position,
					       closest_distance,pixel_x,pixel_y);
#endif
			break;
		case COMMAND_AG_ON_TYPE_RANK:
			/* assumes object list sorted by total counts (Object_Sort_Object_List_By_Total_Counts) */
			/* assumes "first" rank is "1", not "0", as documented in the TCS manual */
			index = 0;
			in_bounds_count = 0;
			while((index < Object_Data.Object_Count)&&(in_bounds_count < rank))
			{
				if(Autoguider_Field_In_Object_Bounds(Object_Data.Object_List[index].CCD_X_Position,
								     Object_Data.Object_List[index].CCD_Y_Position))
				{
					in_bounds_count++;
				}
				index++;
			}/* end while */
			if(in_bounds_count < rank)
			{
				Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
				Autoguider_General_Error_Number = 1016;
				sprintf(Autoguider_General_Error_String,"Autoguider_Object_Guide_Object_Get:"
					"Rank %d out of range 0..%d.",rank,Object_Data.Object_Count);
				return FALSE;
			}
			(*selected_object_index) = index;
			break;
		default:
			/* unlock mutex */
			Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
			Autoguider_General_Error_Number = 1014;
			sprintf(Autoguider_General_Error_String,"Autoguider_Object_Guide_Object_Get:"
				"Illegal on_type %d.",on_type);
			return FALSE;
			break;
	}
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* check results */
	if((*selected_object_index) == -1)
	{
		Autoguider_General_Error_Number = 1015;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_Guide_Object_Get:"
			"Failed to find selected object for on_type %d.",on_type);
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Autoguider_Object_Guide_Object_Get",
			       LOG_VERBOSITY_TERSE,"OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Get the object list formatted as a string, in the form:
 * <pre>
 * Id Frame Number Index CCD X Pos  CCD Y Pos Buffer X Pos  Buffer Y Pos  Total Counts Number of Pixels Peak Counts Is_Stellar FWHM_X FWHM_Y
 * </pre>
 * The relevant Object_List mutex is locked and un-locked.
 * See ngat.autoguider.command.StatusObjectList.java, parseReplyString for GUI software that decodes this string.
 * @param object_list_string The address of a character pointer to malloc and store a string.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Autoguider_Object_Struct
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Add_String
 * @see ../javadocs/ngat/autoguider/command/StatusObjectList.html#parseReplyString
 */
int Autoguider_Object_List_Get_Object_List_String(char **object_list_string)
{
	char buff[256];
	char is_stellar_string[32];
	int i,retval;

	if(object_list_string == NULL)
	{
		Autoguider_General_Error_Number = 1009;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Object_List_String:"
			"object_list_string was NULL.");
		return FALSE;
	}
	if((*object_list_string) != NULL)
	{
		Autoguider_General_Error_Number = 1011;
		sprintf(Autoguider_General_Error_String,"Autoguider_Object_List_Get_Object_List_String:"
			"object_list_string was not initialised to NULL.");
		return FALSE;
	}
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	if(!Autoguider_General_Add_String(object_list_string,"Id Frame_Number Index CCD_Pos_X  CCD_Pos_Y "
		"Buffer_Pos_X Buffer_Pos_Y Total_Counts Number_of_Pixels Peak_Counts Is_Stellar FWHM_X FWHM_Y"))
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
		return FALSE;
	}
	if(!Autoguider_General_Add_String(object_list_string,"\n"))
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
		return FALSE;
	}
	for(i=0;i<Object_Data.Object_Count;i++)
	{
		if(Object_Data.Object_List[i].Is_Stellar)
			strcpy(is_stellar_string,"TRUE");
		else
			strcpy(is_stellar_string,"FALSE");
		retval = sprintf(buff,"%6d %6d %6d %6.2f %6.2f %6.2f %6.2f %6.2f %6d %6.2f %s %6.2f %6.2f",
			Object_Data.Id,Object_Data.Frame_Number,Object_Data.Object_List[i].Index,
			Object_Data.Object_List[i].CCD_X_Position,Object_Data.Object_List[i].CCD_Y_Position,
			Object_Data.Object_List[i].Buffer_X_Position,Object_Data.Object_List[i].Buffer_Y_Position,
			Object_Data.Object_List[i].Total_Counts,Object_Data.Object_List[i].Pixel_Count,
			Object_Data.Object_List[i].Peak_Counts,is_stellar_string,
			Object_Data.Object_List[i].FWHM_X,Object_Data.Object_List[i].FWHM_Y);
#if AUTOGUIDER_DEBUG > 1
		Autoguider_General_Log_Format("object","autoguider_object.c",
					      "Autoguider_Object_List_Get_Object_List_String",
					      LOG_VERBOSITY_VERBOSE,"OBJECT","index %d : '%s' (%d bytes).",
					      i,buff,retval);
#endif
		if(!Autoguider_General_Add_String(object_list_string,buff))
		{
			Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
			return FALSE;
		}
		if(!Autoguider_General_Add_String(object_list_string,"\n"))
		{
			Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
			return FALSE;
		}
	}
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	return TRUE;
}

/**
 * Return the median counts in the last image that had the Autoguider_Object_Detect object detection routine run on it.
 * @return The median counts in ADU of the last buffer passed to Autoguider_Object_Detect, as stored in Object_Data.
 * @see #Object_Data
 */
float Autoguider_Object_Median_Get(void)
{
	return Object_Data.Median;
}

/**
 * Return the mean counts in the last image that had the Autoguider_Object_Detect object detection routine run on it.
 * @return The mean counts in ADU of the last buffer passed to Autoguider_Object_Detect, as stored in Object_Data.
 * @see #Object_Data
 */
float Autoguider_Object_Mean_Get(void)
{
	return Object_Data.Mean;
}

/**
 * Return the background standard deviation in the last image that had the Autoguider_Object_Detect object detection routine run on it.
 * @return The background standard deviation in ADU of the last buffer passed to Autoguider_Object_Detect, as stored in Object_Data.
 * @see #Object_Data
 */
float Autoguider_Object_Background_Standard_Deviation_Get(void)
{
	return Object_Data.Background_Standard_Deviation;
}

/**
 * Return the computed threshold used to detect objects in the last image that had the Autoguider_Object_Detect object detection routine run on it.
 * @return The detect objection threshold in ADU of the last buffer passed to Autoguider_Object_Detect, as stored in Object_Data.
 * @see #Object_Data
 */
float Autoguider_Object_Threshold_Get(void)
{
	return Object_Data.Threshold;
}

/**
 * Return the number of connected pixels with above-threshold pixel values required for a detected object to be deemed
 * valid, as loaded as a config value, and passed into the object detection routine.
 * @return The number of connected pixels with above-threshold pixel values required for a detected object 
 *         to be deemed valid, as stored in Object_Data.
 * @see #Object_Data
 */
int Autoguider_Object_Min_Connected_Pixel_Count_Get(void)
{
	return Object_Data.Min_Connected_Pixel_Count;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Setup buffer for object detection.
 * Locks the Image_Data_Mutex whilst resizing the buffer.
 * @param buffer A float array containing the buffer with reduced data in it.
 * @param naxis1 The number of columns in the buffer.
 * @param naxis2 The number of rows in the buffer.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
static int Object_Buffer_Set(float *buffer,int naxis1,int naxis2)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Buffer_Set",LOG_VERBOSITY_TERSE,
			       "OBJECT","started.");
#endif
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* if there is not enough memory allocated, allocate more */
	if((naxis1*naxis2) > Object_Data.Image_Data_Allocated_Pixel_Count)
	{
		/* we need to allocate more space for image data */
		if(Object_Data.Image_Data == NULL)
		{
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("object","autoguider_object.c","Object_Buffer_Set",
						      LOG_VERBOSITY_VERBOSE,"OBJECT","Allocating Image_Data (%d,%d).",
						      naxis1,naxis2);
#endif
			Object_Data.Image_Data = (float *)malloc((naxis1*naxis2)*sizeof(float));
		}
		else
		{
#if AUTOGUIDER_DEBUG > 9
			Autoguider_General_Log_Format("object","autoguider_object.c","Object_Buffer_Set",
						      LOG_VERBOSITY_VERBOSE,"OBJECT",
						      "Reallocating Image_Data (%d,%d).",naxis1,naxis2);
#endif
			Object_Data.Image_Data = (float *)realloc(Object_Data.Image_Data,
								  (naxis1*naxis2)*sizeof(float));
		}
		if(Object_Data.Image_Data == NULL)
		{
			Object_Data.Image_Data_Allocated_Pixel_Count = 0;
			Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
			Autoguider_General_Error_Number = 1001;
			sprintf(Autoguider_General_Error_String,"Object_Buffer_Set:"
				"failed to allocate object buffer (%d,%d).",naxis1,naxis2);
			return FALSE;
		}
		Object_Data.Image_Data_Allocated_Pixel_Count = naxis1*naxis2;
	}
	/* update dimensional information */
	Object_Data.Binned_NCols = naxis1;
	Object_Data.Binned_NRows = naxis2;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Buffer_Set",
			       LOG_VERBOSITY_TERSE,"OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Copy buffer for object detection. Assumes Object_Buffer_Set has been already called, so the buffer is
 * the right size.
 * Locks the Image_Data_Mutex whilst copying.
 * @param buffer A float array containing the buffer with reduced data in it.
 * @param naxis1 The number of columns in the buffer.
 * @param naxis2 The number of rows in the buffer.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
static int Object_Buffer_Copy(float *buffer,int naxis1,int naxis2)
{
	int retval;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Buffer_Copy",
			       LOG_VERBOSITY_INTERMEDIATE,"OBJECT","started.");
#endif
	if((naxis1*naxis2) > Object_Data.Image_Data_Allocated_Pixel_Count)
	{
		Autoguider_General_Error_Number = 1002;
		sprintf(Autoguider_General_Error_String,"Object_Buffer_Copy:"
			"Allocated object buffer too small (%d * %d) > %d.",naxis1,naxis2,
			Object_Data.Image_Data_Allocated_Pixel_Count);
		return FALSE;
	}
	/* lock mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	memcpy(Object_Data.Image_Data,buffer,(naxis1*naxis2)*sizeof(float));
	Object_Data.Binned_NCols = naxis1;
	Object_Data.Binned_NRows = naxis2;
	/* unlock mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Buffer_Copy",
			       LOG_VERBOSITY_INTERMEDIATE,"OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Create the object list from the copied image data.
 * Locks the Image_Data_Mutex whilst accessing the image data.
 * Locks the Object_List_Mutex whilst modifying the object list.
 * Currently sorted (after setting the index!) into total count order (Object_Sort_Object_List_By_Total_Counts).
 * Object_Set_Threshold is used to compute the threshold pixel value, above which pixels are deemed to be part of objects.
 * The minimum number of connected pixels needed for an object to be valid is read from the Object_Data.Min_Connected_Pixel_Count
 * variable, which has been loaded from config as part of Autoguider_Object_Initialise.
 * @param use_standard_deviation A boolean, whether to use standard deviation when calculating the object 
 *        threshold value. The SD is useful for sky gradients on field buffers, but the guide buffer SD is
 *        skewed by being mostly filled (hopefully) with a star, and so this variable should be set to FALSE
 *        for guide buffers.
 * @param start_x The start of the buffer's X position on the physical CCD. 0 for full frame.
 * @param start_y The start of the buffer's Y position on the physical CCD. 0 for full frame.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Object_Set_Threshold
 * @see #Object_Sort_Object_List_By_Total_Counts
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 * @see ../../libdprt/object/cdocs/object.html#Object_List_Get
 * @see ../../libdprt/object/cdocs/object.html#Object_Get_Error_Number
 * @see ../../libdprt/object/cdocs/object.html#Object_Warning
 * @see ../../libdprt/object/cdocs/object.html#Object_Stellar_Ellipticity_Limit_Set
 */
static int Object_Create_Object_List(int use_standard_deviation,int start_x,int start_y)
{
	Object *object_list = NULL;
	Object *current_object_ptr = NULL;
	struct timespec start_time,stop_time;
	int retval,seeing_flag,index;
	float seeing;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_INTERMEDIATE,"OBJECT","started.");
#endif
	if(!AUTOGUIDER_GENERAL_IS_BOOLEAN(use_standard_deviation))
	{
		Autoguider_General_Error_Number = 1010;
		sprintf(Autoguider_General_Error_String,"Object_Create_Object_List:"
			"Non boolean value for use_standard_deviation(%d).",use_standard_deviation);
		return FALSE;
	}
	/* lock image data mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* This is already done in initialise.
	** We don't need to do this every time we call this routine, unless we add an API to dynamically allow 
	** modification of the value during runtime */
	if(!Object_Stellar_Ellipticity_Limit_Set(Object_Data.Ellipticity_Limit))
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
		return FALSE;
	}
	/* get median/threshold */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Getting statistics.");
#endif
	if(!Object_Set_Threshold(use_standard_deviation))
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Starting object detection.");
#endif
	/* clock_gettime(CLOCK_REALTIME,&start_time);*/
	/* Call the object detection code */
	retval = Object_List_Get(Object_Data.Image_Data,Object_Data.Median,Object_Data.Binned_NCols,
				 Object_Data.Binned_NRows,Object_Data.Threshold,Object_Data.Min_Connected_Pixel_Count,
				 &object_list,&seeing_flag,&seeing);
	/* clock_gettime(CLOCK_REALTIME,&stop_time);*/
	if(retval == FALSE)
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
		Autoguider_General_Error_Number = 1003;
		sprintf(Autoguider_General_Error_String,"Object_Create_Object_List:Object_List_Get failed.");
		return FALSE;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Object detection finished.");
#endif
	/* unlock image data mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Image_Data_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* lock object list mutex */
	retval = Autoguider_General_Mutex_Lock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
	/* copy objects to list */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Counting number of detected objects.");
#endif
	/* count number of detected objects */
	Object_Data.Object_Count = 0;
	current_object_ptr = object_list;
	while(current_object_ptr != NULL)
	{
		Object_Data.Object_Count++;
		current_object_ptr = current_object_ptr->nextobject;
	}
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Create_Object_List",
				      LOG_VERBOSITY_VERBOSE,"OBJECT","We detected %d objects in the frame.",
				      Object_Data.Object_Count);
#endif
	/* reallocate if there is not enough space */
	if(Object_Data.Object_Count > Object_Data.Allocated_Object_Count)
	{
		if(Object_Data.Object_List == NULL)
		{
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("object","autoguider_object.c","Object_Create_Object_List",
						      LOG_VERBOSITY_VERBOSE,"OBJECT",
					   "Allocating Object_List from %d to %d.",Object_Data.Allocated_Object_Count,
					   Object_Data.Object_Count);
#endif
			Object_Data.Object_List = (struct Autoguider_Object_Struct*)malloc(Object_Data.Object_Count*
						   sizeof(struct Autoguider_Object_Struct));
		}
		else
		{
#if AUTOGUIDER_DEBUG > 5
			Autoguider_General_Log_Format("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT",
				    "Reallocating Object_List from %d to %d.",Object_Data.Allocated_Object_Count,
					      Object_Data.Object_Count);
#endif
			Object_Data.Object_List = (struct Autoguider_Object_Struct*)realloc(Object_Data.Object_List,
					  Object_Data.Object_Count*sizeof(struct Autoguider_Object_Struct));
		}
		if(Object_Data.Object_List == NULL)
		{
			Object_Data.Object_Count = 0;
			Object_Data.Allocated_Object_Count = 0;
			Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
			Autoguider_General_Error_Number = 1004;
			sprintf(Autoguider_General_Error_String,"Object_Create_Object_List:"
				"Allocating Object_List failed(%d).",Object_Data.Object_Count);
			return FALSE;
		}
		Object_Data.Allocated_Object_Count = Object_Data.Object_Count;
	}
	/* and do the actual copy */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Copying Objects to Object_List.");
#endif
	current_object_ptr = object_list;
	index = 0;
	while(current_object_ptr != NULL)
	{
		Object_Data.Object_List[index].Index = index;
		Object_Data.Object_List[index].CCD_X_Position = current_object_ptr->xpos + start_x;
		Object_Data.Object_List[index].CCD_Y_Position = current_object_ptr->ypos + start_y;
		Object_Data.Object_List[index].Buffer_X_Position = current_object_ptr->xpos;
		Object_Data.Object_List[index].Buffer_Y_Position = current_object_ptr->ypos;
		Object_Data.Object_List[index].Total_Counts = current_object_ptr->total;
		Object_Data.Object_List[index].Pixel_Count = current_object_ptr->numpix;
		Object_Data.Object_List[index].Peak_Counts = current_object_ptr->peak;
		Object_Data.Object_List[index].Is_Stellar = current_object_ptr->is_stellar;
		Object_Data.Object_List[index].FWHM_X = current_object_ptr->fwhmx;
		Object_Data.Object_List[index].FWHM_Y = current_object_ptr->fwhmy;
#if AUTOGUIDER_DEBUG > 5
		if(index < 1000) /* try to speed up situations where too many objects are being detected */
		{
			if(index == 0)
			{
				Autoguider_General_Log_Format("object","autoguider_object.c",
							      "Object_Create_Object_List",LOG_VERBOSITY_VERBOSE,
							      "OBJECT","Id,Frame Number,Index,CCD X,"
							      "CCD Y,Buffer X,Buffer Y,Total Counts,No of Pixels,"
							      "Peak Counts,Is Stellar,FWHM X,FWHM Y");
			}
			Autoguider_General_Log_Format("object","autoguider_object.c","Object_Create_Object_List",
						      LOG_VERBOSITY_VERBOSE,"OBJECT",
					     "List %d,%d,%d,%6.2f,%6.2f,%6.2f,%6.2f,%6.2f,%6d,%6.2f,%s,%6.2f,%6.2f",
			  Object_Data.Id,Object_Data.Frame_Number,Object_Data.Object_List[index].Index,
			  Object_Data.Object_List[index].CCD_X_Position,Object_Data.Object_List[index].CCD_Y_Position,
 			  Object_Data.Object_List[index].Buffer_X_Position,
						      Object_Data.Object_List[index].Buffer_Y_Position,
                          Object_Data.Object_List[index].Total_Counts,Object_Data.Object_List[index].Pixel_Count,
                          Object_Data.Object_List[index].Peak_Counts,
			  Object_Data.Object_List[index].Is_Stellar ? "TRUE" : "FALSE",
			  Object_Data.Object_List[index].FWHM_X,Object_Data.Object_List[index].FWHM_Y);
		}
#endif
		/* update pointer/index for next one */
		current_object_ptr = current_object_ptr->nextobject;
		index++;
	}
	/* Free object library objects */
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_VERBOSE,"OBJECT","Freeing object library objects.");
#endif
	if(!Object_List_Free(&object_list))
	{
		Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
		Autoguider_General_Error_Number = 1005;
		sprintf(Autoguider_General_Error_String,"Object_Create_Object_List:Object_List_Free failed.");
		return FALSE;
	}
	/* sort by total (integrated) counts */
	qsort(Object_Data.Object_List,Object_Data.Object_Count,sizeof(struct Autoguider_Object_Struct),
	      Object_Sort_Object_List_By_Total_Counts);
	/* unlock object list mutex */
	retval = Autoguider_General_Mutex_Unlock(&(Object_Data.Object_List_Mutex));
	if(retval == FALSE)
		return FALSE;
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Create_Object_List",
			       LOG_VERBOSITY_TERSE,"OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Set up the Stats_List with a (subset) of pixels in Image_Data, using Object_Fill_Stats_List.
 * Find the mean, median and standard deviation of the subset, using Object_Get_Mean_Standard_Deviation_Simple or
 * Object_Get_Mean_Standard_Deviation_Sigma_Reject.
 * Assumes the Image_Data_Mutex has <b>already</b> been locked external to this routine, as it access
 * the Image_Data_List to create the Stats_List.
 * We use Object_Data.Threshold_Stats_Type, loaded from the config file during initialisation from 
 * <b>object.threshold.stats.type</b>,  to determine whether to use simple or sigma clipping stats 
 * when determining the threshold value.
 * We use Object_Data.Threshold_Sigma, loaded from the config file during initialisation from 
 *  <b>object.threshold.sigma</b> to determine the threshold value in this routine.
 * @param use_standard_deviation Whether to use the frame's standard deviation when calculating object threshold 
 *        for detection. Set to TRUE for field, FALSE for guide where the window is mainly filled with star.
 * @return The routine returns TRUE on success, and FALSE on failure.
 * @see #Object_Data
 * @see #Object_Sort_Float_List
 * @see #Object_Fill_Stats_List
 * @see #Object_Get_Mean_Standard_Deviation_Simple
 * @see #Object_Get_Mean_Standard_Deviation_Sigma_Reject
 * @see #Image_Data
 * @see autoguider_general.html#Autoguider_General_Log
 * @see autoguider_general.html#Autoguider_General_Mutex_Lock
 * @see autoguider_general.html#Autoguider_General_Mutex_Unlock
 * @see autoguider_general.html#Autoguider_General_Error_Number
 * @see autoguider_general.html#Autoguider_General_Error_String
 */
static int Object_Set_Threshold(int use_standard_deviation)
{
	int retval;
	float total_value,difference_squared_total,tmp_float,variance,threshold_sigma;

#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Set_Threshold",
			       LOG_VERBOSITY_INTERMEDIATE,"OBJECT","started.");
#endif
	/* get a subset of image data into the Stats_List/Stats_Count */
	Object_Fill_Stats_List();
	/* median */
	qsort(Object_Data.Stats_List,Object_Data.Stats_Count,sizeof(float),Object_Sort_Float_List);
	Object_Data.Median = Object_Data.Stats_List[Object_Data.Stats_Count/2];
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Set_Threshold",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT",
				      "Median pixel value %.2f.",Object_Data.Median);
#endif
	/* calculate the mean and standard deviation using the algorithm determined from config at Initialise time */
	if(Object_Data.Threshold_Stats_Type = OBJECT_THRESHOLD_STATS_TYPE_SIMPLE)
	{
		/* get a simple mean and standard devation from the Stats_List/Stats_Count 
		 * and save into Object_Data.Mean / Object_Data.Background_Standard_Deviation */
		if(!Object_Get_Mean_Standard_Deviation_Simple())
		{
			return FALSE;
		}
	}
	else if(Object_Data.Threshold_Stats_Type = OBJECT_THRESHOLD_STATS_TYPE_SIGMA_CLIP)
	{
		/* get mean and standard devation from the Stats_List/Stats_Count using sigma clipping/rejection 
		 * and save into Object_Data.Mean / Object_Data.Background_Standard_Deviation */
		if(!Object_Get_Mean_Standard_Deviation_Sigma_Reject())
		{
			return FALSE;
		}
	}
	else
	{
		Autoguider_General_Error_Number = 1019;
		sprintf(Autoguider_General_Error_String,"Object_Set_Threshold:"
			"Object_Data.Threshold_Stats_Type had illegal value : %d.",Object_Data.Threshold_Stats_Type);
		return FALSE;
	}
	/* calculate threshold 
	** NB Median is of all Stats_List pixels, Background_Standard_Deviation may not be if iterstat is used. */
	if(use_standard_deviation)
		Object_Data.Threshold = Object_Data.Median+(Object_Data.Threshold_Sigma*
							    Object_Data.Background_Standard_Deviation);
	else
		Object_Data.Threshold = Object_Data.Median;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Set_Threshold",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT",
			      "Using standard deviation = %d (%.2f), Threshold Sigma = %.2f, Threshold value %.2f.",
				      use_standard_deviation,Object_Data.Background_Standard_Deviation,
				      Object_Data.Threshold_Sigma,Object_Data.Threshold);
#endif
#if AUTOGUIDER_DEBUG > 1
	Autoguider_General_Log("object","autoguider_object.c","Object_Set_Threshold",
			       LOG_VERBOSITY_INTERMEDIATE,"OBJECT","finished.");
#endif
	return TRUE;
}

/**
 * Routine to fill Object_Data.Stats_Count/Object_Data.Stats_List with a sensible subset of of
 * Object_Data.Image_Data.
 * @see #Object_Data
 * @see #MAXIMUM_STATS_COUNT
 * @see autoguider_general.html#Autoguider_General_Log_Format
 */
static void Object_Fill_Stats_List(void)
{
	int i,pixel_count;

	/* fill Stats_List with subset of Image_Data */
	pixel_count = (Object_Data.Binned_NCols*Object_Data.Binned_NRows);
	Object_Data.Stats_Count = MIN(pixel_count,MAXIMUM_STATS_COUNT);
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Fill_Stats_List",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT",
				      "Using %d pixels in Stats list.",Object_Data.Stats_Count);
#endif
	for(i=0;i<Object_Data.Stats_Count;i++)
	{
		/* take every nth pixel into the stats array */
		Object_Data.Stats_List[i] = Object_Data.Image_Data[i*(pixel_count/Object_Data.Stats_Count)];
	}
}

/**
 * Get a simple mean and standard deviation measure from Object_Data.Stats_List / Object_Data.Stats_Count.
 * These are stored into Object_Data.Mean / Object_Data.Background_Standard_Deviation.
 * @see #Object_Data
 * @see autoguider_general.html#Autoguider_General_Log_Format
 */
static int Object_Get_Mean_Standard_Deviation_Simple(void)
{
	int i;
	float total_value,difference_squared_total,tmp_float,variance;

	/* find the total value of the Stats_List */
	total_value = 0.0f;
	for(i=0;i<Object_Data.Stats_Count;i++)
	{
		total_value += Object_Data.Stats_List[i];
	}
	/* mean */
	Object_Data.Mean = total_value/Object_Data.Stats_Count;
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Get_Mean_Standard_Deviation_Simple",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT","Mean pixel value %.2f.",Object_Data.Mean);
#endif
	/* standard deviation */
	difference_squared_total = 0.0f;
	for(i=0;i<Object_Data.Stats_Count;i++)
	{
		tmp_float = Object_Data.Stats_List[i]-Object_Data.Mean;
		difference_squared_total += tmp_float*tmp_float;
	}
	variance = difference_squared_total/Object_Data.Stats_Count;
	/* background SD */
	Object_Data.Background_Standard_Deviation = (float)sqrt(((double)variance));
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Get_Mean_Standard_Deviation_Simple",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT",
				      "Background Standard Deviation %.2f (variance %.2f).",
				      Object_Data.Background_Standard_Deviation,variance);
#endif
	return TRUE;
}

/**
 * Get Mean and Standard deviation using a sigma clipped RMS.
 * This routine calls iterstat in dprt_libfits.c (DpRt). The sigma reject parameter is used from 
 * Object_Data.Threshold_Sigma_Reject, which is loaded from the configuration fail during initialisation.
 * @see #Object_Data
 */
static int Object_Get_Mean_Standard_Deviation_Sigma_Reject(void)
{
	int retval;

	retval = iterstat(Object_Data.Stats_List,Object_Data.Stats_Count,Object_Data.Threshold_Sigma_Reject,
			  &(Object_Data.Mean),&(Object_Data.Background_Standard_Deviation));
#if AUTOGUIDER_DEBUG > 5
	Autoguider_General_Log_Format("object","autoguider_object.c","Object_Get_Mean_Standard_Deviation_Sigma_Reject",
				      LOG_VERBOSITY_INTERMEDIATE,"OBJECT",
				      "Mean = %.2f, Background Standard Deviation = %.2f, retval = %d.",
				      Object_Data.Mean,Object_Data.Background_Standard_Deviation,retval);
#endif
	return TRUE;
}

/**
 * Float list sort comparator, for use with qsort.
 */
static int Object_Sort_Float_List(const void *p1, const void *p2)
{
	float f1,f2;

	f1 = (float)(*(float*)p1);
	f2 = (float)(*(float*)p2);
	if(f1 > f2)
		return -1;
	else if(f2 > f1)
		return 1;
	else
		return 0;
}

/**
 * Autoguider_Object_Struct list sort comparator, for use with qsort. Sorts by total (integrated) counts.
 */
static int Object_Sort_Object_List_By_Total_Counts(const void *p1, const void *p2)
{
	struct Autoguider_Object_Struct a1,a2;
	float f1,f2;

	a1 = (struct Autoguider_Object_Struct)(*(struct Autoguider_Object_Struct*)p1);
	a2 = (struct Autoguider_Object_Struct)(*(struct Autoguider_Object_Struct*)p2);
	if(a1.Total_Counts > a2.Total_Counts)
		return -1;
	else if(a2.Total_Counts > a1.Total_Counts)
		return 1;
	else
		return 0;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.17  2009/06/12 13:41:25  cjm
** Object_List_Get ellipticity/is_stellar limit/flag now configured from object.ellipticity.limit.
**
** Revision 1.16  2009/01/30 18:01:33  cjm
** Changed log messges to use log_udp verbosity (absolute) rather than bitwise.
**
** Revision 1.15  2008/03/14 12:04:01  cjm
** Added Autoguider_Object_List_Get_Nearest_Object to return the
** object nearest a certain pixel position. Used to fix
** "multiple sources in the guide window" problem.
**
** Revision 1.14  2007/09/26 17:12:59  cjm
** sp fix theshold -> threshold.
**
** Revision 1.13  2007/08/29 18:09:27  cjm
** Added comment.
**
** Revision 1.12  2007/08/29 17:20:01  cjm
** Fixed sp in Object_Set_Threshold.
**
** Revision 1.11  2007/08/29 17:01:13  cjm
** Rewrote stats generation as per bug #1298.
** Can now choose between simple mean/stddev and iterstat (iterative sigma clipping, which should ignore
** the star flux when calculating the std. deviation of the background, thus producing a smaller/better threshold value).
** More confiurable as well.
**
** Revision 1.10  2007/02/08 17:56:17  cjm
** Fixed Autoguider_Object_List_Get_Object_List_String comment.
**
** Revision 1.9  2006/11/14 18:08:54  cjm
** Changed Autoguider_Object_Guide_Object_Get so it uses Autoguider_Field_In_Object_Bounds
** to ensure selected guide objects are inside a nominal field object bounds on the CCD.
**
** Revision 1.8  2006/09/28 10:09:12  cjm
** Removed test for Object_Get_Error_Number() == 7, as
** this error should now return true (+warning) in libdprt_object.
**
** Revision 1.7  2006/09/26 15:12:35  cjm
** Reformatted object logging.
**
** Revision 1.6  2006/07/14 09:35:12  cjm
** Recalculated distance measure so finding object near a pixel actually works.
** FWHMX/Y always set from libdprt_object value - now always calulated even when not stellar.
**
** Revision 1.5  2006/06/29 17:04:34  cjm
** More logging
**
** Revision 1.4  2006/06/27 20:45:02  cjm
** Added Autoguider_Object_Guide_Object_Get.
** Object_List now sorted by total counts (Object_Sort_Object_List_By_Total_Counts) to make autoguide on rank easier.
**
** Revision 1.3  2006/06/21 17:09:09  cjm
** Made Object_Create_Object_List ignore Object error 7 : All objects were too small.
**
** Revision 1.2  2006/06/02 13:46:56  cjm
** Added FWHM tests. EXtra logging/checks to try and diagnose seg faults (actually in command server logging).
**
** Revision 1.1  2006/06/01 15:18:38  cjm
** Initial revision
**
*/
