/* ngatcil_general.h
** $Header: /home/cjm/cvs/autoguider/ngatcil/include/ngatcil_general.h,v 1.4 2009-01-30 18:01:01 cjm Exp $
*/
#ifndef NGATCIL_GENERAL_H
#define NGATCIL_GENERAL_H
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
#define NGATCIL_GENERAL_IS_BOOLEAN(value)	(((value) == TRUE)||((value) == FALSE))

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
#define NGATCIL_GENERAL_ERROR_STRING_LENGTH	(1024)

/**
 * The number of nanoseconds in one millisecond. A struct timespec has fields in nanoseconds.
 */
#define NGATCIL_GENERAL_ONE_MILLISECOND_NS	(1000000)

/* external variabless */
extern int NGATCil_General_Error_Number;
extern char NGATCil_General_Error_String[];

/* external functions */
extern void NGATCil_General_Error(void);
extern void NGATCil_General_Error_To_String(char *error_string);
extern int NGATCil_General_Is_Error(void);

/* routine used by other modules error code */
extern void NGATCil_General_Get_Current_Time_String(char *time_string,int string_length);

/* logging routines */
extern void NGATCil_General_Log_Format(char *sub_system,char *source_filename,
				       char *function,int level,char *category,char *format,...);
extern void NGATCil_General_Log(char *sub_system,char *source_filename,
				char *function,int level,char *category,char *string);
extern void NGATCil_General_Set_Log_Handler_Function(void (*log_fn)(char *sub_system,char *source_filename,
						     char *function,int level,char *category,char *string));
extern void NGATCil_General_Set_Log_Filter_Function(int (*filter_fn)(char *sub_system,char *source_filename,
						    char *function,int level,char *category,char *string));
extern void NGATCil_General_Log_Handler_Stdout(char *sub_system,char *source_filename,
					       char *function,int level,char *category,char *string);
extern void NGATCil_General_Set_Log_Filter_Level(int level);
extern int NGATCil_General_Log_Filter_Level_Absolute(char *sub_system,char *source_filename,
						     char *function,int level,char *category,char *string);
extern int NGATCil_General_Log_Filter_Level_Bitwise(char *sub_system,char *source_filename,
						    char *function,int level,char *category,char *string);

/* utility routines */
extern int NGATCil_General_Add_String(char **string,char *add);
extern int NGATCil_General_Int_List_Add(int add,int **list,int *count);
extern int NGATCil_General_Int_List_Sort(const void *f,const void *s);

/*
** $Log: not supported by cvs2svn $
** Revision 1.3  2006/07/20 15:16:01  cjm
** Added AGS SDB logging.
**
** Revision 1.2  2006/06/06 10:33:17  cjm
** Added NGATCIL_GENERAL_LOG_BIT_CIL.
**
** Revision 1.1  2006/06/01 15:28:10  cjm
** Initial revision
**
*/

#endif
