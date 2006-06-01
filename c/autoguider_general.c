/* autoguider_general.c
** Autoguider general routines
** $Header: /home/cjm/cvs/autoguider/c/autoguider_general.c,v 1.1 2006-06-01 15:18:38 cjm Exp $
*/
/**
 * General routines (logging, errror etc) for the autoguider program.
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
/**
 * This hash define is needed before including string.h to give us the strdup prototype.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <pthread.h> /* mutex */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "command_server.h"

#include "ccd_general.h"

#include "object.h"

#include "autoguider_general.h"

/* hash defines */
/**
 * Length of some filenames
 */
#define AUTOGUIDER_GENERAL_FILENAME_LENGTH      (256)

/* external variables */
/**
 * Variable holding error code of last operation performed.
 */
int Autoguider_General_Error_Number = 0;
/**
 * Internal variable holding description of the last error that occured.
 * @see #AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH
 */
char Autoguider_General_Error_String[AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH] = "";

/* data types */
/**
 * Data type holding local data to autoguider_general. This consists of the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>Function pointer to the routine that will log messages passed to it.</dd>
 * <dt>Log_Filter</dt> <dd>Function pointer to the routine that will filter log messages passed to it.
 * 		The funtion will return TRUE if the message should be logged, and FALSE if it shouldn't.</dd>
 * <dt>Log_Filter_Level</dt> <dd>A globally maintained log filter level. 
 * 		This is set using Autoguider_General_Set_Log_Filter_Level.
 * 		Autoguider_General_Log_Filter_Level_Absolute and 
 *              Autoguider_General_Log_Filter_Level_Bitwise test it against
 * 		message levels to determine whether to log messages.</dd>
 * <dt>Log_Directory</dt> <dd>Directory to write log messages to.</dd>
 * <dt>Log_Filename</dt> <dd>Filename to write log messages to.</dd>
 * <dt>Log_FP</dt> <dd>File pointer to write log messages to.</dd>
 * <dt>Error_Filename</dt> <dd>Filename to write error messages to.</dd>
 * <dt>Error_FP</dt> <dd>File pointer to write error messages to.</dd>
 * <dt>Config_Filename</dt> <dd>String containing the filename of the config file. Must be allocated before use.</dd>
 * </dl>
 * @see #Autoguider_General_Log
 * @see #Autoguider_General_Set_Log_Filter_Level
 * @see #Autoguider_General_Log_Filter_Level_Absolute
 * @see #Autoguider_General_Log_Filter_Level_Bitwise
 */
struct General_Struct
{
	void (*Log_Handler)(int level,char *string);
	int (*Log_Filter)(int level,char *string);
	int Log_Filter_Level;
	char Log_Directory[AUTOGUIDER_GENERAL_FILENAME_LENGTH];
	char Log_Filename[AUTOGUIDER_GENERAL_FILENAME_LENGTH];
	FILE *Log_Fp;
	char Error_Filename[AUTOGUIDER_GENERAL_FILENAME_LENGTH];
	FILE *Error_Fp;
	char *Config_Filename;
};

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: autoguider_general.c,v 1.1 2006-06-01 15:18:38 cjm Exp $";
/**
 * The instance of General_Struct that contains local data for this module.
 * This is statically initialised to the following:
 * <dl>
 * <dt>Log_Handler</dt> <dd>NULL</dd>
 * <dt>Log_Filter</dt> <dd>NULL</dd>
 * <dt>Log_Filter_Level</dt> <dd>0</dd>
 * <dt>Log_Directory</dt> <dd>NULL</dd>
 * <dt>Log_Filename</dt> <dd>log.txt</dd>
 * <dt>Log_FP</dt> <dd>NULL</dd>
 * <dt>Log_Filename</dt> <dd>error.txt</dd>
 * <dt>Error_FP</dt> <dd>NULL</dd>
 * </dl>
 * @see #General_Struct
 */
static struct General_Struct General_Data = 
{
	NULL,NULL,0,"","autoguider_log.txt",NULL,"autoguider_error.txt",NULL,NULL
};

/* internal functions */
static void General_Log_Handler_Hourly_File_Set_Fp(char *directory,char *basename,char *log_filename,FILE **log_fp);
static void General_Log_Handler_Get_Hourly_Filename(char *directory,char *basename,char *filename);
static void General_Log_Handler_Filename_To_Fp(char *log_filename,FILE **log_fp);

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * A general error routine. 
 * <b>Note</b> you cannot call both Autoguider_General_Error and Autoguider_General_Error_To_String 
 * to print the error string and 
 * get a string copy of it, only one of the error routines can be called after an error has been generated .
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @see #Autoguider_General_Get_Current_Time_String
 * @see #Autoguider_General_Error_Number
 * @see #Autoguider_General_Error_String
 * @see #General_Data
 * @see ../../libdprt/object/cdocs/object.html#Object_Get_Error_Number
 * @see ../../libdprt/object/cdocs/object.html#Object_Error_To_String
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Is_Error
 * @see ../commandserver/cdocs/command_server.html#Command_Server_Error_To_String
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Is_Error
 * @see ../ccd/cdocs/ccd_general.html#CCD_General_Error_To_String
 */
void Autoguider_General_Error(void)
{
	char buff[AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH];
	char time_string[32];
	int found = FALSE;

	/* change error files if necessary */
	General_Log_Handler_Hourly_File_Set_Fp(General_Data.Log_Directory,"autoguider_error",
					       General_Data.Error_Filename,&General_Data.Error_Fp);
	if(General_Data.Error_Fp == NULL)
	{
		fprintf(stderr,"Failed to set error Fp.\n");
		fflush(stderr);
		General_Data.Error_Fp = stderr;
	}
	strcpy(buff,"");
	if(Object_Get_Error_Number())
	{
		found = TRUE;
		Object_Error_To_String(buff);
		fprintf(General_Data.Error_Fp,"\t%s\n",buff);
		fflush(General_Data.Error_Fp);
	}
	strcpy(buff,"");
	if(Command_Server_Is_Error())
	{
		found = TRUE;
		Command_Server_Error_To_String(buff);
		fprintf(General_Data.Error_Fp,"\t%s\n",buff);
		fflush(General_Data.Error_Fp);
	}
	strcpy(buff,"");
	if(CCD_General_Is_Error())
	{
		found = TRUE;
		CCD_General_Error_To_String(buff);
		fprintf(General_Data.Error_Fp,"\t%s\n",buff);
		fflush(General_Data.Error_Fp);
	}
	if(Autoguider_General_Error_Number != 0)
	{
		found = TRUE;
		Autoguider_General_Get_Current_Time_String(time_string,32);
		fprintf(General_Data.Error_Fp,"%s Autoguider_General:Error(%d) : %s\n",time_string,
			Autoguider_General_Error_Number,Autoguider_General_Error_String);
		fflush(General_Data.Error_Fp);
	}
	if(!found)
	{
		fprintf(General_Data.Error_Fp,"Error:Autoguider_General_Error:Error not found\n");
		fflush(General_Data.Error_Fp);
	}
}

/**
 * A general error routine.
 * <b>Note</b> you cannot call both Autoguider_General_Error and Autoguider_General_Error_To_String 
 * to print the error string and 
 * get a string copy of it, only one of the error routines can be called after libccd has generated an error.
 * A second call to one of these routines will generate a 'Error not found' error!.
 * @param error_string A character buffer big enough to store the longest possible error message. It is
 * recomended that it is at least 1024 bytes in size.
 * @see #Autoguider_General_Get_Current_Time_String
 * @see #Autoguider_General_Error_Number
 * @see #Autoguider_General_Error_String
 */
void Autoguider_General_Error_To_String(char *error_string)
{
	char time_string[32];

	strcpy(error_string,"");
	if(Autoguider_General_Error_Number != 0)
	{
		Autoguider_General_Get_Current_Time_String(time_string,32);
		sprintf(error_string+strlen(error_string),"%s Autoguider_General:Error(%d) : %s\n",time_string,
			Autoguider_General_Error_Number,Autoguider_General_Error_String);
	}
	if(strlen(error_string) == 0)
	{
		strcat(error_string,"Error:Autoguider_General_Error:Error not found\n");
	}
}

/**
 * Routine to get the current time in a string. The string is returned in the format
 * '01/01/2000 13:59:59', or the string "Unknown time" if the routine failed.
 * The time is in UTC.
 * @param time_string The string to fill with the current time.
 * @param string_length The length of the buffer passed in. It is recommended the length is at least 20 characters.
 * @see #AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS
 */
void Autoguider_General_Get_Current_Time_String(char *time_string,int string_length)
{
	char timezone_string[16];
	char millsecond_string[8];
	struct timespec current_time;
	struct tm *utc_time = NULL;

	clock_gettime(CLOCK_REALTIME,&current_time);
	utc_time = gmtime(&(current_time.tv_sec));
	strftime(time_string,string_length,"%d-%m-%YT%H:%M:%S",utc_time);
	sprintf(millsecond_string,"%03d",(current_time.tv_nsec/AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS));
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
 * and uses vsprintf to format them i.e. like fprintf. The General_Buff is used to hold the created string,
 * therefore the total length of the generated string should not be longer than AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH.
 * Autoguider_General_Log is then called to handle the log message.
 * @param level An integer, used to decide whether this particular message has been selected for
 * 	logging or not.
 * @param format A string, with formatting statements the same as fprintf would use to determine the type
 * 	of the following arguments.
 * @see #Autoguider_General_Log
 * @see #AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH
 */
void Autoguider_General_Log_Format(int level,char *format,...)
{
	va_list ap;
	char buff[AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH];

/* format the arguments */
	va_start(ap,format);
	vsprintf(buff,format,ap);
	va_end(ap);
/* call the log routine to log the results */
	Autoguider_General_Log(level,buff);
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
void Autoguider_General_Log(int level,char *string)
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
 * Routine to set the General_Data.Log_Handler used by Autoguider_General_Log.
 * @param log_fn A function pointer to a suitable handler.
 * @see #General_Data
 * @see #Autoguider_General_Log
 */
void Autoguider_General_Set_Log_Handler_Function(void (*log_fn)(int level,char *string))
{
	General_Data.Log_Handler = log_fn;
}

/**
 * Routine to set the General_Data.Log_Filter used by Autoguider_General_Log.
 * @param log_fn A function pointer to a suitable filter function.
 * @see #General_Data
 * @see #Autoguider_General_Log
 */
void Autoguider_General_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string))
{
	General_Data.Log_Filter = filter_fn;
}

/**
 * Sets the contents of the General_Data.Log_Directory, used for constructing log/error filenames.
 * @param directory The directory name.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #AUTOGUIDER_GENERAL_FILENAME_LENGTH
 */
int Autoguider_General_Log_Set_Directory(char *directory)
{
	if(directory == NULL)
	{
		Autoguider_General_Error_Number = 103;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Log_Set_Directory:directory was NULL.");
		return FALSE;
	}
	if((AUTOGUIDER_GENERAL_FILENAME_LENGTH - strlen(directory)) < 10)
	{
		Autoguider_General_Error_Number = 104;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Log_Set_Directory:"
			"directory was too long (%d vs %d).",strlen(directory),AUTOGUIDER_GENERAL_FILENAME_LENGTH);
		return FALSE;
	}
	strcpy(General_Data.Log_Directory,directory);
	return TRUE;
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * Just prints the message to stdout, terminated by a newline.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 */
void Autoguider_General_Log_Handler_Stdout(int level,char *string)
{
	if(string == NULL)
		return;
	fprintf(stdout,"%s\n",string);
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * Prints the message to General_Data.Log_Fp, terminated by a newline, and thenflushes the stream.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 * @see #General_Data
 */
void Autoguider_General_Log_Handler_Log_Fp(int level,char *string)
{
	if(string == NULL)
		return;
	fprintf(General_Data.Log_Fp,"%s\n",string);
	fflush(General_Data.Log_Fp);
}

/**
 * A log handler to be used for the General_Data.Log_Handler function.
 * First calls General_Log_Handler_Hourly_File_Set_Fp to open/check the right log file is open.
 * Prints the message to General_Data.Log_Fp, terminated by a newline, and then flushes the stream.
 * @param level The log level for this message.
 * @param string The log message to be logged. 
 * @see #Autoguider_General_Get_Current_Time_String
 * @see #General_Data
 * @see #General_Log_Handler_Log_Hourly_File_Set_Fp
 */
void Autoguider_General_Log_Handler_Log_Hourly_File(int level,char *string)
{
	char time_string[32];

	if(string == NULL)
		return;
	General_Log_Handler_Hourly_File_Set_Fp(General_Data.Log_Directory,"autoguider_log",General_Data.Log_Filename,
					       &General_Data.Log_Fp);
	Autoguider_General_Get_Current_Time_String(time_string,32);
	fprintf(General_Data.Log_Fp,"%s : %s\n",time_string,string);
	fflush(General_Data.Log_Fp);
}

/**
 * Routine to set the General_Data.Log_Filter_Level.
 * @see #General_Data
 */
void Autoguider_General_Set_Log_Filter_Level(int level)
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
int Autoguider_General_Log_Filter_Level_Absolute(int level,char *string)
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
int Autoguider_General_Log_Filter_Level_Bitwise(int level,char *string)
{
	return ((level & General_Data.Log_Filter_Level) > 0);
}

/**
 * Utility function to add a string to an allocated string.
 * @param string The address of a pointer to a string. If a new string, the pointer should have
 *             been initialised to NULL.
 * @param add A non-null string to apend to the current contents of string.
 * @return The routine returns TRUE on success and FALSE on failure.
 * @see #Autoguider_General_Error_Number
 * @see #Autoguider_General_Error_String
 */
int Autoguider_General_Add_String(char **string,char *add)
{
	if(string == NULL)
	{
		Autoguider_General_Error_Number = 100;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Add_String:"
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
		Autoguider_General_Error_Number = 101;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Add_String:"
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
 * @see #Autoguider_General_Error_Number
 * @see #Autoguider_General_Error_String
 */
int Autoguider_General_Int_List_Add(int add,int **list,int *count)
{
	if(list == NULL)
	{
		Autoguider_General_Error_Number = 108;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Int_List_Add:list was NULL.");
		return FALSE;
	}
	if(count == NULL)
	{
		Autoguider_General_Error_Number = 109;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Int_List_Add:count was NULL.");
		return FALSE;
	}
	if((*list)==NULL)
		(*list) = (int*)malloc(sizeof(int));
	else
		(*list) = (int*)realloc(*list,((*count) + 1)*sizeof(int));
	if((*list)==NULL)
	{
		Autoguider_General_Error_Number = 110;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Int_List_Add:list realloc failed(%d).",
			(*count));
		return FALSE;
	}
	(*list)[(*count)++] = add;
	return TRUE;
}

/**
 * Routine to pass to qsort to sort a list on integers in ascending order.
 */
int Autoguider_General_Int_List_Sort(const void *f,const void *s)
{
	return (*(int*)f) - (*(int*)s);
}

/**
 * Routine to lock a access mutex. This will block until the mutex has been acquired,
 * unless an error occurs.
 * @param mutex A pointer to the variable containing the mutex to be locked.
 * @return Returns TRUE if the mutex has been  locked for access by this thread,
 * 	FALSE if an error occured.
 */
int Autoguider_General_Mutex_Lock(pthread_mutex_t *mutex)
{
	int error_number;

	error_number = pthread_mutex_lock(mutex);
	if(error_number != 0)
	{
		Autoguider_General_Error_Number = 102;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Mutex_Lock:Mutex lock failed '%d'.",
			error_number);
		return FALSE;
	}
	return TRUE;
}

/**
 * Routine to unlock the mutex. 
 * @param mutex A pointer to the variable containing the mutex to be unlocked.
 * @return Returns TRUE if the mutex has been unlocked, FALSE if an error occured.
 * @see #DSP_Data
 */
int Autoguider_General_Mutex_Unlock(pthread_mutex_t *mutex)
{
	int error_number;

	error_number = pthread_mutex_unlock(mutex);
	if(error_number != 0)
	{
		Autoguider_General_Error_Number = 105;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Mutex_Unlock:Mutex unlock failed '%d'.",
			error_number);
		return FALSE;
	}
	return TRUE;
}

/**
 * Set the config filename.
 * @param filename The string to copy.
 * @return Returns TRUE on success and FALSE on failure.
 * @see #General_Data
 */
int Autoguider_General_Set_Config_Filename(char *filename)
{
	if(filename == NULL)
	{
		Autoguider_General_Error_Number = 106;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Set_Config_Filename:Filename was NULL.");
		return FALSE;
	}
	General_Data.Config_Filename = strdup(filename);
	if(General_Data.Config_Filename == NULL)
	{
		Autoguider_General_Error_Number = 107;
		sprintf(Autoguider_General_Error_String,"Autoguider_General_Set_Config_Filename:strdup(%s) failed.",
			filename);
		return FALSE;
	}
	return TRUE;
}

/**
 * Get the config filename.
 * @return Returns the current config filename. This can be NULL if it has not been set properly.
 * @see #General_Data
 */
char *Autoguider_General_Get_Config_Filename(void)
{
	return General_Data.Config_Filename;
}

/* ----------------------------------------------------------------------------
** 		internal functions 
** ---------------------------------------------------------------------------- */
/**
 * Set up a log/error file FP correctly.
 * @param directory The directory.
 * @param basename The basename, either "autoguider_log"/"autoguider_error".
 * @param log_filename A string pointer to fill with the current/new log/error filename.
 * @param log_fp The address of a FILE pointer to fill with the new fp, if the filename changed, otherwise
 *               remains the same.
 */
static void General_Log_Handler_Hourly_File_Set_Fp(char *directory,char *basename,char *log_filename,FILE **log_fp)
{
	char new_filename[AUTOGUIDER_GENERAL_FILENAME_LENGTH];

	if((*log_fp) == NULL)
	{
		General_Log_Handler_Get_Hourly_Filename(directory,basename,log_filename);
		General_Log_Handler_Filename_To_Fp(log_filename,log_fp);
	}
	else
	{
		General_Log_Handler_Get_Hourly_Filename(directory,basename,new_filename);
		if(strcmp(new_filename,log_filename) != 0)
		{
			fflush(*log_fp);
			fclose(*log_fp);
			(*log_fp) = NULL;
			strcpy(log_filename,new_filename);
			General_Log_Handler_Filename_To_Fp(log_filename,log_fp);
		}
	}
}

/**
 * Given a directory and  basename, set a filename.
 * @param directory The directory string (can be NULL).
 * @param basename The log file basename (e.g. log/error), cannot be NULL.
 * @param filename A string to fill in with the constructed filename.
 */
static void General_Log_Handler_Get_Hourly_Filename(char *directory,char *basename,char *filename)
{
	time_t now_time;
	int doy,hod;
	struct tm *now_tm;

	if(filename == NULL)
		return;
	now_time = time(NULL);
	if(now_time == (time_t)-1)
	{
		/* what do I do here? */
		strcpy(filename,basename);
		return;
	}
	now_tm = gmtime(&now_time);
	if(now_tm == NULL)
	{
		/* what do I do here? */
		strcpy(filename,basename);
		return;
	}
	doy = now_tm->tm_yday;
	hod = now_tm->tm_hour;
	if(directory != NULL)
	{
		sprintf(filename,"%s/%s_%d_%d.txt",directory,basename,doy,hod);
	}
	else
	{
		sprintf(filename,"%s_%d_%d.txt",basename,doy,hod);
	}
}

/**
 * Try to open the specified file for appending.
 * The FP will not be opened if it is NULL, the filename is NULL, or the fopen fails.
 * @param log_filename The string containing the filename to open.
 * @param log_fp The address of a pointer to open the Fp.
 */
static void General_Log_Handler_Filename_To_Fp(char *log_filename,FILE **log_fp)
{
	if(log_fp == NULL)
	{
		return;
	}
	if(log_filename == NULL)
	{
		(*log_fp) = NULL;
		return;
	}
	(*log_fp) = fopen(log_filename,"a");
	/* (*log_fp) can still be NULL here on failure */
}

/*
** $Log: not supported by cvs2svn $
*/
