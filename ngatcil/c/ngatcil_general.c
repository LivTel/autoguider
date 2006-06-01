/* ngatcil_general.c
** NGATCil general routines
** $Header: /home/cjm/cvs/autoguider/ngatcil/c/ngatcil_general.c,v 1.1 2006-06-01 15:28:06 cjm Exp $
*/
/**
 * General routines (logging, errror etc) for the NGAT Cil library.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ngatcil_general.h"

/* hash defines */

/* external variables */
/**
 * Variable holding error code of last operation performed.
 */
int NGATCil_General_Error_Number = 0;
/**
 * Internal variable holding description of the last error that occured.
 * @see #NGATCIL_GENERAL_ERROR_STRING_LENGTH
 */
char NGATCil_General_Error_String[NGATCIL_GENERAL_ERROR_STRING_LENGTH] = "";

/* data types */
/**
 * Data type holding local data to ngatcil_general. This consists of the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>Function pointer to the routine that will log messages passed to it.</dd>
 * <dt>Log_Filter</dt> <dd>Function pointer to the routine that will filter log messages passed to it.
 * 		The funtion will return TRUE if the message should be logged, and FALSE if it shouldn't.</dd>
 * <dt>Log_Filter_Level</dt> <dd>A globally maintained log filter level. 
 * 		This is set using NGATCil_General_Set_Log_Filter_Level.
 * 		NGATCil_General_Log_Filter_Level_Absolute and 
 *              NGATCil_General_Log_Filter_Level_Bitwise test it against
 * 		message levels to determine whether to log messages.</dd>
 * </dl>
 * @see #NGATCil_General_Log
 * @see #NGATCil_General_Set_Log_Filter_Level
 * @see #NGATCil_General_Log_Filter_Level_Absolute
 * @see #NGATCil_General_Log_Filter_Level_Bitwise
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
static char rcsid[] = "$Id: ngatcil_general.c,v 1.1 2006-06-01 15:28:06 cjm Exp $";

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
 * <b>Note</b> you cannot call both NGATCil_General_Error and NGATCil_General_Error_To_String 
 * to print the error string and 
 * get a string copy of it, only one of the error routines can be called after an error has been generated .
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @see #NGATCil_General_Get_Current_Time_String
 * @see #NGATCil_General_Error_Number
 * @see #NGATCil_General_Error_String
 * @see #General_Data
 */
void NGATCil_General_Error(void)
{
	char time_string[32];

	NGATCil_General_Get_Current_Time_String(time_string,32);
	if(NGATCil_General_Error_Number == 0)
	{
		sprintf(NGATCil_General_Error_String,
			 "NGATCil_General_Error:Internal Error:Error code was zero.\n");
	}
	fprintf(stderr,"%s NGATCil Error (%d) : %s\n",
		 time_string,NGATCil_General_Error_Number,NGATCil_General_Error_String);
}

/**
 * A general error routine.
 * <b>Note</b> you cannot call both NGATCil_General_Error and NGATCil_General_Error_To_String 
 * to print the error string and 
 * get a string copy of it, only one of the error routines can be called after libccd has generated an error.
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @param error_string A character buffer big enough to store the longest possible error message. It is
 * recomended that it is at least 1024 bytes in size.
 * @see #NGATCil_General_Get_Current_Time_String
 * @see #NGATCil_General_Error_Number
 * @see #NGATCil_General_Error_String
 */
void NGATCil_General_Error_To_String(char *error_string)
{
	char time_string[32];

	NGATCil_General_Get_Current_Time_String(time_string,32);
	/*
	 * if the error number is zero an error message has not been set up
	 * This is in itself an error as we should not be calling this routine
	 * without there being an error to display
	 */
	if(NGATCil_General_Error_Number == 0)
	{
		sprintf(NGATCil_General_Error_String,"Logic Error: No Error defined");
	}
	/* print error to the string */
	sprintf(error_string,"%s NGATCil Error (%d) : %s\n",time_string,NGATCil_General_Error_Number,
		NGATCil_General_Error_String);
	/* reset error number */
	NGATCil_General_Error_Number = 0;
}

/**
 * Routine checking whether NGATCil currently has an 'active' error.
 * @return The routine returns TRUE if there is an 'active' error, otherwise FALSE.
 * @see #NGATCil_General_Error_Number
 */
int NGATCil_General_Is_Error(void)
{
	return (NGATCil_General_Error_Number != 0);
}

/**
 * Routine to get the current time in a string. The string is returned in the format
 * '01/01/2000 13:59:59.123 +0000', or the string "Unknown time" if the routine failed.
 * The time is in the specified timezone.
 * @param time_string The string to fill with the current time.
 * @param string_length The length of the buffer passed in. It is recommended the length is at least 20 characters.
 * @see #NGATCIL_GENERAL_ONE_MILLISECOND_NS
 */
void NGATCil_General_Get_Current_Time_String(char *time_string,int string_length)
{
	char timezone_string[16];
	char millsecond_string[8];
	struct timespec current_time;
	struct tm *utc_time = NULL;

	clock_gettime(CLOCK_REALTIME,&current_time);
	utc_time = gmtime(&(current_time.tv_sec));
	strftime(time_string,string_length,"%d-%m-%YT%H:%M:%S",utc_time);
	sprintf(millsecond_string,"%03d",(current_time.tv_nsec/NGATCIL_GENERAL_ONE_MILLISECOND_NS));
	strftime(timezone_string,16,"%z",utc_time);
	if((strlen(time_string)+strlen(millsecond_string)+strlen(timezone_string)+3) < string_length)
	{
		strcat(time_string,".");
		strcat(time_string,millsecond_string);
		strcat(time_string," ");
		strcat(time_string,timezone_string);
	}
}

/**
 * Routine to log a message to a defined logging mechanism. This routine has an arbitary number of arguments,
 * and uses vsprintf to format them i.e. like fprintf. An internal buffer is used to hold the created string,
 * therefore the total length of the generated string should not be longer than NGATCIL_GENERAL_ERROR_STRING_LENGTH.
 * NGATCil_General_Log is then called to handle the log message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param format A string, with formatting statements the same as fprintf would use to determine the type
 * 	of the following arguments.
 * @see #NGATCil_General_Log
 * @see #NGATCIL_GENERAL_ERROR_STRING_LENGTH
 */
void NGATCil_General_Log_Format(int level,char *format,...)
{
	va_list ap;
	char buff[NGATCIL_GENERAL_ERROR_STRING_LENGTH];

/* format the arguments */
	va_start(ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);
/* call the log routine to log the results */
	NGATCil_General_Log(level,buff);
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
void NGATCil_General_Log(int level,char *string)
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
 * Routine to set the General_Data.Log_Handler used by NGATCil_General_Log.
 * @param log_fn A function pointer to a suitable handler.
 * @see #General_Data
 * @see #NGATCil_General_Log
 */
void NGATCil_General_Set_Log_Handler_Function(void (*log_fn)(int level,char *string))
{
	General_Data.Log_Handler = log_fn;
}

/**
 * Routine to set the General_Data.Log_Filter used by NGATCil_General_Log.
 * @param log_fn A function pointer to a suitable filter function.
 * @see #General_Data
 * @see #NGATCil_General_Log
 */
void NGATCil_General_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string))
{
	General_Data.Log_Filter = filter_fn;
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * Just prints the message to stdout, terminated by a newline.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 */
void NGATCil_General_Log_Handler_Stdout(int level,char *string)
{
	if(string == NULL)
		return;
	fprintf(stdout,"%s\n",string);
}

/**
 * Routine to set the General_Data.Log_Filter_Level.
 * @see #General_Data
 */
void NGATCil_General_Set_Log_Filter_Level(int level)
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
int NGATCil_General_Log_Filter_Level_Absolute(int level,char *string)
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
int NGATCil_General_Log_Filter_Level_Bitwise(int level,char *string)
{
	return ((level & General_Data.Log_Filter_Level) > 0);
}

/**
 * Utility function to add a string to an allocated string.
 * @param string The address of a pointer to a string. If a new string, the pointer should have
 *             been initialised to NULL.
 * @param add A non-null string to apend to the current contents of string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #NGATCil_General_Error_Number
 * @see #NGATCil_General_Error_String
 */
int NGATCil_General_Add_String(char **string,char *add)
{
	if(string == NULL)
	{
		NGATCil_General_Error_Number = 1;
		sprintf(NGATCil_General_Error_String,"NGATCil_General_Add_String:"
			"NULL string pointer to add %s to.",add);
		return FALSE;
	}
	if(add == NULL)
		return TRUE;
	if((*string) == NULL)
	{
		(*string) = (char*)malloc((strlen(add)+1)*sizeof(char));
		if((*string) != NULL)
			(*string)[0] = '\0';
	}
	else
		(*string) = (char*)realloc((*string),(strlen((*string))+strlen(add)+1)*sizeof(char));
	if((*string) == NULL)
	{
		NGATCil_General_Error_Number = 2;
		sprintf(NGATCil_General_Error_String,"NGATCil_General_Add_String:"
			"Memory allocation error (%s).",add);
		return FALSE;
	}
	strcat((*string),add);
	return TRUE;
}

/**
 * Add an integer to a list of integers.
 * @param add The integer value to add.
 * @param list The address of a list of integers.
 * @param count The address of an integer storing the number of integers in the list.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #NGATCil_General_Error_Number
 * @see #NGATCil_General_Error_String
 */
int NGATCil_General_Int_List_Add(int add,int **list,int *count)
{
	if(list == NULL)
	{
		NGATCil_General_Error_Number = 3;
		sprintf(NGATCil_General_Error_String,"NGATCil_General_Int_List_Add:list was NULL.");
		return FALSE;
	}
	if(count == NULL)
	{
		NGATCil_General_Error_Number = 4;
		sprintf(NGATCil_General_Error_String,"NGATCil_General_Int_List_Add:count was NULL.");
		return FALSE;
	}
	if((*list)==NULL)
		(*list) = (int*)malloc(sizeof(int));
	else
		(*list) = (int*)realloc(*list,((*count) + 1)*sizeof(int));
	if((*list)==NULL)
	{
		NGATCil_General_Error_Number = 5;
		sprintf(NGATCil_General_Error_String,"NGATCil_General_Int_List_Add:list realloc failed(%d).",
			(*count));
		return FALSE;
	}
	(*list)[(*count)++] = add;
	return TRUE;
}

/**
 * Routine to pass to qsort to sort a list on integers in ascending order.
 */
int NGATCil_General_Int_List_Sort(const void *f,const void *s)
{
	return (*(int*)f) - (*(int*)s);
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/*
** $Log: not supported by cvs2svn $
*/
