/* autoguider_get_fits.h
** $Header: /home/cjm/cvs/autoguider/include/autoguider_get_fits.h,v 1.1 2006-06-01 15:19:05 cjm Exp $
*/
#ifndef AUTOGUIDER_GET_FITS_H
#define AUTOGUIDER_GET_FITS_H

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

extern int Autoguider_Get_Fits(int buffer_type,int buffer_state,void **buffer_ptr,size_t *buffer_length);
extern int Autoguider_Get_Fits_From_Buffer(void **buffer_ptr,size_t *buffer_length,int buffer_state,
				    void *buffer_data_ptr,size_t buffer_data_length,int ncols,int nrows);

/*
** $Log: not supported by cvs2svn $
*/
#endif
