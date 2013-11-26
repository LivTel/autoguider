/* fli_general.h
** $Header: /home/cjm/cvs/autoguider/ccd/fli/include/fli_general.h,v 1.1 2013-11-26 16:29:11 cjm Exp $
*/
#ifndef FLI_GENERAL_H
#define FLI_GENERAL_H

/* get config keyword root. */
#include "ccd_config.h"
/* get log block. */
#include "ccd_general.h"

/* hash defines */
/**
 * Root string of keywords used by the autoguider CCD library.
 * @see ../cdocs/ccd_config.html#CCD_CONFIG_KEYWORD_ROOT
 */
#define FLI_CCD_KEYWORD_ROOT                  CCD_CONFIG_KEYWORD_ROOT"fli."

#ifndef fdifftime
/**
 * Return double difference (in seconds) between two struct timespec's.
 * @param t0 A struct timespec.
 * @param t1 A struct timespec.
 * @return A double, in seconds, representing the time elapsed from t0 to t1.
 * @see #CCD_GENERAL_ONE_SECOND_NS
 */
#define fdifftime(t1, t0) (((double)(((t1).tv_sec)-((t0).tv_sec))+(double)(((t1).tv_nsec)-((t0).tv_nsec))/CCD_GENERAL_ONE_SECOND_NS))
#endif


/* diddly extern char* FLI_General_ErrorCode_To_String(unsigned int error_code);*/
/*
** $Log: not supported by cvs2svn $
*/
#endif
