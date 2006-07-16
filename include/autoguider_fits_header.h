/* autoguider_fits_header.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_fits_header.h,v 1.1 2006-07-16 20:15:06 cjm Exp $
*/
#ifndef AUTOGUIDER_FITS_HEADER_H
#define AUTOGUIDER_FITS_HEADER_H

/* for fitsfile declaration */
#include "fitsio.h"

/**
 * Structure defining the contents of a FITS header. Note the common basic FITS cards may not
 * be defined in this list.
 * <dl>
 * <dt>Card_List</dt> <dd>A list of Fits_Header_Card_Struct.</dd>
 * <dt>Card_Count</dt> <dd>The number of cards in the (reallocatable) list.</dd>
 * <dt>Allocated_Card_Count</dt> <dd>The amount of memory allocated in the Card_List pointer, 
 *     in terms of number of cards.</dd>
 * </dl>
 * @see #Fits_Header_Card_Struct
 */
struct Fits_Header_Struct
{
	struct Fits_Header_Card_Struct *Card_List;
	int Card_Count;
	int Allocated_Card_Count;
};

extern int Autoguider_Fits_Header_Clear(struct Fits_Header_Struct *header);
extern int Autoguider_Fits_Header_Delete(struct Fits_Header_Struct *header,char *keyword);
extern int Autoguider_Fits_Header_Add_String(struct Fits_Header_Struct *header,char *keyword,char *value,
					     char *comment);
extern int Autoguider_Fits_Header_Add_Int(struct Fits_Header_Struct *header,char *keyword,int value,char *comment);
extern int Autoguider_Fits_Header_Add_Float(struct Fits_Header_Struct *header,char *keyword,double value,
					    char *comment);
extern int Autoguider_Fits_Header_Add_Logical(struct Fits_Header_Struct *header,char *keyword,int value,char *comment);
extern int Autoguider_Fits_Header_Free(struct Fits_Header_Struct *header);

extern int Autoguider_Fits_Header_Write_To_Fits(struct Fits_Header_Struct header,fitsfile *fits_fp);

/*
** $Log: not supported by cvs2svn $
*/
#endif
