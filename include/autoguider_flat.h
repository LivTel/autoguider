/* autoguider_flat.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_flat.h,v 1.1 2006-06-01 15:19:05 cjm Exp $
*/
#ifndef AUTOGUIDER_FLAT_H
#define AUTOGUIDER_FLAT_H

extern int Autoguider_Flat_Initialise(void);
extern int Autoguider_Flat_Set_Dimension(int ncols,int nrows,int x_bin,int y_bin);
extern int Autoguider_Flat_Set(int bin_x,int bin_y);
extern int Autoguider_Flat_Field(float *buffer_ptr,int pixel_count,int ncols,int nrows,int use_window,
			  struct CCD_Setup_Window_Struct window);
extern int Autoguider_Flat_Shutdown(void);

/*
** $Log: not supported by cvs2svn $
*/
#endif
