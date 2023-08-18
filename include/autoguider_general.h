/* autoguider_general.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_general.h,v 1.4 2009-01-30 18:01:48 cjm Exp $
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
extern void Autoguider_General_Error(char *sub_system,char *source_filename,char *function,int level,char *category);
extern void Autoguider_General_Error_To_String(char *sub_system,char *source_filename,char *function,int level,
					       char *category,char *error_string);

/* routine used by other modules error code */
extern void Autoguider_General_Get_Current_Time_String(char *time_string,int string_length);

/* logging routines */
extern void Autoguider_General_Log_Format(const char *sub_system,const char *source_filename,const char *function,
					  int level,const char *category,const char *format,...);
extern void Autoguider_General_Log(const char *sub_system,const char *source_filename,const char *function,int level,
				   const char *category,const char *string);
extern void Autoguider_General_Call_Log_Handlers(char *sub_system,char *source_filename,char *function,int level,
						 char *category,char *message);
extern void Autoguider_General_Call_Log_Handlers_Const(const char *sub_system,const char *source_filename,
						       const char *function,int level,const char *category,
						       const char *message);
extern int Autoguider_General_Add_Log_Handler_Function(void (*log_fn)(const char *sub_system,
						       const char *source_filename,const char *function,int level,
						       const char *category,const char *message));
extern void Autoguider_General_Set_Log_Filter_Function(int (*filter_fn)(const char *sub_system,
						       const char *source_filename,const char *function,int level,
						       const char *category,const char *message));
extern int Autoguider_General_Log_Set_Directory(char *directory);
extern int Autoguider_General_Log_Set_UDP(int active,char *hostname,int port_number);
extern void Autoguider_General_Log_Handler_Stdout(const char *sub_system,const char *source_filename,
						  const char *function,int level,
						  const char *category,const char *message);
extern void Autoguider_General_Log_Handler_Log_Fp(const char *sub_system,const char *source_filename,
						  const char *function,int level,
						  const char *category,const char *message);
extern void Autoguider_General_Log_Handler_Log_Hourly_File(const char *sub_system,const char *source_filename,
							   const char *function,
							   int level,const char *category,const char *message);
extern void Autoguider_General_Log_Handler_Log_UDP(const char *sub_system,const char *source_filename,
						   const char *function,
						   int level,const char *category,const char *message);
extern void Autoguider_General_Set_Log_Filter_Level(int level);
extern int Autoguider_General_Log_Filter_Level_Absolute(const char *sub_system,const char *source_filename,
							const char *function,
							int level,const char *category,const char *message);
extern int Autoguider_General_Log_Filter_Level_Bitwise(const char *sub_system,const char *source_filename,
						       const char *function,
						       int level,const char *category,const char *message);

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
** Revision 1.3  2006/07/14 14:02:14  cjm
** Added FITS header logging bit.
**
** Revision 1.2  2006/06/12 19:24:55  cjm
** Added CIL log bit.
** Added fdifftime.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
