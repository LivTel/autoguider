/* fli_setup.h
** $Header: /home/cjm/cvs/autoguider/ccd/fli/include/fli_setup.h,v 1.1 2013-11-26 16:28:41 cjm Exp $
*/
#ifndef FLI_SETUP_H
#define FLI_SETUP_H

/* get CCD_Setup_Window_Struct structure definition. */
#include "ccd_setup.h"
/* get config keyword root. */
#include "fli_general.h"
/* get flidev_t */
#include "libfli.h"

/**
 * Root string of keywords used by the autoguider CCD library.
 * Might want to be defined in a different header file.
 * @see fli_general.h#FLI_CCD_KEYWORD_ROOT
 */
#define FLI_SETUP_KEYWORD_ROOT    FLI_CCD_KEYWORD_ROOT"setup."


extern void FLI_Setup_Initialise(void);
extern int FLI_Setup_Startup(void);
extern int FLI_Setup_Shutdown(void);
extern int FLI_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
extern void FLI_Setup_Abort(void);
extern flidev_t FLI_Setup_Get_Dev(void);
extern int FLI_Setup_Get_NCols(void);
extern int FLI_Setup_Get_NRows(void);
extern int FLI_Setup_Get_Buffer_Length(void);
extern int FLI_Setup_Get_Detector_Columns(void);
extern int FLI_Setup_Get_Detector_Rows(void);
extern int FLI_Setup_Allocate_Image_Buffer(void **buffer,size_t *buffer_length);

/*
** $Log: not supported by cvs2svn $
*/
#endif
