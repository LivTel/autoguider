/* autoguider_guide.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_guide.h,v 1.5 2007-02-09 14:35:04 cjm Exp $
*/
#ifndef AUTOGUIDER_GUIDE_H
#define AUTOGUIDER_GUIDE_H

/* for CCD_Setup_Window_Struct */
#include "ccd_setup.h"

extern int Autoguider_Guide_Initialise(void);
extern int Autoguider_Guide_Window_Set(int sx,int sy,int ex,int ey);
extern int Autoguider_Guide_Exposure_Length_Set(int ms,int lock);
extern int Autoguider_Guide_On(void);
extern int Autoguider_Guide_Off(void);
extern int Autoguider_Guide_Is_Guiding(void);
extern int Autoguider_Guide_Get_Last_Buffer_Index(void);
extern int Autoguider_Guide_Set_Do_Dark_Subtract(int doit);
extern int Autoguider_Guide_Get_Do_Dark_Subtract(void);
extern int Autoguider_Guide_Set_Do_Flat_Field(int doit);
extern int Autoguider_Guide_Get_Do_Flat_Field(void);
extern int Autoguider_Guide_Set_Do_Object_Detect(int doit);
extern int Autoguider_Guide_Get_Do_Object_Detect(void);
extern int Autoguider_Guide_Set_Guide_Window_Tracking(int doit);
extern int Autoguider_Guide_Get_Guide_Window_Tracking(void);
extern int Autoguider_Guide_Set_Guide_Object(int index);
extern int Autoguider_Guide_Window_Set_From_XY(int ccd_x_position,int ccd_y_position);
extern double Autoguider_Guide_Loop_Cadence_Get(void);
extern int Autoguider_Guide_Exposure_Length_Get(void);
extern int Autoguider_Guide_Unbinned_NCols_Get(void);
extern int Autoguider_Guide_Unbinned_NRows_Get(void);
extern int Autoguider_Guide_Bin_X_Get(void);
extern int Autoguider_Guide_Bin_Y_Get(void);
extern int Autoguider_Guide_Binned_NCols_Get(void);
extern int Autoguider_Guide_Binned_NRows_Get(void);
extern struct CCD_Setup_Window_Struct Autoguider_Guide_Window_Get(void);
extern int Autoguider_Guide_Timecode_Scaling_Set(float value);
extern float Autoguider_Guide_Timecode_Scaling_Get(void);
/*
** $Log: not supported by cvs2svn $
** Revision 1.4  2007/01/26 18:03:46  cjm
** Added dimension getters.
**
** Revision 1.3  2007/01/19 14:23:46  cjm
** Added autoguider guide window tracking:
** Autoguider_Guide_Set_Guide_Window_Tracking
** Autoguider_Guide_Get_Guide_Window_Tracking
** Autoguider_Guide_Window_Set_From_XY
**
** Revision 1.2  2006/06/22 15:53:05  cjm
** Added Autoguider_Guide_Set_Guide_Object/Autoguider_Guide_Loop_Cadence_Get/Autoguider_Guide_Exposure_Length_Get.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
