/* andor_general.h
** $Header: /home/cjm/cvs/autoguider/ccd/andor/include/andor_general.h,v 1.3 2006-06-29 20:11:46 cjm Exp $
*/
#ifndef ANDOR_GENERAL_H
#define ANDOR_GENERAL_H

/* get config keyword root. */
#include "ccd_config.h"
/* get log block. */
#include "ccd_general.h"

/* hash defines */
/**
 * Root string of keywords used by the autoguider CCD library.
 * @see ../cdocs/ccd_config.html#CCD_CONFIG_KEYWORD_ROOT
 */
#define ANDOR_CCD_KEYWORD_ROOT                  CCD_CONFIG_KEYWORD_ROOT"andor."
/**
 * Value to pass into logging calls, used for setup code logging.
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_LOG_BLOCK_ANDOR
 */
#define ANDOR_GENERAL_LOG_BIT_SETUP	        (1<<(0+CCD_GENERAL_LOG_BLOCK_ANDOR))
/**
 * Value to pass into logging calls, used for exposure code logging.
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_LOG_BLOCK_ANDOR
 */
#define ANDOR_GENERAL_LOG_BIT_EXPOSURE	        (1<<(1+CCD_GENERAL_LOG_BLOCK_ANDOR))
/**
 * Value to pass into logging calls, used for temperature code logging.
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_LOG_BLOCK_ANDOR
 */
#define ANDOR_GENERAL_LOG_BIT_TEMPERATURE	(1<<(2+CCD_GENERAL_LOG_BLOCK_ANDOR))
/**
 * Value to pass into logging calls, used for driver code logging.
 * @see ../../cdocs/ccd_general.html#CCD_General_Log
 * @see ../../cdocs/ccd_general.html#CCD_GENERAL_LOG_BLOCK_ANDOR
 */
#define ANDOR_GENERAL_LOG_BIT_DRIVER	        (1<<(3+CCD_GENERAL_LOG_BLOCK_ANDOR))

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


extern char* Andor_General_ErrorCode_To_String(unsigned int error_code);
/*
** $Log: not supported by cvs2svn $
** Revision 1.2  2006/06/01 15:25:36  cjm
** Added driver log bit.
**
** Revision 1.1  2006/03/27 14:03:01  cjm
** Initial revision
**
*/
#endif
