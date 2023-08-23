/* ccd_general.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_general.h,v 1.3 2010-07-29 09:53:03 cjm Exp $
*/
#ifndef CCD_GENERAL_H
#define CCD_GENERAL_H

#include <time.h>

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
#define CCD_GENERAL_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))

/**
 * This is the length of error string of modules in the library.
 */
#define CCD_GENERAL_ERROR_STRING_LENGTH	(1024)

/**
 * This is the number of bytes used to represent one pixel on the CCD. We read out the Andor Controller
 * as unsigned shorts (16 bits) (2 bytes). The library will currently only compile when this
 * is two, as some parts assume 16 bit values.
 */
#define CCD_GENERAL_BYTES_PER_PIXEL	(2)

/**
 * The number of nanoseconds in one second. A struct timespec has fields in nanoseconds.
 */
#define CCD_GENERAL_ONE_SECOND_NS	(1000000000)
/**
 * The number of nanoseconds in one millisecond. A struct timespec has fields in nanoseconds.
 */
#define CCD_GENERAL_ONE_MILLISECOND_NS	(1000000)
/**
 * The number of milliseconds in one second.
 */
#define CCD_GENERAL_ONE_SECOND_MS	(1000)
/**
 * The number of nanoseconds in one microsecond.
 */
#define CCD_GENERAL_ONE_MICROSECOND_NS	(1000)

/* We now need CCD_General external prototypes to be declared extern C when compiling using C++ compilers.
** This is required for the PCO camera driver that is compiled with C++ and logs via the CCD_General API.
** The following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/* external variabless */
extern int CCD_General_Error_Number;
extern char CCD_General_Error_String[];

/* external functions */
extern void CCD_General_Error(void);
extern void CCD_General_Error_To_String(char *error_string);
extern int CCD_General_Is_Error(void);

/* routine used by other modules error code */
extern void CCD_General_Get_Current_Time_String(char *time_string,int string_length);
/* time format */
extern int CCD_General_Get_Time_String(struct timespec time,char *time_string,int string_length);

/* logging routines */
extern void CCD_General_Log_Format(const char *sub_system,const char *source_filename,const char *function,int level,
				   const char *category,const char *format,...);
extern void CCD_General_Log(const char *sub_system,const char *source_filename,const char *function,int level,
			    const char *category,const char *string);
extern void CCD_General_Set_Log_Handler_Function(void (*log_fn)(const char *sub_system,const char *source_filename,
						 const char *function,int level,const char *category,
						 const char *string));
extern void CCD_General_Set_Log_Filter_Function(int (*filter_fn)(const char *sub_system,const char *source_filename,
						const char *function,int level,const char *category,
						const char *string));
extern void CCD_General_Log_Handler_Stdout(const char *sub_system,const char *source_filename,const char *function,
					   int level,const char *category,const char *string);
extern void CCD_General_Set_Log_Filter_Level(int level);
extern int CCD_General_Log_Filter_Level_Absolute(const char *sub_system,const char *source_filename,
						 const char *function,int level,
						 const char *category,const char *string);
extern int CCD_General_Log_Filter_Level_Bitwise(const char *sub_system,const char *source_filename,
						const char *function,int level,
						const char *category,const char *string);

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2009/01/30 18:00:36  cjm
** Removed log bits.
**
** Revision 1.1  2006/06/01 15:27:58  cjm
** Initial revision
**
*/
#ifdef __cplusplus
}
#endif

#endif
