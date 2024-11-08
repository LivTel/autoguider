/* andor_setup.c
** Autoguider Andor CCD Library setup routines
** $Header: /home/cjm/cvs/autoguider/ccd/andor/c/andor_setup.c,v 1.5 2014-02-03 09:52:06 cjm Exp $
*/
/**
 * Setup routines for the Andor autoguider CCD library.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
/* andor CCD library */
#include "atmcdLXd.h"
#include "log_udp.h"
#include "ccd_general.h"
#include "andor_setup.h"

/* data types */
/**
 * Data type holding local data to andor_setup. This consists of the following:
 * <dl>
 * <dt>Camera_Handle</dt> <dd>The Andor library camera handle (an at_32).</dd>
 * <dt>Detector_X_Pixel_Count</dt> <dd>The number of pixels on the detector in the X direction.</dd>
 * <dt>Detector_Y_Pixel_Count</dt> <dd>The number of pixels on the detector in the Y direction.</dd>
 * <dt>Horizontal_Bin</dt> <dd>Horizontal (X) binning factor.</dd>
 * <dt>Vertical_Bin</dt> <dd>Vertical (Y) binning factor.</dd>
 * <dt>Horizontal_Start</dt> <dd>Horizontal (X) start pixel of the imaging window (inclusive).</dd>
 * <dt>Horizontal_End</dt> <dd>Horizontal (X) end pixel of the imaging window (inclusive).</dd>
 * <dt>Vertical_Start</dt> <dd>Vertical (Y) start pixel of the imaging window (inclusive).</dd>
 * <dt>Vertical_End</dt> <dd>Vertical (Y) end pixel of the imaging window (inclusive).</dd>
 * </dl>
 */
struct Setup_Struct
{
        at_32 Camera_Handle;
	int Detector_X_Pixel_Count;
	int Detector_Y_Pixel_Count;
	int Horizontal_Bin;
	int Vertical_Bin;
	int Horizontal_Start;
	int Horizontal_End;
	int Vertical_Start;
	int Vertical_End;

};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: andor_setup.c,v 1.5 2014-02-03 09:52:06 cjm Exp $";

/**
 * Instance of the setup data.
 * @see #Setup_Struct
 */
static struct Setup_Struct Setup_Data = 
{
	0,0,0,0,0,0,0,0,0
};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise Andor setup.
 */
void Andor_Setup_Initialise(void)
{
	int i;
	/* do nothing? */
}

/**
 * Do startup for an Andor CCD.
 * <ul>
 * <li>Uses <b>GetAvailableCameras</b> to get the available number of cameras.
 * <li>Uses <b>CCD_Config_Get_Integer</b> to get the selected camera from config keyword "ccd.andor.selected_camera".
 * <li>Uses <b>GetCameraHandle</b> to get a camera handle for the selected camera and store it in 
 *          Setup_Data.Camera_Handle.
 * <li>Uses <b>SetCurrentCamera</b> to set the Andor libraries current camera.
 * <li>Uses <b>CCD_Config_Get_String</b> to get the Andor library configuration directory 
 *          (containing detector.ini and the .cof files) config keyword "ccd.andor.config_directory".
 * <li>Call <b>Initialize</b> to initialise the andor library using the selected config directory.
 * <li>Calls <b>SetReadMode</b> to set the Andor library read mode to image.
 * <li>Calls <b>SetAcquisitionMode</b> to set the Andor library to acquire a single image at a time.
 * <li>Calls <b>GetDetector</b> to get the detector dimensions and save then to <b>Setup_Data</b>.
 * <li>Calls <b>SetShutter</b> to set the Andor library shutter settings to auto with no shutter delay.
 * <li>Calls <b>SetFrameTransferMode</b> to enable frame transfer mode.
 * </ul>
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #ANDOR_SETUP_KEYWORD_ROOT
 * @see #Setup_Data
 * @see andor_general.html#Andor_General_ErrorCode_To_String
 * @see ../../cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../../cdocs/ccd_config.html#CCD_Config_Get_Integer
 * @see ../../cdocs/ccd_config.html#CCD_Config_Get_Long
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int Andor_Setup_Startup(void)
{
	char *config_directory = NULL;
	at_32 camera_count,selected_camera;
	int retval;
	unsigned int andor_retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	/* sort out selected camera. */
	andor_retval = GetAvailableCameras(&camera_count);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1003;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: GetAvailableCameras() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Andor library reports %d cameras.",camera_count);
#endif
	/* at_32 is an integer when compiling 64-bit, and a long when compiling 32-bit. See atmcdLXd.h for details */
#ifdef _LP64
	retval = CCD_Config_Get_Integer(ANDOR_SETUP_KEYWORD_ROOT"selected_camera",&selected_camera);
#else
	retval = CCD_Config_Get_Long(ANDOR_SETUP_KEYWORD_ROOT"selected_camera",&selected_camera);	
#endif
	if(retval == FALSE)
		return FALSE;
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file has selected camera %ld.",selected_camera);
#endif
	if((selected_camera >= camera_count) || (selected_camera < 0))
	{
		CCD_General_Error_Number = 1000;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: Selected camera %d out of range [0..%d].",
			selected_camera,camera_count);
		return FALSE;
	}
	/* get andor camera handle */
	andor_retval = GetCameraHandle(selected_camera,&Setup_Data.Camera_Handle);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1004;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: GetCameraHandle(%d) failed %s(%u).",
			selected_camera,Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Andor library camera handle:%ld.",Setup_Data.Camera_Handle);
#endif
	/* set current camera */
	andor_retval = SetCurrentCamera(Setup_Data.Camera_Handle);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1005;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: SetCurrentCamera() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* intialise the andor library, passing the .ini / .cof directory. */
	retval = CCD_Config_Get_String(ANDOR_SETUP_KEYWORD_ROOT"config_directory",&config_directory);
	if(retval == FALSE)
		return FALSE;
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file has Andor config directory:%s.",config_directory);
#endif
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling Andor Initialize.");
#endif
	andor_retval = Initialize(config_directory);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1001;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: Initialize(%s) failed %s(%u).",
			config_directory,Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		if(config_directory != NULL)
			free(config_directory);
		return FALSE;
	}
	/* free config directory */
	if(config_directory != NULL)
		free(config_directory);
	/* sleep to allow setup to complete */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Sleeping whilst waiting for Initialize to complete .");
#endif
	sleep(2);
	/* Set read mode to image */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling SetReadMode.");
#endif
	andor_retval = SetReadMode(4);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1006;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: SetReadMode(4) failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* Set acquisition mode to single scan */
	/* this might be something else eventually (5 run til abort - but not for fielding!) */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling SetAcquisitionMode.");
#endif
	andor_retval = SetAcquisitionMode(1);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1007;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: SetAcquisitionMode(1) failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* get the detector dimensions */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling GetDetector.");
#endif
	andor_retval = GetDetector(&Setup_Data.Detector_X_Pixel_Count,&Setup_Data.Detector_Y_Pixel_Count);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1002;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: GetDetector() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* initialise the shutter  - is this needed? See andor_exposure.c */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling SetShutter.");
#endif
	andor_retval = SetShutter(1,0,0,0);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1008;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: SetShutter() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
	/* set frame transfer mode ? */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling SetFrameTransferMode.");
#endif
	andor_retval = SetFrameTransferMode(1);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1010;
		sprintf(CCD_General_Error_String,"Andor_Setup_Startup: SetFrameTransferMode() failed %s(%u).",
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/**
 * Shutdown setup to the Andor CCD. Calls Andor library routine <b>ShutDown</b>.
 */
int Andor_Setup_Shutdown(void)
{
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	/* Note documentation says temp should be > -20 before calling this */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling Shutdown.");
#endif
	ShutDown();
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Check the dimensions (particularily the window dimensions) are valid for the Andor camera. SetImage supports
 * arbitary windows, so this routine does not modify the window in any way.
 * @param ncols The address of an integer, on entry to the function containing the number of unbinned image columns (X).
 * @param nrows The address of an integer, on entry to the function containing the number of unbinned image rows (Y).
 * @param hbin The address of an integer, on entry to the function containing the binning in X.
 * @param vbin The address of an integer, on entry to the function containing the binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A pointer to a structure containing window data. These dimensions are inclusive, and in binned pixels.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_general.html#CCD_General_Log
 */
int Andor_Setup_Dimensions_Check(int *ncols,int *nrows,int *hbin,int *vbin,
					int window_flags,struct CCD_Setup_Window_Struct *window)
{
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Dimensions_Check",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	/* do nothing for an andor camera */
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Dimensions_Check",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Setup dimension information. Calls Andor library <b>SetImage</b>.
 * @param ncols Number of image columns (X).
 * @param nrows Number of image rows (Y).
 * @param hbin Binning in X.
 * @param vbin Binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A structure containing window data.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see andor_general.html#Andor_General_ErrorCode_To_String
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int Andor_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,int window_flags,
			   struct CCD_Setup_Window_Struct window)
{
	unsigned int andor_retval;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(window_flags > 0)
	{
		Setup_Data.Horizontal_Bin = hbin;
		Setup_Data.Vertical_Bin = vbin;
		Setup_Data.Horizontal_Start = window.X_Start;
		Setup_Data.Horizontal_End = window.X_End;
		Setup_Data.Vertical_Start = window.Y_Start;
		Setup_Data.Vertical_End = window.Y_End;
	}
	else
	{
		Setup_Data.Horizontal_Bin = hbin;
		Setup_Data.Vertical_Bin = vbin;
		Setup_Data.Horizontal_Start = 1;
		Setup_Data.Horizontal_End = ncols;
		Setup_Data.Vertical_Start = 1;
		Setup_Data.Vertical_End = nrows;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log_Format("ccd","andor_setup.c","Andor_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			"Calling SetImage(hbin=%d,vbin=%d,hstart=%d,hend=%d,vstart=%d,vend=%d).",
			Setup_Data.Horizontal_Bin,Setup_Data.Vertical_Bin,
			Setup_Data.Horizontal_Start,Setup_Data.Horizontal_End,
			Setup_Data.Vertical_Start,Setup_Data.Vertical_End);
#endif
        andor_retval = SetImage(Setup_Data.Horizontal_Bin,Setup_Data.Vertical_Bin,
				Setup_Data.Horizontal_Start,Setup_Data.Horizontal_End,
				Setup_Data.Vertical_Start,Setup_Data.Vertical_End);
	if(andor_retval != DRV_SUCCESS)
	{
		CCD_General_Error_Number = 1009;
		sprintf(CCD_General_Error_String,"Andor_Setup_Dimensions: SetImage(hbin=%d,vbin=%d,"
			"hstart=%d,hend=%d,vstart=%d,vend=%d) failed %s(%u).",
			Setup_Data.Horizontal_Bin,Setup_Data.Vertical_Bin,
			Setup_Data.Horizontal_Start,Setup_Data.Horizontal_End,
			Setup_Data.Vertical_Start,Setup_Data.Vertical_End,
			Andor_General_ErrorCode_To_String(andor_retval),andor_retval);
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Abort a setup. Currently does nothing.
 */
void Andor_Setup_Abort(void)
{
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Abort",LOG_VERBOSITY_VERBOSE,NULL,
			"This does nothing at the moment..");
#endif
}

/**
 * Get the number of columns setup to be read out from the last Andor_Setup_Dimensions.
 * Currently, (Setup_Data.Horizontal_End - Setup_Data.Horizontal_Start)+1.
 * Plus 1 as dimensions are inclusive.
 * (Need to think about binning).
 * @return The number of binned columns.
 * @see #Setup_Data
 */
int Andor_Setup_Get_NCols(void)
{
	return (Setup_Data.Horizontal_End - Setup_Data.Horizontal_Start)+1;
}

/**
 * Get the number of rows setup to be read out from the last Andor_Setup_Dimensions.
 * Currently, (Setup_Data.Vertical_End - Setup_Data.Vertical_Start)+1.
 * Plus 1 as dimensions are inclusive.
 * (Need to think about binning).
 * @return The number of binned rows.
 * @see #Setup_Data
 */
int Andor_Setup_Get_NRows(void)
{
	return (Setup_Data.Vertical_End - Setup_Data.Vertical_Start)+1;
}

/**
 * Return the length of buffer required to hold one image with the current setup.
 * @return The required length of buffer in pixels.
 * @see #Andor_Setup_Get_NCols
 * @see #Andor_Setup_Get_NRows 
 */
int Andor_Setup_Get_Buffer_Length(void)
{
	return Andor_Setup_Get_NCols() * Andor_Setup_Get_NRows();
}

/**
 * Get the number of detector columns as read from the camera head during Andor_Setup_Startup.
 * @return The number of columns on the detector (<b>Setup_Data.Detector_X_Pixel_Count</b>).
 * @see #Andor_Setup_Startup
 * @see #Setup_Data
 */
int Andor_Setup_Get_Detector_Columns(void)
{
	return Setup_Data.Detector_X_Pixel_Count;
}

/**
 * Get the number of detector rows as read from the camera head during Andor_Setup_Startup.
 * @return The number of rows on the detector (<b>Setup_Data.Detector_Y_Pixel_Count</b>).
 * @see #Andor_Setup_Startup
 * @see #Setup_Data
 */
int Andor_Setup_Get_Detector_Rows(void)
{
	return Setup_Data.Detector_Y_Pixel_Count;
}

/**
 * Allocate memory to hold a single image using the current setup.
 * @param buffer The address of a pointer to store the location of the allocated memory.
 * @param buffer_length The address opf a size_t to hold the length of the buffer.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #Andor_Setup_Get_Buffer_Length
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int Andor_Setup_Allocate_Image_Buffer(void **buffer,size_t *buffer_length)
{
	int width,height;

#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Allocate_Image_Buffer",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	if(buffer == NULL)
	{
		CCD_General_Error_Number = 1011;
		sprintf(CCD_General_Error_String,"Andor_Setup_Allocate_Image_Buffer: buffer was NULL.");
		return FALSE;
	}
	if(buffer_length == NULL)
	{
		CCD_General_Error_Number = 1012;
		sprintf(CCD_General_Error_String,"Andor_Setup_Allocate_Image_Buffer: buffer_length was NULL.");
		return FALSE;
	}
	(*buffer_length) = Andor_Setup_Get_Buffer_Length();
	(*buffer) = (void *)malloc((*buffer_length)*sizeof(char));
	if((*buffer) == NULL)
	{
		CCD_General_Error_Number = 1013;
		sprintf(CCD_General_Error_String,"Andor_Setup_Allocate_Image_Buffer: Failed to allocate buffer (%ld).",
			(*buffer_length));
		return FALSE;
	}
#ifdef ANDOR_DEBUG
	CCD_General_Log("ccd","andor_setup.c","Andor_Setup_Allocate_Image_Buffer",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2011/09/08 09:20:29  cjm
** Added #include <stdlib.h>.
**
** Revision 1.3  2009/01/30 15:41:14  cjm
** *** empty log message ***
**
** Revision 1.2  2006/04/10 15:51:23  cjm
** Comment fix.
**
** Revision 1.1  2006/03/27 14:02:36  cjm
** Initial revision
**
*/
