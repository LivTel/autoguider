/* test_exposure_low_level.c
** $Header: /home/cjm/cvs/autoguider/ccd/fli/test/test_exposure_low_level.c,v 1.1 2013-12-03 09:34:12 cjm Exp $
*/
/**
 * This program does a low level test of the FLI library exposure code.
 * @author $Author: cjm $
 * @version $Revision: 1.1 $
 */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "libfli.h"
#include "fitsio.h"

/* hash definitions */
/**
 * Default temperature to set the CCD to, in degrees celcius.
 */
#define DEFAULT_TEMPERATURE	(-20.0)

/**
 * The string length of some strings.
 */
#define STRING_LENGTH           (256)

/* structs */
/**
 * Structure defining an area on the CCD, using two sets of pixel coordinates
 * for the upper left and lower right corners.
 * <ul>
 * <li><b>Upper_Left_X</b>
 * <li><b>Upper_Left_Y</b>
 * <li><b>Lower_Right_X</b>
 * <li><b>Lower_Right_Y</b>
 * </ul>
 */
struct Area_Struct
{
	long Upper_Left_X;
	long Upper_Left_Y;
	long Lower_Right_X;
	long Lower_Right_Y;
};

/* enums */
/**
 * Enumeration determining which command this program executes. One of:
 * <ul>
 * <li>COMMAND_ID_NONE
 * <li>COMMAND_ID_BIAS
 * <li>COMMAND_ID_DARK
 * <li>COMMAND_ID_EXPOSURE
 * </ul>
 */
enum COMMAND_ID
{
	COMMAND_ID_NONE=0,COMMAND_ID_BIAS,COMMAND_ID_DARK,COMMAND_ID_EXPOSURE
};

/* internal variables */
/**
 * Revision control system identifier.
 */
static char rcsid[] = "$Id: test_exposure_low_level.c,v 1.1 2013-12-03 09:34:12 cjm Exp $";
/**
 * A boolean, whether to set the array temperature or not.
 */
static int Set_Target_Temperature = FALSE;
/**
 * Temperature to set the CCD to, in degrees Celcius.
 * @see #DEFAULT_TEMPERATURE
 */
static double Target_Temperature = DEFAULT_TEMPERATURE;
/**
 * The number of columns in the CCD.
 */
static int Size_X;
/**
 * The number of rows in the CCD.
 */
static int Size_Y;
/**
 * The number binning factor in columns.
 * According to FLI documentation should be between 1 and 16.
 */
static int Bin_X = 1;
/**
 * The number binning factor in rows.
 * According to FLI documentation should be between 1 and 16.
 */
static int Bin_Y = 1;
/**
 * Boolean determining whether to use a window or not.
 */
static int Use_Window = FALSE;
/**
 * The unbinned pixels coordinates of the window to use, when Use_Window == TRUE.
 * This is programmed into the FLI API as follows:
 * <pre>FLISetImageArea(dev,ulx,uly,lrx',lry')</pre>
 * where:
 * (ulx,uly) is the upper left hand corner of the window in unbinned pixels,
 * (lrx,lry) is the lower right hand corner of the window in unbinned pixels,
 * <pre>lrx' = ulx + (lrx - ulx) / hbin</pre>
 * <pre>lry' = uly + (lry - uly) / vbin</pre>
 * @see #Use_Window
 */
static struct Area_Struct Window;
/**
 * Which type ef exposure command to call.
 * @see #COMMAND_ID
 */
static enum COMMAND_ID Command = COMMAND_ID_NONE;
/**
 * If doing a dark or exposure, the exposure length in milliseconds.
 */
static int Exposure_Length = 0;
/**
 * Filename to store resultant fits image in. 
 */
static char *Fits_Filename = NULL;
/**
 * The device identifier for the attached camera.
 */
static flidev_t  Fli_Dev = FLI_INVALID_DEVICE;
/**
 * The pixel size of a pixel in the CCD, X dimension in microns.
 */
static double Pixel_Size_X = 0.0;
/**
 * The pixel size of a pixel in the CCD, Y dimension in microns.
 */
static double Pixel_Size_Y = 0.0;

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);
static int Find_FLI_Camera(flidev_t *fli_dev);
static int Test_Save_Fits(char *obstype,int exposure_time,int ncols,int nrows,unsigned short *image_data,
			  char *filename);
static void Test_Fits_Error(int status);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #STRING_LENGTH
 */
int main(int argc, char *argv[])
{
	struct Area_Struct image_area,visible_area;
	char buff[STRING_LENGTH];
	char obstype[16];
	char lib_version_string[STRING_LENGTH];
	long fli_retval,fli_val,remaining_exposure_length,camera_status;
	long binned_pixels_x,binned_pixels_y;
	unsigned short *image_data = NULL;
	double temperature,power;
	int done,y;

	/* parse arguments */
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* fli library version */
	if(FLIGetLibVersion(lib_version_string, STRING_LENGTH) != 0)
	{
		fprintf(stderr,"test_exposure_low_level: FLIGetLibVersion failed.\n");
		return 1;
	}
	fprintf(stdout,"test_exposure_low_level: FLI library version: %s.\n",lib_version_string);
	/* find and open a valid USB camera device */
	if(!Find_FLI_Camera(&Fli_Dev))
	{
		fprintf(stderr,"test_exposure_low_level: Find_FLI_Camera failed.\n");
		return 2;
	}
	/* get some information about the connected device */
	fli_retval = FLIGetModel(Fli_Dev,buff,STRING_LENGTH);
	if(fli_retval == 0)
		fprintf(stdout,"Camera Model: %s\n",buff);
	fli_retval = FLIGetSerialString(Fli_Dev,buff,STRING_LENGTH);
	if(fli_retval == 0)
		fprintf(stdout,"Serial Number: %s\n",buff);
	fli_retval = FLIGetHWRevision(Fli_Dev,&fli_val);
	if(fli_retval == 0)
		fprintf(stdout,"Hardware Revision: %ld\n",fli_val);
	fli_retval = FLIGetFWRevision(Fli_Dev,&fli_val);
	if(fli_retval == 0)
		fprintf(stdout,"Firmware Revision: %ld\n",fli_val);
	fli_retval = FLIGetPixelSize(Fli_Dev,&Pixel_Size_X,&Pixel_Size_Y);
	/* get array area into image_area */
	if(fli_retval == 0)
		fprintf(stdout,"Pixel Size (microns): %.8f x %.8f\n",Pixel_Size_X,Pixel_Size_Y);
	fli_retval = FLIGetArrayArea(Fli_Dev,&(image_area.Upper_Left_X),&(image_area.Upper_Left_Y),
				     &(image_area.Lower_Right_X),&(image_area.Lower_Right_Y));
	if(fli_retval == 0)
	{
		fprintf(stdout,"Array Area: Upper Left (%ld,%ld) Lower Right (%ld,%ld)\n",image_area.Upper_Left_X,
			image_area.Upper_Left_Y,image_area.Lower_Right_X,image_area.Lower_Right_Y);
	}
	/* get visible array area into visible_area */
	fli_retval = FLIGetVisibleArea(Fli_Dev,&(visible_area.Upper_Left_X),&(visible_area.Upper_Left_Y),
				     &(visible_area.Lower_Right_X),&(visible_area.Lower_Right_Y));
	if(fli_retval == 0)
	{
		fprintf(stdout,"Visible Area: Upper Left (%ld,%ld) Lower Right (%ld,%ld)\n",visible_area.Upper_Left_X,
			visible_area.Upper_Left_Y,visible_area.Lower_Right_X,visible_area.Lower_Right_Y);
	}
	/* configure target temperature */
	if(Set_Target_Temperature)
	{
		fprintf(stdout,"Setting temperature to %.2f C\n",Target_Temperature);
		fli_retval = FLISetTemperature(Fli_Dev,Target_Temperature);
		if(fli_retval != 0)
		{
			fprintf(stderr,"Find_FLI_Camera: FLISetTemperature(fli_dev=%ld,temperature=%.2f) failed "
				"%ld: %s.\n",
				Fli_Dev,Target_Temperature,fli_retval, strerror((int)-fli_retval));
			FLIClose(Fli_Dev);
			return 3;
		}
	}
	/* Get some temperature information */
	fli_retval = FLIGetTemperature(Fli_Dev,&temperature);
	if(fli_retval == 0)
		fprintf(stdout,"CCD Camera cold finger Temperature: %.2f.\n",temperature);
	fli_retval = FLIReadTemperature(Fli_Dev,FLI_TEMPERATURE_CCD,&temperature);
	if(fli_retval == 0)
		fprintf(stdout,"CCD Temperature: %.2f.\n",temperature);
	fli_retval = FLIReadTemperature(Fli_Dev,FLI_TEMPERATURE_BASE,&temperature);
	if(fli_retval == 0)
		fprintf(stdout,"Base Temperature: %.2f.\n",temperature);
	fli_retval = FLIGetCoolerPower(Fli_Dev,&power);
	if(fli_retval == 0)
		fprintf(stdout,"Cooler Power: %.2f.\n",power);
	/* are we using a window or not ? */
	if(Use_Window)
	{
		image_area = Window;
		fprintf(stdout,"Using Window with unbinned corners (ulx=%d,uly=%d)(lrx=%d,lry=%d).\n",
			image_area.Upper_Left_X,image_area.Upper_Left_Y,
			image_area.Lower_Right_X,image_area.Lower_Right_Y);
	}
	/* calculate the number of binned pixels */
	binned_pixels_x = (image_area.Lower_Right_X-image_area.Upper_Left_X)/Bin_X;
	binned_pixels_y = (image_area.Lower_Right_Y-image_area.Upper_Left_Y)/Bin_Y;
	fprintf(stdout,"main: Allocating image data (%d x %d).\n",binned_pixels_x,binned_pixels_y);
	image_data = (unsigned short*)malloc(binned_pixels_x * binned_pixels_y * sizeof(unsigned short));
	if(image_data == NULL)
	{
		fprintf(stderr,"main: Failed to allocate image data (%ld x %ld).\n",
			binned_pixels_x,binned_pixels_y);
		FLIClose(Fli_Dev);
		return 4;
	}
	/* set the image area to readout */
	fprintf(stdout,"main: Setting image data to (%ld,%ld) (%ld,%ld).\n",
		image_area.Upper_Left_X,image_area.Upper_Left_Y,
		image_area.Upper_Left_X+binned_pixels_x,image_area.Upper_Left_Y+binned_pixels_y);
	fli_retval = FLISetImageArea(Fli_Dev,image_area.Upper_Left_X,image_area.Upper_Left_Y,
				     image_area.Upper_Left_X+binned_pixels_x,
				     image_area.Upper_Left_Y+binned_pixels_y);
	if(fli_retval != 0)
	{
		fprintf(stderr,
			"main: FLISetImageArea(fli_dev=%ld,ulx=%ld,uly=%ld,lrx=%ld,lry=%ld) failed %ld: %s.\n",
			Fli_Dev,image_area.Upper_Left_X,image_area.Upper_Left_Y,
			image_area.Upper_Left_X+binned_pixels_x,image_area.Upper_Left_Y+binned_pixels_y,
			fli_retval, strerror((int)-fli_retval));
		FLIClose(Fli_Dev);
		return 5;
	}
	/* set the exposure length/shutter option */
	switch(Command)
	{
		case COMMAND_ID_BIAS:
			fprintf(stdout,"main: Setting Bias up.\n");
			fli_retval = FLISetExposureTime(Fli_Dev,0L);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetExposureTime(fli_dev=%ld,0L) failed %ld: %s.\n",
				       Fli_Dev,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 6;
			}
			fli_retval = FLISetFrameType(Fli_Dev,FLI_FRAME_TYPE_DARK);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetFrameType(fli_dev=%ld,FLI_FRAME_TYPE_DARK) failed %ld: %s.\n",
				       Fli_Dev,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 7;
			}
			strcpy(obstype,"BIAS");
			break;
		case COMMAND_ID_DARK:
			fprintf(stdout,"main: Setting %ld ms Dark up.\n",(long)Exposure_Length);
			fli_retval = FLISetExposureTime(Fli_Dev,(long)Exposure_Length);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetExposureTime(fli_dev=%ld,exposure length=%ld) failed %ld: %s.\n",
				       Fli_Dev,(long)Exposure_Length,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 8;
			}
			fli_retval = FLISetFrameType(Fli_Dev,FLI_FRAME_TYPE_DARK);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetFrameType(fli_dev=%ld,FLI_FRAME_TYPE_DARK) failed %ld: %s.\n",
				       Fli_Dev,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 9;
			}
			strcpy(obstype,"DARK");
			break;
		case COMMAND_ID_EXPOSURE:
			fprintf(stdout,"main: Setting %ld ms Exposure up.\n",(long)Exposure_Length);
			fli_retval = FLISetExposureTime(Fli_Dev,(long)Exposure_Length);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetExposureTime(fli_dev=%ld,exposure length=%ld) failed %ld: %s.\n",
				       Fli_Dev,(long)Exposure_Length,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 10;
			}
			fli_retval = FLISetFrameType(Fli_Dev,FLI_FRAME_TYPE_NORMAL);
			if(fli_retval != 0)
			{
				fprintf(stderr,
				 "main: FLISetFrameType(fli_dev=%ld,FLI_FRAME_TYPE_NORMAL) failed %ld: %s.\n",
				       Fli_Dev,fli_retval,strerror((int)-fli_retval));
				FLIClose(Fli_Dev);
				return 11;
			}
			strcpy(obstype,"EXPOSURE");
			break;
		default:
			fprintf(stderr,"main: No command type set: Choose one of -bias, -dark, or -exposure.\n");
			FLIClose(Fli_Dev);
			return 12;
	}
	/* setup binning */
	fprintf(stdout,"main: Setting Binning to %ld,%ld.\n",(long)Bin_X,(long)Bin_Y);
	fli_retval = FLISetHBin(Fli_Dev,(long)Bin_X);
	if(fli_retval != 0)
	{
		fprintf(stderr,
			"main: FLISetHBin(fli_dev=%ld,hbin=%ld) failed %ld: %s.\n",
			Fli_Dev,(long)Bin_X,fli_retval,strerror((int)-fli_retval));
		FLIClose(Fli_Dev);
		return 13;
	}
	fli_retval = FLISetVBin(Fli_Dev,(long)Bin_Y);
	if(fli_retval != 0)
	{
		fprintf(stderr,
			"main: FLISetVBin(fli_dev=%ld,vbin=%ld) failed %ld: %s.\n",
			Fli_Dev,(long)Bin_Y,fli_retval,strerror((int)-fli_retval));
		FLIClose(Fli_Dev);
		return 14;
	}
	/* start exposure */
	fprintf(stdout,"main: Starting exposure.\n");
	fli_retval = FLIExposeFrame(Fli_Dev);
	if(fli_retval != 0)
	{
		fprintf(stderr,
			"main: FLIExposeFrame(fli_dev=%ld) failed %ld: %s.\n",
			Fli_Dev,fli_retval,strerror((int)-fli_retval));
		FLIClose(Fli_Dev);
		return 15;
	}
	/* wait for the exposure to finish */
	fprintf(stdout,"main: Waiting for the exposure to finish.\n");
	done = FALSE;
	while(done == FALSE)
	{
		fli_retval = FLIGetDeviceStatus(Fli_Dev, &camera_status);
		if(fli_retval == 0)
		{
			if(camera_status == FLI_CAMERA_STATUS_UNKNOWN)
				fprintf(stdout,"main: Camera status is UNKNOWN.\n");
			else
			{
				if((camera_status&FLI_CAMERA_STATUS_MASK) == FLI_CAMERA_STATUS_IDLE)
					fprintf(stdout,"main: Camera status is IDLE.\n");
				if((camera_status&FLI_CAMERA_STATUS_WAITING_FOR_TRIGGER) > 0)
					fprintf(stdout,"main: Camera status is WAITING_FOR_TRIGGER.\n");
				if((camera_status&FLI_CAMERA_STATUS_EXPOSING) > 0)
					fprintf(stdout,"main: Camera status is EXPOSING.\n");
				if((camera_status&FLI_CAMERA_STATUS_READING_CCD) > 0)
					fprintf(stdout,"main: Camera status is READING_CCD.\n");
				if((camera_status&FLI_CAMERA_DATA_READY) > 0)
					fprintf(stdout,"main: Camera has data ready.\n");
			}
		}
		fli_retval = FLIGetExposureStatus(Fli_Dev, &remaining_exposure_length);
		if(fli_retval == 0)
		{
			fprintf(stdout,"main:Remaining exposure length = %ld ms.\n",remaining_exposure_length);
			/*if(remaining_exposure_length == 0) This doesn't work for BIAS frames as it fires before
			** the data has been digitized, the bottom of the image has pixels with value 0 in it */
			if((camera_status & FLI_CAMERA_DATA_READY) != 0)
			{
//if ( ((camera_status == FLI_CAMERA_STATUS_UNKNOWN) && (remaining_exposure == 0)) ||
//    ((camera_status != FLI_CAMERA_STATUS_UNKNOWN) && ((camera_status & FLI_CAMERA_DATA_READY) != 0))
				done = TRUE;
			}
		}
		if(done == FALSE)
		{
			sleep(1);
		}
	}/* end while */
	/* download the image data */
	for(y = 0;y < binned_pixels_y; y++)
	{
		fli_retval = FLIGrabRow(Fli_Dev, &image_data[y*binned_pixels_x],binned_pixels_x);
		if(fli_retval != 0)
		{
			fprintf(stderr,
				"main: FLIGrabRow(fli_dev=%ld) (y=%d)failed %ld: %s.\n",
				Fli_Dev,y,fli_retval,strerror((int)-fli_retval));
			FLIClose(Fli_Dev);
			return 16;
		}
	}
	/* save the image data */
	if(!Test_Save_Fits(obstype,Exposure_Length,binned_pixels_x,binned_pixels_y,image_data,Fits_Filename))
	{
		fprintf(stderr,"main: Saving FITS headers to filename %s fails.\n",Fits_Filename);
		FLIClose(Fli_Dev);
		return 17;
	}
	/* free allocated image data */
	if(image_data != NULL)
		free(image_data);
	image_data = NULL;
	/* close the device */
	fli_retval = FLIClose(Fli_Dev);
	if(fli_retval != 0)
	{
		fprintf(stderr,"main: FLIClose(fli_dev=%ld) failed %ld: %s.\n",
			Fli_Dev,fli_retval, strerror((int)-fli_retval));
		return 17;
	}
	return 0;
}

/* -----------------------------------------------------------------------------
**      Internal routines
** ----------------------------------------------------------------------------- */
/**
 * Find and open the FLI camera.
 * @param fli_dev The address of a flidev_t to return with the opened device ID.
 * @return TRUE if one FLI camera is found and opened successfully, FALSE if an error occurs.
 */
static int Find_FLI_Camera(flidev_t *fli_dev)
{
	char first_filename[STRING_LENGTH], first_name[STRING_LENGTH];
	char filename[STRING_LENGTH], name[STRING_LENGTH];
	long domain,first_domain,fli_retval;
	int camera_count = 0;

	camera_count = 0;
	FLICreateList(FLIDOMAIN_USB | FLIDEVICE_CAMERA);
	if(FLIListFirst(&domain, filename,STRING_LENGTH, name,STRING_LENGTH) == 0)
	{
		fprintf(stdout,"Find_FLI_Camera: Camera %d has filename %s name %s and domain %ld.\n",
			camera_count,filename,name,domain);
		strcpy(first_filename,filename);
		strcpy(first_name,name);
		first_domain = domain;
		camera_count++;
		while(FLIListNext(&domain,filename,STRING_LENGTH,name,STRING_LENGTH) == 0)
		{
			fprintf(stdout,"Find_FLI_Camera: Camera %d has filename %s name %s and domain %ld.\n",
				camera_count,filename,name,domain);
			camera_count++;
		}
	}
	FLIDeleteList();
	if(camera_count != 1)
	{
		fprintf(stderr,"Find_FLI_Camera: Wrong number of cameras found (%d).\n",camera_count);
		return FALSE;
	}
	fprintf(stdout,"Find_FLI_Camera: Opening Camera with filename %s name %s and domain %ld.\n",
		first_filename,first_name,first_domain);
	fli_retval = FLIOpen(fli_dev,first_filename,first_domain);
	if(fli_retval != 0)
	{
		fprintf(stderr,"Find_FLI_Camera: FLIOpen(filename=%s,domain=%ld) failed %ld: %s.\n",
			first_filename,first_domain,fli_retval, strerror((int)-fli_retval));
		return FALSE;
	}
	return TRUE;
}

/**
 * Help routine.
 */
static void Help(void)
{
	fprintf(stdout,"Test Exposure (low level):Help.\n");
	fprintf(stdout,"test_exposure_low_level <-b[ias]|-d[ark] <exposure length ms>|-e[xpose] <exposure length ms>>\n");
	fprintf(stdout,"\t-f[its_filename] <fits filename> [-h[elp]] -t[emperature] <target temp C>\n");
	fprintf(stdout,"\t[-xb[in] <n>] [-yb[in] <n>].\n");
	fprintf(stdout,"\t[-w[indow] <ulx> <uly> <lrx> <lry>].\n");
	fprintf(stdout,"The window is defined in unbinned pixels with upper left corner (<ulx>,<uly>) and lower right corner (<lrx>,<lry>).\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Target_Temperature
 * @see #Set_Target_Temperature
 * @see #Bin_X
 * @see #Bin_Y
 * @see #Window
 * @see #Use_Window
 * @see #Exposure_Length
 * @see #Fits_Filename
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-bias")==0)||(strcmp(argv[i],"-b")==0))
		{
			Command = COMMAND_ID_BIAS;
		}
		else if((strcmp(argv[i],"-dark")==0)||(strcmp(argv[i],"-d")==0))
		{
			Command = COMMAND_ID_DARK;
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Exposure_Length);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing exposure length %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:dark requires exposure length.\n");
				return FALSE;
			}

		}
		else if((strcmp(argv[i],"-expose")==0)||(strcmp(argv[i],"-e")==0))
		{
			Command = COMMAND_ID_EXPOSURE;
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Exposure_Length);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing exposure length %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:exposure requires exposure length.\n");
				return FALSE;
			}

		}
		else if((strcmp(argv[i],"-fits_filename")==0)||(strcmp(argv[i],"-f")==0))
		{
			if((i+1)<argc)
			{
				Fits_Filename = argv[i+1];
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:filename required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
		{
			Help();
			exit(0);
		}
		else if((strcmp(argv[i],"-temperature")==0)||(strcmp(argv[i],"-t")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%lf",&Target_Temperature);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing temperature %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				Set_Target_Temperature = TRUE;
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:temperature required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-window")==0)||(strcmp(argv[i],"-w")==0))
		{
			if((i+4)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&(Window.Upper_Left_X));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing window upper left X %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				retval = sscanf(argv[i+2],"%d",&(Window.Upper_Left_Y));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing window upper left Y %s failed.\n",
						argv[i+2]);
					return FALSE;
				}
				retval = sscanf(argv[i+3],"%d",&(Window.Lower_Right_X));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing window lower right X %s failed.\n",
						argv[i+3]);
					return FALSE;
				}
				retval = sscanf(argv[i+4],"%d",&(Window.Lower_Right_Y));
				if(retval != 1)
				{
					fprintf(stderr,"Parse_Arguments:Parsing window lower right Y %s failed.\n",
						argv[i+4]);
					return FALSE;
				}
				i+= 4;
				Use_Window = TRUE;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:window <ulx> <uly> <lrx> <lry>.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-xbin")==0)||(strcmp(argv[i],"-xb")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Bin_X);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing X Bin %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:bin required.\n");
				return FALSE;
			}
		}
		else if((strcmp(argv[i],"-ybin")==0)||(strcmp(argv[i],"-yb")==0))
		{
			if((i+1)<argc)
			{
				retval = sscanf(argv[i+1],"%d",&Bin_Y);
				if(retval != 1)
				{
					fprintf(stderr,
						"Parse_Arguments:Parsing Y Bin %s failed.\n",
						argv[i+1]);
					return FALSE;
				}
				i++;
			}
			else
			{
				fprintf(stderr,"Parse_Arguments:bin required.\n");
				return FALSE;
			}
		}
		else
		{
			fprintf(stderr,"Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Internal routine that saves some basic FITS headers to the relevant filename.
 * This is needed as CCD_Exposure_Expose/Bias routines need saved FITS headers to
 * not give an error.
 * @param exposure_time The amount of time, in milliseconds, of the exposure.
 * @param ncols The number of columns in the FITS file.
 * @param nrows The number of rows in the FITS file.
 * @param filename The filename to save the FITS headers in.
 * @return The routine returns TRUE on success and FALSE on failure.
 */
static int Test_Save_Fits(char *obstype,int exposure_time,int ncols,int nrows,unsigned short *image_data,
			  char *filename)
{
	static fitsfile *fits_fp = NULL;
	char buff[32]; /* fits_get_errstatus returns 30 chars max */
	long axes[2];
	int status = 0,retval,ivalue;
	double dvalue;

/* open file */
	if(fits_create_file(&fits_fp,filename,&status))
	{
		Test_Fits_Error(status);
		return FALSE;
	}
/* SIMPLE keyword */
	ivalue = TRUE;
	retval = fits_update_key(fits_fp,TLOGICAL,(char*)"SIMPLE",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BITPIX keyword */
	ivalue = 16;
	retval = fits_update_key(fits_fp,TINT,(char*)"BITPIX",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* NAXIS keyword */
	ivalue = 2;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}

/* NAXIS1 keyword */
	ivalue = ncols;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS1",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* NAXIS2 keyword */
	ivalue = nrows;
	retval = fits_update_key(fits_fp,TINT,(char*)"NAXIS2",&ivalue,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BZERO keyword */
	dvalue = 32768.0;
	retval = fits_update_key_fixdbl(fits_fp,(char*)"BZERO",dvalue,6,
		(char*)"Number to offset data values by",&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
/* BSCALE keyword */
	dvalue = 1.0;
	retval = fits_update_key_fixdbl(fits_fp,(char*)"BSCALE",dvalue,6,
		(char*)"Number to multiply data values by",&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
	/* OBSTYPE keyword */
	retval = fits_update_key(fits_fp,TSTRING,(char*)"OBSTYPE",obstype,NULL,&status);
	if(retval != 0)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
	/* image data */
	/*
	axes[0] = ncols;
	axes[1] = nrows;
	retval = fits_create_img(fits_fp,USHORT_IMG,2,axes,&status);
	if(retval)
	{
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		return FALSE;
	}
	*/
	retval = fits_write_img(fits_fp,TUSHORT,1,ncols*nrows,image_data,&status);
	if(retval)
	{
		fits_get_errstatus(status,buff);
		Test_Fits_Error(status);
		fits_close_file(fits_fp,&status);
		fprintf(stderr,"Test_Save_Fits: File write image failed(%s,%d,%s).",
			filename,status,buff);
		return FALSE;
	}
/* close file */
	if(fits_close_file(fits_fp,&status))
	{
		Test_Fits_Error(status);
		return FALSE;
	}
	return TRUE;
}

/**
 * Internal routine to write the complete CFITSIO error stack to stderr.
 * @param status The status returned by CFITSIO.
 */
static void Test_Fits_Error(int status)
{
	/* report the whole CFITSIO error message stack to stderr. */
	fits_report_error(stderr, status);
}


/*
** $Log: not supported by cvs2svn $
*/
