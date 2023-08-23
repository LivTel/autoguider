/* pco_setup.h */

#ifndef PCO_SETUP_H
#define PCO_SETUP_H

/* get CCD_Setup_Window_Struct structure definition. */
#include "ccd_setup.h"

/**
 * Root string of setup keywords used by the PCO driver library.
 * @see pco_general.h#PCO_CCD_KEYWORD_ROOT
 */
#define PCO_SETUP_KEYWORD_ROOT    PCO_CCD_KEYWORD_ROOT"setup."

/*  the following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/* functions */
extern int PCO_Setup_Startup(void);
extern int PCO_Setup_Shutdown(void);
extern int PCO_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
extern void PCO_Setup_Abort(void);
extern int PCO_Setup_Get_NCols(void);
extern int PCO_Setup_Get_NRows(void);

#ifdef __cplusplus
}
#endif

#endif
