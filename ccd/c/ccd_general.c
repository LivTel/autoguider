/* ccd_general.c
** Autoguider CCD Library general routines
** $Header: /home/cjm/cvs/autoguider/ccd/c/ccd_general.c,v 1.1 2006-06-01 15:27:37 cjm Exp $
*/
/**
 * General routines (logging, errror etc) for the autoguider CCD library.
 * @author SDSU, Chris Mottram
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
#include "ccd_general.h"


/* external variables */
/**
 * Variable holding error code of last operation performed.
 */
int CCD_General_Error_Number = 0;
/**
 * Internal variable holding description of the last error that occured.
 * @see #CCD_GENERAL_ERROR_STRING_LENGTH
 */
char CCD_General_Error_String[CCD_GENERAL_ERROR_STRING_LENGTH] = "";

/* data types */
/**
 * Data type holding local data to ccd_general. This consists of the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>Function pointer to the routine that will log messages passed to it.</dd>
 * <dt>Log_Filter</dt> <dd>Function pointer to the routine that will filter log messages passed to it.
 * 		The funtion will return TRUE if the message should be logged, and FALSE if it shouldn't.</dd>
 * <dt>Log_Filter_Level</dt> <dd>A globally maintained log filter level. 
 * 		This is set using CCD_General_Set_Log_Filter_Level.
 * 		CCD_General_Log_Filter_Level_Absolute and CCD_General_Log_Filter_Level_Bitwise test it against
 * 		message levels to determine whether to log messages.</dd>
 * </dl>
 * @see #CCD_General_Log
 * @see #CCD_General_Set_Log_Filter_Level
 * @see #CCD_General_Log_Filter_Level_Absolute
 * @see #CCD_General_Log_Filter_Level_Bitwise
 */
struct General_Struct
{
	void (*Log_Handler)(int level,char *string);
	int (*Log_Filter)(int level,char *string);
	int Log_Filter_Level;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: ccd_general.c,v 1.1 2006-06-01 15:27:37 cjm Exp $";
/**
 * The instance of General_Struct that contains local data for this module.
 * This is statically initialised to the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>NULL</dd>
 * <dt>Log_Filter</dt> <dd>NULL</dd>
 * <dt>Log_Filter_Level</dt> <dd>0</dd>
 * </dl>
 * @see #General_Struct
 */
static struct General_Struct General_Data = 
{
	NULL,NULL,0
};

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * A general error routine. 
 * <b>Note</b> you cannot call both CCD_General_Error and CCD_CCD_General_Error_String to print the error string and 
 * get a string copy of it, only one of the error routines can be called after libccd has generated an error.
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @see #CCD_General_Get_Current_Time_String
 * @see #CCD_General_Error_Number
 * @see #CCD_General_Error_String
 */
void CCD_General_Error(void)
{
	char time_string[32];
	int found = FALSE;

	if(CCD_General_Error_Number != 0)
	{
		found = TRUE;
		CCD_General_Get_Current_Time_String(time_string,32);
		fprintf(stderr,"%s CCD_General:Error(%d) : %s\n",time_string,
			CCD_General_Error_Number,CCD_General_Error_String);
	}
	if(!found)
	{
		fprintf(stderr,"Error:CCD_General_Error:Error not found\n");
	}
}

/**
 * A general error routine.
 * <b>Note</b> you cannot call both CCD_General_Error and CCD_CCD_General_Error_String to print the error string and 
 * get a string copy of it, only one of the error routines can be called after libccd has generated an error.
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @param error_string A character buffer big enough to store the longest possible error message. It is
 * recomended that it is at least 1024 bytes in size.
 * @see #CCD_General_Get_Current_Time_String
 * @see #CCD_General_Error_Number
 * @see #CCD_General_Error_String
 */
void CCD_General_Error_To_String(char *error_string)
{
	char time_string[32];

	strcpy(error_string,"");
	if(CCD_General_Error_Number != 0)
	{
		CCD_General_Get_Current_Time_String(time_string,32);
		sprintf(error_string+strlen(error_string),"%s CCD_General:Error(%d) : %s\n",time_string,
			CCD_General_Error_Number,CCD_General_Error_String);
	}
	if(strlen(error_string) == 0)
	{
		strcat(error_string,"Error:CCD_General_Error:Error not found\n");
	}
}

/**
 * Routine checking whether the CCD library currently has an 'active' error.
 * @return The routine returns TRUE if there is an 'active' error, otherwise FALSE.
 * @see #CCD_General_Error_Number
 */
int CCD_General_Is_Error(void)
{
	return (CCD_General_Error_Number != 0);
}

/**
 * Routine to get the current time in a string. The string is returned in the format
 * '01/01/2000 13:59:59', or the string "Unknown time" if the routine failed.
 * The time is in UTC.
 * @param time_string The string to fill with the current time.
 * @param string_length The length of the buffer passed in. It is recommended the length is at least 20 characters.
 */
void CCD_General_Get_Current_Time_String(char *time_string,int string_length)
{
	time_t current_time;
	struct tm *utc_time = NULL;

	if(time(&current_time) > -1)
	{
		utc_time = gmtime(&current_time);
		strftime(time_string,string_length,"%d/%m/%Y %H:%M:%S",utc_time);
	}
	else
		strncpy(time_string,"Unknown time",string_length);
}

/**
 * Routine to log a message to a defined logging mechanism. This routine has an arbitary number of arguments,
 * and uses vsprintf to format them i.e. like fprintf. The General_Buff is used to hold the created string,
 * therefore the total length of the generated string should not be longer than CCD_GENERAL_ERROR_STRING_LENGTH.
 * CCD_General_Log is then called to handle the log message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param format A string, with formatting statements the same as fprintf would use to determine the type
 * 	of the following arguments.
 * @see #CCD_General_Log
 * @see #CCD_GENERAL_ERROR_STRING_LENGTH
 */
void CCD_General_Log_Format(int level,char *format,...)
{
	va_list ap;
	char buff[CCD_GENERAL_ERROR_STRING_LENGTH];

/* format the arguments */
	va_start(ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);
/* call the log routine to log the results */
	CCD_General_Log(level,buff);
}

/**
 * Routine to log a message to a defined logging mechanism. If the string or General_Data.Log_Handler are NULL
 * the routine does not log the message. If the General_Data.Log_Filter function pointer is non-NULL, the
 * message is passed to it to determoine whether to log the message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param string The message to log.
 * @see #General_Data
 */
void CCD_General_Log(int level,char *string)
{
/* If the string is NULL, don't log. */
	if(string == NULL)
		return;
/* If there is no log handler, return */
	if(General_Data.Log_Handler == NULL)
		return;
/* If there's a log filter, check it returns TRUE for this message */
	if(General_Data.Log_Filter != NULL)
	{
		if(General_Data.Log_Filter(level,string) == FALSE)
			return;
	}
/* We can log the message */
	(*General_Data.Log_Handler)(level,string);
}

/**
 * Routine to set the General_Data.Log_Handler used by CCD_General_Log.
 * @param log_fn A function pointer to a suitable handler.
 * @see #General_Data
 * @see #CCD_General_Log
 */
void CCD_General_Set_Log_Handler_Function(void (*log_fn)(int level,char *string))
{
	General_Data.Log_Handler = log_fn;
}

/**
 * Routine to set the General_Data.Log_Filter used by CCD_General_Log.
 * @param log_fn A function pointer to a suitable filter function.
 * @see #General_Data
 * @see #CCD_General_Log
 */
void CCD_General_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string))
{
	General_Data.Log_Filter = filter_fn;
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * Just prints the message to stdout, terminated by a newline.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 */
void CCD_General_Log_Handler_Stdout(int level,char *string)
{
	if(string == NULL)
		return;
	fprintf(stdout,"%s\n",string);
}

/**
 * Routine to set the General_Data.Log_Filter_Level.
 * @see #General_Data
 */
void CCD_General_Set_Log_Filter_Level(int level)
{
	General_Data.Log_Filter_Level = level;
}

/**
 * A log message filter routine, to be used for the General_Data.Log_Filter function pointer.
 * @param level The log level of the message to be tested.
 * @param string The log message to be logged, not used in this filter. 
 * @return The routine returns TRUE if the level is less than or equal to the General_Data.Log_Filter_Level,
 * 	otherwise it returns FALSE.
 * @see #General_Data
 */
int CCD_General_Log_Filter_Level_Absolute(int level,char *string)
{
	return (level <= General_Data.Log_Filter_Level);
}

/**
 * A log message filter routine, to be used for the General_Data.Log_Filter function pointer.
 * @param level The log level of the message to be tested.
 * @param string The log message to be logged, not used in this filter. 
 * @return The routine returns TRUE if the level has bits set that are also set in the 
 * 	General_Data.Log_Filter_Level, otherwise it returns FALSE.
 * @see #General_Data
 */
int CCD_General_Log_Filter_Level_Bitwise(int level,char *string)
{
	return ((level & General_Data.Log_Filter_Level) > 0);
}

/*
** $Log: not supported by cvs2svn $
*/
