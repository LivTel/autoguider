/* ccd_general.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_general.h,v 1.1 2006-06-01 15:27:58 cjm Exp $
*/
#ifndef CCD_GENERAL_H
#define CCD_GENERAL_H

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
 * Value to pass into logging calls, used for general code logging.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BIT_GENERAL	(1<<0)
/**
 * Value to pass into logging calls, used for config code logging.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BIT_CONFIG	(1<<1)
/**
 * Value to pass into logging calls, used for setup code logging.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BIT_SETUP	(1<<2)
/**
 * Value to pass into logging calls, used for exposure code logging.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BIT_EXPOSURE	(1<<3)
/**
 * Value to pass into logging calls, used for temperature code logging.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BIT_TEMPERATURE	(1<<4)

/**
 * Shift offset value to pass into logging calls, used for andor sub-library code logging.
 * Reserves bits 8-16.
 * @see #CCD_General_Log
 */
#define CCD_GENERAL_LOG_BLOCK_ANDOR	(8)

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

/* external variabless */
extern int CCD_General_Error_Number;
extern char CCD_General_Error_String[];

/* external functions */
extern void CCD_General_Error(void);
extern void CCD_General_Error_To_String(char *error_string);
extern int CCD_General_Is_Error(void);

/* routine used by other modules error code */
extern void CCD_General_Get_Current_Time_String(char *time_string,int string_length);

/* logging routines */
extern void CCD_General_Log_Format(int level,char *format,...);
extern void CCD_General_Log(int level,char *string);
extern void CCD_General_Set_Log_Handler_Function(void (*log_fn)(int level,char *string));
extern void CCD_General_Set_Log_Filter_Function(int (*filter_fn)(int level,char *string));
extern void CCD_General_Log_Handler_Stdout(int level,char *string);
extern void CCD_General_Set_Log_Filter_Level(int level);
extern int CCD_General_Log_Filter_Level_Absolute(int level,char *string);
extern int CCD_General_Log_Filter_Level_Bitwise(int level,char *string);

/*
** $Log: not supported by cvs2svn $
*/
#endif
