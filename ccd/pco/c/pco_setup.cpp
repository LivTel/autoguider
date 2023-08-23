/* pco_setup.cpp
** Autoguider PCO CMOS library setup routines
*/
/**
 * Setup routines for the PCO camera driver.
 * @author Chris Mottram
 * @version $Id$
 */
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_SOURCE 1
/**
 * This hash define is needed before including source files give us POSIX.4/IEEE1003.1b-1993 prototypes.
 */
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <unistd.h>
#include "log_udp.h"
#include "ccd_general.h"
#include "pco_command.h"
#include "pco_setup.h"

/* hash defines */
/**
 * The maximum length of enumerated value strings in Setup_Data.
 * The longest SensorReadoutMode value is 25 characters long.
 * The longest SimplePreAmpGainControl value is 40 characters long.
 */
#define SETUP_ENUM_VALUE_STRING_LENGTH (64)

/* data types */
/**
 * Data type holding local data to pco_setup. This consists of the following:
 * <dl>
 * <dt>Camera_Board</dt> <dd>The board parameter passed to Open_Cam, to determine which camera to connect to.</dd>
 * <dt>Timestamp_Mode</dt> <dd>What kind of timestamp to include in the read out image data.</dd>
 * <dt>Binning</dt> <dd>The readout binning, stored as an integer. Can be one of 1,2,3,4,8. </dd>
 * <dt>Serial_Number</dt> <dd>An integer containing the serial number retrieved from the camera head
 *                            Retrieved from the camera library during PCO_Setup_Startup.</dd>
 * <dt>Pixel_Width</dt> <dd>A double storing the pixel width in micrometers. Setup from the sensor type 
 *                          during CCD_Setup_Startup.</dd>
 * <dt>Pixel_Height</dt> <dd>A double storing the pixel height in micrometers. Setup from the sensor type 
 *                          during CCD_Setup_Startup.</dd>
 * <dt>Sensor_Width</dt> <dd>An integer storing the sensor width in pixels retrieved from the camera during 
 *                       CCD_Setup_Startup.</dd>
 * <dt>Sensor_Height</dt> <dd>An integer storing the sensor height in pixels retrieved from the camera during 
 *                       CCD_Setup_Startup.</dd>
 * <dt>Image_Size_Bytes</dt> <dd>An integer storing the image size in bytes.</dd>
 * </dl>
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE
 */
struct Setup_Struct
{
	int Camera_Board;
	enum PCO_COMMAND_TIMESTAMP_MODE Timestamp_Mode;
	int Binning;
	int Serial_Number;
	double Pixel_Width;
	double Pixel_Height;
	int Sensor_Width;
	int Sensor_Height;
	int Image_Size_Bytes;
};

/* internal variables */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id$";
/**
 * The instance of Setup_Struct that contains local data for this module. This is initialised as follows:
 * <dl>
 * <dt>Camera_Board</dt> <dd>0</dd>
 * <dt>Timestamp_Mode</dt> <dd>PCO_COMMAND_TIMESTAMP_MODE_BINARY_ASCII</dd>
 * <dt>Binning</dt> <dd>1</dd>
 * <dt>Serial_Number</dt> <dd>-1</dd>
 * <dt>Pixel_Width</dt> <dd>0.0</dd>
 * <dt>Pixel_Height</dt> <dd>0.0</dd>
 * <dt>Sensor_Width</dt> <dd>0</dd>
 * <dt>Sensor_Height</dt> <dd>0</dd>
 * <dt>Image_Size_Bytes</dt> <dd>0</dd>
 * </dl>
 * @see pco_command.html#PCO_COMMAND_TIMESTAMP_MODE
 */
static struct Setup_Struct Setup_Data = 
{
	0,PCO_COMMAND_TIMESTAMP_MODE_BINARY_ASCII,1,-1,0.0,0.0,0,0,0
};

/* internal functions */

/* --------------------------------------------------------
** External Functions
** -------------------------------------------------------- */
int PCO_Setup_Startup(void)
{
	return TRUE;
}

int PCO_Setup_Shutdown(void)
{
	return TRUE;
}

int PCO_Setup_Dimensions(int ncols,int nrows,int hbin,int vbin,
				int window_flags,struct CCD_Setup_Window_Struct window)
{
	return TRUE;
}

void PCO_Setup_Abort(void)
{
}

int PCO_Setup_Get_NCols(void)
{
	return 0;
}

int PCO_Setup_Get_NRows(void)
{
	return 0;
}

