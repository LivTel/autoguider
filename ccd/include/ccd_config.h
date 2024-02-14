/* ccd_config.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_config.h,v 1.2 2006-09-12 11:05:36 cjm Exp $
*/
#ifndef CCD_CONFIG_H
#define CCD_CONFIG_H

/**
 * Root string of keywords used by the autoguider CCD library.
 */
#define CCD_CONFIG_KEYWORD_ROOT    "ccd."

/* We now need CCD_General external prototypes to be declared extern C when compiling using C++ compilers.
** This is required for the PCO camera driver that is compiled with C++ and logs via the CCD_General API.
** The following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

extern int CCD_Config_Initialise(void);
extern int CCD_Config_Load(char *filename);
extern int CCD_Config_Shutdown(void);
extern int CCD_Config_Get_String(char *key, char **value);
extern int CCD_Config_Get_Integer(char *key, int *i);
extern int CCD_Config_Get_Long(char *key, long *l);
extern int CCD_Config_Get_Unsigned_Short(char *key,unsigned short *us);
extern int CCD_Config_Get_Double(char *key, double *d);
extern int CCD_Config_Get_Float(char *key, float *f);
extern int CCD_Config_Get_Boolean(char *key, int *boolean);
/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:27:58  cjm
** Initial revision
**
*/
#ifdef __cplusplus
}
#endif

#endif
