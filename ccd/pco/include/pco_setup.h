/* pco_setup.h */

#ifndef PCO_SETUP_H
#define PCO_SETUP_H

/* get CCD_Setup_Window_Struct structure definition. */
#include "ccd_setup.h"
/* get PCO_COMMAND_TIMESTAMP_MODE enum */
#include "pco_command.h"

/**
 * Is the specified binning value valid. Valid binning numbers are: 1,2,?. 
 * @param b The binning value to test.
 * @return TRUE if the binning is a valid number and FALSE if it not.
 */
#define PCO_SETUP_BINNING_IS_VALID(b) ((b==1)||(b==2))

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
extern int PCO_Setup_Dimensions_Check(int *ncols,int *nrows,int *hbin,int *vbin,
				      int window_flags,struct CCD_Setup_Window_Struct *window);
extern int PCO_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
extern void PCO_Setup_Abort(void);
extern int PCO_Setup_Get_NCols(void);
extern int PCO_Setup_Get_NRows(void);
extern enum PCO_COMMAND_TIMESTAMP_MODE PCO_Setup_Get_Timestamp_Mode(void);
	
#ifdef __cplusplus
}
#endif

#endif
