/* ccd_config.c
** Autoguider CCD Library config routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_config.c,v 1.3 2009-01-30 18:00:24 cjm Exp $
*/
/**
 * Config routines for the autoguider CCD library.
 * Just a a wrapper  for the eSTAR_Config routines at the moment.
 * @author SDSU, Chris Mottram
 * @version $Revision: 1.3 $
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
#include "estar_config.h"
#include "log_udp.h"
#include "ccd_general.h"
#include "ccd_config.h"

/* data types */
/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_config.c,v 1.3 2009-01-30 18:00:24 cjm Exp $";
/**
 * eSTAR config properties.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Properties_t
 */
static eSTAR_Config_Properties_t Config_Properties;

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * This does nothing at the moment.
 */
void CCD_Config_Initialise(void)
{
	int i;

	/* do nothing atm - this is just to pacify cdoc. */
}

/**
 * Load the configuration file. Calls eSTAR_Config_Parse_File.
 * @param filename The filename to load from.
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Parse_File
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Load(char *filename)
{
	int retval;

	if(filename == NULL)
	{
		CCD_General_Error_Number = 106;
		sprintf(CCD_General_Error_String,"CCD_Config_Load failed: filename was NULL.");
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Load",LOG_VERBOSITY_INTERMEDIATE,NULL,"started(%s).",
			       filename);
#endif
	retval = eSTAR_Config_Parse_File(filename,&Config_Properties);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 100;
		sprintf(CCD_General_Error_String,"CCD_Config_Load failed:");
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Load",LOG_VERBOSITY_INTERMEDIATE,NULL,
			       "(%s) returned %d.",filename,retval);
#endif
	return retval;
}

/**
 * Shutdown anything associated with config. Calls eSTAR_Config_Destroy_Properties.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Destroy_Properties
 * @see #Config_Properties
 */
int CCD_Config_Shutdown(void)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_config.c","CCD_Config_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,
			"started: About to call eSTAR_Config_Destroy_Properties.");
#endif
	eSTAR_Config_Destroy_Properties(&Config_Properties);
#ifdef CCD_DEBUG
	CCD_General_Log("ccd","ccd_config.c","CCD_Config_Shutdown",LOG_VERBOSITY_VERBOSE,NULL,"finished.");
#endif
	return retval;
}

/**
 * Get a string value from the configuration file. Calls eSTAR_Config_Get_String.
 * @param key The config keyword.
 * @param value The address of a string pointer to hold the returned string. The returned value is allocated
 *        memory by eSTAR_Config_Get_String, which <b>should be freed</b> after use.
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_String
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_String(char *key, char **value)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_String",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,value);
#endif
	retval = eSTAR_Config_Get_String(&Config_Properties,key,value);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 101;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_String(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	/* we may have to re-think this if keyword value strings are too long */
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_String",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %s (%d).",key,(*value),retval);
#endif
	return retval;
}

/**
 * Get a integer value from the configuration file. Calls eSTAR_Config_Get_Int.
 * @param key The config keyword.
 * @param value The address of an integer to hold the returned value. 
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Int
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Integer(char *key, int *i)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Integer",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,i);
#endif
	retval = eSTAR_Config_Get_Int(&Config_Properties,key,i);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 102;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Integer(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Integer",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %d.",key,*i);
#endif
	return retval;
}

/**
 * Get a long value from the configuration file. Calls eSTAR_Config_Get_Long.
 * @param key The config keyword.
 * @param value The address of a long to hold the returned value. 
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Long
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Long(char *key, long *l)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Long",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,l);
#endif
	retval = eSTAR_Config_Get_Long(&Config_Properties,key,l);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 103;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Long(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Long",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %ld.",key,*l);
#endif
	return retval;
}

/**
 * Get an unsigned short value from the configuration file. Calls eSTAR_Config_Get_Unsigned_Short.
 * @param key The config keyword.
 * @param value The address of a unsigned short to hold the returned value. 
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Unsigned_Short
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Unsigned_Short(char *key,unsigned short *us)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Unsigned_Short",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,us);
#endif
	retval = eSTAR_Config_Get_Unsigned_Short(&Config_Properties,key,us);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 107;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Unsigned_Short(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Unsigned_Short",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %hu.",
			       key,*us);
#endif
	return retval;
}

/**
 * Get a double value from the configuration file. Calls eSTAR_Config_Get_Double.
 * @param key The config keyword.
 * @param value The address of a double to hold the returned value. 
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Double
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Double(char *key, double *d)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Double",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,d);
#endif
	retval = eSTAR_Config_Get_Double(&Config_Properties,key,d);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 104;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Double(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Double",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %.2f.",key,*d);
#endif
	return retval;
}

/**
 * Get a float value from the configuration file. Calls eSTAR_Config_Get_Float.
 * @param key The config keyword.
 * @param value The address of a float to hold the returned value. 
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Float
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Float(char *key,float *f)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Float",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,f);
#endif
	retval = eSTAR_Config_Get_Float(&Config_Properties,key,f);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 108;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Float(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Float",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %.2f.",key,*f);
#endif
	return retval;
}

/**
 * Get a boolean value from the configuration file. Calls eSTAR_Config_Get_Boolean.
 * @param key The config keyword.
 * @param value The address of an integer to hold the returned boolean value. 
 *             The keyword value should have "true/TRUE/True" for TRUE and "false/FALSE/False" for FALSE.
 * @return The routine returns TRUE on sucess, FALSE on failure.
 * @see ../../../estar/config/estar_config.html#eSTAR_Config_Get_Boolean
 * @see #Config_Properties
 * @see ccd_general.html#CCD_General_Log_Format
 * @see ccd_general.html#CCD_General_Log
 * @see ccd_general.html#CCD_CCD_General_Error_Number
 * @see ccd_general.html#CCD_CCD_General_Error_String
 */
int CCD_Config_Get_Boolean(char *key, int *boolean)
{
	int retval;

#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Boolean",LOG_VERBOSITY_VERBOSE,NULL,
			       "started(%s,%p).",key,boolean);
#endif
	retval = eSTAR_Config_Get_Boolean(&Config_Properties,key,boolean);
	if(retval == FALSE)
	{
		CCD_General_Error_Number = 105;
		sprintf(CCD_General_Error_String,"CCD_Config_Get_Boolean(%s) failed:",key);
		eSTAR_Config_Error_To_String(CCD_General_Error_String+strlen(CCD_General_Error_String));
		return FALSE;
	}
#ifdef CCD_DEBUG
	CCD_General_Log_Format("ccd","ccd_config.c","CCD_Config_Get_Boolean",LOG_VERBOSITY_VERBOSE,NULL,
			       "(%s) returned %d.",key,*boolean);
#endif
	return retval;
}

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/09/12 11:06:32  cjm
** Added CCD_Config_Get_Float.
**
** Revision 1.1  2006/06/01 15:27:37  cjm
** Initial revision
**
*/

