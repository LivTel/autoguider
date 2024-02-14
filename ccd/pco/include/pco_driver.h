/* pco_driver.h
** $Id$
*/
#ifndef PCO_DRIVER_H
#define PCO_DRIVER_H

#include "ccd_driver.h"
/*  the following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

extern int PCO_Driver_Register(struct CCD_Driver_Function_Struct *functions);


#ifdef __cplusplus
}
#endif

#endif
