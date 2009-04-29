/* autoguider_get_fits.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_get_fits.h,v 1.3 2009-04-29 10:57:57 cjm Exp $
*/
#ifndef AUTOGUIDER_GET_FITS_H
#define AUTOGUIDER_GET_FITS_H

/* need struct Fits_Header_Struct */
#include "autoguider_fits_header.h"

/**
 * Which buffer to get the fits image for.
 */
#define AUTOGUIDER_GET_FITS_BUFFER_TYPE_FIELD      (0)
/**
 * Which buffer to get the fits image for.
 */
#define AUTOGUIDER_GET_FITS_BUFFER_TYPE_GUIDE      (1)

/**
 * Whether to get the raw or reduced buffer.
 */
#define AUTOGUIDER_GET_FITS_BUFFER_STATE_RAW      (0)
/**
 * Whether to get the raw or reduced buffer.
 */
#define AUTOGUIDER_GET_FITS_BUFFER_STATE_REDUCED  (1)

extern int Autoguider_Get_Fits(int buffer_type,int buffer_state,int object_index,
			       void **buffer_ptr,size_t *buffer_length);
extern int Autoguider_Get_Fits_From_Buffer(void **buffer_ptr,size_t *buffer_length,int buffer_state,
					   void *buffer_data_ptr,size_t buffer_data_length,int ncols,int nrows,
					   struct Fits_Header_Struct *fits_header);

/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2007/01/26 18:03:46  cjm
** Added extra parameter to Autoguider_Get_Fits_From_Buffer to pass in
** fits headers
**
** Revision 1.1  2006/06/01 15:19:05  cjm
** Initial revision
**
*/
#endif
