/* autoguider_general.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_general.h,v 1.3 2006-07-14 14:02:14 cjm Exp $
*/
#ifndef AUTOGUIDER_GENERAL_H
#define AUTOGUIDER_GENERAL_H

#include <pthread.h>

/* hash defines */
/**
 * TRUE is the value usually returned from routines to indicate success.
 */
#ifndef TRUE
#define TRUE 1
#endif
/**
 * FALSE is the value usually returned from routines to indicate failure.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * Macro to check whether the parameter is either TRUE or FALSE.
 */
#define AUTOGUIDER_GENERAL_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))

#ifndef MAX
/**
 * Return maximum of two values passed in.
 */
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef MIN
/**
 * Return minimum of two values passed in.
 */
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

/**
 * This is the length of error string of modules in the library.
 */
#define AUTOGUIDER_GENERAL_ERROR_STRING_LENGTH	(1024)

/**
 * Value to pass into logging calls, used for general code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_GENERAL	(1<<0)
/**
 * Value to pass into logging calls, used for command code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_COMMAND	(1<<1)
/**
 * Value to pass into logging calls, used for server code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_SERVER	(1<<2)
/**
 * Value to pass into logging calls, used for buffer code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_BUFFER	(1<<3)
/**
 * Value to pass into logging calls, used for field code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_FIELD	(1<<4)
/**
 * Value to pass into logging calls, used for guide code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_GUIDE	(1<<5)
/**
 * Value to pass into logging calls, used for get fits code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_GET_FITS	(1<<6)
/**
 * Value to pass into logging calls, used for dark code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_DARK	        (1<<7)
/**
 * Value to pass into logging calls, used for flat code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_FLAT	        (1<<8)
/**
 * Value to pass into logging calls, used for object code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_OBJECT	(1<<9)
/**
 * Value to pass into logging calls, used for CIL code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_CIL	        (1<<10)
/**
 * Value to pass into logging calls, used for FITS Header code logging.
 * @see #Autoguider_General_Log
 */
#define AUTOGUIDER_GENERAL_LOG_BIT_FITS_HEADER	(1<<11)

/**
 * The number of nanoseconds in one second. A struct timespec has fields in nanoseconds.
 */
#define AUTOGUIDER_GENERAL_ONE_SECOND_NS	(1000000000)
/**
 * The number of nanoseconds in one millisecond. A struct timespec has fields in nanoseconds.
 */
#define AUTOGUIDER_GENERAL_ONE_MILLISECOND_NS	(1000000)
/**
 * The number of milliseconds in one second.
 */
#define AUTOGUIDER_GENERAL_ONE_SECOND_MS	(1000)
/**
 * The number of nanoseconds in one microsecond.
 */
#define AUTOGUIDER_GENERAL_ONE_MICROSECOND_NS	(1000)

#ifndef fdifftime
/**
 * Return double difference (in seconds) between two struct timespec's.
 * @param t0 A struct timespec.
 * @param t1 A struct timespec.
 * @return A double, in seconds, representing the time elapsed from t0 to t1.
 * @see #AUTOGUIDER_GENERAL_ONE_SECOND_NS
 */
#define fdifftime(t1, t0) (((double)(((t1).tv_sec)-((t0).tv_sec))+(double)(((t1).tv_nsec)-((t0).tv_nsec))/AUTOGUIDER_GENERAL_ONE_SECOND_NS))
#endif

/* external variabless */
extern int Autoguider_General_Error_Number;
extern char Autoguider_General_Error_String[];

/* external functions */
extern void Autoguider_General_Error(void);
extern void Autoguider_General_Error_To_String(char *error_string);

/* routine used by other modules error code */
extern void Autoguider_General_Get_Current_Time_String(char *time_string,int string_length);

/* logging routines */
extern void Autoguider_General_Log_Format(int level,char *format,...);
extern void Autoguider_General_Log(int level,char *string);
extern void Autoguider_General_Set_Log_Handler_Function(void (*log_fn)(int level,char *string));
extern void Autoguider_General_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string));
extern int Autoguider_General_Log_Set_Directory(char *directory);
extern void Autoguider_General_Log_Handler_Stdout(int level,char *string);
extern void Autoguider_General_Log_Handler_Log_Fp(int level,char *string);
extern void Autoguider_General_Log_Handler_Log_Hourly_File(int level,char *string);
extern void Autoguider_General_Set_Log_Filter_Level(int level);
extern int Autoguider_General_Log_Filter_Level_Absolute(int level,char *string);
extern int Autoguider_General_Log_Filter_Level_Bitwise(int level,char *string);

/* utility routines */
extern int Autoguider_General_Add_String(char **string,char *add);
extern int Autoguider_General_Int_List_Add(int add,int **list,int *count);
extern int Autoguider_General_Int_List_Sort(const void *f,const void *s);
extern int Autoguider_General_Mutex_Lock(pthread_mutex_t *mutex);
extern int Autoguider_General_Mutex_Unlock(pthread_mutex_t *mutex);

extern int Autoguider_General_Set_Config_Filename(char *filename);
extern char *Autoguider_General_Get_Config_Filename(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/12 19:24:55  cjm
** Added CIL log bit.
** Added fdifftime.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
