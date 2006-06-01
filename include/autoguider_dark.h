/* autoguider_dark.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_dark.h,v 1.1 2006-06-01 15:19:05 cjm Exp $
*/
#ifndef AUTOGUIDER_DARK_H
#define AUTOGUIDER_DARK_H

extern int Autoguider_Dark_Initialise(void);
extern int Autoguider_Dark_Set_Dimension(int ncols,int nrows,int x_bin,int y_bin);
extern int Autoguider_Dark_Set(int bin_x,int bin_y,int exposure_length);
extern int Autoguider_Dark_Subtract(float *buffer_ptr,int pixel_count,int ncols,int nrows,int use_window,
				    struct CCD_Setup_Window_Struct window);
extern int Autoguider_Dark_Shutdown(void);
extern int Autoguider_Dark_Get_Exposure_Length_Nearest(int *exposure_length,int *exposure_length_index);
extern int Autoguider_Dark_Get_Exposure_Length_Index(int index,int *exposure_length);
extern int Autoguider_Dark_Get_Exposure_Length_Count(void);
/*
** $Log: not supported by cvs2svn $
*/
#endif
