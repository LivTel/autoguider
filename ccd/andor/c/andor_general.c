/* andor_general.c
** Autoguider Andor CCD Library general routines
** $Header: /home/cjm/cvs/autoguider/ccd/andor/c/andor_general.c,v 1.2 2022-04-11 09:38:45 cjm Exp $
*/
/**
 * General routines for the Andor autoguider CCD library.
 * @author Chris Mottram
 * @version $Revision: 1.2 $
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
/* andor CCD library */
#include "atmcdLXd.h"

#include "ccd_general.h"
#include "andor_general.h"

/* internal data */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: andor_general.c,v 1.2 2022-04-11 09:38:45 cjm Exp $";

/* ----------------------------------------------------------------------------
** 		external functions 
** ---------------------------------------------------------------------------- */
/**
 * Return a string for an Andor error code. See ~dev/src/autoguider/andor/andor/examples/common/atmcdLXd.h.
 * @param error_code The Andor error code.
 * @return A static string representation of the error code.
 */
char* Andor_General_ErrorCode_To_String(unsigned int error_code)
{
	switch(error_code)
	{
		case DRV_SUCCESS:
			return "DRV_SUCCESS";
		case DRV_ERROR_ACK:
			return "DRV_ERROR_ACK";
		case DRV_ACQUIRING:
			return "DRV_ACQUIRING";
		case DRV_IDLE:
			return "DRV_IDLE";
		case DRV_P1INVALID:
			return "DRV_P1INVALID";
		case DRV_P2INVALID:
			return "DRV_P2INVALID";
		case DRV_P3INVALID:
			return "DRV_P3INVALID";
		case DRV_P4INVALID:
			return "DRV_P4INVALID";
		default:
			return "UNKNOWN";
	}
	return "UNKNOWN";
}
/*
** $Log: not supported by cvs2svn $
** Revision 1.1  2006/03/27 14:02:36  cjm
** Initial revision
**
*/
