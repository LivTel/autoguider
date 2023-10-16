/* andor_setup.h
** $Header: /home/cjm/cvs/autoguider/ccd/andor/include/andor_setup.h,v 1.2 2014-01-31 17:21:30 cjm Exp $
*/
#ifndef ANDOR_SETUP_H
#define ANDOR_SETUP_H

/* get CCD_Setup_Window_Struct structure definition. */
#include "ccd_setup.h"
/* get config keyword root. */
#include "andor_general.h"

/**
 * Root string of keywords used by the autoguider CCD library.
 * Might want to be defined in a different header file.
 * @see #ANDOR_CCD_KEYWORD_ROOT
 */
#define ANDOR_SETUP_KEYWORD_ROOT    ANDOR_CCD_KEYWORD_ROOT"setup."


extern void Andor_Setup_Initialise(void);
extern int Andor_Setup_Startup(void);
extern int Andor_Setup_Shutdown(void);
extern int Andor_Setup_Dimensions_Check(int *ncols,int *nrows,int *hbin,int *vbin,
					int window_flags,struct CCD_Setup_Window_Struct *window);
extern int Andor_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				  int window_flags,struct CCD_Setup_Window_Struct window);
extern void Andor_Setup_Abort(void);
extern int Andor_Setup_Get_NCols(void);
extern int Andor_Setup_Get_NRows(void);
extern int Andor_Setup_Get_Buffer_Length(void);
extern int Andor_Setup_Get_Detector_Columns(void);
extern int Andor_Setup_Get_Detector_Rows(void);
extern int Andor_Setup_Allocate_Image_Buffer(void **buffer,size_t *buffer_length);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/03/27 14:03:01  cjm
** Initial revision
**
*/
#endif
