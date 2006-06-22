/* autoguider_guide.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_guide.h,v 1.2 2006-06-22 15:53:05 cjm Exp $
*/
#ifndef AUTOGUIDER_GUIDE_H
#define AUTOGUIDER_GUIDE_H

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
extern int Autoguider_Guide_Set_Guide_Object(int index);
extern double Autoguider_Guide_Loop_Cadence_Get(void);
extern int Autoguider_Guide_Exposure_Length_Get(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
