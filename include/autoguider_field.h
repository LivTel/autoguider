/* autoguider_field.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_field.h,v 1.4 2007-01-26 18:03:46 cjm Exp $
*/
#ifndef AUTOGUIDER_FIELD_H
#define AUTOGUIDER_FIELD_H

extern int Autoguider_Field_Initialise(void);
extern int Autoguider_Field_Exposure_Length_Set(int ms,int lock);
extern int Autoguider_Field(void);
extern int Autoguider_Field_Expose(void);
extern int Autoguider_Field_Is_Fielding(void);
extern int Autoguider_Field_Get_Last_Buffer_Index(void);
extern int Autoguider_Field_Set_Do_Dark_Subtract(int doit);
extern int Autoguider_Field_Get_Do_Dark_Subtract(void);
extern int Autoguider_Field_Set_Do_Flat_Field(int doit);
extern int Autoguider_Field_Get_Do_Flat_Field(void);
extern int Autoguider_Field_Set_Do_Object_Detect(int doit);
extern int Autoguider_Field_Get_Do_Object_Detect(void);
extern int Autoguider_Field_In_Object_Bounds(float ccd_x,float ccd_y);
extern int Autoguider_Field_Get_Unbinned_NCols(void);
extern int Autoguider_Field_Get_Unbinned_NRows(void);
extern int Autoguider_Field_Get_Bin_X(void);
extern int Autoguider_Field_Get_Bin_Y(void);

/*
** $Log: not supported by cvs2svn $
** Revision 1.3  2006/11/14 18:10:40  cjm
** Added Autoguider_Field_In_Object_Bounds.
**
** Revision 1.2  2006/06/20 13:10:38  cjm
** Added locking parameter to Autoguider_Field_Exposure_Length_Set.
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
