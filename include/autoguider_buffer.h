/* autoguider_buffer.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_buffer.h,v 1.3 2007-01-30 17:41:43 cjm Exp $
*/
#ifndef AUTOGUIDER_BUFFER_H
#define AUTOGUIDER_BUFFER_H

extern int Autoguider_Buffer_Initialise(void);
extern int Autoguider_Buffer_Set_Field_Dimension(int ncols,int nrows,int x_bin,int y_bin);
extern int Autoguider_Buffer_Set_Guide_Dimension(int ncols,int nrows,int x_bin,int y_bin);
extern int Autoguider_Buffer_Get_Field_Pixel_Count(void);
extern int Autoguider_Buffer_Get_Field_Binned_NCols(void);
extern int Autoguider_Buffer_Get_Field_Binned_NRows(void);
extern int Autoguider_Buffer_Get_Guide_Pixel_Count(void);
extern int Autoguider_Buffer_Get_Guide_Binned_NCols(void);
extern int Autoguider_Buffer_Get_Guide_Binned_NRows(void);

extern int Autoguider_Buffer_Raw_Field_Lock(int index,unsigned short **buffer_ptr);
extern int Autoguider_Buffer_Raw_Field_Unlock(int index);
extern int Autoguider_Buffer_Raw_Guide_Lock(int index,unsigned short **buffer_ptr);
extern int Autoguider_Buffer_Raw_Guide_Unlock(int index);
extern int Autoguider_Buffer_Raw_Field_Copy(int index,unsigned short *buffer_ptr,size_t buffer_length);
extern int Autoguider_Buffer_Raw_Guide_Copy(int index,unsigned short *buffer_ptr,size_t buffer_length);

extern int Autoguider_Buffer_Reduced_Field_Lock(int index,float **buffer_ptr);
extern int Autoguider_Buffer_Reduced_Field_Unlock(int index);
extern int Autoguider_Buffer_Reduced_Guide_Lock(int index,float **buffer_ptr);
extern int Autoguider_Buffer_Reduced_Guide_Unlock(int index);
extern int Autoguider_Buffer_Reduced_Field_Copy(int index,float *buffer_ptr,size_t buffer_length);
extern int Autoguider_Buffer_Reduced_Guide_Copy(int index,float *buffer_ptr,size_t buffer_length);

extern int Autoguider_Buffer_Raw_To_Reduced_Field(int index);
extern int Autoguider_Buffer_Raw_To_Reduced_Guide(int index);

extern int Autoguider_Buffer_Field_Exposure_Start_Time_Set(int index,struct timespec start_time);
extern int Autoguider_Buffer_Field_Exposure_Start_Time_Get(int index,struct timespec *start_time);
extern int Autoguider_Buffer_Field_Exposure_Length_Set(int index,int exposure_length_ms);
extern int Autoguider_Buffer_Field_Exposure_Length_Get(int index,int *exposure_length_ms);
extern int Autoguider_Buffer_Field_CCD_Temperature_Set(int index,double current_ccd_temperature);
extern int Autoguider_Buffer_Field_CCD_Temperature_Get(int index,double *current_ccd_temperature);

extern int Autoguider_Buffer_Guide_Exposure_Start_Time_Set(int index,struct timespec start_time);
extern int Autoguider_Buffer_Guide_Exposure_Start_Time_Get(int index,struct timespec *start_time);
extern int Autoguider_Buffer_Guide_Exposure_Length_Set(int index,int exposure_length_ms);
extern int Autoguider_Buffer_Guide_Exposure_Length_Get(int index,int *exposure_length_ms);
extern int Autoguider_Buffer_Guide_CCD_Temperature_Set(int index,double current_ccd_temperature);
extern int Autoguider_Buffer_Guide_CCD_Temperature_Get(int index,double *current_ccd_temperature);

extern int Autoguider_Buffer_Shutdown(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2007/01/26 18:03:46  cjm
** Field and Guide Exposure Length/Exposure Start Time Getters/Setters.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
