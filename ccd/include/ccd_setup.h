/* ccd_setup.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_setup.h,v 1.2 2014-01-31 17:19:37 cjm Exp $
*/
#ifndef CCD_SETUP_H
#define CCD_SETUP_H

/**
 * Structure holding position information for one window on the CCD. Fields are:
 * <dl>
 * <dt>X_Start</dt> <dd>The pixel number of the X start position of the window (upper left corner).</dd>
 * <dt>Y_Start</dt> <dd>The pixel number of the Y start position of the window (upper left corner).</dd>
 * <dt>X_End</dt> <dd>The pixel number of the X end position of the window (lower right corner).</dd>
 * <dt>Y_End</dt> <dd>The pixel number of the Y end position of the window (lower right corner).</dd>
 * </dl>
 * The dimensions are inclusive, i.e. a window of ((X_Start=100,Y_Start=200),(X_End=200,Y_End=300)) has a size
 * of (101,101) pixels.
 * @see #CCD_Setup_Dimensions
 */
struct CCD_Setup_Window_Struct
{
	int X_Start;
	int Y_Start;
	int X_End;
	int Y_End;
};

extern int CCD_Setup_Initialise(void);
extern int CCD_Setup_Startup(void);
extern int CCD_Setup_Shutdown(void);
extern int CCD_Setup_Dimensions_Check(int *ncols,int *nrows,int *nsbin,int *npbin,
				int window_flags,struct CCD_Setup_Window_Struct *window);
extern int CCD_Setup_Dimensions(int ncols,int nrows,int nsbin,int npbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
extern void CCD_Setup_Abort(void);
extern int CCD_Setup_Get_NCols(void);
extern int CCD_Setup_Get_NRows(void);
extern int CCD_Setup_Get_Flip_X(void);
extern int CCD_Setup_Get_Flip_Y(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:27:58  cjm
** Initial revision
**
*/
#endif
