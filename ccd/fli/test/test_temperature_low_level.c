/* test_temperature_low_level.c
** $Header: /home/cjm/cvs/autoguider/ccd/fli/test/test_temperature_low_level.c,v 1.1 2013-12-03 09:34:12 cjm Exp $
*/
/**
 * This program does a low level test of the FLI library temperature code.
 * @author $Author: cjm $
 * @version $Revision: 1.1 $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libfli.h"

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

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
static char rcsid[] = "$Id: test_temperature_low_level.c,v 1.1 2013-12-03 09:34:12 cjm Exp $";
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
 * The device identifier for the attached camera.
 */
static flidev_t Fli_Dev = FLI_INVALID_DEVICE;

/* internal routines */
static int Parse_Arguments(int argc, char *argv[]);
static void Help(void);
static int Find_FLI_Camera(flidev_t *fli_dev);

/**
 * Main program.
 * @param argc The number of arguments to the program.
 * @param argv An array of argument strings.
 * @return This function returns 0 if the program succeeds, and a positive integer if it fails.
 * @see #STRING_LENGTH
 * @see #Target_Temperature
 */
int main(int argc, char *argv[])
{
	char buff[STRING_LENGTH];
	char obstype[16];
	char lib_version_string[STRING_LENGTH];
	long fli_retval,fli_val;
	double temperature,power;

	/* parse arguments */
	fprintf(stdout,"Parsing Arguments.\n");
	if(!Parse_Arguments(argc,argv))
		return 1;
	/* fli library version */
	if(FLIGetLibVersion(lib_version_string, STRING_LENGTH) != 0)
	{
		fprintf(stderr,"test_temperature_low_level: FLIGetLibVersion failed.\n");
		return 1;
	}
	fprintf(stdout,"test_temperature_low_level: FLI library version: %s.\n",lib_version_string);
	/* find and open a valid USB camera device */
	if(!Find_FLI_Camera(&Fli_Dev))
	{
		fprintf(stderr,"test_temperature_low_level: Find_FLI_Camera failed.\n");
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
	/* configure target temperature */
	if(Set_Target_Temperature)
	{
		fprintf(stdout,"Setting temperature to %.2f C\n",Target_Temperature);
		fli_retval = FLISetTemperature(Fli_Dev,Target_Temperature);
		if(fli_retval != 0)
		{
			fprintf(stderr,"Find_FLI_Camera: FLISetTemperature(fli_dev=%ld,temperature=%.2f) failed "
				"%ld: %s.\n",Fli_Dev,Target_Temperature,fli_retval, strerror((int)-fli_retval));
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
	fprintf(stdout,"Test Temperature (low level):Help.\n");
	fprintf(stdout,"test_temperature_low_level [-h[elp]] [-t[emperature] <target temp C>]\n");
}

/**
 * Routine to parse command line arguments.
 * @param argc The number of arguments sent to the program.
 * @param argv An array of argument strings.
 * @see #Help
 * @see #Target_Temperature
 * @see #Set_Target_Temperature
 */
static int Parse_Arguments(int argc, char *argv[])
{
	int i,retval,log_level;

	for(i=1;i<argc;i++)
	{
		if((strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"-h")==0))
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
		else
		{
			fprintf(stderr,"Parse_Arguments:argument '%s' not recognized.\n",argv[i]);
			return FALSE;
		}
	}
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
