/* ccd_setup.c
** Autoguider CCD Library setup routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_setup.c,v 1.2 2009-01-30 18:00:24 cjm Exp $
*/
/**
 * Setup routines for the autoguider CCD library.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_config.h"
#include "ccd_driver.h"
#include "ccd_general.h"
#include "ccd_setup.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_setup.c,v 1.2 2009-01-30 18:00:24 cjm Exp $";

/* data types */
/**
 * Internal setup data structure. Contains loaded config about whether to flip the read out image data in X and/or Y.
 * <dl>
 * <dt>Flip_X</dt> <dd>A boolean (as an integer), if true flip the image data in x, 
 *                     if false don't flip the image data in x.</dd>
 * <dt>Flip_Y</dt> <dd>A boolean (as an integer), if true flip the image data in y, 
 *                     if false don't flip the image data in y.</dd>
 * </dl>
 */
struct Setup_Struct
{
	int Flip_X;
	int Flip_Y;
};

/* internal variables */
/**
 * Internal setup Data.
 * @see #Setup_Struct
 */
static struct Setup_Struct Setup_Data = {FALSE,FALSE};

/* internal functions */
static void Setup_Dimensions_Flip(int ncols,int nrows,int nsbin,int npbin,int window_flags,
				  struct CCD_Setup_Window_Struct window,struct CCD_Setup_Window_Struct *flipped_window);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Load the flip data from the ccd config file, which should be initialised and loaded before this routine is called.
 * @see #Setup_Data
 * @see ccd_config.html#CCD_Config_Get_Boolean
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 */
void CCD_Setup_Initialise(void)
{
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Initialise",LOG_VERBOSITY_VERY_VERBOSE,NULL,"started.");
#endif
	if(!CCD_Config_Get_Boolean("ccd.flip.x",&(Setup_Data.Flip_X)))
		return FALSE;
	if(!CCD_Config_Get_Boolean("ccd.flip.y",&(Setup_Data.Flip_Y)))
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Initialise",LOG_VERBOSITY_VERY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Initially setup the connection to the actual driver. Calls driver function Setup_Startup.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Startup(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Startup",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Setup_Startup == NULL)
	{
		CCD_General_Error_Number = 300;
		sprintf(CCD_General_Error_String,"CCD_Setup_Startup:Setup_Startup function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Setup_Startup))();
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Startup",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Shutdown the connection to the actual driver. Calls driver function Setup_Shutdown.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Shutdown(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Setup_Shutdown == NULL)
	{
		CCD_General_Error_Number = 301;
		sprintf(CCD_General_Error_String,"CCD_Setup_Shutdown:Setup_Shutdown function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Setup_Shutdown))();
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Check the setup dimension information. This function can adjust the passed in dimensions data to take
 * account of restrictions in the particular camera system implementation. Only modified window data
 * is currently propogated into the main autoguider code at the moment.
 * @param ncols The address of an integer, on entry to the function containing the number of unbinned image columns (X).
 * @param nrows The address of an integer, on entry to the function containing the number of unbinned image rows (Y).
 * @param hbin The address of an integer, on entry to the function containing the binning in X.
 * @param vbin The address of an integer, on entry to the function containing the binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A pointer to a structure containing window data. These dimensions are inclusive, and in binned pixels.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #Setup_Dimensions_Flip
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Dimensions_Check(int *ncols,int *nrows,int *nsbin,int *npbin,
			       int window_flags,struct CCD_Setup_Window_Struct *window)
{
	struct CCD_Driver_Function_Struct functions;
	struct CCD_Setup_Window_Struct buffer_window,ccd_window;
	int retval,buffer_ncols,buffer_nrows,buffer_nsbin,buffer_npbin;
	
	/* check parameters are not NULL */
	if(ncols == NULL)
	{
		CCD_General_Error_Number = 306;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:ncols was NULL.");
		return FALSE;
	}
	if(nrows == NULL)
	{
		CCD_General_Error_Number = 307;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:nrows was NULL.");
		return FALSE;
	}
	if(nsbin == NULL)
	{
		CCD_General_Error_Number = 308;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:nsbin was NULL.");
		return FALSE;
	}
	if(npbin == NULL)
	{
		CCD_General_Error_Number = 309;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:npbin was NULL.");
		return FALSE;
	}
	if(window == NULL)
	{
		CCD_General_Error_Number = 310;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:window was NULL.");
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_setup.c","CCD_Setup_Dimensions_Check",LOG_VERBOSITY_TERSE,NULL,
			       "Started with ncols=%d, nrows=%d, nsbin=%d, npbin=%d, window_flags=%d, "
			       "window={xstart=%d,ystart=%d,xend=%d,yend=%d}.",(*ncols),(*nrows),(*nsbin),(*npbin),
			       window_flags,window->X_Start,window->Y_Start,window->X_End,window->Y_End);
#endif
	/* the input coordinates are buffer coordinates i.e. the window position will be in binned pixels after any
	** flips in the image data have been done. The coordinates need to be converted to CCD coordinates
	** before the dimensions are checked, as the detector restrictions are based on coordinate 
	** position on the CCD not the buffer, one set of coordinates can potentially be flipped wrt to the other. */
	buffer_ncols = (*ncols);
	buffer_nrows = (*nrows);
	buffer_nsbin = (*nsbin);
	buffer_npbin = (*npbin);
	buffer_window = (*window);
	Setup_Dimensions_Flip(buffer_ncols,buffer_nrows,buffer_nsbin,buffer_npbin,window_flags,buffer_window,&ccd_window);
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Setup_Dimensions_Check == NULL)
	{
		CCD_General_Error_Number = 311;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions_Check:Setup_Dimensions_Check function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Setup_Dimensions_Check))(ncols,nrows,nsbin,npbin,window_flags,&ccd_window);
	if(retval == FALSE)
		return FALSE;
	/* Setup_Dimensions_Check returns the updated window in ccd dimensions. We need to change these back to
	** buffer dimensions, so that CCD_Setup_Dimensions is called with buffer dimensions, that are then
	** correctly flipped back to CCD dimensions before being configured by the detector API */
	Setup_Dimensions_Flip(buffer_ncols,buffer_nrows,buffer_nsbin,buffer_npbin,window_flags,ccd_window,window);
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Dimensions_Check",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Setup dimension information.
 * @param ncols Number of unbinned image columns (X).
 * @param nrows Number of unbinned image rows (Y).
 * @param hbin Binning in X.
 * @param vbin Binning in Y.
 * @param window_flags Whether to use the specified window or not.
 * @param window A structure containing window data. These dimensions are inclusive, and in binned pixels.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Dimensions(int ncols,int nrows,int nsbin,int npbin,
			 int window_flags,struct CCD_Setup_Window_Struct window)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_setup.c","CCD_Setup_Dimensions",LOG_VERBOSITY_TERSE,NULL,
			       "Started with ncols=%d, nrows=%d, nsbin=%d, npbin=%d, window_flags=%d, "
			       "window={xstart=%d,ystart=%d,xend=%d,yend=%d}.",ncols,nrows,nsbin,npbin,window_flags,
			       window.X_Start,window.Y_Start,window.X_End,window.Y_End);
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return FALSE;
	/* is there a function implementing this operation? */
	if(functions.Setup_Dimensions == NULL)
	{
		CCD_General_Error_Number = 302;
		sprintf(CCD_General_Error_String,"CCD_Setup_Dimensions:Setup_Dimensions function was NULL.");
		return FALSE;
	}
	/* call driver function */
	retval = (*(functions.Setup_Dimensions))(ncols,nrows,nsbin,npbin,window_flags,window);
	if(retval == FALSE)
		return FALSE;
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Dimensions",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
	return TRUE;
}

/**
 * Try to abort a setup.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
void CCD_Setup_Abort(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Abort",LOG_VERBOSITY_TERSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return;
	/* is there a function implementing this operation? */
	if(functions.Setup_Abort == NULL)
	{
		CCD_General_Error_Number = 303;
		sprintf(CCD_General_Error_String,"CCD_Setup_Abort:Setup_Abort function was NULL.");
		return;
	}
	/* call driver function */
	(*(functions.Setup_Abort))();
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Abort",LOG_VERBOSITY_TERSE,NULL,"finished.");
#endif
}

/**
 * Get the number of columns in the setup image dimensions.
 * @return The number of column pixels, or -1 if an internal driver related error occured.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Get_NCols(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Get_NCols",LOG_VERBOSITY_VERY_VERBOSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return -1;
	/* is there a function implementing this operation? */
	if(functions.Setup_Get_NCols == NULL)
	{
		CCD_General_Error_Number = 304;
		sprintf(CCD_General_Error_String,"CCD_Setup_Get_NCols:Setup_Get_NCols function was NULL.");
		return -1;
	}
	/* call driver function */
	retval = (*(functions.Setup_Get_NCols))();
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_setup.c","CCD_Setup_Get_NCols",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "finished with ncols %d.",retval);
#endif
	return retval;
}

/**
 * Get the number of rows in the setup image dimensions.
 * @return The number of row pixels, or -1 if an internal driver related error occured.
 * @see ccd_driver.html#CCD_Driver_Get_Functions
 * @see ccd_driver.html#CCD_Driver_Function_Struct
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
int CCD_Setup_Get_NRows(void)
{
	struct CCD_Driver_Function_Struct functions;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_setup.c","CCD_Setup_Get_NRows",LOG_VERBOSITY_VERY_VERBOSE,NULL,"started.");
#endif
	/* get driver functions */
	retval = CCD_Driver_Get_Functions(&functions);
	if(retval == FALSE)
		return -1;
	/* is there a function implementing this operation? */
	if(functions.Setup_Get_NRows == NULL)
	{
		CCD_General_Error_Number = 305;
		sprintf(CCD_General_Error_String,"CCD_Setup_Get_NRows:Setup_Get_NRows function was NULL.");
		return -1;
	}
	/* call driver function */
	retval = (*(functions.Setup_Get_NRows))();
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_setup.c","CCD_Setup_Get_NRows",LOG_VERBOSITY_VERY_VERBOSE,NULL,
			       "finished with nrows %d.",retval);
#endif
	return retval;
}

/**
 * Routine to flip the coordinates in a window from buffer to ccd (or visa versa) taking account of any flips made to
 * read out data. This is needed as upstream autoguider software makes use of read-out data which may have been flipped
 * with respect to ccd orientation, this means computed centroids and therefore windows are in buffer orientation, 
 * and need to be flipped into ccd orientation before passed into detector API's to configure or check sub-window readout.
 * @param ncols An integer, on entry to the function containing the number of unbinned image columns (X).
 * @param nrows An integer, on entry to the function containing the number of unbinned image rows (Y).
 * @param hbin An integer, on entry to the function containing the binning in X.
 * @param vbin An integer, on entry to the function containing the binning in Y.
 * @param window_flags A boolean as an integer, whether to use the specified window or not.
 * @param window A structure containing window data. These dimensions are inclusive, and in binned pixels.
 * @param window A pointer to a structure containing window data. On a successful return from the function, if
 *        window_flags are true, these will contain a window whose coordinates are flipped in the direction
 *        configured by Setup_Data.Flip_X/Setup_Data.Flip_X, taking into account the binned detector dimensions 
 *        computer from ncols/nrows/hbin/vbin/.
 * @return The routine returns TRUE on success, and FALSE if an error occurs.
 * @see #Setup_Data
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_General_Error_Number
 * @see ccd_general.html#CCD_General_Error_String
 */
static int Setup_Dimensions_Flip(int ncols,int nrows,int hbin,int vbin,int window_flags,
				 struct CCD_Setup_Window_Struct window,struct CCD_Setup_Window_Struct *flipped_window)
{
	int binned_ncols;
	int binned_nrows;

	if(flipped_window == NULL)
	{
		CCD_General_Error_Number = ;
		sprintf(CCD_General_Error_String,"Setup_Dimensions_Flip:flipped_window was NULL.");
		return FALSE;
	}
	/* simple case, if the window is not to be used, just return the input window */
	if(window_flags == FALSE)
	{
		(*flipped_window) = window;
		return TRUE;
	}
	/* check binning is legal before computing binned image dimensions */
	if(hbin < 1)
	{
		CCD_General_Error_Number = ;
		sprintf(CCD_General_Error_String,"Setup_Dimensions_Flip:hbin was less than 1.");
		return FALSE;
	}
	if(vbin < 1)
	{
		CCD_General_Error_Number = ;
		sprintf(CCD_General_Error_String,"Setup_Dimensions_Flip:vbin was less than 1.");
		return FALSE;
	}
	/* compute binned image size */
	binned_ncols = ncols/hbin;
	binned_nrows = nrows/vbin;
	/* flip window if setup to do so */
	if(Setup_Data.Flip_X)
	{
		flipped_window->X_Start = binned_ncols-window.X_Start;
		flipped_window->X_End = binned_ncols-window.X_End;
	}
	else
	{
		flipped_window->X_Start = window.X_Start;
		flipped_window->X_End = window.X_End;
	}
	if(Setup_Data.Flip_Y)
	{
		flipped_window->Y_Start = binned_nrows-window.Y_Start;
		flipped_window->Y_End = binned_nrows-window.Y_End;
	}
	else
	{
		flipped_window->Y_Start = window.Y_Start;
		flipped_window->Y_End = window.Y_End;
	}
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:27:37  cjm
** Initial revision
**
*/
