/* autoguider_object.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_object.h,v 1.1 2006-06-01 15:19:05 cjm Exp $
*/
#ifndef AUTOGUIDER_OBJECT_H
#define AUTOGUIDER_OBJECT_H

/**
 * Structure defining a single object recognised in the image. Consists of:
 * <dl>
 * <dt>Index</dt> <dd>Initial index in the list, a sort of ID.</dd>
 * <dt>CCD_X_Position</dt> <dd>The x position of the centre of the object on the CCD, 
 *                   weighted by pixel * value in pixels.</dd>
 * <dt>CCD_Y_Position</dt> <dd>The x position of the centre of the object on the CCD, 
 *                   weighted by pixel * value in pixels.</dd>
 * <dt>Buffer_X_Position</dt> <dd>The x position of the centre of the object in the (sub)window buffer, 
 *                      weighted by pixel * value in pixels.</dd>
 * <dt>Buffer_Y_Position</dt> <dd>The x position of the centre of the object in the (sub)window buffer, 
 *                      weighted by pixel * value in pixels.</dd>
 * <dt>Total_Counts</dt> <dd>The total number of counts above the median for all pixels in the object.</dd>
 * <dt>Pixel_Count</dt> <dd>The number of pixels this object covers.</dd>
 * <dt>Peak_Counts</dt> <dd>The number of counts above the median for the brightest pixel in the object.</dd>
 * <dt>Is_Stellar</dt> <dd>Boolean, TRUE if the object is star-like according to the object detection.</dd>
 * <dt>FWHM_X</dt> <dd>Full Width Half Maximum of the object in X in <b>pixels</b>, 
 *                 only set if Is_Stellar is TRUE.</dd>
 * <dt>FWHM_Y</dt> <dd>Full Width Half Maximum of the object in Y in <b>pixels</b>, 
 *                 only set if Is_Stellar is TRUE.</dd>
 * </dl>
 * Based on data in libdprt_object object.h
 * @see ../../libdprt/object/dcocs/object.html#Object_Struct
 */
struct Autoguider_Object_Struct 
{
	int Index;
	float CCD_X_Position;
	float CCD_Y_Position;
	float Buffer_X_Position;
	float Buffer_Y_Position;
	float Total_Counts;
	int Pixel_Count;
	float Peak_Counts;
	int Is_Stellar;
	float FWHM_X;
	float FWHM_Y;
};

/* extern int Autoguider_Object_Initialise(void);*/
/* extern int Autoguider_Object_Set_Dimension(int ncols,int nrows,int x_bin,int y_bin);*/
extern int Autoguider_Object_Detect(float *buffer,int naxis1,int naxis2,int start_x,int start_y,
				    int use_standard_deviation,int id,int frame_number);
extern int Autoguider_Object_Shutdown(void);
extern int Autoguider_Object_List_Get_Count(int *count);
extern int Autoguider_Object_List_Get_Object(int index,struct Autoguider_Object_Struct *object);
extern int Autoguider_Object_List_Get_Object_List_String(char **object_list_string);

/*
** $Log: not supported by cvs2svn $
*/
#endif
