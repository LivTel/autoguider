/* pco_command.h */

#ifndef PCO_COMMAND_H
#define PCO_COMMAND_H

/* get config keyword root. */
#include "ccd_config.h"
#include "ccd_general.h"

/* hash defines */
/**
 * Root string of keywords used by the PCO driver library.
 * @see ../cdocs/ccd_config.html#CCD_CONFIG_KEYWORD_ROOT
 */
#define PCO_CCD_KEYWORD_ROOT                  CCD_CONFIG_KEYWORD_ROOT"pco."

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

/* enumerations */
/**
 * Setup flags used as a parameter to PCO_Command_Set_Camera_Setup - mainly used to control the shutter readout mode.
 * <ul>
 * <li><b>PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER</b> Rolling shutter (the default).
 * <li><b>PCO_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER</b> Global shutter mode.
 * <li><b>PCO_COMMAND_SETUP_FLAG_GLOBAL_RESET</b> Global reset mode.
 * </ul>
 * @see #PCO_Command_Set_Camera_Setup
 */
enum PCO_COMMAND_SETUP_FLAG
{
	PCO_COMMAND_SETUP_FLAG_ROLLING_SHUTTER = 0x00000001,
	PCO_COMMAND_SETUP_FLAG_GLOBAL_SHUTTER  = 0x00000002,
	PCO_COMMAND_SETUP_FLAG_GLOBAL_RESET    = 0x00000004
};

/**
 * Trigger mode, used to determine how exposures are started by the camera head.
 * <ul>
 * <li><b>PCO_COMMAND_TRIGGER_MODE_INTERNAL</b> The camera head internally triggers (software/auto sequence).
 * <li><b>PCO_COMMAND_TRIGGER_MODE_EXTERNAL</b> The exposures are externally triggered.
 * </ul>
 * There is also an 0x3 option (external exposure control) which we don't use.
 */
enum PCO_COMMAND_TRIGGER_MODE
{
	PCO_COMMAND_TRIGGER_MODE_INTERNAL = 0x0,
	PCO_COMMAND_TRIGGER_MODE_EXTERNAL = 0x2
};

/**
 * Timebase, used to decide what units are used when specifying exposure/delay lengths.
 * <ul>
 * <li><b>PCO_COMMAND_TIMEBASE_NS</b> We are using nanoseconds to specify exposure/delay lengths.
 * <li><b>PCO_COMMAND_TIMEBASE_US</b> We are using microseconds to specify exposure/delay lengths.
 * <li><b>PCO_COMMAND_TIMEBASE_MS</b> We are using milliseconds to specify exposure/delay lengths.
 * </ul>
 */
enum PCO_COMMAND_TIMEBASE
{
	PCO_COMMAND_TIMEBASE_NS = 0x0,
	PCO_COMMAND_TIMEBASE_US = 0x1,
	PCO_COMMAND_TIMEBASE_MS = 0x2
};

/**
 * Timestamp mode, used to determine what timestamp data is included in the read-out.
 * <ul>
 * <li><b>PCO_COMMAND_TIMESTAMP_MODE_OFF</b> No timestamp data in the image.
 * <li><b>PCO_COMMAND_TIMESTAMP_MODE_BINARY</b> A BCD encoded timestamp is in the first 14 pixels of the image.
 * <li><b>PCO_COMMAND_TIMESTAMP_MODE_BINARY_ASCII</b> A BCD encoded timestamp is in the first 14 pixels of the image,
 *                                                  and an ASCII representation as well.
 * <li><b>PCO_COMMAND_TIMESTAMP_MODE_ASCII</b> An ASCII representation of the timestamp is 
 *                                            in the top corner of the image.
 * </ul>
 * @see #PCO_Command_Set_Timestamp_Mode
 */
enum PCO_COMMAND_TIMESTAMP_MODE
{
	PCO_COMMAND_TIMESTAMP_MODE_OFF          = 0x0000,
	PCO_COMMAND_TIMESTAMP_MODE_BINARY       = 0x0001,
	PCO_COMMAND_TIMESTAMP_MODE_BINARY_ASCII = 0x0002,
	PCO_COMMAND_TIMESTAMP_MODE_ASCII        = 0x0003
};

/*  the following 3 lines are needed to support C++ compilers */
#ifdef __cplusplus
extern "C" {
#endif

/* functions */
extern int PCO_Command_Initialise_Camera(void);
extern int PCO_Command_Finalise_Camera(void);
extern int PCO_Command_Open(int board);
extern int PCO_Command_Close_Camera(void);
extern int PCO_Command_Initialise_Grabber(void);
extern int PCO_Command_Finalise_Grabber(void);
extern int PCO_Command_Close_Grabber(void);
extern int PCO_Command_Get_Camera_Setup(enum PCO_COMMAND_SETUP_FLAG *setup_flag);
extern int PCO_Command_Set_Camera_Setup(enum PCO_COMMAND_SETUP_FLAG setup_flag);
extern int PCO_Command_Reboot_Camera(void);
extern int PCO_Command_Arm_Camera(void);
extern int PCO_Command_Grabber_Post_Arm(void);
extern int PCO_Command_Set_Camera_To_Current_Time(void);
extern int PCO_Command_Set_Recording_State(int rec_state);
extern int PCO_Command_Reset_Settings(void);
extern int PCO_Command_Set_Timestamp_Mode(enum PCO_COMMAND_TIMESTAMP_MODE mode);
extern int PCO_Command_Set_Timebase(enum PCO_COMMAND_TIMEBASE delay_timebase,
				    enum PCO_COMMAND_TIMEBASE exposure_timebase);
extern int PCO_Command_Set_Delay_Exposure_Time(int delay_time,int exposure_time);
extern int PCO_Command_Set_ADC_Operation(int num_adcs);
extern int PCO_Command_Set_Bit_Alignment(int bit_alignment);
extern int PCO_Command_Set_Noise_Filter_Mode(int mode);
extern int PCO_Command_Set_HW_LED_Signal(int onoff);
extern int PCO_Command_Set_Trigger_Mode(enum PCO_COMMAND_TRIGGER_MODE mode);
extern int PCO_Command_Set_Binning(int bin_x,int bin_y);
extern int PCO_Command_Set_ROI(int start_x,int start_y,int end_x,int end_y);
extern int PCO_Command_Set_Cooling_Setpoint_Temperature(int temperature);
extern int PCO_Command_Grabber_Get_Actual_Size(int *w,int *h,int *bp);
extern int PCO_Command_Grabber_Acquire_Image_Async_Wait(void *image_buffer);
extern int PCO_Command_Grabber_Acquire_Image_Async_Wait_Timeout(void *image_buffer,int timeout_ms);
extern int PCO_Command_Get_Temperature(int *valid_sensor_temp,double *sensor_temp,int *camera_temp,
				       int *valid_psu_temp,int *psu_temp);
extern int PCO_Command_Description_Get_Num_ADCs(int *adc_count);
extern int PCO_Command_Description_Get_Exposure_Time_Min(double *minimum_exposure_length_s);
extern int PCO_Command_Description_Get_Exposure_Time_Max(double *maximum_exposure_length_s);
extern int PCO_Command_Description_Get_Max_Horizontal_Size(int *max_hor_size);
extern int PCO_Command_Description_Get_Max_Vertical_Size(int *max_ver_size);
extern int PCO_Command_Description_Get_ROI_Horizontal_Step_Size(int *step_size);
extern int PCO_Command_Description_Get_ROI_Vertical_Step_Size(int *step_size);
extern int PCO_Command_Description_Get_Default_Cooling_Setpoint(int *temperature);
extern int PCO_Command_Description_Get_Min_Cooling_Setpoint(int *temperature);
extern int PCO_Command_Description_Get_Max_Cooling_Setpoint(int *temperature);
extern int PCO_Command_Description_Get_Sensor_Type(int *sensor_type,int *sensor_subtype);
extern int PCO_Command_Get_Camera_Type(int *camera_type,int *serial_number);
extern int PCO_Command_Get_ROI(int *start_x,int *start_y,int *end_x,int *end_y);
extern int PCO_Command_Get_Actual_Size(int *image_width,int *image_height);
extern int PCO_Command_Get_Image_Size_Bytes(int *image_size);
extern int PCO_Command_Get_Trigger_Mode(enum PCO_COMMAND_TRIGGER_MODE *mode);
extern int PCO_Command_Get_Delay_Exposure_Time(int *delay_time,int *exposure_time);
extern int PCO_Command_Get_Cooling_Setpoint_Temperature(int *temperature);
extern int PCO_Command_Get_Image_Number_From_Metadata(void *image_buffer,size_t image_buffer_length,
						      int *image_number);
extern int PCO_Command_Get_Timestamp_From_Metadata(void *image_buffer,size_t image_buffer_length,
						   struct timespec *camera_timestamp);
#ifdef __cplusplus
}
#endif

#endif
