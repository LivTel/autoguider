/* fli_setup.c
** Autoguider FLI CCD Library setup routines
** $Header: /home/cjm/cvs/autoguider/ccd/fli/c/fli_setup.c,v 1.2 2013-12-03 09:33:33 cjm Exp $
*/

/**
 * Setup routines for the FLI autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
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
/* fli CCD library */
#include "libfli.h"
#include "log_udp.h"
#include "ccd_general.h"
#include "fli_setup.h"

/* structs */
/**
 * Structure defining an area on the CCD, using two sets of pixel coordinates
 * for the upper left and lower right corners.
 * <ul>
 * <li><b>Upper_Left_X</b>
 * <li><b>Upper_Left_Y</b>
 * <li><b>Lower_Right_X</b>
 * <li><b>Lower_Right_Y</b>
 * </ul>
 */
struct Area_Struct
{
	long Upper_Left_X;
	long Upper_Left_Y;
	long Lower_Right_X;
	long Lower_Right_Y;
};

/**
 * Data type holding local data to fli_setup. This consists of the following:
 * <dl>
 * <dt>FLI_Dev</dt> <dd>The FLI library camera handle (a flidev_t).</dd>
 * <dt>Detector_Area</dt> <dd>The pixel coordinates of the detector area.</dd>
 * <dt>Horizontal_Bin</dt> <dd>Horizontal (X) binning factor.</dd>
 * <dt>Vertical_Bin</dt> <dd>Vertical (Y) binning factor.</dd>
 * <dt>Image_Area</dt> <dd>The pixel coordinates of the current window/in use imaging area.</dd>
 * </dl>
 * @see #Area_Struct
 */
struct Setup_Struct
{
	flidev_t FLI_Dev;
	struct Area_Struct Detector_Area;
	long Horizontal_Bin;
	long Vertical_Bin;
	struct Area_Struct Image_Area;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: fli_setup.c,v 1.2 2013-12-03 09:33:33 cjm Exp $";

/**
 * Instance of the setup data.
 * @see #Setup_Struct
 */
static struct Setup_Struct Setup_Data = 
{
	0,{0L,0L,0L,0L},0L,0L,{0L,0L,0L,0L}
};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Initialise FLI setup.
 */
void FLI_Setup_Initialise(void)
{
	int i;
	/* do nothing? */
}

/**
 * Do startup for an FLI CCD.
 * <ul>
 * <li>We call CCD_Config_Get_String to get the FLI device name to use from the properties file: 
 *     <b>ccd.fli.setup.device_name</b>
 * <li>We call FLIOpen to open the device (as a USB camera).
 * <li>We call FLIGetArrayArea to get the detector area, and save it in Setup_Data.Detector_Area.
 * </ul>
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #FLI_SETUP_KEYWORD_ROOT
 * @see #Setup_Data
 * @see ../../cdocs/ccd_config.html#CCD_Config_Get_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int FLI_Setup_Startup(void)
{
	char *device_name = NULL;
	long fli_retval;
	int retval;

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"started.");
#endif
	/* get the device name to use in FLIOpen */
	retval = CCD_Config_Get_String(FLI_SETUP_KEYWORD_ROOT"device_name",&device_name);
	if(retval == FALSE)
		return FALSE;
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Config file FLI device name:%s.",device_name);
#endif
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Startup",LOG_VERBOSITY_VERBOSE,NULL,
			       "Opening conenction to FLI camera %s.",device_name);
#endif
	/* Open the connection to the FLI camera */
	fli_retval = FLIOpen(&(Setup_Data.FLI_Dev),device_name,FLIDOMAIN_USB|FLIDEVICE_CAMERA);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1100;
		sprintf(CCD_General_Error_String,"FLI_Setup_Startup: FLIOpen(%s) failed %s(%ld).",
			device_name,strerror((int)-fli_retval),fli_retval);
		if(device_name != NULL)
			free(device_name);
		return FALSE;
	}
	/* free device_name */
	if(device_name != NULL)
		free(device_name);
	/* get total area of detector */
	fli_retval = FLIGetArrayArea(Setup_Data.FLI_Dev,&(Setup_Data.Detector_Area.Upper_Left_X),
				     &(Setup_Data.Detector_Area.Upper_Left_Y),
				     &(Setup_Data.Detector_Area.Lower_Right_X),
				     &(Setup_Data.Detector_Area.Lower_Right_Y));
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1101;
		sprintf(CCD_General_Error_String,"FLI_Setup_Startup: FLIGetArrayArea failed %s(%ld).",
			strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
	/* visible area */
	/*
	fli_retval = FLIGetVisibleArea(Fli_Dev,&(visible_area.Upper_Left_X),&(visible_area.Upper_Left_Y),
				     &(visible_area.Lower_Right_X),&(visible_area.Lower_Right_Y));
	*/
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Startup",LOG_VERBOSITY_INTERMEDIATE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Shutdown the connection to the FLI CCD. Calls the FLIClose routine.
 */
int FLI_Setup_Shutdown(void)
{
	long fli_retval;

	fli_retval = FLIClose(Setup_Data.FLI_Dev);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1102;
		sprintf(CCD_General_Error_String,"FLI_Setup_Shutdown: FLIClose(fli_dev=%ld) failed %s(%ld).",
			Setup_Data.FLI_Dev,strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
	return TRUE;
}
/**
 * Setup dimension information. 
 * <ul>
 * <li>If the windows_flags is set, we set the Setup_Data.Image_Area to the defined window.
 * <li>If the windows_flags is <b>not</b> set, we set the Setup_Data.Image_Area to (1,1,ncols,nrows).
 * <li>We save the supplied binning values in Setup_Data.
 * <li>We calculate the number of binned pixels.
 * <li>We call FLISetHBin and FLISetVBin to tell the FLI library the binning we want to use.
 * <li>We call FLISetImageArea to set the image area we want use (in binned pixels, using the above calculations).
 * </ul>
 * @param ncols Number of image columns (X). These appear to be unbinned.
 * @param nrows Number of image rows (Y). These appear to be unbinned.
 * @param hbin Binning in X.
 * @param vbin Binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A structure containing window data.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int FLI_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				int window_flags,struct CCD_Setup_Window_Struct window)
{
	long fli_retval,binned_pixels_x,binned_pixels_y;

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	if(window_flags > 0)
	{
		Setup_Data.Image_Area.Upper_Left_X = window.X_Start;
		Setup_Data.Image_Area.Upper_Left_Y = window.Y_Start;
		Setup_Data.Image_Area.Lower_Right_X = window.X_End;
		Setup_Data.Image_Area.Lower_Right_Y = window.Y_End;
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Window: (ulx=%d,uly=%d,lrx=%d,lry=%d).",Setup_Data.Image_Area.Upper_Left_X,
			       Setup_Data.Image_Area.Upper_Left_Y,
			       Setup_Data.Image_Area.Lower_Right_X,Setup_Data.Image_Area.Lower_Right_Y);
#endif
	}
	else
	{
		/*Setup_Data.Image_Area = Setup_Data.Detector_Area;*/
		Setup_Data.Image_Area.Upper_Left_X = 1;
		Setup_Data.Image_Area.Upper_Left_Y = 1;
		Setup_Data.Image_Area.Lower_Right_X = ncols;
		Setup_Data.Image_Area.Lower_Right_Y = nrows;
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Full Frame: (ulx=%d,uly=%d,lrx=%d,lry=%d).",Setup_Data.Image_Area.Upper_Left_X,
			       Setup_Data.Image_Area.Upper_Left_Y,
			       Setup_Data.Image_Area.Lower_Right_X,Setup_Data.Image_Area.Lower_Right_Y);
#endif
	}
	Setup_Data.Horizontal_Bin = hbin;
	Setup_Data.Vertical_Bin = vbin;
	/* calculate the number of binned pixels */
	binned_pixels_x = (Setup_Data.Image_Area.Lower_Right_X-Setup_Data.Image_Area.Upper_Left_X)/
		Setup_Data.Horizontal_Bin;
	binned_pixels_y = (Setup_Data.Image_Area.Lower_Right_Y-Setup_Data.Image_Area.Upper_Left_Y)/
		Setup_Data.Vertical_Bin;
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Binned pixels in image: (%ld x %ld).",binned_pixels_x,binned_pixels_y);
#endif
	/* tell FLI camera what binning to use */
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Setting Binning to (%ld,%ld).",Setup_Data.Horizontal_Bin,Setup_Data.Vertical_Bin);
#endif
	fli_retval = FLISetHBin(Setup_Data.FLI_Dev,Setup_Data.Horizontal_Bin);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1103;
		sprintf(CCD_General_Error_String,
			"FLI_Setup_Dimensions: FLISetHBin(fli_dev=%ld,hbin=%ld) failed %s(%ld).",
			Setup_Data.FLI_Dev,strerror((int)-fli_retval),fli_retval);
		return FALSE;
	}
	fli_retval = FLISetVBin(Setup_Data.FLI_Dev,Setup_Data.Vertical_Bin);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1104;
		sprintf(CCD_General_Error_String,
			"FLI_Setup_Dimensions: FLISetVBin(fli_dev=%ld,vbin=%ld) failed %ld: %s.\n",
			Setup_Data.FLI_Dev,Setup_Data.Vertical_Bin,fli_retval,strerror((int)-fli_retval));
		return FALSE;
	}
	/* tell FLI camera image area to use */
#ifdef FLI_DEBUG
	CCD_General_Log_Format("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,
			       "Calling FLISetImageArea(ulx=%d,uly=%d,lrx=%d,lry=%d).",
			       Setup_Data.Image_Area.Upper_Left_X,Setup_Data.Image_Area.Upper_Left_Y,
			       Setup_Data.Image_Area.Upper_Left_X+binned_pixels_x,
			       Setup_Data.Image_Area.Upper_Left_Y+binned_pixels_y);

#endif
	fli_retval = FLISetImageArea(Setup_Data.FLI_Dev,Setup_Data.Image_Area.Upper_Left_X,
				     Setup_Data.Image_Area.Upper_Left_Y,
				     Setup_Data.Image_Area.Upper_Left_X+binned_pixels_x,
				     Setup_Data.Image_Area.Upper_Left_Y+binned_pixels_y);
	if(fli_retval != 0)
	{
		CCD_General_Error_Number = 1105;
		sprintf(CCD_General_Error_String,
		  "FLI_Setup_Dimensions: FLISetImageArea(fli_dev=%ld,ulx=%ld,uly=%ld,lrx=%ld,lry=%ld) failed %ld: %s.",
			Setup_Data.FLI_Dev,Setup_Data.Image_Area.Upper_Left_X,Setup_Data.Image_Area.Upper_Left_Y,
			Setup_Data.Image_Area.Upper_Left_X+binned_pixels_x,
			Setup_Data.Image_Area.Upper_Left_Y+binned_pixels_y,
			fli_retval, strerror((int)-fli_retval));
		return FALSE;
	}

#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Dimensions",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Abort a setup. Currently does nothing.
 */
void FLI_Setup_Abort(void)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Abort",LOG_VERBOSITY_VERBOSE,NULL,
			"This does nothing at the moment..");
#endif
}

/**
 * Access routine to return the camera device currently being used by the autoguider.
 * @return An instance of flidev_t that represents the camera in use.
 * @see #Setup_Data
 */
flidev_t FLI_Setup_Get_Dev(void)
{
	return Setup_Data.FLI_Dev;
}

/**
 * Get the number of columns setup to be read out from the last FLI_Setup_Dimensions.
 * Currently, (<b>((Setup_Data.Image_Area.Lower_Right_X-Setup_Data.Image_Area.Upper_Left_X)-1)/
 * Setup_Data.Horizontal_Bin</b>).
 * @return The number of binned columns.
 * @see #Setup_Data
 */
int FLI_Setup_Get_NCols(void)
{
	long binned_pixels_x;

	binned_pixels_x = ((Setup_Data.Image_Area.Lower_Right_X-Setup_Data.Image_Area.Upper_Left_X)+1)/
		Setup_Data.Horizontal_Bin;
	return binned_pixels_x;
}

/**
 * Get the number of columns setup to be read out from the last FLI_Setup_Dimensions.
 * Currently, (<b>((Setup_Data.Image_Area.Lower_Right_Y-Setup_Data.Image_Area.Upper_Left_Y)+1)/
 * Setup_Data.Vertical_Bin</b>).
 * @return The number of binned rows.
 * @see #Setup_Data
 */
int FLI_Setup_Get_NRows(void)
{
	long binned_pixels_y;

	binned_pixels_y = ((Setup_Data.Image_Area.Lower_Right_Y-Setup_Data.Image_Area.Upper_Left_Y)+1)/
		Setup_Data.Vertical_Bin;
	return binned_pixels_y;
}

/**
 * Return the length of buffer required to hold one image with the current setup.
 * @return The required length of buffer in pixels.
 * @see #FLI_Setup_Get_NCols
 * @see #FLI_Setup_Get_NRows 
 */
int FLI_Setup_Get_Buffer_Length(void)
{
	return FLI_Setup_Get_NCols() * FLI_Setup_Get_NRows();
}

/**
 * Get the number of detector columns as read from the camera head during FLI_Setup_Startup.
 * @return The number of columns on the detector (<b>(Setup_Data.Detector_Area.Lower_Right_X - 
 *         Setup_Data.Detector_Area.Upper_Left_X)+1</b>). Currently unbinned pixels.
 * @see #FLI_Setup_Startup
 * @see #Setup_Data
 */
int FLI_Setup_Get_Detector_Columns(void)
{
	return (Setup_Data.Detector_Area.Lower_Right_X - Setup_Data.Detector_Area.Upper_Left_X)+1;
}

/**
 * Get the number of detector rows as read from the camera head during FLI_Setup_Startup.
 * @return The number of rows on the detector (<b>(Setup_Data.Detector_Area.Lower_Right_Y- 
 *         Setup_Data.Detector_Area.Upper_Left_Y)+1</b>). Currently unbinned pixels.
 * @see #FLI_Setup_Startup
 * @see #Setup_Data
 */
int FLI_Setup_Get_Detector_Rows(void)
{
	return (Setup_Data.Detector_Area.Lower_Right_Y - Setup_Data.Detector_Area.Upper_Left_Y)+1;
}

/**
 * Allocate memory to hold a single image using the current setup.
 * @param buffer The address of a pointer to store the location of the allocated memory.
 * @param buffer_length The address opf a size_t to hold the length of the buffer.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #FLI_Setup_Get_Buffer_Length
 * @see ../../cdocs/ccd_setup.html#CCD_Setup_Window_Struct
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_Number
 * @see ../../cdocs/ccd_general.html#CCD_General_Error_String
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 */
int FLI_Setup_Allocate_Image_Buffer(void **buffer,size_t *buffer_length)
{
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Allocate_Image_Buffer",LOG_VERBOSITY_VERBOSE,NULL,
			"started.");
#endif
	if(buffer == NULL)
	{
		CCD_General_Error_Number = 1106;
		sprintf(CCD_General_Error_String,"FLI_Setup_Allocate_Image_Buffer: buffer was NULL.");
		return FALSE;
	}
	if(buffer_length == NULL)
	{
		CCD_General_Error_Number = 1107;
		sprintf(CCD_General_Error_String,"FLI_Setup_Allocate_Image_Buffer: buffer_length was NULL.");
		return FALSE;
	}
	(*buffer_length) = FLI_Setup_Get_Buffer_Length();
	(*buffer) = (void *)malloc((*buffer_length)*sizeof(unsigned short));
	if((*buffer) == NULL)
	{
		CCD_General_Error_Number = 1108;
		sprintf(CCD_General_Error_String,"FLI_Setup_Allocate_Image_Buffer: Failed to allocate buffer (%d).",
			(*buffer_length));
		return FALSE;
	}
#ifdef FLI_DEBUG
	CCD_General_Log("ccd","fli_setup.c","FLI_Setup_Allocate_Image_Buffer",LOG_VERBOSITY_VERBOSE,NULL,
			"finished.");
#endif
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2013/11/26 16:28:36  cjm
** Initial revision
**
*/
