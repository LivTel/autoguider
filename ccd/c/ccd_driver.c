/* ccd_driver.c
** Autoguider CCD Library driver routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_driver.c,v 1.1 2006-04-28 14:27:23 cjm Exp $
*/
/**
 * Driver routines for the autoguider CCD library.
 * Sets up function pointers to driver routines from a loaded shared library.
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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
/* dynamic library loader */
#include <dlfcn.h>

#include "ccd_config.h"
#include "ccd_driver.h"
#include "ccd_general.h"
#include "ccd_setup.h"
#include "ccd_temperature.h"

/* data types */
/**
 * Structure holding local data to ccd_driver.
 * <ul>
 * <li><b>Dynamic_Library_Handle</b> Void pointer containing handle returned from dlopen.
 * <li><b>Register</b> Function pointer to registration function symbol retrieved from loaded dynamic library.
 * <li><b>Functions</b> Structure containing function pointers into the loaded driver. Filled in by a call
 *       to the libraries register function.
 * </ul>
 * @see #CCD_Driver_Function_Struct
 */
struct Driver_Struct
{
	void *Dynamic_Library_Handle;
	int (*Register)(struct CCD_Driver_Function_Struct *functions);
	struct CCD_Driver_Function_Struct Functions;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_driver.c,v 1.1 2006-04-28 14:27:23 cjm Exp $";
/**
 * Instance of the loaded driver.
 * @see #Driver_Struct
 */
static struct Driver_Struct Driver_Data;

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Load the shared library, and call the registration function to register a driver.
 * @param shared_library_name The name of the shared library to load e.g. "libm.so".
 * @param registration_function The registration function to call in the loaded shared library e.g. "cos".
 * @return The function returns TRUE on success and FALSE on failure.
 * @see #Driver_Data
 */
int CCD_Driver_Register(char *shared_library_name,char *registration_function)
{
	char *error_string = NULL;
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_CONFIG,"CCD_Driver_Register() started.");
#endif
	if(shared_library_name == NULL)
	{
		CCD_General_Error_Number = 200;
		sprintf(CCD_General_Error_String,"CCD_Driver_Register:shared_library_name is NULL.");
		return FALSE;
	}
	if(registration_function == NULL)
	{
		CCD_General_Error_Number = 201;
		sprintf(CCD_General_Error_String,"CCD_Driver_Register:registration_function is NULL.");
		return FALSE;
	}
	/* open shared library */
	Driver_Data.Dynamic_Library_Handle = dlopen(shared_library_name,RTLD_NOW);
	if(Driver_Data.Dynamic_Library_Handle == NULL)
	{
		CCD_General_Error_Number = 202;
		sprintf(CCD_General_Error_String,"CCD_Driver_Register:dlopen(%s) failed (%s).",shared_library_name,
			dlerror());
		return FALSE;
	}
	/* get registration fn pointer */
	Driver_Data.Register = dlsym(Driver_Data.Dynamic_Library_Handle, registration_function);
	/* check whether dlsym failed. This should be done by calling dlerror, as the symbol
	 * may actually be NULL. The return from dlerror is saved, as a second call to dlerror returns NULL. */
	error_string = dlerror();
	if(error_string != NULL)
	{
		CCD_General_Error_Number = 203;
		sprintf(CCD_General_Error_String,"CCD_Driver_Register:dlsym(%s) failed (%s).",registration_function,
			error_string);
		return FALSE;
	}
	/* returning a function pointer of NULL may not be an error according to dlsym, but it is to us! */
	if(Driver_Data.Register == NULL)
	{
		CCD_General_Error_Number = 204;
		sprintf(CCD_General_Error_String,"CCD_Driver_Register:dlsym(%s) returned NULL.",registration_function);
		return FALSE;
	}
	retval = (*Driver_Data.Register)(&Driver_Data.Functions);
	if(retval == FALSE)
	{
		/* hopefully the register function will have set a sensible error message */
		if(CCD_General_Error_Number == 0)
		{
			CCD_General_Error_Number = 205;
			sprintf(CCD_General_Error_String,"CCD_Driver_Register:"
				"Calling register function %s in library %s failed, but no error was set.",
				registration_function,shared_library_name);
		}
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_CONFIG,"CCD_Driver_Register() finished.");
#endif
	return TRUE;
}

/**
 * Get the functions registered by the loaded driver library. Copied from Driver_Data.
 * @param functions The address of a CCD_Driver_Function_Struct structure to store the function pointers in.
 * @return The function returns TRUE on success and FALSE on failure.
 * @see #CCD_Driver_Function_Struct
 * @see #Driver_Data
 */
int CCD_Driver_Get_Functions(struct CCD_Driver_Function_Struct *functions)
{
	if(functions == NULL)
	{
		CCD_General_Error_Number = 206;
		sprintf(CCD_General_Error_String,"CCD_Driver_Get_Functions:functions is NULL.");
		return FALSE;
	}
	(*functions) = Driver_Data.Functions;
	return TRUE;
}

/**
 * Close the opened dynamic library using dlclose(Driver_Data.Dynamic_Library_Handle);
 * @return The function returns TRUE on success and FALSE on failure.
 * @see #Driver_Data
 */
int CCD_Driver_Close(void)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_CONFIG,"CCD_Driver_Close() started.");
#endif
	retval = dlclose(Driver_Data.Dynamic_Library_Handle);
	if(retval != 0)
	{
		CCD_General_Error_Number = 207;
		sprintf(CCD_General_Error_String,"CCD_Driver_Close:dlclose(%p) failed (%s).",
			Driver_Data.Dynamic_Library_Handle,dlerror());
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log(CCD_GENERAL_LOG_BIT_CONFIG,"CCD_Driver_Close() finished.");
#endif
	return TRUE;
}
/*
** $Log: not supported by cvs2svn $
*/

