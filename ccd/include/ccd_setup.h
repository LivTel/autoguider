/* ccd_setup.h
** $Header: /home/cjm/cvs/autoguider/ccd/include/ccd_setup.h,v 1.1 2006-06-01 15:27:58 cjm Exp $
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
 * @see #CCD_Setup_Dimensions
 */
struct CCD_Setup_Window_Struct
{
	int X_Start;
	int Y_Start;
	int X_End;
	int Y_End;
};

extern void CCD_Setup_Initialise(void);
extern int CCD_Setup_Startup(void);
extern int CCD_Setup_Shutdown(void);
extern int CCD_Setup_Dimensions(int ncols,int nrows,int nsbin,int npbin,
				int window_flags,struct CCD_Setup_Window_Struct window);
extern void CCD_Setup_Abort(void);
extern int CCD_Setup_Get_NCols(void);
extern int CCD_Setup_Get_NRows(void);
/* diddly delete?
extern int CCD_Setup_Get_NSBin(void);
extern int CCD_Setup_Get_NPBin(void);
extern int CCD_Setup_Get_Window_Pixel_Count(int window_index);
extern int CCD_Setup_Get_Window_Width(int window_index);
extern int CCD_Setup_Get_Window_Height(int window_index);
extern int CCD_Setup_Get_Window_Flags(void);
extern int CCD_Setup_Get_Window(struct CCD_Setup_Window_Struct *window);
extern int CCD_Setup_Get_Setup_Complete(void);
extern int CCD_Setup_Get_Setup_In_Progress(void);
*/

/*
** $Log: not supported by cvs2svn $
*/
#endif
